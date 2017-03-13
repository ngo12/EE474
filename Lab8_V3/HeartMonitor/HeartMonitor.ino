/*
  HeartMonitor.ino
  Lab 8
  Course: EE 474

  Names and Student Numbers:
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375

*/
// Hello


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
int bradycardia = 0; 
int tachycardia = 0;
int pac = 0;

double adaptiveThresh = 0;

unsigned long myTimer = 0;
unsigned long startTimer = 0;
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

// For SD Card
File myFile;
String initials = "KARLBN";
String sRate = "250";
const int chipSelect = 4;
int fileHeadingNumber = 0;
String fileName = "KARLBN0.txt";

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
  connectScreen();
  //blueInit();
  tft.fillScreen(BG_COLOR);

  bufferPos = 0;
  adcValue = 0;

  defaultScreen2(adcValue);
  myTimer = millis();

  // Setup SD Card
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
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
  //  Serial.println("Main Loop");

  buttonState = onOff();
  if (buttonState) {
    if (progRunning) {
      //  Serial.println("Set Prog Running to 0");
      progRunning = 0;
      
      // Enable ADC interrupt, configure pin to ECG output 
      ADC0_SC1A = ADC_SC1_AIEN | 0;
      ADC0_SC1A = ADC_SC1_AIEN | channel2sc1a[7];
    } else {
      //  Serial.println("Set Prog Running to 1");
      chooseOption(adcValue);
    }
    buttonState = 0;
    //  Serial.println("Button On");
    tft.fillScreen(BG_COLOR);
  }

  if (progRunning) {
    if (!stabilizedFlag) {
      //Serial.println("Stabilizing..");
      tft.fillScreen(BG_COLOR);
      stabilize2();
      //tft.fillScreen(BG_COLOR);
//      drawGrid();
//      textResultsBorder();
//      textResults(0, 0);
    }
    if (progRunning) {
      //Serial.println("EKG Prog");
      ekgProg();
    } else {
      //Serial.println("Start Screen");
      defaultScreen2(adcValue);
    }
  } else {
    defaultScreen2(adcValue);
    stabilizedFlag = 0;
  }
}

void startScreen() {
  tft.setCursor(0, 80);
  tft.println("EKG");
  tft.println("Press Button to Start!");
}

long sampleTimer = 0;
int currentIx = 0;
int LPF_Data;
int LPF_Data_BL;
int LPF_Baseline;
float LPF_Beta = 0.15; // 0<Beta<1
//float LPF_Beta = 0.15; // 0<Beta<1
//#define PDB_SC_PDBEN_MASK                        0x80u
//#define PDB_S_ERR_MASK                           0xFFu
int saveRaw = 1; // at prog end, chooses what type of data to save
// filter used for filtering buffer data if chosen
double inBPF[5] = {0, 0, 0, 0, 0};
double outBPF[5] = {0, 0, 0, 0, 0}; 
void ekgProg() {
  if (millis() - startTimer <= PROG_MAX_TIME) {
    adcValueCopy = adcValue;
//    adcValueCopy = 4095 - adcValueCopy;
//    LPF_Data = LPF_Data - (LPF_Beta * (LPF_Data - adcValueCopy));
    //LPF_Data_BL = LPF();
    //LPF_Baseline = LPF_Data - LPF_Data_BL;

    // drawNewData2();
    
    //MovingAvg = movingAvgFilter();
    //MovingAvg = adcValueCopy;
    //Serial.println("Moving AVG: ");
    //Serial.println(MovingAvg);
//    Serial.println(myTimer);
    myTimer = millis() - myTimer;
    if (myTimer >= 1000 ) {
      drawNewData();
      delay(10);
      myTimer = millis();
    }
  } else {
    progRunning  = 0;
    tft.fillScreen(BG_COLOR);
    
    displayResults(heartRate, QRS_timer, heartRate < 60, heartRate > 110, 0);
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
    
    // Enable ADC interrupt, configure pin to Pot
    ADC0_SC1A = ADC_SC1_AIEN | 0;
    ADC0_SC1A = ADC_SC1_AIEN | channel2sc1a[7];
  }
}

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

//int drawIndex = 0;
//void drawNewData2() {
//  y = map(LPF_Data, 0, 4095, 0, screenHeight);
//  tft.drawLine(myBuffer[, yPrev, x, y, LINE_COLOR);
//}

int y2Prev = 0;
int QRS_startFlag = 0; 
int countBrad = 0;
int countTach = 0;

