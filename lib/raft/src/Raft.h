#pragma once

const unsigned char MESSAGE_TYPE_APPEND_ENTRIES = 0x01;
const unsigned char MESSAGE_TYPE_APPEND_ENTRIES_RESPONSE = 0x02;
const unsigned char MESSAGE_TYPE_REQUEST_VOTE = 0xfe;
const unsigned char MESSAGE_TYPE_VOTE = 0xff;

bool IPequals(unsigned char *ip1, unsigned char *ip2);

class RaftNode;
class CandidateState;
class FollowerState;
class LeaderState;

#include "raftNode.h"
#include "candidateState.h"
#include "followerState.h"
#include "leaderState.h"
#include "logEntry.h"
