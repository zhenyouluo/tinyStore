#ifndef FOLLOWER_STATE_H
#define FOLLOWER_STATE_H

#include <State.h>
#include "Raft.h"

class FollowerState : public State {
  void readPacket();
protected:
  RaftNode *_raft;

  unsigned long timeout;
  unsigned char leader[4];

public:
  FollowerState(RaftNode *raft);

  virtual void setup();
  virtual void loop();
  virtual void cleanup();
};

#endif
