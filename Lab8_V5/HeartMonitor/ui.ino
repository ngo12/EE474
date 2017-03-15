/*
  UI: Contains all the user-interface screens 
  used in the ECG Program 
  
  Lab 8
  Course: EE 474

  Names and Student Numbers:
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375
*/

#include "font_AwesomeF200.h"
#include "font_AwesomeF080.h"
#include "font_RussoOne-Regular.h"

// Screen indicating that a phone must connect 
// to the bluefruit nodule via the "Bluefruit" 
// app in order to continue 
void connectFruitScreen() {
  // LCD Screen Setup
  tft.begin();
  tft.setRotation(3);
  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(BG_COLOR);

  tft.setFont(AwesomeF100_60);
  tft.setTextColor(ILI9341_BLUE);
  tft.setCursor(100, 60);
  tft.print((char)121); // from 0 to 127
  tft.setCursor(170, 65);
  tft.print((char)11); // from 0 to 127
  tft.setFont(DEF_FONT);

  tft.setCursor(20, 160);
  tft.setTextColor(TEXT_COLOR);
  tft.println("Please Connect to the" );
  tft.setCursor(20, 180);
  tft.println("Adafruit Through" );
  tft.setCursor(20, 200);
  tft.setTextColor(ILI9341_BLUE);
  tft.println("your Bluefruit App" );
}

// Screen indicating that a phone must connect 
// to the bluefruit nodule via the "nRF Toolbox" 
// app in order to continue 
void connectHBMScreen() {
  // LCD Screen Setup
  tft.begin();
  tft.setRotation(3);
  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(BG_COLOR);

  tft.setTextColor(ILI9341_BLUE);
  tft.setFont(AwesomeF080_60);
  tft.setCursor(90, 60);
  tft.print((char)45); // from 0 to 127
  tft.setFont(AwesomeF100_60);
  tft.setCursor(170, 65);
  tft.print((char)11); // from 0 to 127
  tft.setFont(DEF_FONT);

  tft.setCursor(20, 160);
  tft.setTextColor(TEXT_COLOR);
  tft.println("Please Connect to the" );
  tft.setCursor(20, 180);
  tft.println("Adafruit Through" );
  tft.setCursor(20, 200);
  tft.setTextColor(ILI9341_BLUE);
  tft.println("your nRF Toolbox App" );
}

// Screen indicating which file is currently 
// being pointed at during file recall/browsing
void chooseFileScreen() {
   tft.setFont(RussoOne_28);
  tft.setCursor(35, 10);
  tft.setTextColor(TEXT_COLOR);
  tft.println("File Browser");
  tft.setCursor(116, 60); 
  tft.setTextColor(ILI9341_GREEN);
  tft.setFont(AwesomeF100_60);
  tft.setCursor(120, 60);
  tft.print((char)91); // from 0 to 127
  tft.setFont(DEF_FONT);

  if (optionCurr == 0) {
    tft.setCursor(110, 180);
    tft.setTextColor(ILI9341_RED);
    tft.println("CANCEL");
  } else {
    tft.setCursor(70, 180);
    fileName = directory[optionCurr-1];
    tft.setTextColor(ILI9341_MAGENTA);
    tft.println(fileName);
  }
}

// Screen indicating that the file data 
// is being sent. Includes a percentage of 
// data that has been sent 
void sendingSDScreen(int percent) {
  tft.fillScreen(BG_COLOR);
  tft.setFont(DEF_FONT);
  tft.setTextSize(2);

  tft.setCursor(20, 60);
  tft.setTextColor(TEXT_COLOR);
  tft.println("Sending Data" );

  tft.setCursor(20, 80);
  tft.setTextColor(ILI9341_RED);
  tft.println("Press Button to Stop");

  tft.setCursor(140, 120);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(100);
  tft.println(String(percent) + "%");

  tft.setFont(RussoOne_28);
  tft.setCursor(135, 150);
  tft.setFont(AwesomeF100_60);
  tft.setTextColor(ILI9341_BLUE);
  tft.print((char)11); // from 0 to 127
}

/**************************************************************************/
/* Sets default screen for ECG user-interface. The option that is 
 * in green will be the option that is chosen when the button is pressed. 
 * 
 * Presents two options:
 *  1. "Measure": option for measuring the user's heart beat 
 *  2. "File Display": option for recalling a file on the SD card and 
 *     sending the data to the Bluefruit app via bluetooth 
 */
