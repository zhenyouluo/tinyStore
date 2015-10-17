#include "FSM.h"

#ifndef ARDUINO
#include <string.h>
#endif


FSM::FSM(){
    _initialState = -1;
    _states = std::vector<State*>();
    _transitions = std::vector<Transition>();
    _current = 0;
    _next = 0;
}

void FSM::addState(State* state){
  _states.push_back(state);
  state->setFSM(this);
}

void FSM::addTransition(int from, int to, const char *name){
  Transition t(from, to, name);
  _transitions.push_back(t);
}

void FSM::setInitialState(int index){
  _initialState = index;
}

void FSM::transition(const char *name){
  for (int i = 0; i < _transitions.size(); i++){
    if (_transitions[i].from == i && strcmp(_transitions[i].name, name) == 0){
      _next = _states[_transitions[i].to];
      return;
    }
  }
}

void FSM::setup(){
  if (_initialState > -1){
    if (_initialState < _states.size()){
      _next = _states[_initialState];
    }
  }
  else if (_states.size() > 0) {
    _next = _states[0];
  }
}

void FSM::loop(){
  if (_next){
    if (_current){
      _current->cleanup();
    }
    _current = _next;
    _next = 0;
    _current->setup();
  }
  if (_current){
    _current->loop();
  }
}
