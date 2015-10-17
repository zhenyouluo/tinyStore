#ifndef FSM_H
#define FSM_H

class State;

#include <vector>

#include "State.h"
#include "Transition.h"

class FSM {

  State *_current;
  State *_next;
  int _initialState;
  
protected:
  State **_states;
  int _stateCount;
  Transition *_transitions;
  int _transitionCount;

public:
  FSM(State** states, int stateCount, Transition* transitions, int transitionCount);
  FSM();

  void setStates(State** states, int stateCount);
  void setTransitions(Transition* transitions, int transitionCount);

  void setInitialState(int index);
  void transition(const char *name);

  void setup();
  void loop();
};


#endif
