#pragma once

class FSM;

#include "FSM.h"

class State {
  FSM *_fsm;
protected:
  void _transition (const char *name);
public:
  void setFSM(FSM *fsm);
  virtual void setup() = 0;
  virtual void loop() = 0;
  virtual void cleanup() = 0;
};