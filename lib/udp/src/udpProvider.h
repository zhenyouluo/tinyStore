#pragma once

class AbstractUdpProvider {
public:
    virtual bool begin(int port) = 0;
    virtual bool end() = 0;
	virtual void read(char *packetBuffer, unsigned int maxSize) = 0;
	virtual bool sendPacket(unsigned char *remoteIP, int remotePort, char *buffer, unsigned int size) = 0;
	virtual int parsePacket() = 0;
	virtual unsigned int available() = 0;
	virtual const unsigned char* remoteIP() = 0;
};