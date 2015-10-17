#ifndef LEADER_STATE_H
#define LEADER_STATE_H

#include <State.h>

class LeaderState : public State {
public:
  virtual void setup();
  virtual void loop();
  virtual void cleanup();
};

#endif