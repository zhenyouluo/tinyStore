#ifndef FOLLOWER_STATE_H
#define FOLLOWER_STATE_H


#include <State.h>

class FollowerState : public State {
public:
  virtual void setup();
  virtual void loop();
  virtual void cleanup();
};

#endif