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

IPAddress ArduinoUdpProvider::remoteIP() {
  return _udp.remoteIP();
}
