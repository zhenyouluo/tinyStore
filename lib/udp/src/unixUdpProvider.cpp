#if !ARDUINO

#include "unixUdpProvider.h"

#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <Log.h>


std::queue<message> UnixUdpProvider::_messageQueue;
char *UnixUdpProvider::_messageBuf;
uv_udp_t UnixUdpProvider::_sock;
unsigned char UnixUdpProvider::_localIP[4];

bool UnixUdpProvider::begin(int port) {
    uv_loop_t *loop = uv_default_loop();
    if (uv_udp_init(loop, &_sock)){
        return false;
    }
    
    char buf[32];
    uv_interface_address_t *info;
    int count, i;
    
    uv_interface_addresses(&info, &count);
    i = count;
    while (i--) {
        uv_interface_address_t interface = info[i];
        if (interface.is_internal){
            continue;
        }
        if (interface.address.address4.sin_family != AF_INET){
            continue;
        }
        uv_ip4_name(&interface.address.address4, buf, sizeof(buf));
        uv_inet_pton(AF_INET, buf, _localIP);
      i = 0;
    }
    
    uv_ip4_addr("0.0.0.0", port, &_recvAddr);
    uv_udp_bind(&_sock, (const struct sockaddr*) &_recvAddr, UV_UDP_REUSEADDR);
    uv_udp_set_broadcast(&_sock, 1);
    if (uv_udp_recv_start(&_sock, allocBuffer, onRead)){
        return false;
    }

    return true;
}

void UnixUdpProvider::allocBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    if (_messageBuf == NULL){
        _messageBuf = (char*) malloc(suggested_size);
    }
    buf->base = _messageBuf;
    buf->len = suggested_size;
}

void UnixUdpProvider::onRead(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
    if (nread < 0) {
        fprintf(stderr, "Read error %s\n", uv_err_name((int) nread));
        uv_close((uv_handle_t*) handle, NULL);
        free(buf->base);
        _messageBuf = NULL;
        return;
    }
    if (addr == NULL){
        return;
    }

    char senderAddr[17] = {'\0'};
    uv_ip4_name((struct sockaddr_in*) addr, senderAddr, 16);

    unsigned char *remoteAddress = (unsigned char*) malloc(sizeof(unsigned char) * 4);
    uv_inet_pton(AF_INET, senderAddr, remoteAddress);


    char *messageBuf = (char*) malloc(sizeof(char) * nread);
    strncpy(messageBuf, buf->base, nread);

    message m = {
        (int) nread,
        remoteAddress,
        messageBuf,
        0
    };
    _messageQueue.push(m);
}

void UnixUdpProvider::onSent(uv_udp_send_t* req, int status) {
    if (status < 0){
        fprintf(stderr, "sending error: %s\n", uv_err_name(status));
    }
    free(req);
}

void UnixUdpProvider::stop() {
    free(_messageBuf);
    _messageBuf = NULL;
    _messageQueue.empty();
    uv_udp_recv_stop(&_sock);
}

int UnixUdpProvider::read(unsigned char *packetBuffer, unsigned int maxSize) {
    if (_messageQueue.size() < 1){
        return 0;
    }
    if (_messageQueue.front().length - _messageQueue.front().readOffset <= maxSize){
        
        strncpy((char*)packetBuffer, _messageQueue.front().packetBuffer + _messageQueue.front().readOffset, _messageQueue.front().length - _messageQueue.front().readOffset);
        _messageQueue.front().readOffset = _messageQueue.front().length;
        return _messageQueue.front().length;
    }
    else {
        strncpy((char*) packetBuffer, _messageQueue.front().packetBuffer + _messageQueue.front().readOffset, maxSize);
        _messageQueue.front().readOffset += maxSize;
        return maxSize;
    }
}

bool UnixUdpProvider::sendPacket(unsigned char *remoteIP, int remotePort, unsigned char *buffer, unsigned int size) {
    char targetAddrString [17] = {'\0'};
    uv_udp_send_t *req = (uv_udp_send_t*) malloc(sizeof(uv_udp_send_t));

    uv_inet_ntop(AF_INET, remoteIP, targetAddrString, 16);

    struct sockaddr_in targetAddr;
    uv_ip4_addr(targetAddrString, 55056, &targetAddr);
    uv_buf_t sendBuf;
    sendBuf.base = (char*) buffer;
    sendBuf.len = size;

    int result = uv_udp_send(req, &_sock, &sendBuf, 1, (const struct sockaddr*) &targetAddr, onSent);
    if (result < 0){
        printf("error while sending: %s\n", uv_err_name(result));
        return false;
    }
    return true;
}

int UnixUdpProvider::parsePacket() {
    if (_messageQueue.size() > 0){
        free((void*) _messageQueue.front().packetBuffer);
        free((void*) _messageQueue.front().remoteIP);
        _messageQueue.pop();
    }
    uv_loop_t *loop = uv_default_loop();
    uv_run(loop, UV_RUN_NOWAIT);
    if (_messageQueue.size() < 1){
        return 0;
    }
    return _messageQueue.front().length - _messageQueue.front().readOffset;
}

unsigned int UnixUdpProvider::available() {
    if (_messageQueue.size() == 0){
        return 0;
    }
    return _messageQueue.front().length - _messageQueue.front().readOffset;
}

void UnixUdpProvider::remoteIP(unsigned char *buffer) {
    if (_messageQueue.size() > 0){
        for (int i = 0; i < 4; i++){
            buffer[i] = _messageQueue.front().remoteIP[i];
        }
    }
}

void UnixUdpProvider::localIP(unsigned char *buffer) {
  for (int i = 0; i < 4; i++){
    buffer[i] = _localIP[i];
  }
}
#endif
