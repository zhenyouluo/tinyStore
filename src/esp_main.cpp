
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <Raft.h>
#include <ArduinoUdpProvider.h>

const unsigned int MAX_MESSAGE_SIZE = 512;

ArduinoUdpProvider udp;
unsigned char ip[4] = {192,168,43,244};
unsigned char messageBuffer[MAX_MESSAGE_SIZE];
RaftNode raft(&udp, analogRead(0));

void setup() {

  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  Serial.write("connecting to WiFi ...\n");
  WiFi.begin("test", "bremen12");
  Serial.write("connected!\n");
}

void loop() {
  String m = "Hello, World!";
  m.getBytes(messageBuffer, MAX_MESSAGE_SIZE);
  Serial.write("sending packet ...\n");
  if (udp.sendPacket(ip, 55056, messageBuffer, m.length())){
    Serial.write("packet sent!\n");
  }
  else {
    Serial.write("error sending packet\n");
  }
  delay(1000);
}
