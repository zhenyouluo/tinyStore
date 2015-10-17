#include "raftNode.h"

RaftNode::RaftNode() {
    _candidate = CandidateState();
    _follower = FollowerState();
    _leader = LeaderState();
    
    addState(&_follower);
    addState(&_candidate);
    addState(&_leader);
    setInitialState(0);
}