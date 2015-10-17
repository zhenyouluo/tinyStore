#include "Raft.h"

bool IPequals(unsigned char *ip1, unsigned char *ip2) {
  for (int i = 0; i < 4; i++){
    if (ip1[i] != ip2[i]){
      return false;
    }
  }
  return true;
}

void IPcopy(unsigned char *from, unsigned char *to){
  for (int i = 0; i < 4; i++){
    to[i] = from[i];
  }
}

bool IPisEmpty(unsigned char *ip){
  for (int i = 0; i < 4; i++){
    if (ip[i] != 0){
      return false;
    }
  }
  return true;
}

void IPempty(unsigned char *ip){
  for (int i = 0; i < 4; i++){
    ip[i] = 0;
  }
}
