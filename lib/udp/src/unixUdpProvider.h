#ifndef ARDUINO

#ifndef unixUdpProvider_h
#define unixUdpProvider_h

#include "udpProvider.h"

#include <uv.h>
#include <queue>

struct message {
    int length;
    const unsigned char *remoteIP;
    const char *packetBuffer;
    int readOffset;
};

class UnixUdpProvider : public AbstractUdpProvider {
private:
    static uv_udp_t _sock;
    sockaddr_in _recvAddr;
    sockaddr_in _broadcastAddr;
  static unsigned char _localIP[4];

    static std::queue<message> _messageQueue;
    static char *_messageBuf;

    static void allocBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
    static void onRead(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
    static void onSent(uv_udp_send_t* req, int status);
public:
    virtual bool begin(int port);
    virtual void stop();
    virtual int read(unsigned char *packetBuffer, unsigned int maxSize);
    virtual bool sendPacket(unsigned char *remoteIP, int remotePort, unsigned char *buffer, unsigned int size);
    virtual int parsePacket();
    virtual unsigned int available();
    virtual void remoteIP(unsigned char *buffer);
    virtual void localIP(unsigned char *buffer);
};

#endif
#endif // !ARDUINO
