/*
  detectPeak: Used for calibration during the 
  ECG program before beginning measurement. 
  
  Lab 8
  Course: EE 474

  Names and Student Numbers:
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375
*/

#include "font_AwesomeF200.h"
#include "font_AwesomeF100.h"


double filtSignal = 0;           // filter signal, used to find peaks

int countedBeats = 0;            // consecutive heart beats found
const int maxBeats = 5;          // once we reach this many consecutive beats, move to main prog

double threshStart = 700;        // start calibration threshold here
double calThresh = threshStart;  // compare current signal to this
double localThresh = 0;          // threshold that will eventually become global for R peak detection
int threshNotFound = 0;          // keep track or times we don't meet thresh, if too many something is wrong

unsigned long calTimer = 0;   // only stay in calibration mode until we reach the max alloted time
const unsigned long calTimerMax = 1000 * 20; // 1000 * seconds

// Calibrate our incoming signal before displaying ECG
// Do this by analyzing incoming signal, adapting threshold to find where we get a recognizable heartbeat
// Once we see a number of consecutive heart beats, set the global threshold for R peak based on this value
// If no consistent peak is found within the max alloted time, use a default threshold value that works for most
void stabilize2() {

  // get init vals
  calThresh = threshStart;
  countedBeats = 0;
  buttonState = onOff();

  // setup display for calibrating mode
  displayCalibrating();
  delay(1500);
  tft.setCursor(136, 210);
  
  calTimer = millis();
  while (!stabilizedFlag && buttonState != 1) {
    
    // button press can manually move to main program   
    buttonState = onOff();
    if (buttonState) {
      stabilizedFlag = 1;
      progRunning = 1;
      adaptiveThresh = DEF_THRESH;
      displayCalibrated();
      delay(1000);
      mainProgSetup();
      break;
    }

    // get current signal and filter it
    adcValueCopy = adcValue;
    filtSignal = lowPassExponential(0.8, filtSignal);
    // check for consistent heartbeats and display progress to LCD
    checkHeart();
    updateCalibrating(); 

    // if calibration takes too long, move on with default threshold
    if ((millis() - calTimer) >= calTimerMax) {
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
      mainProgSetup();
      break;
    }

    // if enough consecutive beats found, move to main prog
    if (countedBeats > maxBeats) {
      adaptiveThresh = localThresh / (maxBeats+1);
      stabilizedFlag = 1;
      progRunning = 1;
      displayCalibrated();
      delay(1000);
      mainProgSetup();
    }
  }
}

// Checks heart rate to see if it is a viable number, uses adaptive thresholds to search for it
double lastHeartRate = 0;
void checkHeart() {
  if (threshNotFound > 15000) {
    threshNotFound = 0;
    calThresh = threshStart;
  }
  if (filtSignal > calThresh) {
    threshNotFound = 0;
    PeriodOfRR = (millis() - heartRateTimer);
    if (PeriodOfRR > QRS_MAX_TIME) {
      double period = PeriodOfRR / 1000;
      heartRate =  60 / period;
      heartRateTimer = millis();
      
      if (heartRate > 160) {
        countedBeats = 0;
        localThresh = 0;
        calThresh = 0.95 * filtSignal;
      } else if (heartRate < 15) {
        countedBeats = 0;
        localThresh = 0;
        calThresh = 0.75 * filtSignal;
//      } else if (lastHeartRate != 0 && abs(heartRate - lastHeartRate) > 40) {
//        countedBeats = 0;
//        localThresh = 0;
//        lastHeartRate = 0;
//        calThresh = 0.8 * filtSignal;
      } else {
        calThresh = 0.8 * filtSignal;
        if (0.04 * filtSignal < 40) {
          localThresh = localThresh + 0.020 * filtSignal;
        } else {
          localThresh = localThresh + 0.025 * filtSignal;
        }
        lastHeartRate = heartRate;
        countedBeats++;
      }
    }
  } else {
    threshNotFound++;
  }
}



