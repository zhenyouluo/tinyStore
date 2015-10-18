#ifndef ARDUINO

#include "ArduinoHelper.h"
#include <cstdlib>

std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

unsigned long millis(){
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
}

int random (int min, int max) {
    return rand() % (max - min - 1) + min;
}

void randomSeed(unsigned int seed) {
    srand(seed);
}

#endif

