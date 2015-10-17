#include "Raft.h"

bool IPequals(unsigned char *ip1, unsigned char *ip2) {
  for (int i = 0; i < 4; i++){
    if (ip1[i] != ip2[i]){
      return false;
    }
  }
  return true;
}
