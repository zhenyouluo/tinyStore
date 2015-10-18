#include "candidateState.h"

#include <ArduinoHelper.h>
#include <Log.h>

CandidateState::CandidateState(RaftNode *raft){
  _raft = raft;
}

void CandidateState::setup() {
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    votes[i] = -1;
  }
  unsigned long rand = (unsigned long) random(150, 300);
  timeout = millis() + rand;
  ++_raft->currentTerm;
  sendVoteRequestMessage();
  
  unsigned char localIP[4];
  _raft->udp->localIP(localIP);
  IPcopy(localIP,_raft->votedFor);
  int ownIndex = _raft->getNodeIndex(localIP);
  if (ownIndex < 0){
    ownIndex = _raft->addNode(localIP);
  }
  if (ownIndex >= 0){
    votes[ownIndex] = true;
  }
  Log(_raft->currentTerm);
  Log("\n");
}

void CandidateState::loop() {
  if (_raft->udp->parsePacket()){
    readPacket();
  }
  if (millis() > timeout){
    removeNonVoters();
    if (hasMajority()){
      _raft->transition("elected");
    }
    else {
      _raft->transition("startVote");
    }
  }
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

void CandidateState::cleanup() {
  Log(_raft->currentTerm);
  Log("\n");
}

void CandidateState::readPacket() {
  if (!_raft->udp->read(_raft->messageBuffer, MESSAGE_BUFFER_SIZE)){
    return;
  }
  switch (_raft->messageBuffer[0]){
    case MESSAGE_TYPE_VOTE:
      parseVoteMessage();
      break;
    case MESSAGE_TYPE_APPEND_ENTRIES:
      parseAppendEntriesMessage();
      break;
  }
}

void CandidateState::parseVoteMessage(){
  int i = 1;
  unsigned short term = _raft->messageBuffer[i++];
  term |= _raft->messageBuffer[i++] << 8;
  if (term > _raft->currentTerm){
    _raft->currentTerm = term;
    _raft->transition("notElected");
    return;
  }
  unsigned char vote = _raft->messageBuffer[i++];
  unsigned char remoteIP[4];
  _raft->udp->remoteIP(remoteIP);

  int remoteIndex = _raft->getNodeIndex(remoteIP);
  if (remoteIndex < 0){
    remoteIndex = _raft->addNode(remoteIP);
  }
  if (remoteIndex >= 0){
    if (vote){
      votes[remoteIndex] = 0x01;
    }
    else {
      votes[remoteIndex] = 0x00;
    }
  }
  if (hasMajority()){
    _raft->transition("elected");
  }
}

void CandidateState::parseAppendEntriesMessage() {
  
  int i = 1;
  unsigned short term = _raft->messageBuffer[i++];
  term |= _raft->messageBuffer[i++] << 8;
  unsigned char remoteIP[4];
  _raft->udp->remoteIP(remoteIP);
  if (_raft->getNodeIndex(remoteIP) < 0){
    _raft->addNode(remoteIP);
  }
  if (term > _raft->currentTerm) {
    _raft->currentTerm = term;
    IPcopy(remoteIP, _raft->currentLeader);
    _raft->transition("notElecetd");
  }
  i = 0;
  _raft->messageBuffer[i++] = MESSAGE_TYPE_APPEND_ENTRIES_RESPONSE;
  _raft->messageBuffer[i++] = _raft->currentTerm & 0xff;
  _raft->messageBuffer[i++] = (_raft->currentTerm >> 8) & 0xff;
  _raft->udp->sendPacket(remoteIP, PORT, _raft->messageBuffer, i);
}

void CandidateState::sendVoteRequestMessage() {
  int i = 0;
  _raft->messageBuffer[i++] = MESSAGE_TYPE_REQUEST_VOTE;
  _raft->messageBuffer[i++] = _raft->currentTerm & 0xff;
  _raft->messageBuffer[i++] = (_raft->currentTerm >> 8) & 0xff;
  _raft->messageBuffer[i++] = _raft->commitIndex & 0xff;
  _raft->messageBuffer[i++] = (_raft->commitIndex >> 8) & 0xff;
  if (_raft->commitIndex == 0){
    _raft->messageBuffer[i++] = 0;
    _raft->messageBuffer[i++] = 0;
  }
  else {
    LogEntry lastEntry = _raft->getLastLogEntry();
    _raft->messageBuffer[i++] = lastEntry.term & 0xff;
    _raft->messageBuffer[i++] = (lastEntry.term >> 8) & 0xff;
  }
  
  _raft->udp->sendPacket((unsigned char*) BRAODCAST_ADDRESS, PORT, _raft->messageBuffer, i);
}

void CandidateState::removeNonVoters() {
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    if (votes[i] > 0){
      _raft->removeNodeAt(i);
    }
  }

}

bool CandidateState::hasMajority(){
  int voteCount = 0;
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    if (votes[i] > 0){
      ++voteCount;
    }
  }
  int majority = (int)(_raft->nodeCount() / 2) + 1;
  return voteCount >= majority;
}
