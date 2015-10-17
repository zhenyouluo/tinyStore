
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <Raft.h>

const unsigned int MAX_MESSAGE_SIZE = 512;

WiFiUDP udp;
IPAddress ip(192,168,178,31);
byte messageBuffer[MAX_MESSAGE_SIZE];

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("test", "bremen12");
}

void loop() {
  udp.beginPacket(ip, 55056);
  String m = "Hello, World!";
  m.getBytes(messageBuffer, MAX_MESSAGE_SIZE);
  udp.write(messageBuffer, m.length());
  udp.endPacket();
  delay(1000);
}
