#include "followerState.h"

FollowerState::FollowerState(RaftNode *raft){
  _raft = raft;
}

void FollowerState::setup() {
    timeout = millis() + (unsigned long) random(150, 300);
}

void FollowerState::loop() {
  if (_raft->udp->parsePacket()){
    readPacket();
  }
}

void FollowerState::cleanup() {

}

void FollowerState::readPacket() {
  _raft->udp->read(_raft->messageBuffer, MESSAGE_BUFFER_SIZE);
}
