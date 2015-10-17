#ifndef LEADER_STATE_H
#define LEADER_STATE_H

#include <State.h>
#include "Raft.h"

class LeaderState : public State {
protected:
  RaftNode *_raft;

public:
  LeaderState(RaftNode *raft);

  virtual void setup();
  virtual void loop();
  virtual void cleanup();
};

#endif
