#ifndef RAFT_NODE_H
#define RAFT_NODE_H

#define MAX_LOG_LENGTH 512
#define MAX_NODE_COUNT 16
#define MESSAGE_BUFFER_SIZE 512

#include <FSM.h>
#include <udpProvider.h>

#include "Raft.h"
#include "candidateState.h"
#include "followerState.h"
#include "leaderState.h"
#include "notConnectedState.h"
#include "logEntry.h"

class RaftNode : public FSM {
  NotConnectedState *_notConnected;
  CandidateState *_candidate;
  FollowerState *_follower;
  LeaderState *_leader;

public:
  LogEntry log[MAX_LOG_LENGTH];
  unsigned char nodes[MAX_NODE_COUNT][4];
  AbstractUdpProvider *udp;
  unsigned short currentTerm;
  unsigned char votedFor[4];
  unsigned char currentLeader[4];
  short commitIndex = -1;
  short lastApplied = -1;
  short lastEntryIndex = -1;
  unsigned char messageBuffer[MESSAGE_BUFFER_SIZE];

  RaftNode(AbstractUdpProvider *udp, int seed);
  int nodeCount();
  int addNode(unsigned char *ip);
  void removeNode(unsigned char *ip);
  void removeNodeAt(int index);
  int getNodeIndex(unsigned char *ip);
  LogEntry getLogEntry(unsigned int index);
  LogEntry getLastLogEntry();
};

#endif
