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
