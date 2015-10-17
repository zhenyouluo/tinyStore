#pragma once

#include <State.h>

class NotConnectedState : public State {
public:
  void setup() {}
  void loop() {}
  void cleanup() {}
};