/**************************************************************************/
void defaultScreen(int option) {
  tft.setRotation(3);
  screenHeader();
  if (option <= ((4096 / 2) - 1)) {
    tft.setCursor(20, 170);
    tft.drawRoundRect(15, 165, 120, 30, 2, ILI9341_BLUE);
    tft.setTextColor(TEXT_HI);
    tft.println("Measure");
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(210, 170);
    tft.drawRoundRect(205, 165, 75, 30, 2, BG_COLOR);
    tft.println("Files" );
  } else {
    tft.setCursor(20, 170);
    tft.drawRoundRect(15, 165, 120, 30, 2, BG_COLOR);
    tft.setTextColor(TEXT_COLOR);
    tft.println("Measure");
    tft.setTextColor(TEXT_HI);
    tft.setCursor(210, 170);
    tft.drawRoundRect(205, 165, 75, 30, 2, ILI9341_BLUE);
    tft.println("Files" );
  }
}

// Draw a nice border around the edges
#define SCREEN_BORDER_COLOR ILI9341_DARKGREEN
void screenHeader() {
  // draw border
  // top

  tft.drawLine(0, 0, screenWidth - 1, 0, SCREEN_BORDER_COLOR);
  tft.drawLine(0, 0, screenWidth - 1, 1, SCREEN_BORDER_COLOR);
  tft.drawLine(0, 0, screenWidth - 1, 2, SCREEN_BORDER_COLOR);
  // bottom
  tft.drawLine(0, screenHeight - 1, screenWidth - 1, screenHeight - 1, SCREEN_BORDER_COLOR);
  tft.drawLine(0, screenHeight - 1, screenWidth - 1, screenHeight - 2, SCREEN_BORDER_COLOR);
  tft.drawLine(0, screenHeight - 1, screenWidth - 1, screenHeight - 3, SCREEN_BORDER_COLOR);
  // left
  tft.drawLine(0, 0, 0, screenHeight - 1, SCREEN_BORDER_COLOR);
  tft.drawLine(0, 0, 1, screenHeight - 1, SCREEN_BORDER_COLOR);
  tft.drawLine(0, 0, 2, screenHeight - 1, SCREEN_BORDER_COLOR);
  // right
  tft.drawLine(screenWidth - 1, 0, screenWidth - 1, screenHeight - 1, SCREEN_BORDER_COLOR);
  tft.drawLine(screenWidth - 1, 0, screenWidth - 2, screenHeight - 1, SCREEN_BORDER_COLOR);
  tft.drawLine(screenWidth - 1, 0, screenWidth - 3, screenHeight - 1, SCREEN_BORDER_COLOR);

  tft.setFont(RussoOne_28);
  tft.setCursor(118, 10);
  tft.setTextColor(TEXT_COLOR);
  tft.println("ECG");
  tft.setCursor(116, 60);
  tft.setFont(AwesomeF200_60);
  tft.setTextColor(ILI9341_RED);
  tft.print((char)30); // from 0 to 127
  tft.setFont(DEF_FONT);
}

