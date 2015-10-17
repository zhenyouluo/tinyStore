#pragma once

struct LogEntry {
  unsigned int term;
  unsigned char type;
  unsigned char data[8];
};
