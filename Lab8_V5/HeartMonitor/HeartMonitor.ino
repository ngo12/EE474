/*
  HeartMonitor: The main sketch for the 
  ECG. Contains the main loop that drives the 
  ECG Program. 
  
  Lab 8
  Course: EE 474

  Names and Student Numbers:
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375

*/
#include <stdint.h>
#include <SPI.h>
#include <SD.h>
#include <kinetis.h>
#include "ILI9341_t3.h"
#include "ADC.h"
#include <IntervalTimer.h>
#include "BluefruitConfig.h"
#include "font_RussoOne-Regular.h"
#include "font_AwesomeF100.h"

#define DEF_FONT RussoOne_18
#define BG_COLOR ILI9341_WHITE
#define GRID_COLOR ILI9341_RED
#define LINE_COLOR ILI9341_BLACK
#define LINE_COLOR2 ILI9341_BLUE
#define TEXT_COLOR ILI9341_BLACK
#define TEXT_HI ILI9341_GREEN
// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

#define PDB_CH0C1_TOS 0x0100
#define PDB_CH0C1_EN 0x01

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

#define BUFFER_SIZE 7500
#define STABILITY_THRESHOLD 0.05
#define INPUT_PIN A1

#define UPPER_THRESH 2500
#define LOWER_THRESH 1000
#define PROG_MAX_TIME 30000 //ms
#define QRS_MAX_TIME 300  //ms
#define AVERAGING 1 // num can be 1, 4, 8, 16 or 32.
#define MIN_COUNT_OF_REPEAT 10
#define DEF_THRESH 30

const int buttonPin =  1;     // the number of the pushbutton pin.

// Variables will change:
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
int startEKG = 1;                  // flag for whether the cube should rotate
double heartRate = 0;
int countBrad = 0;
int countTach = 0;
int bradycardia = 0; 
int tachycardia = 0;
int pac = 0;

double RR[8] = {0};  // holds current and prev RR interval values
double adaptiveThresh = 0;

unsigned long myTimer = 0;
unsigned long startTimer = 0;
unsigned long timeStamp = 0;
double heartRateTimer = 0;
double QRS_timer = 0;
double PeriodOfRR = 0;
const int adcTimer = 4000 / AVERAGING; // us
int screenWidth = 320;
int screenHeight = 240;
// grid line amounts
int numLinesWidth = 20;
int numLinesHeight = 15;
IntervalTimer myADCTimer;

// For ECG program 
volatile int bufferPos = 0;
volatile int stabilizedFlag = 0;
int myBuffer[BUFFER_SIZE];
volatile int adcValue;

// For setting PDB registers  
static const uint8_t channel2sc1a[] = {
  5, 14, 8, 9, 13, 12, 6, 7, 15, 4,
  0, 19, 3, 21, 26, 22
};

