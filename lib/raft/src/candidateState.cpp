#include "candidateState.h"

CandidateState::CandidateState(RaftNode *raft){
  _raft = raft;
}

void CandidateState::setup() {
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    votes[i] = false;
  }
  unsigned long rand = (unsigned long) random(150, 300);
  timeout = millis() + rand;

  _raft->messageBuffer[0] = MESSAGE_TYPE_REQUEST_VOTE;
  _raft->messageBuffer[1] = (unsigned char) _raft->currentTerm;
  _raft->messageBuffer[2] = (unsigned char) (_raft->currentTerm >> 8);
  _raft->messageBuffer[3] = (unsigned char) _raft->commitIndex;
  _raft->messageBuffer[4] = (unsigned char) (_raft->commitIndex >> 8);
  if (_raft->commitIndex == 0){
    _raft->messageBuffer[5] = 0;
    _raft->messageBuffer[6] = 0;
  }
  else {
    LogEntry lastEntry = _raft->getLastLogEntry();
    _raft->messageBuffer[5] = (unsigned char) lastEntry.term;
    _raft->messageBuffer[6] = (unsigned char) (lastEntry.term >> 8);
  }

  _raft->udp->sendPacket((unsigned char*) BRAODCAST_ADDRESS, PORT, _raft->messageBuffer, 7);
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
}

void CandidateState::loop() {
  if (_raft->udp->parsePacket()){
    readPacket();
  }
  if (millis() > timeout){
    if (hasMajority()){
      _raft->transition("elected");
    }
    else {
      _raft->transition("startVote");
    }
  }
}

void CandidateState::cleanup() {

}

void CandidateState::readPacket() {
  if (!_raft->udp->read(_raft->messageBuffer, MESSAGE_BUFFER_SIZE)){
    return;
  }
  switch (_raft->messageBuffer[0]){
    case MESSAGE_TYPE_VOTE:
      parseVoteMessage();
      break;
  }
}

void CandidateState::parseVoteMessage(){
  unsigned int term = _raft->messageBuffer[1] | _raft->messageBuffer[2] << 8;
  if (term > _raft->currentTerm){
    _raft->currentTerm = term;
    _raft->transition("not elected");
    return;
  }
  unsigned char vote = _raft->messageBuffer[3];
  unsigned char remoteIP[4];
  _raft->udp->remoteIP(remoteIP);

  int remoteIndex = _raft->getNodeIndex(remoteIP);
  if (remoteIndex < 0){
    remoteIndex = _raft->addNode(remoteIP);
  }
  if (remoteIndex >= 0){
    if (vote){
      votes[remoteIndex] = true;
    }
    else {
      votes[remoteIndex] = false;
    }
  }
  if (hasMajority()){
    _raft->transition("elected");
  }
}

bool CandidateState::hasMajority(){
  int voteCount = 0;
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    if (votes[i]){
      ++voteCount;
    }
  }
  int majority = (int)(_raft->nodeCount() / 2) + 1;
  return voteCount >= majority;
}
