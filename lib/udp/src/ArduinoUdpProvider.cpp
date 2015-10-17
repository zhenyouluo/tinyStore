#include "ArduinoUdpProvider.h"


ArduinoUdpProvider::ArduinoUdpProvider(){
}

bool ArduinoUdpProvider::begin(int port) {
  return _udp.begin(port);
}

void ArduinoUdpProvider::stop() {
  _udp.stop();
}

int ArduinoUdpProvider::read(unsigned char *packetBuffer, unsigned int maxSize) {
  return _udp.read(packetBuffer, maxSize);
}

int ArduinoUdpProvider::parsePacket() {
  return _udp.parsePacket();
}

bool ArduinoUdpProvider::sendPacket(unsigned char *remoteIP, int remotePort, unsigned char *buffer, unsigned int size) {
  IPAddress targetIP(remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3]);
  Serial.print("send ");
  for (int i = 0; i < size; i++){
    Serial.print((unsigned int) buffer[i]);
    Serial.print(" ");
  }
  Serial.print("\n");
  if (!_udp.beginPacket(targetIP, remotePort)){
    return false;
  }

  if (!_udp.write(buffer, size)) {
    return false;
  }

  if (!_udp.endPacket()){
    return false;
  }

  return true;
}

unsigned int ArduinoUdpProvider::available() {
  return _udp.available();
}

void ArduinoUdpProvider::remoteIP(unsigned char *buffer) {
  IPAddress addr = _udp.remoteIP();
  buffer[0] = addr[0];
  buffer[1] = addr[1];
  buffer[2] = addr[2];
  buffer[3] = addr[3];
}

void ArduinoUdpProvider::localIP(unsigned char *buffer) {
  IPAddress addr = WiFi.localIP();
  if (addr[0] == 0 && addr[1] == 0 && addr[2] == 0 && addr[3] == 0){
    addr = WiFi.softAPIP();
  }
  buffer[0] = addr[0];
  buffer[1] = addr[1];
  buffer[2] = addr[2];
  buffer[3] = addr[3];
}
