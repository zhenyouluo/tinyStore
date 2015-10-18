#ifndef CANDIDATE_STATE_H
#define CANDIDATE_STATE_H

#include <State.h>
#include "Raft.h"

class CandidateState : public State {
protected:
  RaftNode *_raft;

  unsigned long timeout;
  short votes[MAX_NODE_COUNT];

  void readPacket();
  void parseVoteMessage();
  void parseAppendEntriesMessage();
  void sendVoteRequestMessage();
  void removeNonVoters();
  bool hasMajority();

public:
  CandidateState(RaftNode *raft);

  virtual void setup();
  virtual void loop();
  virtual void cleanup();
};

#endif
