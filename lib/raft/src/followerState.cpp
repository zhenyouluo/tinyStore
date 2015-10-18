#include "followerState.h"

#include <ArduinoHelper.h>
#include <Log.h>

FollowerState::FollowerState(RaftNode *raft){
  _raft = raft;
}

void FollowerState::setup() {
  unsigned long rand = (unsigned long) random(150, 300);
  timeout = millis() + rand;
  Log(_raft->currentTerm);
  Log("\n");
}

void FollowerState::loop() {
  if (_raft->udp->parsePacket()){
    readPacket();
  }
  else if (millis() > timeout){
    _raft->removeNode(_raft->currentLeader);
    _raft->transition("startVote");
  }
}

void FollowerState::cleanup() {
  Log(_raft->currentTerm);
  Log("\n");
}

void FollowerState::readPacket() {
  _raft->udp->read(_raft->messageBuffer, MESSAGE_BUFFER_SIZE);
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
  int i = 1;
  unsigned short term = _raft->messageBuffer[i++];
  term = term | _raft->messageBuffer[i++] << 8;
  unsigned short candidateLastLogIndex = _raft->messageBuffer[i++];
  candidateLastLogIndex |= _raft->messageBuffer[i++] << 8;
  unsigned short candidateLastLogTerm = _raft->messageBuffer[i++];
  candidateLastLogTerm |= _raft->messageBuffer[i++] << 8;
  unsigned char ip[4];
  _raft->udp->remoteIP(ip);
  unsigned char result = 0x01;

  if (term < _raft->currentTerm || ((!IPisEmpty(_raft->votedFor) && !IPequals(_raft->votedFor, ip)) || candidateLastLogIndex < _raft->commitIndex || candidateLastLogTerm < _raft->getLastLogEntry().term)){
    result = 0x00;
  }
  
  i = 0;
  _raft->messageBuffer[i++] = MESSAGE_TYPE_VOTE;
  _raft->messageBuffer[i++] = term & 0xff;
  _raft->messageBuffer[i++] = (term >> 8) & 0xff;
  _raft->messageBuffer[i++] = result;
  _raft->udp->sendPacket(ip, PORT, _raft->messageBuffer, i);
  if (result){
    IPcopy(ip, _raft->votedFor);
  }
}

void FollowerState::parseAppendEntriesMessage(){
  int i = 1;
  unsigned short term = _raft->messageBuffer[i++];
  term |= _raft->messageBuffer[i++] << 8;
  unsigned char remoteIP[4];
  _raft->udp->remoteIP(remoteIP);
  if (_raft->getNodeIndex(remoteIP) < 0){
    _raft->addNode(remoteIP);
  }
  if (term >= _raft->currentTerm) {
    _raft->currentTerm = term;
    IPcopy(remoteIP, _raft->currentLeader);
    timeout = millis() + (unsigned long) random(150, 300);
  }
  i = 0;
  _raft->messageBuffer[i++] = MESSAGE_TYPE_APPEND_ENTRIES_RESPONSE;
  _raft->messageBuffer[i++] = _raft->currentTerm & 0xff;
  _raft->messageBuffer[i++] = (_raft->currentTerm >> 8) & 0xff;
  _raft->udp->sendPacket(remoteIP, PORT, _raft->messageBuffer, i);
}
