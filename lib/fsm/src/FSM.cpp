#include "FSM.h"

#if ARDUINO
#include <Arduino.h>
#endif
#include <Log.h>

FSM::FSM(State** states, int stateCount, Transition* transitions, int transitionCount){
    _initialState = -1;
    _states = states;
    _stateCount = stateCount;
    _transitions = transitions;
    _transitionCount = transitionCount;
    _current = -1;
    _next = -1;
}

FSM::FSM (){
  _initialState = -1;
  _stateCount = 0;
  _transitionCount = 0;
  _current = -1;
  _next = -1;
}

void FSM::setStates(State** states, int stateCount) {
  _states = states;
  _stateCount = stateCount;
}

void FSM::setTransitions(Transition* transitions, int transitionCount) {
  _transitions = transitions;
  _transitionCount = transitionCount;
}

void FSM::setInitialState(int index){
  _initialState = index;
}

void FSM::transition(const char *name){
  for (int i = 0; i < _transitionCount; i++){
    if (_transitions[i].from == _current && strcmp(_transitions[i].name, name) == 0){
      _next = _transitions[i].to;
      Log("transition from ");
      Log(_transitions[i].from);
      Log(" to ");
      Log(_transitions[i].to);
      Log(" via ");
      Log(name);
      Log("\n");
      return;
    }
  }
}

void FSM::setup(){
  if (_initialState > -1){
    if (_initialState < _stateCount) {
      _next = _initialState;
    }
  }
  else if (_stateCount > 0) {
    _next = 0;
  }
}

void FSM::loop(){
  if (_next >= 0 ){
    if (_current >= 0){
      _states[_current]->cleanup();
    }
    _current = _next;
    _next = -1;
    _states[_current]->setup();
  }
  if (_current >= 0){
    _states[_current]->loop();
  }
}