void setup() {

  Serial.begin(9600);
  
  // ADC INIT
  pinMode(INPUT_PIN, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  adcInit();
  pdbInit();
  sdInit(); 
  tft.begin();
  tft.fillScreen(BG_COLOR);

  bufferPos = 0;
  adcValue = 0;

  defaultScreen(adcValue);
  myTimer = millis();

}

int adcValueCopy = 0;
int x, y, y2 = 0;
int xPrev, yPrev = 0;
int progRunning = 0;
unsigned long progTimer = 0;

// calculated by: screen width / number of grid lines along width = pixels per grid block
// then, 40 ms / pixels per grid block. (40ms is the length of time per grid block)
unsigned long timePerPixel = 40 / (screenWidth / numLinesWidth);  // ms

void loop() {
  buttonState = onOff();
  if (buttonState) {
    if (progRunning) {
      progRunning = 0;
      
      // Enable ADC interrupt, configure pin to ECG output 
      ADC0_SC1A = ADC_SC1_AIEN | 0;
      ADC0_SC1A = ADC_SC1_AIEN | channel2sc1a[7];
    } else {
      chooseOption(adcValue);
    }
    buttonState = 0;
    tft.fillScreen(BG_COLOR);
  }

  if (progRunning) {
    if (!stabilizedFlag) {
      tft.fillScreen(BG_COLOR);
      stabilize2();
    }
    if (progRunning) {
      ekgProg();
    } else {
      defaultScreen(adcValue);
      bradycardia = 0;
      countBrad = 0;
      tachycardia = 0;
      countTach = 0;
    }
  } else {
    bradycardia = 0;
    countBrad = 0;
    tachycardia = 0;
    countTach = 0;
    defaultScreen(adcValue);
    stabilizedFlag = 0;
  }
}

// ECG Analysis Variables 
int heartAvg = 0;
int qrsAvg = 0;
int heartSum = 0;
int countPAC = 0;
int QRScopy = 0;

long sampleTimer = 0;
int currentIx = 0;

// Low-pass Filter variables 
int LPF_Data;
int LPF_Data_BL;
int LPF_Baseline;
float LPF_Beta = 0.15; // 0<Beta<1

int saveRaw = 1; // at prog end, chooses what type of data to save

// Buffers for the Band-pass Filters 
double inBPF[5] = {0, 0, 0, 0, 0};
double outBPF[5] = {0, 0, 0, 0, 0}; 

void ekgProg() {
  timeStamp = millis() - startTimer;
  if (timeStamp <= PROG_MAX_TIME) {
    adcValueCopy = adcValue;
    myTimer = millis() - myTimer;
    if (myTimer >= 1000 ) {
      drawNewData();
      delay(10);
      myTimer = millis();
    }
  } else {
    progRunning  = 0;
    tft.fillScreen(BG_COLOR);
    
    displayResults(heartAvg, qrsAvg, heartAvg < 60, heartAvg > 110, countPAC);
    tft.setFont(RussoOne_28);
    tft.setCursor(40, 10);
    tft.setTextColor(TEXT_COLOR);
    tft.println("Saving File...");
    tft.setFont(DEF_FONT);

    if (saveRaw) {
      writeCard(myBuffer, BUFFER_SIZE);
    } else {
      // reset BPF buffer values
      for (int i = 0; i < 5; i++) {
        inBPF[i] = 0;
        outBPF[i] = 0;
      }
      // filter buffer data
      for (int i = 0; i < BUFFER_SIZE; i++) {
        myBuffer[i] = BPF((double)myBuffer[i]);
      }
      writeCard(myBuffer, BUFFER_SIZE);
    }
    bufferPos = 0;
    delay(1000); // show file name on screen for second
    tft.fillScreen(BG_COLOR);
    
    // Enable ADC interrupt, configure pin to Pot
    ADC0_SC1A = ADC_SC1_AIEN | 0;
    ADC0_SC1A = ADC_SC1_AIEN | channel2sc1a[7];
  }
}

// 
int onOff() {
  // read the state of the switch into a local variable:
  int myButtonState = digitalRead(buttonPin);

  // Looks for a LOW to HIGH transition and
  // toggles the state-variable for ech button press
  if (myButtonState == LOW && lastButtonState == HIGH) {
    lastButtonState = myButtonState;
    return 1;
  } else {
    lastButtonState = myButtonState;
    return 0;
  }

}

// Variables for additional ECG analysis 
int y2Prev = 0;
int QRS_startFlag = 0; 
int heartArray[8] = {0};
double qrsArray[7] = {0};
double sqrWind = 0;

int foundStableQRS = 0;
int foundStableHeart = 0;
int heartWithinRange = 0;
int qrsWithinRange = 0;

int heartLow = 0;
int heartHigh = 0;
int qrsLow = 0;
int qrsHigh = 0;
void drawNewData() {
    
  // For 2nd order 3-18Hz
  y = map(BPF(LPF()),0, 4095, screenHeight + 360, 0) * 2;  // NEW FILTER
//  y = map(LPF(),0, 4095, screenHeight + 360, 0) * 2;
  y = y - 510;
  
  double sqr = squaring(); // KEEP THIS
  sqrWind = lowPassExponential(0.8, sqrWind); // input first param [0 to 1]
  if (sqrWind < 4  && sqrWind > 0.5) {
      if (QRS_startFlag) {
        QRS_timer = millis() - QRS_timer;
        QRScopy = (int)QRS_timer;
        tft.drawLine(x, screenHeight-20, x, screenHeight, ILI9341_MAGENTA);
        tft.drawLine(x + 1, screenHeight-20, x + 1, screenHeight, ILI9341_MAGENTA);
        tft.drawLine(x - 1, screenHeight-20, x - 1, screenHeight, ILI9341_MAGENTA);
      }
      QRS_startFlag = 0;
      QRS_timer = millis();
  }

  if (sqrWind > adaptiveThresh) {
    PeriodOfRR = (millis() - heartRateTimer);
    if (PeriodOfRR > QRS_MAX_TIME) {
      QRS_startFlag = 1;
      double period = PeriodOfRR / 1000;
      heartRate =  60 / period;

      
      if (heartRate < 60) {     
        countBrad++; 
//        Serial.print("Heart Rate: " );
//        Serial.println(heartRate);
//        Serial.print("CountBrad: ");
//        Serial.println(countBrad);
        if (countBrad >= MIN_COUNT_OF_REPEAT) {
          bradycardia = 1;
        } 
      } else {
        countBrad = 0;
      }
      if (heartRate > 110) {
        countTach++;
//        Serial.print("Heart Rate: " );
//        Serial.println(heartRate);
//        Serial.print("CountTach: ");
//        Serial.println(countTach);
        if (countTach >= MIN_COUNT_OF_REPEAT) {
          tachycardia = 1;
        }  
      } else {
        countTach = 0; 

      }
      
      if (!foundStableHeart) {
        heartWithinRange = 1;
        for (int i = 0; i < 3; i++) {
//          Serial.print(heartArray[i]);
//          Serial.print(", ");
          if (heartArray[i] < 160 && heartArray[i] > 40) {
            // heart within range
          } else {
            heartWithinRange = 0;
          }
        }
        if (heartWithinRange) {
          if ( (abs(heartArray[0] - heartArray[1])) < 20 && (abs(heartArray[1] - heartArray[2]) < 20) && (abs(heartArray[2] - heartArray[3]) < 20) ) {
            foundStableHeart = 1;
//            Serial.println("FOUND HEART STABLE!!");
            heartAvg = (heartArray[0] + heartArray[1] + heartArray[2]) / 3;
//            Serial.println(heartAvg);
          }
        }
      }
//      Serial.println();

      if (!foundStableQRS) {
        qrsWithinRange = 1;
        for (int i = 0; i < 3; i++) {
          Serial.print(qrsArray[i]);
          Serial.print(", ");
          if (qrsArray[i] > 60 && qrsArray[i] < 150) {
            // heart within range
          } else {
            qrsWithinRange = 0;
          }
        }
        if (qrsWithinRange) {
          if ( (abs(qrsArray[0] - qrsArray[1])) < 30 && (abs(qrsArray[0] - qrsArray[2]) < 30) && (abs(qrsArray[0] - qrsArray[3]) < 30) ) {
            foundStableQRS = 1;
            Serial.println("FOUND QRS STABLE!!");
            qrsAvg = (qrsArray[0] + qrsArray[1] + qrsArray[2]) / 3;
            Serial.println(qrsAvg);
          }
        }
      }
      Serial.println();

      // looks for 3 consecutive, periodic heart beats
      if (!foundStableHeart) {
        for (int i = 2; i >= 0; i--) {
          heartArray[i+1] = heartArray[i];
        } 
        heartArray[0] = heartRate;
      } else {
        // check heart beat within tolerance
        heartLow = heartAvg * 0.8;
        heartHigh = heartAvg * 1.2;
        if (heartRate > heartLow && heartRate < heartHigh) {
          for (int i = 6; i >= 0; i--) {
            heartArray[i+1] = heartArray[i];
          } 
          heartArray[0] = heartRate;
          // check if array is full
          if (heartArray[7] != 0) {
            heartAvg = 0;
            for (int i = 0; i < 8; i++) {
//              Serial.print(heartArray[i]);
//              Serial.print(", ");
              heartAvg = heartAvg + heartArray[i];
            }
            heartAvg = heartAvg / 8;
//            Serial.println();
//            Serial.print("HEART AVG: ");
//            Serial.println(heartAvg);
          }
        }
      }

      // looks for 3 consecutive, periodic heart beats
      if (!foundStableQRS) {
        for (int i = 2; i >= 0; i--) {
          qrsArray[i+1] = qrsArray[i];
        } 
        qrsArray[0] = QRScopy;
      } else {
        // check heart beat within tolerance
        qrsLow = qrsAvg * 0.8;
        qrsHigh = qrsAvg * 1.2;
        if (QRScopy > qrsLow && QRScopy < qrsHigh) {
          for (int i = 5; i >= 0; i--) {
            qrsArray[i+1] = qrsArray[i];
          } 
          qrsArray[0] = QRScopy;
          // check if array is full
          if (qrsArray[6] != 0) {
            qrsAvg = 0;
            for (int i = 0; i < 7; i++) {
              Serial.print(qrsArray[i]);
              Serial.print(", ");
              qrsAvg = qrsAvg + qrsArray[i];
            }
            qrsAvg = qrsAvg / 7;
            Serial.println();
            Serial.print("QRS AVG: ");
            Serial.println(qrsAvg);
          }
        }
      }

      heartRateTimer = millis();
      
    }
    tft.drawLine(x, screenHeight-20, x, screenHeight, ILI9341_GREEN);
    tft.drawLine(x + 1, screenHeight-20, x + 1, screenHeight, ILI9341_GREEN);
    tft.drawLine(x - 1, screenHeight-20, x - 1, screenHeight, ILI9341_GREEN);

  }
  // draw new data point, use thicker line
  tft.drawLine(xPrev, yPrev, x, y, LINE_COLOR);
  tft.drawLine(xPrev, yPrev + 1, x, y + 1, LINE_COLOR);
  tft.drawLine(xPrev, yPrev - 1, x, y - 1, LINE_COLOR);
  tft.drawLine(xPrev - 1, yPrev, x - 1, y, LINE_COLOR);
  tft.drawLine(xPrev + 1, yPrev - 1, x + 1, y, LINE_COLOR);

  x++;

  tft.drawLine(xPrev, yPrev, x, y, LINE_COLOR);
  tft.drawLine(xPrev, yPrev + 1, x, y + 1, LINE_COLOR);
  tft.drawLine(xPrev, yPrev - 1, x, y - 1, LINE_COLOR);
  tft.drawLine(xPrev - 1, yPrev, x - 1, y, LINE_COLOR);
  tft.drawLine(xPrev + 1, yPrev - 1, x + 1, y, LINE_COLOR);

  x++;

  xPrev = x - 1;
  yPrev = y;
  y2Prev = y2;

  if (x > screenWidth) {
    x = 0;
    xPrev = 0;
    tft.fillScreen(BG_COLOR);
    drawGrid();
    textResultsBorder();
    textResults((int)heartRate, QRScopy);
    updateMeasuring(timeStamp);
    blueHBM();
    if (foundStableQRS) {
      if (detectPAC()) {
        tft.fillRect(0,0,10, 10, ILI9341_MAGENTA);
        countPAC++;
      } else {
        tft.fillRect(0,0,10, 10, BG_COLOR);
      }
    }
  }
}

// Function for redrawing the grid during ECG measurements 
void drawGrid() {
  int stepSizeWidth = (screenWidth / numLinesWidth);
  int stepSizeHeight = (screenHeight / numLinesHeight);
  // draw outer grid
  // top
  tft.drawLine(0, 0, screenWidth - 1, 0, GRID_COLOR);
  tft.drawLine(0, 0, screenWidth - 1, 1, GRID_COLOR);
  tft.drawLine(0, 0, screenWidth - 1, 2, GRID_COLOR);
  // bottom
  tft.drawLine(0, screenHeight - 1, screenWidth - 1, screenHeight - 1, GRID_COLOR);
  tft.drawLine(0, screenHeight - 1, screenWidth - 1, screenHeight - 2, GRID_COLOR);
  tft.drawLine(0, screenHeight - 1, screenWidth - 1, screenHeight - 3, GRID_COLOR);
  // left
  tft.drawLine(0, 0, 0, screenHeight - 1, GRID_COLOR);
  tft.drawLine(0, 0, 1, screenHeight - 1, GRID_COLOR);
  tft.drawLine(0, 0, 2, screenHeight - 1, GRID_COLOR);
  // right
  tft.drawLine(screenWidth - 1, 0, screenWidth - 1, screenHeight - 1, GRID_COLOR);
  tft.drawLine(screenWidth - 1, 0, screenWidth - 2, screenHeight - 1, GRID_COLOR);
  tft.drawLine(screenWidth - 1, 0, screenWidth - 3, screenHeight - 1, GRID_COLOR);
  // draw width lines
  for (int i = 0; i < numLinesWidth; i++) {
    if (i % 5 == 0) {
      tft.drawLine(i * stepSizeWidth - 1, 0, i * stepSizeWidth - 1, screenHeight, GRID_COLOR);
      tft.drawLine(i * stepSizeWidth, 0, i * stepSizeWidth, screenHeight, GRID_COLOR);
      tft.drawLine(i * stepSizeWidth + 1, 0, i * stepSizeWidth + 1, screenHeight, GRID_COLOR);
    } else {
      tft.drawLine(i * stepSizeWidth, 0, i * stepSizeWidth, screenHeight, GRID_COLOR);
    }
  }
  // draw height lines
  for (int i = 0; i < numLinesHeight; i++) {
    if (i % 5 == 0) {
      tft.drawLine(0, i * stepSizeHeight - 1, screenWidth, i * stepSizeHeight - 1, GRID_COLOR);
      tft.drawLine(0, i * stepSizeHeight, screenWidth, i * stepSizeHeight, GRID_COLOR);
      tft.drawLine(0, i * stepSizeHeight + 1, screenWidth, i * stepSizeHeight + 1, GRID_COLOR);
    } else {
      tft.drawLine(0, i * stepSizeHeight, screenWidth, i * stepSizeHeight, GRID_COLOR);
    }

  }
}

// Adds the int val to the buffer myBuffer 
void addToBuffer(int val) {
  if (stabilizedFlag) {
    myBuffer[bufferPos] = val;
    bufferPos++;
    if (bufferPos >= BUFFER_SIZE) {
      bufferPos = 0;
    }
  }
}

// Writes the test results on the top of the 
// ECG measurement screen
void textResults(int hr, int qrsi) {
  tft.setFont(RussoOne_14);
  tft.setCursor(15, 10);
  tft.setTextColor(ILI9341_CYAN);
  
  tft.print("HR: ");
  if (hr > 200 || hr == -1 || hr == 0) {
    tft.print("---");
  } else if (hr < 100) {
    tft.print(" ");
    tft.print(hr);
  } else {
    tft.print(hr);
  }
  tft.print("  ");

  tft.setTextColor(ILI9341_GREEN);
  tft.print("QRSI: ");
  if (qrsi > 200 || qrsi == -1 || qrsi == 0) {
    tft.print("---");
  } else if (qrsi < 100) {
    tft.print(" ");
    tft.print(qrsi);
  } else {
    tft.print(qrsi);
  }
  tft.print("  ");

  tft.setTextColor(ILI9341_MAGENTA);
  tft.print("BC: ");
  tft.setFont(AwesomeF100_14);
  if (hr < 60 && hr != -1) {
    tft.setTextColor(ILI9341_RED);
    tft.print((char)25);
  } else {
    tft.setTextColor(ILI9341_GREEN);
    tft.print((char)24);
  }
  tft.setTextColor(ILI9341_MAGENTA);
  tft.setFont(RussoOne_14);
  tft.print(" ");
  
  tft.print("TC: ");
  tft.setFont(AwesomeF100_14);
  if (hr > 110 && hr != -1) {
    tft.setTextColor(ILI9341_RED);
    tft.print((char)25);
  } else {
    tft.setTextColor(ILI9341_GREEN);
    tft.print((char)24);
  }
  
  tft.setTextColor(TEXT_COLOR);
  tft.setFont(DEF_FONT);
}





