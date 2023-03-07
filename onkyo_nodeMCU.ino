#include "Arduino.h"
#include <map>
#include "config.h"
#include <EEPROM.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266SSDP.h>
#include <uri/UriBraces.h>

//Variables
uint i = 0;
uint statusCode;
String st;
String selectSSID;
String content;
char hexadecimalnum[5];

//Function Decalration
bool testWifi(void);
void launchAPWeb(void);
void setupAP(void);

// Clear EEPROM
void clearEEPROM() {
  for (int i = 0; i < 96; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  delay(100);
  ESP.reset();
}

ESP8266WebServer server(80);

void setup() {
  EEPROM.begin(512);  //Initialasing EEPROM
  delay(10);

#ifdef DEBUG_ON
  Serial.begin(115200);  // Start serial debug console monitoring
  while (!Serial)
    ;
#endif

  DEBUG();
  DEBUG("Disconnecting previously connected WiFi");
  WiFi.disconnect();

  pinMode(ONKYO_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, HIGH);//this will turn off the NodeMCU blue LED
  DEBUG();
  DEBUG();
  DEBUG("Startup");

  //-- Read EEPROM for SSID and pass
  DEBUG("Reading EEPROM ssid");
  String esid;
  for (int i = 0; i < 32; ++i) {
    esid += char(EEPROM.read(i));
  }
  DEBUG();
  DEB("SSID: ");
  DEBUG(esid);
  DEBUG("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i) {
    epass += char(EEPROM.read(i));
  }
  DEB("PASS: ");
  DEBUG(epass);

  WiFi.begin(esid.c_str(), epass.c_str());

  if (testWifi()) {
    DEBUG("Succesfully Connected!!!");
    if (MDNS.begin("Onkyo NodeMCU")) {
      DEBUG("MDNS responder started");
    }
    // Set server routing
    restServerRouting();
    // Set not found response
    server.onNotFound(handleNotFound);
    // Start server
    server.begin();
    MDNS.addService("http", "tcp", 80);
    DEBUG("Starting SSDP...");
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName("Onkyo NodeMCU");
    SSDP.setURL("/");
    SSDP.setModelName("D1 Mini NodeMCU");
    SSDP.begin();
    DEBUG("HTTP server started");
    return;
  } else {
    DEBUG("Turning the HotSpot On");
    launchAPWeb();
    setupAP();  // Setup HotSpot
  }

  DEBUG();
  DEB("Waiting.");

  while ((WiFi.status() != WL_CONNECTED)) {
    server.handleClient();
    delay(1);
  }
}

//-- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change
bool testWifi(void) {
  uint i = 0;
  DEBUG("Waiting for Wifi to connect");
  while (i < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    DEB("*");
    i++;
  }
  DEBUG();
  DEBUG("Connect timed out");
  return false;
}

void launchAPWeb() {
  DEBUG();
  if (WiFi.status() == WL_CONNECTED)
    DEBUG("WiFi connected");
  DEB("Local IP: ");
  DEBUG(WiFi.localIP());
  DEB("SoftAP IP: ");
  DEBUG(WiFi.softAPIP());
  createAPWebServer();
  if (MDNS.begin("Onkyo NodeMCU")) {
    DEBUG("MDNS responder started");
  }
  server.begin();
  MDNS.addService("http", "tcp", 80);
  DEBUG("Starting SSDP");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("Onkyo NodeMCU");
  SSDP.setURL("/");
  SSDP.setModelName("D1 Mini NodeMCU");
  SSDP.begin();
  DEBUG("HTTP server started");
}

// Define routing
void restServerRouting() {

  server.on(UriBraces("/{}"), HTTP_GET, []() {
    DEB("route: " + server.pathArg(0));
    String cmd = (server.pathArg(0));
    cmd.toLowerCase();
    DEBUG(" - lower: " + cmd);
    if (cmds.count(cmd)) {
      sprintf(hexadecimalnum, "0x%.3X", cmds[cmd]);
      content = "{\"route\": \"" + cmd + "\",\"cmd\": \"" + hexadecimalnum + "\"}";
      statusCode = 200;
      onkyoSend(cmds[cmd]);
      server.send(statusCode, "application/json", content);
    } else if (cmd == "") {
      content = "[";
      for (auto const& [key, val] : cmds) {
        sprintf(hexadecimalnum, "0x%.3X", val);
        content += "{\"route\": \"" + key + "\",\"cmd\": \"" + hexadecimalnum + "\"},";
      }
      content += "{\"route\": \"initialize\",\"cmd\": \"Reset WiFi settings\"},{\"route\": \"custom\",\"URI parameters\": [{\"cmd\": []}]}]";
      statusCode = 200;
      server.send(statusCode, "application/json", content);
    } else if (cmd == "initialize") {
      server.send(200, "application/json", "{\"route\": \"initialize\",\"cmd\": \"EEPROM ssid & pass data cleared!\"}");
      DEBUG("EEPROM ssid & pass data cleared!");
      clearEEPROM();
    } else if (cmd == "custom") {
      String custom_cmd = server.arg("cmd");
      if (custom_cmd.length() == 0) {
        content = "{\"Error\":\"422 required parameter not found\"}";
        statusCode = 422;
      } else {
        int cmd_len = custom_cmd.length() + 1;
        char cmd_array[cmd_len];
        custom_cmd.toCharArray(cmd_array, cmd_len);
        int cmd_int = (int)strtol(cmd_array, 0, 16);

        if (cmd_int != 0) {
          DEBUG(cmd_array);
          DEBUG(cmd_int);
          onkyoSend(cmd_int);

          content = "{\"route\": \"custom\",\"cmd\": \"" + custom_cmd + "\"}";
          statusCode = 200;
        } else {
          content = "{\"Error\":\"400 Bad request\"}";
          statusCode = 400;
        }
      }
      //server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);
    } else if (cmd == "description.xml") {
      SSDP.schema(server.client());
    } else {
      handleNotFound();
    }
  });
}

// Manage not found URL
void handleNotFound() {
  String message = "Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  DEBUG("scan done");
  if (n == 0)
    DEBUG("no networks found");
  else {
    DEB(n);
    DEBUG(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      DEB(i + 1);
      DEB(": ");
      DEB(WiFi.SSID(i));
      DEB(" (");
      DEB(WiFi.RSSI(i));
      DEB(")");
      DEBUG((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  DEBUG();
  st = "<ol>";
  selectSSID = "<input type='text' id='ssid' name='ssid' list='ssidList'/><datalist id='ssidList'>";
  for (int i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    selectSSID += "<option value='";
    selectSSID += WiFi.SSID(i);
    selectSSID += "'>";
    selectSSID += WiFi.SSID(i);
    selectSSID += "</option>";
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  selectSSID += "</datalist>";
  delay(100);
  WiFi.softAP("Onkyo NodeMCU");
  DEBUG("softap");
  launchAPWeb();
  DEBUG("over");
}

void createAPWebServer() {
  server.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(server.client());
  });
  server.on("/", []() {
    IPAddress ip = WiFi.softAPIP();
    String ipStr = ip.toString().c_str();  //String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    content = "<!DOCTYPE HTML><html>";
    content += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    content += "<link rel=\"icon\" href=\"data:,\">";
    content += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
    content += ".button { background-color: #253342; border: none; color: white; padding: 16px 40px;";
    content += "text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}";
    content += ".button2 {background-color: #555555;}</style></head>";
    content += "<body><h1>Onkyo NodeMCU</h1>";
    content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\" class=\"button\"></form>";
    // content += ipStr;
    content += "<p>";
    content += st;
    content += "</p><form method='get' action='setting'><label>SSID: </label>";
    content += selectSSID;
    content += "<br><label>Password: </label><input name='pass' length=64><br><input type=\"submit\" class=\"button\"></form>";
    content += "</body></html>";
    server.send(200, "text/html", content);
  });

  server.on("/scan", []() {
    IPAddress ip = WiFi.softAPIP();
    String ipStr = ip.toString().c_str();  //String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

    content = "<!DOCTYPE HTML>\r\n<html><button onclick='window.location.href=\"/\";'>Back</button></html>";
    server.send(200, "text/html", content);
  });

  server.on("/setting", []() {
    String qsid = server.arg("ssid");
    String qpass = server.arg("pass");
    if (qsid.length() > 0 && qpass.length() > 0) {
      DEBUG("clearing eeprom");
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      DEBUG(qsid);
      DEBUG();
      DEBUG(qpass);
      DEBUG();

      DEB("writing eeprom ssid: ");
      for (int i = 0; i < qsid.length(); ++i) {
        EEPROM.write(i, qsid[i]);
        DEB(qsid[i]);
      }
      DEBUG();
      DEB("writing eeprom pass: ");
      for (int i = 0; i < qpass.length(); ++i) {
        EEPROM.write(32 + i, qpass[i]);
        DEB(qpass[i]);
      }
      DEBUG();
      EEPROM.commit();

      content = "{\"Success\":\"saved to eeprom... The device will now reset to boot into new wifi\"}";
      statusCode = 200;

    } else {
      content = "{\"Error\":\"404 not found\"}";
      statusCode = 404;
      DEBUG("Sending 404");
    }

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(statusCode, "application/json", content);

    if (statusCode == 200) {
      delay(100);
      ESP.reset();
    }
  });
}

/*ONKYO*/
void onkyoSend(int command) {
  onkyoWriteHeader();

  for (int i = 0; i < 12; i++) {
    bool level = command & 0x800;
    command <<= 1;
    onkyoWriteBit(level);
  }

  onkyoWriteFooter();
}

void onkyoWriteHeader() {
  //DEBUG(micros());
  digitalWrite(ONKYO_PIN, HIGH);
  delayMicroseconds(3000);
  digitalWrite(ONKYO_PIN, LOW);
  delayMicroseconds(1000);
  //DEBUG(micros());
}
void onkyoWriteBit(bool level) {
  digitalWrite(ONKYO_PIN, HIGH);
  delayMicroseconds(1000);
  digitalWrite(ONKYO_PIN, LOW);

  if (level)
    delayMicroseconds(2000);
  else
    delayMicroseconds(1000);
}

void onkyoWriteFooter() {
  digitalWrite(ONKYO_PIN, HIGH);
  delayMicroseconds(1000);
  digitalWrite(ONKYO_PIN, LOW);
  delay(20);
}

void loop(void) {
  server.handleClient();
  delay(1);
}
