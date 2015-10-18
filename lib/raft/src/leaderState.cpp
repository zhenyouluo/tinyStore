#include "leaderState.h"

#include <ArduinoHelper.h>
#include <Log.h>

LeaderState::LeaderState(RaftNode *raft){
  _raft = raft;
}

void LeaderState::setup() {
  timeout = millis();
  Log(_raft->currentTerm);
  Log("\n");
}

void LeaderState::loop() {
  if (_raft->udp->parsePacket()){
    readPacket();
  }
  if (millis() > timeout){
    sendAppendEntries();
    resetTimer();
  }
}

void LeaderState::cleanup() {
  Log(_raft->currentTerm);
  Log("\n");
}

void LeaderState::resetTimer() {
  timeout = millis() + 30;
}

void LeaderState::sendAppendEntries() {
  int i = 0;
  _raft->messageBuffer[i++] = MESSAGE_TYPE_APPEND_ENTRIES;
  _raft->messageBuffer[i++] = _raft->currentTerm & 0xff;
  _raft->messageBuffer[i++] = (_raft->currentTerm >> 8) & 0xff;
  unsigned short prevLogIndex = 0;
  _raft->messageBuffer[i++] = prevLogIndex & 0xff;
  _raft->messageBuffer[i++] = (prevLogIndex >> 8 & 0xff);
  unsigned short prevLogTerm = 0;
  _raft->messageBuffer[i++] = prevLogTerm & 0xff;
  _raft->messageBuffer[i++] = (prevLogTerm >> 8) & 0xff;
  unsigned short newEntryCount = 0;
  _raft->messageBuffer[i++] = newEntryCount & 0xff;
  _raft->messageBuffer[i++] = (newEntryCount >> 8) & 0xff;
  for (int j = 0; j < newEntryCount; j++){
    //add entries
  }
  _raft->messageBuffer[i++] = _raft->commitIndex & 0xff;
  _raft->messageBuffer[i++] = (_raft->commitIndex >> 8) & 0xff;
  
  _raft->udp->sendPacket((unsigned char*) BRAODCAST_ADDRESS, PORT, _raft->messageBuffer, i);
}

void LeaderState::readPacket() {
  if (!_raft->udp->read(_raft->messageBuffer, MESSAGE_BUFFER_SIZE)){
    return;
  }
  switch (_raft->messageBuffer[0]){
    case MESSAGE_TYPE_APPEND_ENTRIES_RESPONSE:
      parseAppendEntriesResponseMessage();
      break;
    case MESSAGE_TYPE_REQUEST_VOTE:
      parseRequestVoteMessage();
      break;
  }
}

void LeaderState::parseAppendEntriesResponseMessage(){
  int i = 1;
  unsigned short term = _raft->messageBuffer[i++];
  term |= _raft->messageBuffer[i++] << 8;
  
  unsigned char remoteIP[4];
  _raft->udp->remoteIP(remoteIP);
  
  int remoteIndex = _raft->getNodeIndex(remoteIP);
  if (remoteIndex < 0){
    _raft->addNode(remoteIP);
  }
  
  if (term >_raft->currentTerm){
    _raft->currentTerm = term;
    _raft->udp->remoteIP(_raft->currentLeader);
    _raft->transition("higherTerm");
    return;
  }
}
void LeaderState::parseRequestVoteMessage(){
  int i = 1;
  unsigned short term = _raft->messageBuffer[i++];
  term |= _raft->messageBuffer[i++] << 8;
  unsigned short candidateLastLogIndex = _raft->messageBuffer[i++];
  candidateLastLogIndex |= _raft->messageBuffer[i++] << 8;
  unsigned short candidateLastLogTerm = _raft->messageBuffer[i++];
  candidateLastLogTerm |= _raft->messageBuffer[i++] << 8;
  unsigned char ip[4];
  _raft->udp->remoteIP(ip);
  i = 0;
  unsigned char result = 0x01;

  if (term < _raft->currentTerm || ((!IPisEmpty(_raft->votedFor) && !IPequals(_raft->votedFor, ip)) || candidateLastLogIndex < _raft->commitIndex || candidateLastLogTerm < _raft->getLastLogEntry().term)){
    result = 0x00;
  }
  _raft->messageBuffer[i++] = MESSAGE_TYPE_VOTE;
  _raft->messageBuffer[i++] = _raft->currentTerm & 0xff;
  _raft->messageBuffer[i++] = (_raft->currentTerm >> 8) & 0xff;
  _raft->messageBuffer[i++] = result;
  _raft->udp->sendPacket(ip, PORT, _raft->messageBuffer, i);
  if (result){
      IPcopy(ip, _raft->votedFor);
    _raft->transition("higherTerm");
  }
}