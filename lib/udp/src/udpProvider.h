#pragma once

class AbstractUdpProvider {
public:
  virtual bool begin(int port) = 0;
  virtual void stop() = 0;
	virtual int read(unsigned char *packetBuffer, unsigned int maxSize) = 0;
	virtual bool sendPacket(unsigned char *remoteIP, int remotePort, unsigned char *buffer, unsigned int size) = 0;
	virtual int parsePacket() = 0;
	virtual unsigned int available() = 0;
	virtual void remoteIP(unsigned char *buffer) = 0;
	virtual void localIP(unsigned char *buffer) = 0;
};
