#include "raftNode.h"

RaftNode::RaftNode() {
    _candidate = CandidateState();
    _follower = FollowerState();
    _leader = LeaderState();

    _stateCount = 3;
    _states = new State*[_stateCount]{
      &_candidate,
      &_follower,
      &_leader
    };

    setInitialState(0);
}
