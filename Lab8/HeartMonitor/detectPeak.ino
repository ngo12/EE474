#define THRESHOLD 1

int peakVal = 0;
int upFlag = 0;
int prevSignal = 0;
int detectPeak(int currentSignal) {
  // Checks if there is a peak
  if (upFlag && (currentSignal < prevSignal) && (prevSignal > THRESHOLD) && (currentSignal >> THRESHOLD)) {
    peakVal = prevSignal;
  }

  // Checks if there is a rising edge 
  if (currentSignal > prevSignal) {
    upFlag = 1; 
  } else {
    upFlag = 0;
  }
  prevSignal = currentSignal;       // Saves the previous value
  return peakVal;
}



void stabilize2() {
  tft.setCursor(0, 0);
  tft.println("Stabilizing...");

  int upperThreshold = UPPER_THRESH;
  int lowerThreshold = LOWER_THRESH;

  int buffLength = 20;
  int valCopy;
  int IntCopy;
  int peakVal;
  int prevValues[buffLength];
  int enoughDataFlag = 0;
  int bufferStableFlag = 0;
  int bufferIndex = 0;
  buttonState = onOff();
  while (!stabilizedFlag && buttonState != 1) {
    buttonState = onOff();
    if (buttonState) {
      stabilizedFlag = 1;
      progRunning = 1;
      startTimer = millis();
      break;
    }
    Serial.print("Stable Loop");
    noInterrupts();

    adcValueCopy = adcValue;
    Serial.print("STABBBLE: ");
    Serial.println(LPF());
    valCopy = LPF();
    IntCopy = movingAvgFilter();
    interrupts();
    
    peakVal = detectPeak(IntCopy);
    Serial.print("peak is: ");
    Serial.println(peakVal);
    prevValues[bufferIndex] =   adcValue; // should be the copy???
    bufferIndex++;
    if (bufferIndex > buffLength) {
      bufferIndex = 0;
      enoughDataFlag = 1;
    }
    if (enoughDataFlag) {
      bufferStableFlag = 1;
      for (int i = 0; i < buffLength; i++) {
        if (!bufferStableFlag) {
          break;
        }
        if (prevValues[i] > lowerThreshold && prevValues[i] < upperThreshold) {
          bufferStableFlag = 1;
        } else {
          bufferStableFlag = 0;
        }
      }
    }
    if (bufferStableFlag) {
      stabilizedFlag = 1;
      progRunning = 1;
      startTimer = millis();
    }

    clearTextNumbers("0000");
    clearTextNumbers("1111");
    clearTextNumbers("2222");
    clearTextNumbers("3333");
    clearTextNumbers("4444");
    clearTextNumbers("5555");
    clearTextNumbers("6666");
    clearTextNumbers("7777");
    clearTextNumbers("8888");
    clearTextNumbers("9999");
    tft.setCursor(0, 0);
    tft.println("Stabilizing...");
    tft.print("Upper Threshold: ");
    tft.println(UPPER_THRESH);
    tft.print("Lower Threshold: ");
    tft.println(LOWER_THRESH);
    tft.print("Current value: ");
    tft.println(valCopy);
    delay(50);
  }

}

