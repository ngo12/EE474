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

#include "font_AwesomeF200.h"
#include "font_AwesomeF100.h"

double threshStart = 100;
double otherThresh = threshStart;
int countedBeats = 0;
double localThresh = 0;
  double filtSignal = 0;
int threshNotFound = 0;

unsigned long stableTimer = 0;
const unsigned long stableTimerMax = 1000 * 20; // 1000 * seconds
void stabilize2() {

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
  int maxBeats = 5;
  double localMax = 0;
  int localMaxCounter = 0;
  int localMaxRange = 5;
  buttonState = onOff();
  unsigned long timeCounter = 0;

  displayCalibrating();
  tft.setCursor(136, 210);
  stableTimer = millis();
  while (!stabilizedFlag && buttonState != 1) {
//     Serial.println("While Not Stable Loop");
    buttonState = onOff();
    if (buttonState) {
      stabilizedFlag = 1;
      progRunning = 1;
      adaptiveThresh = DEF_THRESH; // DEFAULT VAL
      displayCalibrated();
      delay(1000);
      tft.fillScreen(BG_COLOR);
      drawGrid();
      textResultsBorder();
      textResults(-1, -1);
      startTimer = millis();
      break;
    }


    
      adcValueCopy = adcValue;
      filtSignal = lowPassExponential(0.8, filtSignal);
//      Serial.print("FS1: ");
//      Serial.println(filtSignal);
      checkHeart();
      updateCalibrating();

    if ((millis() - stableTimer) >= stableTimerMax) {
      tft.setTextColor(BG_COLOR);
      tft.setCursor(136, 210);
      tft.print("......");
      tft.setTextColor(TEXT_COLOR);
      tft.setCursor(136, 210);
      tft.print("!!!!!!");
      delay(400);
      adaptiveThresh = DEF_THRESH; // DEFAULT VAL
      stabilizedFlag = 1;
      progRunning = 1;
       
      displayCalibrated();
      delay(1000);
      tft.fillScreen(BG_COLOR);
      drawGrid();
      textResultsBorder();
      textResults(-1, -1);
      startTimer = millis();
      break;
    }
      
      if (countedBeats > maxBeats) {
        adaptiveThresh = localThresh / (maxBeats+1);
        stabilizedFlag = 1;
        progRunning = 1;
       
        displayCalibrated();
        delay(1000);
        tft.fillScreen(BG_COLOR);
        drawGrid();
        textResultsBorder();
        textResults(-1, -1);
        startTimer = millis();
        
      }

  }
    Serial.println(adaptiveThresh);
    Serial.println(localThresh / (maxBeats+1));
    Serial.println(adaptiveThresh);
    Serial.println(localThresh / (maxBeats+1));

}


#define MAX_THRESH_DIFF 300
double lastHeartRate = 0;
void checkHeart() {
//  Serial.println(threshNotFound);
  if (threshNotFound > 15000) {
    threshNotFound = 0;
    otherThresh = threshStart;
  }
//  if ( (abs(filtSignal - otherThresh) < MAX_THRESH_DIFF) && (filtSignal > 500) ) {
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

      if (heartRate > 160) {
        countedBeats = 0;
        localThresh = 0;
        otherThresh = 0.95 * filtSignal;
      } else if (heartRate < 15) {
        countedBeats = 0;
        localThresh = 0;
        otherThresh = 0.75 * filtSignal;
//      } else if (lastHeartRate != 0 && abs(heartRate - lastHeartRate) > 40) {
//        countedBeats = 0;
//        localThresh = 0;
//        lastHeartRate = 0;
//        otherThresh = 0.8 * filtSignal;
      } else {
        otherThresh = 0.8 * filtSignal;
        if (0.04 * filtSignal < 40) {
          localThresh = localThresh + 0.012 * filtSignal;
        } else {
          localThresh = localThresh + 0.028 * filtSignal;
        }
        lastHeartRate = heartRate;
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

int lastCountedBeats = 0;
void displayCalibrating() {
  tft.setFont(RussoOne_28);
  tft.setCursor(30, 10);
  tft.setTextColor(TEXT_COLOR);
  tft.println("Calibrating...");
  tft.setFont(DEF_FONT);
  tft.setCursor(50, 50);
  tft.println("Please hold still...");
  
  tft.setCursor(118, 80);
  tft.setFont(AwesomeF200_60);
  tft.setTextColor(ILI9341_RED);
   tft.print((char)84); // print timer

  tft.setTextColor(TEXT_COLOR);
  tft.setFont(DEF_FONT);
}

void updateCalibrating() {

  if (countedBeats  < lastCountedBeats) {
    tft.setTextColor(BG_COLOR);
    tft.setCursor(136, 210);
    tft.print("......");
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(136, 210);
    
  } else if (countedBeats > lastCountedBeats) {
//    for (int i = 0;  i < countedBeats; i++) {
      tft.print("."); // print hearts
//    }    
  }
  lastCountedBeats = countedBeats;
}

void displayCalibrated() {
  tft.fillScreen(BG_COLOR);
  tft.setFont(RussoOne_28);
  tft.setCursor(30, 10);
  tft.setTextColor(TEXT_COLOR);
  tft.println("Calibrated!!!");
  tft.setFont(AwesomeF100_60);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(126, 65);
  tft.print((char)74); // from 0 to 127 
    tft.setTextColor(TEXT_COLOR);
  tft.setFont(DEF_FONT);
}


