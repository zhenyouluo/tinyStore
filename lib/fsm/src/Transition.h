#ifndef TRANSITION_H
#define TRANSITION_H

struct Transition {
public:
  int from;
  int to;
  const char *name;
  Transition (int from, int to, const char *name) : from(from), to(to), name(name){}
  Transition() : from (-1), to(-1), name(""){}
};

#endif
