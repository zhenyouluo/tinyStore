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
  commandTimeout = millis() + 1000;
}

void LeaderState::loop() {
  if (_raft->udp->parsePacket()){
    readPacket();
  }
  if (millis() > timeout){
    sendAppendEntries();
    resetTimer();
  }
  if (millis() > commandTimeout){
    LogEntry newEntry;
    newEntry.term = _raft->currentTerm;
    newEntry.type = 0x42;
    newEntry.data[0] = 0x12;
    newEntry.data[1] = 0x34;
    newEntry.data[2] = 0x56;
    newEntry.data[3] = 0x78;
    newEntry.data[4] = 0x90;
    newEntry.data[5] = 0xab;
    newEntry.data[6] = 0xcd;
    newEntry.data[7] = 0xef;
    _raft->log[(++_raft->lastEntryIndex) % MAX_LOG_LENGTH] = newEntry;
    unsigned char localIP[4];
    _raft->udp->localIP(localIP);
    int ownIndex = _raft->getNodeIndex(localIP);
    if (ownIndex < 0){
      ownIndex = _raft->addNode(localIP);
    }
    _nextIndex[ownIndex] = _raft->lastEntryIndex + 1;
    _matchIndex[ownIndex] = _raft->lastEntryIndex;
    Log("Leader Submit ");
    Log(newEntry.type);
    Log(": ");
    for (int i = 0; i < sizeof(newEntry.data); i++){
      Log(newEntry.data[i]);
      Log(" ");
    }
    Log("\n");
    commandTimeout = millis() + 1000;
  }

  sendNewEntries();
  increaseCommitIndex();

  while (_raft->lastApplied <= _raft->commitIndex){
    LogEntry entry = _raft->log[(_raft->lastApplied++) % MAX_LOG_LENGTH];
    Log("Leader Apply ");
    Log(entry.type);
    Log(": ");
    for (int i = 0; i < sizeof(entry.data); i++){
      Log(entry.data[i]);
      Log(" ");
    }
    Log("\n");
  }
}

void LeaderState::cleanup() {

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
  short lastEntryIndex = _raft->messageBuffer[i++];
  lastEntryIndex |= _raft->messageBuffer[i++] << 8;

  unsigned char remoteIP[4];
  _raft->udp->remoteIP(remoteIP);

  int remoteIndex = _raft->getNodeIndex(remoteIP);
  if (remoteIndex < 0){
    remoteIndex = _raft->addNode(remoteIP);
  }
  if (remoteIndex >= 0){
    _nextIndex[remoteIndex] = _raft->lastEntryIndex + 1;
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
  short candidateLastLogIndex = _raft->messageBuffer[i++];
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
    if (IPisEmpty(_raft->nodes[i])){
      continue;
    }
    if (_raft->lastEntryIndex < _nextIndex[i]){
      continue;
    }

    int j = 0;
    _raft->messageBuffer[j++] = MESSAGE_TYPE_APPEND_ENTRIES;
    _raft->messageBuffer[j++] = _raft->currentTerm & 0xff;
    _raft->messageBuffer[j++] = (_raft->currentTerm >> 8) & 0xff;
    short prevLogIndex = _nextIndex[i] - 1;
    _raft->messageBuffer[j++] = prevLogIndex & 0xff;
    _raft->messageBuffer[j++] = (prevLogIndex >> 8 & 0xff);
    LogEntry prevLogEntry = _raft->log[prevLogIndex % MAX_LOG_LENGTH];
    _raft->messageBuffer[j++] = prevLogEntry.term & 0xff;
    _raft->messageBuffer[j++] = (prevLogEntry.term >> 8) & 0xff;
    unsigned short newEntryCount = _raft->lastEntryIndex - prevLogIndex;
    _raft->messageBuffer[j++] = newEntryCount & 0xff;
    _raft->messageBuffer[j++] = (newEntryCount >> 8) & 0xff;
    

    for (int k = _nextIndex[i]; k <= _raft->lastEntryIndex; k++){
      LogEntry newEntry = _raft->log[(k) % MAX_LOG_LENGTH];
      _raft->messageBuffer[j++] = newEntry.term & 0xff;
      _raft->messageBuffer[j++] = (newEntry.term >> 8) & 0xff;
      _raft->messageBuffer[j++] = newEntry.type;
      for (int l = 0; l < sizeof(newEntry.data); l++){
        _raft->messageBuffer[j++] = newEntry.data[l];
      }
    }
    
//    Log("send ");
//    Log(newEntryCount);
//    Log(" entries to ");
//    Log(_raft->nodes[i][0]);
//    Log(".");
//    Log(_raft->nodes[i][1]);
//    Log(".");
//    Log(_raft->nodes[i][2]);
//    Log(".");
//    Log(_raft->nodes[i][3]);
//    Log("\n");

    _raft->messageBuffer[j++] = _raft->commitIndex & 0xff;
    _raft->messageBuffer[j++] = (_raft->commitIndex >> 8) & 0xff;

    _raft->udp->sendPacket(_raft->nodes[i], PORT, _raft->messageBuffer, j);
  }
}

void LeaderState::increaseCommitIndex() {
  short highestIndex = -1;
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
    if (consensusCount >= (int)(_raft->nodeCount() / 2) + 1 && _raft->log[(_raft->commitIndex + 1) % MAX_LOG_LENGTH].term == _raft->currentTerm){
      ++_raft->commitIndex;
    }
    else {
      return;
    }
  }
}
