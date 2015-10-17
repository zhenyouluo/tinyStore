#ifndef CANDIDATE_STATE_H
#define CANDIDATE_STATE_H

#include <State.h>

class CandidateState : public State {
public:
  virtual void setup();
  virtual void loop();
  virtual void cleanup();
};

#endif