#ifndef ARDUINO
#include <ArduinoHelper.h>
#include <Raft.h>
#include <unixUdpProvider.h>
#include <chrono>

UnixUdpProvider udp;
RaftNode raft(&udp, (int)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
bool APMode = false;
unsigned long reconnectTimer;

void setup() {
  unsigned char test[2];
  unsigned short uint = 12345;
  test[0] = uint & 0xff;
  test[1] = (uint >> 8) & 0xff;
  unsigned short other_uint = test[0];
  other_uint |= test[1] << 8;
  printf("from: 0x%04x | %u to:0x%04x | %u\n", uint, uint, other_uint, other_uint);
  
  raft.setup();
}

void loop() {
    raft.transition("connected");

  raft.loop();
}

int main(){
    setup();
    while(true){
        loop();
    }
    return 0;
}
#endif
