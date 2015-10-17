#ifndef RAFT_NODE_H
#define RAFT_NODE_H

#include <FSM.h>

#include "candidateState.h"
#include "followerState.h"
#include "leaderState.h"

class RaftNode : public FSM {
    CandidateState _candidate;
    FollowerState _follower;
    LeaderState _leader;
public:
    RaftNode();
};

#endif
