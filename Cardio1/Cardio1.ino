/* 
  Cardio1.ino
  Lab 5, Part 1 
  Course: EE 474 
 
  Names and Student Numbers: 
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375

  Initializes the Programmable Delay Block 
  in setup in order to have the LED blink one 
  second on, one second off at a frequency of 120 Hz.
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
#define INPUT_PIN A0

int stabilizedFlag = 0;
unsigned long myTimer = 0;
int screenWidth = 320;
int screenHeight = 240;
// grid line amounts
int numLinesWidth = 40;
int numLinesHeight = 20;
volatile int myBuffer[BUFFER_SIZE];
volatile int bufferPos;
volatile int stBuffer[ST_BUFFER_SIZE];
volatile int stBufferPos;
volatile int adcValue;
void setup() {

  bufferPos = 0;
  adcValue = 0;

  pinMode(INPUT_PIN, INPUT);

  analogReadResolution(12);

  // LCD Screen Setup 
  tft.begin(); 
  tft.setRotation(2); 
  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(BG_COLOR);
  drawGrid();

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
  
  myTimer = millis();
}

void pdb_isr() {
  adcValue = analogRead(INPUT_PIN);
  addToBuffer(adcValue);
  PDB0_SC = PDB_CONFIG | PDB_SC_LDOK;  // (also clears interrupt flag)
}

int adcValueCopy = 0;
int x, y = 0;
int xPrev, yPrev = 0;
// calculated by: screen width / number of grid lines along width = pixels per grid block
// then, 40 ms / pixels per grid block. (40ms is the length of time per grid block)
unsigned long timePerPixel = 40 / (screenWidth / numLinesWidth);  // ms
void loop() {
  stabilize();
  tft.fillScreen(BG_COLOR);
  drawGrid();
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
  int miniBufferSize = 50;
  float stThreshold = 0.2;
  float stRatio = 0.0;
  while (!stabilizedFlag) {

    // get values
    noInterrupts();
    for (int i = 0; i < ST_BUFFER_SIZE; i++) {
      stBufferCopy[i] = stBuffer[i];
    }
    interrupts();

    // computer entire stability buffer avg
    bufferSum = 0;
    for (int i = 0; i < ST_BUFFER_SIZE; i++) {
      bufferSum = bufferSum + stBufferCopy[i];
    }
    bufferAvg = bufferSum / ST_BUFFER_SIZE;

    // average the last X number of data points to check with entire buffer avg
    miniBufferSum = 0;
    for (int i = 0; i < miniBufferSize; i++) {
      miniBufferSum = miniBufferSum + stBufferCopy[ST_BUFFER_SIZE - i - 1];
    }
    miniBufferAvg = miniBufferSum / miniBufferSize;

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
  if (stabilizedFlag) {
    myBuffer[bufferPos] = val;
    bufferPos++;
    if (bufferPos > BUFFER_SIZE) {
     bufferPos = 0;

    }
  } else {
    stBuffer[stBufferPos] = val;
    stBufferPos++;
    if (stBufferPos > ST_BUFFER_SIZE) {
     stBufferPos = 0;
     if (stBufferFullFlag == 0) {
       stBufferFullFlag = 1;
     }
    }
  }
}



