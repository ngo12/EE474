/* 
  Cardio1.ino
  Lab 7, Part 2
  Course: EE 474 
 
  Names and Student Numbers: 
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375

*/

#include <stdint.h>
#include <kinetis.h>
#include "ILI9341_t3.h"

#define BG_COLOR ILI9341_BLACK
#define GRID_COLOR ILI9341_PINK
#define LINE_COLOR ILI9341_GREEN
#define TEXT_COLOR ILI9341_BLUE
// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

#define PDB_CONFIG (PDB_SC_TRGSEL(15) | (PDB_SC_PDBEN) | (PDB_SC_PDBIE) | (PDB_SC_CONT) | PDB_SC_PRESCALER(7) | PDB_SC_MULT(1))
#define BUFFER_SIZE 7500
#define ST_BUFFER_SIZE 750
#define MR_BUFFER_SIZE 50  // MR = Most Recent, will hold a very small amount of values from adc
#define STABILITY_THRESHOLD 0.05
#define INPUT_PIN A1

#define UPPER_THRESH 3000
#define LOWER_THRESH 1000
#define PROG_MAX_TIME 30000 //ms

const int buttonPin =  1;     // the number of the pushbutton pin.
// Variables will change:
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
int startEKG = 1;                  // flag for whether the cube should rotate


int stabilizedFlag = 0;
unsigned long myTimer = 0;
int screenWidth = 320;
int screenHeight = 240;
// grid line amounts
int numLinesWidth = 40;
int numLinesHeight = 20;
volatile int myBuffer[BUFFER_SIZE];
volatile int bufferPos = 0;
volatile int stBuffer[ST_BUFFER_SIZE];
volatile int stBufferPos = 0;
volatile int mrBuffer[MR_BUFFER_SIZE];
volatile int mrBufferPos = 0;
volatile int adcValue;
void setup() {

  Serial.begin(9600);

  bufferPos = 0;
  adcValue = 0;

  pinMode(INPUT_PIN, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);  

  analogReadResolution(12);

  // LCD Screen Setup 
  tft.begin(); 
  tft.setRotation(3); 
  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(BG_COLOR);

  ////Enable the PDB clock
  SIM_SCGC6 |= SIM_SCGC6_PDB;
  ////Modulus Register, 1/(48 MHz / 128 / 10) * 37500 = 1 s
  ////Modulus Register, 1/(48 MHz / 128 / 10) * x = (12/937) s

  // Solved equation 1/(48 MHz / 128 / 10) * X = (1/240)
  // based on Modulus Register equation for a period of 
  // (1/120)s, or an on time of (1/240)s. 
  PDB0_MOD = 156;  //1 = 9.36khz
  
  //Interrupt delay = 0
  PDB0_IDLY = 0;
  
  ////Set PDB status and control bits to 1:
  //  PDB_SC_TRGSEL(15);   //     Select software trigger
  //  PDB_SC_PRESCALER(7);  //    Prescaler = 128
  //  PDB_SC_MULT(1);      //    Prescaler multiplication factor = 10

  PDB0_SC = PDB_CONFIG;
  
  ////Hint: double check the order of the bits in the definition of PDB0_SC in the manual.
  ////Software trigger (reset and restart counter)
  PDB0_SC |= PDB_SC_SWTRIG;
  ////Load OK
  PDB0_SC |= PDB_SC_LDOK;
  ////Enable interrupt request
  NVIC_ENABLE_IRQ(IRQ_PDB);

//  stabilize2();
//  tft.fillScreen(BG_COLOR);
//  drawGrid();

  tft.fillScreen(BG_COLOR);
  startScreen();
  myTimer = millis();
}

int adcValueCopy = 0;
int x, y = 0;
int xPrev, yPrev = 0;
int progRunning = 0;
unsigned long progTimer = 0;

// calculated by: screen width / number of grid lines along width = pixels per grid block
// then, 40 ms / pixels per grid block. (40ms is the length of time per grid block)
unsigned long timePerPixel = 40 / (screenWidth / numLinesWidth);  // ms
void loop() {
  Serial.println("Main Loop");

  buttonState = onOff();
  if (buttonState) {
    Serial.println("Button On");
    if (progRunning) {
      Serial.println("Set Prog Running to 0");
      progRunning = 0;
    } else {
      Serial.println("Set Prog Running to 1");
      progRunning = 1;
    }
    buttonState = 0;
    tft.fillScreen(BG_COLOR);
  }

  if (progRunning) {
    if (!stabilizedFlag) {
      Serial.println("Stabilizing..");
      stabilize2();
      tft.fillScreen(BG_COLOR);
    }
      Serial.println("EKG Prog");
      ekgProg();     
  } else {
    Serial.println("Start Screen");
    startScreen();
    //stabilizedFlag = 0;
  }

  
//  if (buttonState == 1) {
//    if (!progRunning) {
//      Serial.println("Loop1");
//      progRunning = 1;
//      stabilizedFlag = 0;
//      tft.fillScreen(BG_COLOR);
//      stabilize2();
//      tft.fillScreen(BG_COLOR);
//      drawGrid();
//      progTimer = millis();
//    } else {
//      Serial.println("Loop2");
//      progRunning = 0;
//      tft.fillScreen(BG_COLOR);
//    }
//  } else if (buttonState == 0) {
//    if (!progRunning) {
//      Serial.println("Loop3");
//      startScreen();
//    } else {
//      Serial.println("Loop4");
//      ekgProg();
//      if (millis() - progTimer >= PROG_MAX_TIME) {
//        progRunning = 0;
//        buttonState = 0;
//        tft.fillScreen(BG_COLOR);
//      }
//    }
//  }
//  Serial.println("outside loops");

}

