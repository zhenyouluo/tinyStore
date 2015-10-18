
#include <Arduino.h>
#include <Log.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <Raft.h>
#include <ArduinoUdpProvider.h>

const unsigned int MAX_RECONNECT_MILLIS = 10000;
const char ssid[] = "test";
const char pass[] = "bremen12";

ArduinoUdpProvider udp;
RaftNode raft(&udp, analogRead(0));
int wifiStatus = WL_IDLE_STATUS;
bool APMode = false;
unsigned long reconnectTimer;

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  Serial.write("\nconnecting to WiFi ...\n");
  wifiStatus = WiFi.begin(ssid, pass);
  reconnectTimer = millis() + MAX_RECONNECT_MILLIS;
  raft.setup();
}

void createAP() {
  Log("create AP ...\n");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  APMode = true;
}

void resetTimer(){
  reconnectTimer = millis() + MAX_RECONNECT_MILLIS;
}

void loop() {
  if (!APMode){
    wifiStatus = WiFi.status();
    switch (wifiStatus){
      case WL_CONNECTED:
        resetTimer();
        raft.transition("connected");
        break;
      default:
        raft.transition("connectionLost");
        break;
    }
    if (millis() > reconnectTimer){
      createAP();
    }
  }
  else {
    raft.transition("connected");
  }

  raft.loop();
}
