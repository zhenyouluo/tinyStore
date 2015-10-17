#include "raftNode.h"

RaftNode::RaftNode(AbstractUdpProvider *udp, int seed) {
  this->udp = udp;
  _notConnected = new NotConnectedState();
  _candidate = new CandidateState(this);
  _follower = new FollowerState(this);
  _leader = new LeaderState(this);

  _stateCount = 4;
  _states = new State*[4]{
    _notConnected,
    _follower,
    _candidate,
    _leader
  };

  _transitionCount = 10;
  _transitions = new Transition[10]{
    Transition(0, 1, "connected"),
    Transition(1, 0, "connectionLost"),
    Transition(2, 0, "connectionLost"),
    Transition(3, 0, "connectionLost"),
    Transition(1, 2, "startVote"),
    Transition(2, 2, "startVote"),
    Transition(2, 3, "elected"),
    Transition(2, 2, "timedOut"),
    Transition(2, 1, "not elected"),
    Transition(3, 1, "higherTerm")
  };

  randomSeed(seed);

  setInitialState(0);
}

int RaftNode::nodeCount() {
  int result = 0;
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    bool allNull = true;
    for (int j = 0; j  < 4; j++){
      if (nodes[i][j] != 0){
        allNull = false;
      }
    }
    if (!allNull){
      ++result;
    }
  }
  return result;
}

bool RaftNode::addNode(unsigned char *ip) {
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    bool allNull = true;
    for (int j = 0; j  < 4; j++){
      if (nodes[i][j] != 0){
        allNull = false;
      }
    }
    if (allNull){
      for (int j = 0; j  < 4; j++){
        nodes[i][j] = ip[j];
      }
      return true;
    }
  }
  return false;
}

void RaftNode::removeNode(unsigned char *ip) {
  for (int i = 0; i < MAX_NODE_COUNT; i++){
    if (IPequals(ip, nodes[i])){
      for (int k = 0; k  < 4; k++){
        nodes[i][k] = 0;
      }
      return;
    }
  }
}

int RaftNode::getNodeIndex(unsigned char *ip) {
    for (int i = 0; i < MAX_NODE_COUNT; i++){
      if (IPequals(ip, nodes[i])) {
        return i;
      }
    }
    return -1;
}

LogEntry RaftNode::getLogEntry(unsigned int index) {
  return log[index % MAX_LOG_LENGTH];
}

LogEntry RaftNode::getLastLogEntry() {
  return getLogEntry(commitIndex % MAX_LOG_LENGTH);
}
