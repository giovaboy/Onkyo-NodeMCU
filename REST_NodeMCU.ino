/*
 *  Simple hello world Json REST response
  *  by Mischianti Renzo <https://www.mischianti.org>
 *
 *  https://www.mischianti.org/
 *
 */
 
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// UART
#define UART_SPEED  9600
// ONKYO
#define ONKYO_PIN 2//13

const char* ssid = "TP-LINK_379D";
const char* password = "RoccaRaso";
 
ESP8266WebServer server(80);

void onkyoSend(int command)
{
  onkyoWriteHeader();
  
  for(int i=0;i<12;i++)
  {
    bool level = command & 0x800;
    command <<= 1;
    onkyoWriteBit(level);
  }

  onkyoWriteFooter();
}

void onkyoWriteHeader()
{
  //Serial.println(micros());
  digitalWrite(ONKYO_PIN,HIGH);
  delayMicroseconds(3000);
  digitalWrite(ONKYO_PIN,LOW);
  delayMicroseconds(1000);
  //Serial.println(micros());
}
void onkyoWriteBit(bool level)
{
  digitalWrite(ONKYO_PIN,HIGH);
  delayMicroseconds(1000);  
  digitalWrite(ONKYO_PIN,LOW);
    
  if(level)
    delayMicroseconds(2000); 
  else
    delayMicroseconds(1000); 
}

void onkyoWriteFooter()
{
  digitalWrite(ONKYO_PIN,HIGH);
  delayMicroseconds(1000);
  digitalWrite(ONKYO_PIN,LOW);
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

// Serving Command List
void getRoutesList() {
    server.send(200, "text/json", "[{\"route\": \"on\",\"cmd\": \"0x02F\"}, {\"route\": \"off\",\"cmd\": \"0x0DA\"}, {\"route\": \"mute\",\"cmd\": \"0x005\"}, {\"route\": \"volup\",\"cmd\": \"0x002\"}, {\"route\": \"voldown\",\"cmd\": \"0x003\"}, {\"route\": \"source\",\"cmd\": \"0x0D5\"}]");
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
 
void setup(void) {
  pinMode(ONKYO_PIN, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
 
  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void) {
  server.handleClient();
}
