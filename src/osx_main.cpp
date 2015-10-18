#ifndef ARDUINO
#include <ArduinoHelper.h>
#include <Raft.h>
#include <unixUdpProvider.h>
#include <chrono>

UnixUdpProvider udp;
RaftNode raft(&udp, (int)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

void setup() {
  raft.setup();
}

void loop() {
  raft.loop();
  raft.transition("connected");
}

int main(){
  setup();
  while(true){
    loop();
  }
  return 0;
}
#endif
