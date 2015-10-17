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
  if (timeout <= millis()){
    _raft->transition("startVote");
  }
}

void FollowerState::cleanup() {

}

void FollowerState::readPacket() {
  if (!_raft->udp->read(_raft->messageBuffer, MESSAGE_BUFFER_SIZE)){
    return;
  }
  switch (_raft->messageBuffer[0]){
    case MESSAGE_TYPE_REQUEST_VOTE:
      parseRequestVoteMessage();
      break;
    case MESSAGE_TYPE_APPEND_ENTRIES:
      parseAppendEntriesMessage();
      break;
  }
}

void FollowerState::parseRequestVoteMessage(){
  unsigned int term = _raft->messageBuffer[1] | _raft->messageBuffer[2] << 8;
  unsigned int candidateLastLogIndex = _raft->messageBuffer[3] | _raft->messageBuffer[4] << 8;
  unsigned int candidateLastLogTerm = _raft->messageBuffer[5] | _raft->messageBuffer[6] << 8;
  unsigned char ip[4];
  _raft->udp->remoteIP(ip);

  if (term < _raft->currentTerm || (!IPisEmpty(_raft->votedFor) && !IPequals(_raft->votedFor, ip) || candidateLastLogIndex < _raft->commitIndex || candidateLastLogTerm < _raft->getLastLogEntry().term)){
    _raft->messageBuffer[0] = MESSAGE_TYPE_VOTE;
    _raft->messageBuffer[1] = (unsigned char) term;
    _raft->messageBuffer[2] = (unsigned char) (term >> 8);
    _raft->messageBuffer[3] = 0x00;
    _raft->udp->sendPacket(ip, 55056, _raft->messageBuffer, 4);
    return;
  }

}

void FollowerState::parseAppendEntriesMessage(){

}
