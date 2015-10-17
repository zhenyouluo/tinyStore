#ifndef FSM_H
#define FSM_H

class State;

#include <vector>

#include "State.h"
#include "Transition.h"

class FSM {
  std::vector<State*> _states;
  std::vector<Transition> _transitions;
  State *_current;
  State *_next;
  int _initialState;

public:
  FSM();

  void addState(State* state);
  void addTransition(int from, int to, const char *name);

  void setInitialState(int index);
  void transition(const char *name);

  void setup();
  void loop();
};


#endif
