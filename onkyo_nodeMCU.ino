#include "Arduino.h"
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include <DoubleResetDetector.h>

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
//#define DRD_TIMEOUT 5

// RTC Memory Address for the DoubleResetDetector to use
//#define DRD_ADDRESS 0

// UART
#define UART_SPEED 9600
// ONKYO
#define ONKYO_PIN 2  //13

//Variables
uint i = 0;
uint statusCode;
String st;
String selectSSID;
String content;

//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

//Establishing Local server at port 80 whenever required
//DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);
ESP8266WebServer server(80);

void setup() {
  /*if (drd.detectDoubleReset()) {
    clearEEPROM();
  }*/
  //drd.loop();
  pinMode(ONKYO_PIN, OUTPUT);
  Serial.begin(115200);  //Initialising if(DEBUG)Serial Monitor
  Serial.println();

  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512);  //Initialasing EEPROM
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.println("Startup");



  //---------------------------------------- Read EEPROM for SSID and pass
  Serial.println("Reading EEPROM ssid");

  String esid;
  for (int i = 0; i < 32; ++i) {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i) {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);


  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi()) {
    Serial.println("Succesfully Connected!!!");
    if (MDNS.begin("onkyo_nodeMCU")) {
      Serial.println("MDNS responder started");
    }
    // Set server routing
    restServerRouting();
    // Set not found response
    server.onNotFound(handleNotFound);
    // Start server
    server.begin();
    Serial.println("HTTP server started");
    return;
  } else {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();  // Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");

  while ((WiFi.status() != WL_CONNECTED)) {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
}

//-------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change
bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while (c < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb() {
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}


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
  //Serial.println(micros());
  digitalWrite(ONKYO_PIN, HIGH);
  delayMicroseconds(3000);
  digitalWrite(ONKYO_PIN, LOW);
  delayMicroseconds(1000);
  //Serial.println(micros());
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

void onkyoSendOn() {
  server.send(200, "text/json", "{\"route\": \"on\",\"cmd\": \"0x02F\"}");
  onkyoSend(0x02F);
}

void onkyoSendOff() {
  server.send(200, "text/json", "{\"route\": \"off\",\"cmd\": \"0x0DA\"}");
  onkyoSend(0x0DA);
}

void onkyoSendMute() {
  server.send(200, "text/json", "{\"route\": \"mute\",\"cmd\": \"0x005\"}");
  onkyoSend(0x005);
}

void onkyoSendVolUp() {
  server.send(200, "text/json", "{\"route\": \"volup\",\"cmd\": \"0x002\"}");
  onkyoSend(0x002);
}

void onkyoSendVolDown() {
  server.send(200, "text/json", "{\"route\": \"voldown\",\"cmd\": \"0x003\"}");
  onkyoSend(0x003);
}

void onkyoSendSource() {
  server.send(200, "text/json", "{\"route\": \"source\",\"cmd\": \"0x0D5\"}");
  onkyoSend(0x0D5);
}

/*
void onkyoSendCustom(char* cmd) {
  server.send(200, "text/json", "{\"route\": \"source\",\"cmd\": \"" + cmd +"\"}");
  unsigned long value = strtol(cmd, 0, 16);
  onkyoSend(value);
}
*/
// Serving Command List
void getRoutesList() {
  server.send(200, "text/json", "[{\"route\": \"on\",\"cmd\": \"0x02F\"}, {\"route\": \"off\",\"cmd\": \"0x0DA\"}, {\"route\": \"mute\",\"cmd\": \"0x005\"}, {\"route\": \"volup\",\"cmd\": \"0x002\"}, {\"route\": \"voldown\",\"cmd\": \"0x003\"}, {\"route\": \"source\",\"cmd\": \"0x0D5\"},{\"route\": \"initialize\",\"cmd\": \"Reset WiFi settings\"}]");
}

// Clear EEPROM
void clearEEPROM() {
  for (int i = 0; i < 96; i++) {
    EEPROM.write(i, 0);
  }
  delay(500);
  EEPROM.commit();
  server.send(200, "text/json", "{\"route\": \"initialize\",\"cmd\": \"EEPROM ssid & pass data cleared!\"}");
  Serial.println("EEPROM ssid & pass data cleared!");
  ESP.reset();
}

// Define routing
void restServerRouting() {
  server.on("/", HTTP_GET, getRoutesList);
  server.on(F("/on"), HTTP_GET, onkyoSendOn);
  server.on(F("/off"), HTTP_GET, onkyoSendOff);
  server.on(F("/mute"), HTTP_GET, onkyoSendMute);
  server.on(F("/volup"), HTTP_GET, onkyoSendVolUp);
  server.on(F("/voldown"), HTTP_GET, onkyoSendVolDown);
  server.on(F("/source"), HTTP_GET, onkyoSendSource);
  server.on(F("/initialize"), HTTP_GET, clearEEPROM);
}

// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
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
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  selectSSID = "<select id='ssid' name='ssid'>";
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
  selectSSID += "</select>";
  delay(100);
  WiFi.softAP("onkyo_nodeMCU", "");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}

void createWebServer() {
  server.on("/", []() {
    IPAddress ip = WiFi.softAPIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    content = "<!DOCTYPE HTML>\r\n<html>Onkyo_nodeMCU";
    content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
    content += ipStr;
    content += "<p>";
    content += st;
    content += "</p><form method='get' action='setting'><label>SSID: </label><br>";
    content += selectSSID;
    content += "<label>Password: </label><input name='pass' length=64><br><input type='submit'></form>";
    content += "</html>";
    server.send(200, "text/html", content);
  });
  server.on("/scan", []() {
    //setupAP();
    IPAddress ip = WiFi.softAPIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

    content = "<!DOCTYPE HTML>\r\n<html>go back";
    server.send(200, "text/html", content);
  });

  server.on("/setting", []() {
    String qsid = server.arg("ssid");
    String qpass = server.arg("pass");
    if (qsid.length() > 0 && qpass.length() > 0) {
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      Serial.println(qsid);
      Serial.println("");
      Serial.println(qpass);
      Serial.println("");

      Serial.println("writing eeprom ssid:");
      for (int i = 0; i < qsid.length(); ++i) {
        EEPROM.write(i, qsid[i]);
        Serial.print("Wrote: ");
        Serial.println(qsid[i]);
      }
      Serial.println("writing eeprom pass:");
      for (int i = 0; i < qpass.length(); ++i) {
        EEPROM.write(32 + i, qpass[i]);
        Serial.print("Wrote: ");
        Serial.println(qpass[i]);
      }
      EEPROM.commit();

      content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
      statusCode = 200;
      ESP.reset();
    } else {
      content = "{\"Error\":\"404 not found\"}";
      statusCode = 404;
      Serial.println("Sending 404");
    }
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(statusCode, "application/json", content);
  });
}

void loop(void) {
  server.handleClient();
  delay(1);
}
