#include "raftNode.h"

RaftNode::RaftNode(AbstractUdpProvider *udp, int seed) {
  this->udp = udp;
  _candidate = new CandidateState(this);
  _follower = new FollowerState(this);
  _leader = new LeaderState(this);

  _stateCount = 3;
  _states = new State*[3]{
    _follower,
    _candidate,
    _leader
  };

  _transitionCount = 5;
  _transitions = new Transition[5]{
    Transition(0, 1, "candidate"),
    Transition(1, 2, "elected"),
    Transition(1, 1, "timedOut"),
    Transition(1, 0, "not elected"),
    Transition(2, 0, "higherTerm")
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
    for (int j = 0; j  < 4; j++){
      if (IPequals(ip, nodes[i])){
        for (int k = 0; k  < 4; k++){
          nodes[i][k] = 0;
        }
        return;
      }
    }
  }
}
