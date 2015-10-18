#include "leaderState.h"

#include <ArduinoHelper.h>
#include <Log.h>

LeaderState::LeaderState(RaftNode *raft){
  _raft = raft;
}

void LeaderState::setup() {
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    _nextIndex[i] = _raft->lastEntryIndex + 1;
  }
  
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
  
  sendNewEntries();
  increaseCommitIndex();
  
  while (_raft->lastApplied <= _raft->commitIndex){
    LogEntry entry = _raft->log[(_raft->lastApplied++) % MAX_LOG_LENGTH];
    Log(entry.type);
    Log(" ");
    for (int i = 0; i < sizeof(entry.data); i++){
      Log(entry.data[i]);
    }
    Log("\n");
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
  unsigned char result = _raft->messageBuffer[i++];
  unsigned short lastEntryIndex = _raft->messageBuffer[i++];
  lastEntryIndex |= _raft->messageBuffer[i++] << 8;
  
  unsigned char remoteIP[4];
  _raft->udp->remoteIP(remoteIP);
  
  int remoteIndex = _raft->getNodeIndex(remoteIP);
  if (remoteIndex < 0){
    remoteIndex = _raft->addNode(remoteIP);
  }
  
  if (term >_raft->currentTerm){
    _raft->currentTerm = term;
    _raft->udp->remoteIP(_raft->currentLeader);
    _raft->transition("higherTerm");
    return;
  }
  
  if (remoteIndex < 0 || !result){
    return;
  }
  
  _matchIndex[remoteIndex] = lastEntryIndex;
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
    _raft->currentTerm = term;
    _raft->udp->remoteIP(_raft->currentLeader);
    _raft->transition("higherTerm");
  }
}

void LeaderState::sendNewEntries() {
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    if (_raft->lastEntryIndex <= _nextIndex[i]){
      continue;
    }
    if (IPisEmpty(_raft->nodes[i])){
      continue;
    }
    
    int j = 0;
    _raft->messageBuffer[j++] = MESSAGE_TYPE_APPEND_ENTRIES;
    _raft->messageBuffer[j++] = _raft->currentTerm & 0xff;
    _raft->messageBuffer[j++] = (_raft->currentTerm >> 8) & 0xff;
    unsigned short prevLogIndex = _nextIndex[i] - 1;
    _raft->messageBuffer[j++] = prevLogIndex & 0xff;
    _raft->messageBuffer[j++] = (prevLogIndex >> 8 & 0xff);
    LogEntry prevLogEntry = _raft->log[prevLogIndex % MAX_LOG_LENGTH];
    _raft->messageBuffer[j++] = prevLogEntry.term & 0xff;
    _raft->messageBuffer[j++] = (prevLogEntry.term >> 8) & 0xff;
    unsigned short newEntryCount = _raft->lastEntryIndex - prevLogIndex;
    _raft->messageBuffer[j++] = newEntryCount & 0xff;
    _raft->messageBuffer[j++] = (newEntryCount >> 8) & 0xff;
    
    for (; _nextIndex[i] <= _raft->lastEntryIndex; _nextIndex[i]++){
      LogEntry newEntry = _raft->log[(_nextIndex[i]) % MAX_LOG_LENGTH];
      _raft->messageBuffer[j++] = newEntry.term & 0xff;
      _raft->messageBuffer[j++] = (newEntry.term >> 8) & 0xff;
      _raft->messageBuffer[j++] = newEntry.type;
      for (int l = 0; l < sizeof(newEntry.data); l++){
        _raft->messageBuffer[j++] = newEntry.data[l];
      }
    }
    
    _raft->messageBuffer[j++] = _raft->commitIndex & 0xff;
    _raft->messageBuffer[j++] = (_raft->commitIndex >> 8) & 0xff;
    
    _raft->udp->sendPacket(_raft->nodes[i], PORT, _raft->messageBuffer, j);
  }
}

void LeaderState::increaseCommitIndex() {
  unsigned short highestIndex = 0;
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    if (IPisEmpty(_raft->nodes[i])){
      continue;
    }
    if (_matchIndex[i] > highestIndex){
      highestIndex = _matchIndex[i];
    }
  }
  
  while (_raft->commitIndex < highestIndex) {
    int consensusCount = 0;
    for (int i = 0; i < MAX_NODE_COUNT; i++){
      if (_matchIndex[i] > _raft->commitIndex){
        ++consensusCount;
      }
    }
    if (consensusCount >= _raft->nodeCount() / 2 + 1 && _raft->log[(_raft->commitIndex + 1) % MAX_LOG_LENGTH].term == _raft->currentTerm){
      ++_raft->commitIndex;
    }
    else {
      return;
    }
  }
}