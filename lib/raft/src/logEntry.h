#pragma once

struct LogEntry {
  unsigned short term;
  unsigned char type;
  unsigned char data[8];
};
