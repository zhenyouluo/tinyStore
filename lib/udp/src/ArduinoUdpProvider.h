#ifndef ARDUINO_UDP_PROVIDER_H
#define ARDUINO_UDP_PROVIDER_H

#include "udpProvider.h"

class ArduinoUdpProvider : public AbstractUdpProvider {
private:
  WiFiUDP _udp;
public:
  ArduinoUdpProvider();
  virtual bool begin(int port);
  virtual void stop();
  virtual int read(unsigned char *packetBuffer, unsigned int maxSize);
  virtual int parsePacket();
  virtual bool sendPacket(unsigned char *remoteIP, int remotePort, unsigned char *buffer, unsigned int size);
  virtual unsigned int available();
  virtual void remoteIP(unsigned char *buffer);
};

#endif
