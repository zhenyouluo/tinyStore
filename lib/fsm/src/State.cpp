#include "State.h"

void State::setFSM(FSM *fsm) {
  _fsm = fsm;
}

void State::_transition(const char *name){
  _fsm->transition(name);
}