void startScreen() {

  tft.setCursor(0, 80);
  tft.println("EKG");
  tft.println("Press Button to Start!");
  delay(50);
}

void ekgProg() {
  // put your main code here, to run repeatedly:
  noInterrupts();
  adcValueCopy = adcValue;
  interrupts();
  myTimer = millis() - myTimer;
  if (myTimer >= timePerPixel) {
    drawNewData();
    myTimer = millis();
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

void pdb_isr() {
  adcValue = analogRead(INPUT_PIN);
  addToBuffer(adcValue);
  PDB0_SC = PDB_CONFIG | PDB_SC_LDOK;  // (also clears interrupt flag)
}

void drawNewData() {
  y = map(adcValueCopy, 0, 4095, 0, screenHeight);
  // draw over line before writing new value
  tft.drawLine(x, 0, x, screenHeight, BG_COLOR);
  // make sure grid is not erased
  drawGrid();
  // draw new data point
  tft.drawLine(xPrev, yPrev, x, y, LINE_COLOR);
  xPrev = x;
  yPrev = y;
  x++;
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

  int valCopy;
  int prevValues[50];
  int enoughDataFlag = 0;
  int bufferStableFlag = 0;
  int bufferIndex = 0;
  buttonState = onOff();
  while (!stabilizedFlag && buttonState != 1) {
    Serial.print("Stable Loop");
    noInterrupts();
    valCopy = adcValue;
    interrupts();
    prevValues[bufferIndex] = adcValue;
    bufferIndex++;
    if (bufferIndex > 50) {
      bufferIndex = 0;
      enoughDataFlag = 1;
    }
    if (enoughDataFlag) {
      bufferStableFlag = 1;
      for (int i = 0; i < 50; i++) {
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
  progRunning = 0;
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

volatile int stBufferFullFlag = 0;
void stabilize() {
  tft.setCursor(0, 0);
  tft.println("Stabilizing...");
  tft.println("Waiting for buffer to fill.");
  while (!stBufferFullFlag) {
    // wait for buffer to fill from adc interrupt
  }
  tft.fillScreen(BG_COLOR);
  tft.setCursor(0, 0);
  tft.println("Stabilizing...");
  tft.print("Average Value: ");
  tft.println('-');
  tft.print("Current Value: ");
  tft.println('-');
  tft.print("Ratio: ");
  tft.println('-');
  
  long bufferSum = 0;
  long bufferAvg = 0;
  long miniBufferSum = 0;
  long miniBufferAvg = 0;
  int stBufferCopy[ST_BUFFER_SIZE];
  int mrBufferCopy[ST_BUFFER_SIZE];
  float stThreshold = STABILITY_THRESHOLD;
  float stRatio = 0.0;
  while (!stabilizedFlag) {

    // get values
    noInterrupts();
    for (int i = 0; i < ST_BUFFER_SIZE; i++) {
      stBufferCopy[i] = stBuffer[i];
    }
    for (int i = 0; i < MR_BUFFER_SIZE; i++) {
      mrBufferCopy[i] = mrBuffer[i];
    }
    interrupts();

    // computer entire stability buffer avg
    bufferSum = 0;
    for (int i = 0; i < ST_BUFFER_SIZE; i++) {
      bufferSum = bufferSum + stBufferCopy[i];
    }
    bufferAvg = bufferSum / ST_BUFFER_SIZE;

    // TODO: This is not right because the miniBuffer takes from the end of the stBuffer,
    // but the end of the stBuffer is not always the most recent data.
    // average the last X number of data points to check with entire buffer avg
    miniBufferSum = 0;
    for (int i = 0; i < MR_BUFFER_SIZE; i++) {
      miniBufferSum = miniBufferSum + mrBufferCopy[MR_BUFFER_SIZE];
    }
    miniBufferAvg = miniBufferSum / MR_BUFFER_SIZE;

    // See if stability ratio passes threshold, end stable test if it does
    stRatio = abs(1.0 - ( (1.0 * miniBufferAvg) / (1.0 * bufferAvg) ) );
    if (stRatio > stThreshold) {
      stabilizedFlag = 1;
    }
    tft.fillScreen(BG_COLOR);
    tft.setCursor(0, 0);
    tft.println("Stabilizing...");
    tft.print("Average Value: ");
    tft.println(bufferAvg);
    tft.print("Current Value: ");
    tft.println(miniBufferAvg);
    tft.print("Ratio: ");
    tft.println(stRatio);
    delay(50);
  }
}

void drawGrid() {
  int stepSizeWidth = (screenWidth / numLinesWidth);
  int stepSizeHeight = (screenHeight / numLinesHeight);
  // draw width lines
  for (int i = 0; i < numLinesWidth; i++) {
    tft.drawLine(i * stepSizeWidth, 0, i * stepSizeWidth, screenHeight, GRID_COLOR);
  }
  // draw height lines
  for (int i = 0; i < numLinesHeight; i++) {
    tft.drawLine(0, i * stepSizeHeight, screenWidth, i * stepSizeHeight, GRID_COLOR);
  }
}

void addToBuffer(int val) {
//  if (stabilizedFlag) {
    myBuffer[bufferPos] = val;
    bufferPos++;
    if (bufferPos > BUFFER_SIZE) {
     bufferPos = 0;
    }
//  } else {
//    mrBuffer[mrBufferPos] = val;
//    mrBufferPos++;
//    if (mrBufferPos > MR_BUFFER_SIZE) {
//     mrBufferPos = 0;
//    }
//    stBuffer[stBufferPos] = val;
//    stBufferPos++;
//    if (stBufferPos > ST_BUFFER_SIZE) {
//     stBufferPos = 0;
//     if (stBufferFullFlag == 0) {
//       stBufferFullFlag = 1;
//     }
//    }
//  }
}



