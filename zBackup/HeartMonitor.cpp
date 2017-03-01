#include "HeartMonitor.h"
volatile int bufferPos = 0;
volatile int stabilizedFlag = 0;
void addToBuffer(int val) {  
  if (stabilizedFlag) {
      myBuffer[bufferPos] = val;
      bufferPos++;
      if (bufferPos >= BUFFER_SIZE) {
       bufferPos = 0;
      }
  }
}