double sqrWind = 0;
int QRScopy = 0;
void drawNewData() {
//  LPF3();
  // For 4th Order 3-20Hz
//  y = map(LPF(),0, 4095, screenHeight + 360, 0) * 5;
//  y = y - 850;
  // For 2nd order 3-18Hz
  y = map(LPF(),0, 4095, screenHeight + 360, 0) * 2;
//  y = map(LPF(),0, 4095, screenHeight + 360, 0) * 2;
  y = y - 510;
  

  double sqr = squaring();
//  double sqrWind = movingWindowInt();
  sqrWind = lowPassExponential(0.8, sqrWind); // input first param [0 to 1]
//  Serial.println(sqrWind);
  // draw over line before writing new value
//  tft.drawLine(x, 0, x, screenHeight, BG_COLOR);
  //tft.drawLine(x + 1, 0, x + 1, screenHeight, BG_COLOR);
  // make sure grid is not erased
  //       if (sqr > 200.0) {

//    Serial.println(sqrWind);
//    Serial.println(adaptiveThresh);
//    Serial.println(localThresh / (maxBeats+1));
if (sqrWind < 4  && sqrWind > 0.5) {
    if (QRS_startFlag) {
      QRS_timer = millis() - QRS_timer;
      QRScopy = (int)QRS_timer;
      Serial.print("QRS timer is: ");
      Serial.println(QRS_timer);
      Serial.print("QRS(int) timer is: ");
      Serial.println((int)QRS_timer);
      tft.drawLine(x, screenHeight-20, x, screenHeight, ILI9341_MAGENTA);
    tft.drawLine(x + 1, screenHeight-20, x + 1, screenHeight, ILI9341_MAGENTA);
    tft.drawLine(x - 1, screenHeight-20, x - 1, screenHeight, ILI9341_MAGENTA);
    }
    QRS_startFlag = 0;
    QRS_timer = millis();
   // tft.drawLine(x, 0, x, screenHeight, ILI9341_MAGENTA);
}
//  Serial.print("SqrWind: ");
//  Serial.println(sqrWind);
  if (sqrWind > adaptiveThresh) {
    PeriodOfRR = (millis() - heartRateTimer);
    if (PeriodOfRR > QRS_MAX_TIME) {
      QRS_startFlag = 1;
      //Serial.println("RR interval: "); 
      //Serial.println(PeriodOfRR);
      double period = PeriodOfRR / 1000;
      heartRate =  60 / period;
      heartRateTimer = millis();
//      Serial.print("Heart rate is: ");
//      Serial.println(heartRate);
      if (heartRate < 60) {
        
        countBrad++; 
        if (countBrad >= MIN_COUNT_OF_REPEAT) {
          bradycardia = 1;
        }
      } else if (heartRate > 110) {
        countTach++;
        if (countTach >= MIN_COUNT_OF_REPEAT) {
          tachycardia = 1;
        }
      } else {
        countTach = 0;
        countBrad = 0;
      }
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

  //        tft.drawLine(xPrev, yPrev, x, y2, LINE_COLOR2);
  //    tft.drawLine(xPrev, y2Prev+1, x, y2+1, LINE_COLOR2);
  //    tft.drawLine(xPrev, y2Prev-1, x, y2-1, LINE_COLOR2);
  //    tft.drawLine(xPrev-1, y2Prev, x-1, y2, LINE_COLOR2);
  //    tft.drawLine(xPrev+1, y2Prev-1, x+1, y2, LINE_COLOR2);

  x++;

  tft.drawLine(xPrev, yPrev, x, y, LINE_COLOR);
  tft.drawLine(xPrev, yPrev + 1, x, y + 1, LINE_COLOR);
  tft.drawLine(xPrev, yPrev - 1, x, y - 1, LINE_COLOR);
  tft.drawLine(xPrev - 1, yPrev, x - 1, y, LINE_COLOR);
  tft.drawLine(xPrev + 1, yPrev - 1, x + 1, y, LINE_COLOR);

  //        tft.drawLine(xPrev, y2Prev, x, y2, LINE_COLOR2);
  //    tft.drawLine(xPrev, y2Prev+1, x, y2+1, LINE_COLOR2);
  //    tft.drawLine(xPrev, y2Prev-1, x, y2-1, LINE_COLOR2);
  //    tft.drawLine(xPrev-1, y2Prev, x-1, y2, LINE_COLOR2);
  //    tft.drawLine(xPrev+1, y2Prev-1, x+1, y2, LINE_COLOR2);
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
  }

}



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

void writeCard(int* buff, int bufferLength) {

  String fileName = "KARLBN" + (String)fileHeadingNumber;
  fileName = fileName + ".txt";

  int fileNameLength = fileName.length() + 1;
  char file[fileNameLength];
  fileName.toCharArray(file, fileNameLength);

  String header = initials + fileHeadingNumber;
  header = header + ", ";
  header = header + sRate;
  int headerLength = header.length() + 1;
  char headerCharArray[headerLength];
  header.toCharArray(headerCharArray, headerLength);

  Serial.println(file);
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  SD.remove(file);
  myFile = SD.open(file, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing...");
    myFile.println(headerCharArray);
    for (int i = 1; i <= bufferLength; i++) {
      myFile.print(buff[i - 1]);

      if (i % 8 == 0 && i != 1) {
        myFile.println();
        // if not last item, print comma
      } else if (bufferLength != i) {
        myFile.print(", ");
      }
    }
    myFile.println();
    myFile.println("EOF");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
  }
  fileHeadingNumber++;
}

void addToBuffer(int val) {
  if (stabilizedFlag) {
    myBuffer[bufferPos] = val;
    bufferPos++;
    if (bufferPos >= BUFFER_SIZE) {
      bufferPos = 0;
    }
  }
}

void textResultsBorder() {
  tft.fillRect(6, 6, screenWidth-13, 32, ILI9341_DARKGREY);
  tft.fillRect(9, 9, screenWidth-25, 26, ILI9341_BLACK);
}

void textResults(int hr, int qrsi) {
  tft.setFont(RussoOne_14);
  tft.setCursor(15, 10);
  tft.setTextColor(ILI9341_CYAN);
  
  tft.print("HR: ");
  if (hr > 200 || hr == -1) {
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
  if (qrsi > 200 || qrsi == -1) {
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
    tft.setTextColor(ILI9341_PINK);
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
    tft.setTextColor(ILI9341_PINK);
    tft.print((char)25);
  } else {
    tft.setTextColor(ILI9341_GREEN);
    tft.print((char)24);
  }
  
  tft.setTextColor(TEXT_COLOR);
  tft.setFont(DEF_FONT);
}





