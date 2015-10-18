#pragma once
#ifndef ARDUINO

#include <chrono>

unsigned long millis();
int random(int min, int max);
void randomSeed(unsigned int seed);

#else
#include <Arduino.h>
#endif
