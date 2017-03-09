/*
  Cardio1.ino
  Lab 7, Part 2
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

#define BG_COLOR ILI9341_WHITE
#define GRID_COLOR ILI9341_RED
#define LINE_COLOR ILI9341_BLACK
#define LINE_COLOR2 ILI9341_BLUE
#define TEXT_COLOR ILI9341_BLACK
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



//const int buttonPin =  1;     // the number of the pushbutton pin.
const int buttonPin =  1;     // the number of the pushbutton pin.
// Variables will change:
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
int startEKG = 1;                  // flag for whether the cube should rotate
double heartRate = 0;

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

volatile int bufferPos = 0;
volatile int stabilizedFlag = 0;
int myBuffer[BUFFER_SIZE];
volatile int adcValue;
void setup() {

  Serial.begin(9600);

  // ADC INIT
  pinMode(INPUT_PIN, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  adcInit();
  pdbInit();


  bufferPos = 0;
  adcValue = 0;

  // LCD Screen Setup
  tft.begin();
  tft.setRotation(3);
  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(BG_COLOR);

  defaultScreen();
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
    //  Serial.println("Button On");
    if (progRunning) {
      //  Serial.println("Set Prog Running to 0");
      progRunning = 0;
    } else {
      //  Serial.println("Set Prog Running to 1");
      progRunning = 1;
    }
    buttonState = 0;
    tft.fillScreen(BG_COLOR);
  }

  if (progRunning) {
    if (!stabilizedFlag) {
      //Serial.println("Stabilizing..");
      stabilize2();
      tft.fillScreen(BG_COLOR);
    }
    if (progRunning) {
      //Serial.println("EKG Prog");
      ekgProg();
    } else {
      //Serial.println("Start Screen");
      startScreen();
    }
  } else {
    //Serial.println("Start Screen");
    defaultScreen();
    stabilizedFlag = 0;
  }
}

void startScreen() {
  tft.setCursor(0, 80);
  tft.println("EKG");
  tft.println("Press Button to Start!");
}

int LPF_Data;
int LPF_Data_BL;
int LPF_Baseline;
float LPF_Beta = 0.075; // 0<Beta<1
//float LPF_Beta = 0.15; // 0<Beta<1
//#define PDB_SC_PDBEN_MASK                        0x80u
//#define PDB_S_ERR_MASK                           0xFFu
void ekgProg() {
  if (millis() - startTimer <= PROG_MAX_TIME) {

    PDB0_SC &= ~PDB_SC_PDBEN;  // disable interrupts
    adcValueCopy = adcValue;
    PDB0_SC |= PDB_SC_PDBEN;  // enable interrupts
    PDB0_SC |= PDB_SC_SWTRIG;  // reset pdb

    adcValueCopy = 4095 - adcValueCopy;

    LPF_Data = LPF_Data - (LPF_Beta * (LPF_Data - adcValueCopy));
    LPF_Data_BL = LPF();
    LPF_Baseline = LPF_Data - LPF_Data_BL;
    //Serial.println("Filter: ");
    //    Serial.println(LPF_Data);
    //LPF_Data = adcValueCopy;

    //MovingAvg = movingAvgFilter();
    //MovingAvg = adcValueCopy;
    //Serial.println("Moving AVG: ");
    //Serial.println(MovingAvg);
    myTimer = millis() - myTimer;
    if (myTimer >= timePerPixel) {
      drawNewData();
      myTimer = millis();
    }
  } else {
    progRunning  = 0;
    tft.fillScreen(BG_COLOR);
    writeCard(myBuffer, BUFFER_SIZE);
    bufferPos = 0;
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

int y2Prev = 0;
int QRS_startFlag = 0; 
void drawNewData() {
  drawGrid();
  //y = map(LPF_Data, 0, 4095, 0, screenHeight);
  // TODO this is a hack, map values better to fit screen later
  y = map(LPF_Baseline * 2.8, 0, 4095, 0, screenHeight) - 100;
  //  y = map(movingWindowInt(), 0, 4095, 0, screenHeight);

  //    y = map(squaring(), 0, 4095, 0, screenHeight + 100);
 // Serial.print("Window: ");
  Serial.println(movingWindowInt());
  double sqr = squaring();
  double sqrWind = movingWindowInt();



  //  y2 = map(LPF_Data_BL, 0, 4095, 0, screenHeight);
  //Serial.println(y);
  y = y -  230; // offset
  // draw over line before writing new value
  tft.drawLine(x, 0, x, screenHeight, BG_COLOR);
  tft.drawLine(x + 1, 0, x + 1, screenHeight, BG_COLOR);
  // make sure grid is not erased
  //       if (sqr > 200.0) {
  if (movingWindowInt() > 200) {
    PeriodOfRR = (millis() - heartRateTimer);
    if (PeriodOfRR > QRS_MAX_TIME) {
      QRS_startFlag = 1;
      QRS_timer = millis();
      //Serial.println("RR interval: "); 
      //Serial.println(PeriodOfRR);
      double period = PeriodOfRR / 1000;
      heartRate =  60 / period;
      heartRateTimer = millis();
      Serial.print("Heart rate is: ");
      Serial.println(heartRate);
    }
    tft.drawLine(x, 0, x, screenHeight, ILI9341_GREEN);
    tft.drawLine(x + 1, 0, x + 1, screenHeight, ILI9341_GREEN);
    tft.drawLine(x - 1, 0, x - 1, screenHeight, ILI9341_GREEN);

  }
  if (QRS_startFlag && (movingWindowInt() < 30)) {
    QRS_timer = millis() - QRS_timer;
    //Serial.print("QRS timer is: "); 
    //Serial.println(QRS_timer);
    QRS_startFlag = 0;
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
  }

}

void stabilize2() {
  tft.setCursor(0, 0);
  tft.println("Stabilizing...");

  int upperThreshold = UPPER_THRESH;
  int lowerThreshold = LOWER_THRESH;

  int buffLength = 20;
  int valCopy;
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
    interrupts();


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

void clearTextNumbers(char* str) {
  tft.setCursor(0, 0);
  tft.println("Stabilizing...");
  tft.print("Upper Threshold: ");
  tft.println("    ");
  tft.print("Lower Threshold: ");
  tft.println("    ");
  tft.print("Current value: ");
  tft.setTextColor(BG_COLOR);
  tft.println(str);
  tft.setTextColor(TEXT_COLOR);

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
    Serial.println("error opening file");
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







