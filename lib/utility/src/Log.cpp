#include "Log.h"

#ifndef ARDUINO
#include <iostream>
#else
#include <Arduino.h>
#endif

void Log(const char *data) {
#ifdef ARDUINO
    Serial.print(data);
#else
  printf("%s", data);
#endif
}

void Log(char *data) {
#ifdef ARDUINO
  Serial.print(data);
#else
  printf("%s", data);
#endif

}

void Log(int data) {
#ifdef ARDUINO
    Serial.print(data);
#else
    printf("%i", data);
#endif

}
