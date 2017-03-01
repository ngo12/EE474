#ifndef HEARTMONITOR_H
#define HEARTMONITOR_H

#define BUFFER_SIZE 7500
extern volatile int bufferPos;
extern volatile int stabilizedFlag;

int myBuffer[BUFFER_SIZE];
void addToBuffer(int val);
#endif
