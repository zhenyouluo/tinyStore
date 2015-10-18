#pragma once

#include <State.h>
#include "Raft.h"

class LeaderState : public State {
  
  unsigned long timeout;
  unsigned long commandTimeout;
  unsigned short _nextIndex[MAX_NODE_COUNT];
  unsigned short _matchIndex[MAX_NODE_COUNT];
  
  void resetTimer();
  void readPacket();
  void sendAppendEntries();
  void parseAppendEntriesResponseMessage();
  void parseRequestVoteMessage();
  void sendNewEntries();
  void increaseCommitIndex();
  
protected:
  RaftNode *_raft;

public:
  LeaderState(RaftNode *raft);

  virtual void setup();
  virtual void loop();
  virtual void cleanup();
};