// After ECG reading, display results
void displayResults(int hr, int qrsi, int bc, int tc, int pac) {
  tft.fillScreen(BG_COLOR);
  tft.setFont(RussoOne_28);
  tft.setCursor(100, 10);
  tft.setTextColor(TEXT_COLOR);

  tft.println("Results:");
  tft.setFont(DEF_FONT);
  tft.setCursor(10, 50);

  tft.print("Avg HR (BPM): ");
  if (hr == 0) {
    tft.print("---");
  } else {
    tft.print(hr);
  }

  tft.setCursor(10, 74);

  tft.print("Avg QRSI (ms): ");
  if (qrsi == 0) {
    tft.print("---");
  } else {
    tft.print(qrsi); 
  }

  tft.setCursor(10, 98);

  tft.print("Bradycardia: ");
  tft.setFont(AwesomeF100_18);
  if (bc) {
    tft.setTextColor(ILI9341_RED);
    tft.print((char)25);
  } else {
    tft.setTextColor(ILI9341_GREEN);
    tft.print((char)24);
  }
  tft.setFont(DEF_FONT);
  tft.setTextColor(TEXT_COLOR);
  tft.setCursor(10, 122);

  tft.print("Tachycardia: ");
  tft.setFont(AwesomeF100_18);
  if (tc) {
    tft.setTextColor(ILI9341_RED);
    tft.print((char)25);
  } else {
    tft.setTextColor(ILI9341_GREEN);
    tft.print((char)24);
  }
  tft.setFont(DEF_FONT);
  tft.setTextColor(TEXT_COLOR);
  tft.setCursor(10, 146);

  tft.print("PAC: ");
  tft.setFont(AwesomeF100_18);
  if (pac > 7) {
    tft.setTextColor(ILI9341_RED);
    tft.print((char)25);
  } else {
    tft.setTextColor(ILI9341_GREEN);
    tft.print((char)24);
  }
  tft.setFont(DEF_FONT);
  tft.print("\n");

  // Select whether to save raw or filtered data

  //  tft.setCursor(10, screenHeight-20);
  //  tft.setTextColor(ILI9341_GREEN);
  //  tft.print("Save Raw");
  //  tft.setTextColor(TEXT_COLOR);
  //  tft.setCursor(screenWidth - 130, screenHeight-20);
  //  tft.print("Save Filt.");

  // Enable ADC interrupt, configure pin to Pot
  ADC0_SC1A = ADC_SC1_AIEN | 0;
  ADC0_SC1A = ADC_SC1_AIEN | channel2sc1a[7];

  // press button to go back to main menu
  while (!buttonState) {
    if (adcValue <= ((4096 / 2) - 1)) {
      saveRaw = 1;
      tft.setCursor(10, screenHeight - 20);
      tft.setTextColor(ILI9341_GREEN);
      tft.print("Save Raw");
      tft.setTextColor(TEXT_COLOR);
      tft.setCursor(screenWidth - 130, screenHeight - 20);
      tft.print("Save Filt.");
    } else {
      saveRaw = 0;
      tft.setCursor(10, screenHeight - 20);
      tft.setTextColor(TEXT_COLOR);
      tft.print("Save Raw");
      tft.setTextColor(ILI9341_GREEN);
      tft.setCursor(screenWidth - 130, screenHeight - 20);
      tft.print("Save Filt.");
    }
    buttonState = onOff();
  }

  tft.setTextColor(TEXT_COLOR);
  tft.setFont(DEF_FONT);
  tft.fillScreen(BG_COLOR);
}


// Draw initial calibration screen
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

// Show calibration progress
int lastCountedBeats = 0;
void updateCalibrating() {
  if (countedBeats  < lastCountedBeats) {
    tft.setTextColor(BG_COLOR);
    tft.setCursor(136, 210);
    tft.print("......");
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(136, 210);

  } else if (countedBeats > lastCountedBeats) {
    tft.print("."); // print hearts
  }
  lastCountedBeats = countedBeats;
}

// Show we are done calibration on LCD
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

// Setup happens after calibration is done, and before main prog runs
void mainProgSetup() {
  tft.fillScreen(BG_COLOR);
  drawGrid();
  textResultsBorder();
  textResults(-1, -1);
  startTimer = millis();
  countPAC = 0;
  foundStableHeart = 0;
  foundStableQRS = 0;
  heartAvg = 0;
  qrsAvg = 0;
  for (int i = 0; i < 7; i++) {
    heartArray[i] = 0;
    qrsArray[i] = 0;
  }
}

// Sets the border for the text results on the
// ECG measurement screen
void textResultsBorder() {
  tft.fillRect(6, 6, screenWidth-13, 32, ILI9341_DARKGREY);
  tft.fillRect(9, 9, screenWidth-25, 26, ILI9341_BLACK);
}

// Updates the time indicator when measuring 
void updateMeasuring(unsigned long timeStamp) {
  tft.setCursor(260, 50); 
  tft.setFont(AwesomeF200_24);
  tft.setTextColor(ILI9341_RED);
  
  if (timeStamp < 6000) {
    tft.print((char)68);   
  } else if (timeStamp < 12000) {
    tft.print((char)67);  
  } else if (timeStamp < 18000) {
    tft.print((char)66);     
  } else if (timeStamp < 24000) {
    tft.print((char)65);   
  } else {
    tft.setTextColor(ILI9341_GREEN);
    tft.print((char)64); 
  }
  
  tft.setFont(DEF_FONT);
  tft.setTextColor(TEXT_COLOR);
}

