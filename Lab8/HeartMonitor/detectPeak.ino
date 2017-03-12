//#define THRESHOLD 200
//
//double peakVal = 0;
//int upFlag = 0;
//double prevSignal = 0;
//double detectPeak(double currentSignal) {
//  // Checks if there is a peak
//  if (upFlag && (currentSignal < prevSignal) && (prevSignal > THRESHOLD) && (currentSignal > THRESHOLD)) {
//    peakVal = prevSignal;
//  }
//  // Checks if there is a rising edge 
//  if (currentSignal > prevSignal) {
//    upFlag = 1; 
//  } else {
//    upFlag = 0;
//  }
//  Serial.print("in signal: ");
//  Serial.print(currentSignal);
//  prevSignal = currentSignal;       // Saves the previous value
//  return peakVal;
//}
//
//void stabilize2() {
//  tft.setCursor(0, 0);
//  tft.println("Stabilizing...");
//
//  int upperThreshold = UPPER_THRESH;
//  int lowerThreshold = LOWER_THRESH;
//
//  int buffLength = 20;
//  int valCopy;
//  int IntCopy = 0;
//  int peakVal;
//  int prevValues[buffLength];
//  int enoughDataFlag = 0;
//  int bufferStableFlag = 0;
//  int bufferIndex = 0;
//  buttonState = onOff();
//  while (!stabilizedFlag && buttonState != 1) {
//    buttonState = onOff();
//    if (buttonState) {
//      stabilizedFlag = 1;
//      progRunning = 1;
//      startTimer = millis();
//      break;
//    }
//    Serial.print("Stable Loop");
//    noInterrupts();
//
//    adcValueCopy = adcValue;
//    Serial.print("STABBBLE: ");
//    Serial.println(LPF());
//    valCopy = LPF();
//    IntCopy = sqrWind = lowPassExponential(0.8, sqrWind);
//    interrupts();
//    
//    peakVal = detectPeak(IntCopy);
//    Serial.print("peak is: ");
//    Serial.println(peakVal);
//    prevValues[bufferIndex] =   adcValue; // should be the copy???
//    bufferIndex++;
//    if (bufferIndex > buffLength) {
//      bufferIndex = 0;
//      enoughDataFlag = 1;
//    }
//    if (enoughDataFlag) {
//      bufferStableFlag = 1;
//      for (int i = 0; i < buffLength; i++) {
//        if (!bufferStableFlag) {
//          break;
//        }
//        if (prevValues[i] > lowerThreshold && prevValues[i] < upperThreshold) {
//          bufferStableFlag = 1;
//        } else {
//          bufferStableFlag = 0;
//        }
//      }
//    }
//    if (bufferStableFlag) {
//      stabilizedFlag = 1;
//      progRunning = 1;
//      startTimer = millis();
//    }
//
//    clearTextNumbers("0000");
//    clearTextNumbers("1111");
//    clearTextNumbers("2222");
//    clearTextNumbers("3333");
//    clearTextNumbers("4444");
//    clearTextNumbers("5555");
//    clearTextNumbers("6666");
//    clearTextNumbers("7777");
//    clearTextNumbers("8888");
//    clearTextNumbers("9999");
//    tft.setCursor(0, 0);
//    tft.println("Stabilizing...");
//    tft.print("Upper Threshold: ");
//    tft.println(UPPER_THRESH);
//    tft.print("Lower Threshold: ");
//    tft.println(LOWER_THRESH);
//    tft.print("Current value: ");
//    tft.println(valCopy);
//    delay(50);
//  }
//
//}

double threshStart = 100;
double otherThresh = threshStart;
int countedBeats = 0;
double localThresh = 0;
  double filtSignal = 0;
int threshNotFound = 0;
void stabilize2() {
  tft.setCursor(0, 0);
  tft.println("Stabilizing...");

  int upperThreshold = UPPER_THRESH;
  int lowerThreshold = LOWER_THRESH;

  otherThresh = threshStart;
  int buffLength = 20;
  int IntCopy = 0;
  int peakVal;
  int prevValues[buffLength];
  int enoughDataFlag = 0;
  int bufferStableFlag = 0;
  int bufferIndex = 0;
  int foundHeart = 0;
  countedBeats = 0;

  double localMax = 0;
  int localMaxCounter = 0;
  int localMaxRange = 5;
  buttonState = onOff();
  unsigned long timeCounter = 0;
  while (!stabilizedFlag && buttonState != 1) {
    buttonState = onOff();
    if (buttonState) {
      stabilizedFlag = 1;
      progRunning = 1;
      startTimer = millis();
      break;
    }
      adcValueCopy = adcValue;
      filtSignal = lowPassExponential(0.8, filtSignal);
//      Serial.print("FS1: ");
//      Serial.println(filtSignal);
      checkHeart();
      if (countedBeats > 5) {
        adaptiveThresh = localThresh;
        stabilizedFlag = 1;
        progRunning = 1;
        tft.fillScreen(BG_COLOR);
        tft.setCursor(0, 0);
        tft.println("Calibrated!!!");
        tft.print("Counted Beats: ");
        tft.println(countedBeats);
        tft.print("Threshold Level: ");
        tft.println(localThresh);
        delay(1000);
        startTimer = millis();
        
      }
      
      if (timeCounter % 50 == 0) {
//        clearTextNumbers();
        tft.fillScreen(BG_COLOR);
        tft.setCursor(0, 0);
        tft.println("Stabilizing...");
        tft.print("Counted Beats: ");
        tft.println(countedBeats);
        tft.print("Threshold Level: ");
        tft.println(localThresh);
        timeCounter++;
      }

  }

}


void checkHeart() {
  //Serial.println(threshNotFound);
  if (threshNotFound > 50000) {
    threshNotFound = 0;
    otherThresh = threshStart;
  }
  if (filtSignal > otherThresh) {
    threshNotFound = 0;
    PeriodOfRR = (millis() - heartRateTimer);
    if (PeriodOfRR > QRS_MAX_TIME) {
      double period = PeriodOfRR / 1000;
      heartRate =  60 / period;
      heartRateTimer = millis();
            Serial.print("FS2: ");
      Serial.println(filtSignal);
      Serial.print("HR: ");
      Serial.println(heartRate);
      Serial.print("CB: ");
      Serial.println(countedBeats);
      Serial.print("OT: ");
      Serial.println(otherThresh);
      Serial.println();

      if (heartRate < 15 || heartRate > 160) {
        countedBeats = 0;
        otherThresh = 0.85 * filtSignal;
      } else {
        if (0.04 * filtSignal < 40) {
          localThresh = 0.025 * filtSignal;
        } else {
          localThresh = 0.04 * filtSignal;
        }
        countedBeats++;
      }
    }
  } else {
    threshNotFound++;
  }
}

void clearTextNumbers() {
  tft.drawRect(10, screenWidth - 50, 100, 100, BG_COLOR);
}



