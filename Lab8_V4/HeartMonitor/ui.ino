/*
  UI
  Lab 8
  Course: EE 474

  Names and Student Numbers:
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375
*/

#include "font_AwesomeF200.h"
#include "font_RussoOne-Regular.h"

// First screen, must connect to bluefruit to continue
void connectScreen2() {
  // LCD Screen Setup
  tft.begin();
  tft.setRotation(3);
  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(BG_COLOR);
  screenHeader();
  tft.setCursor(20, 160);
  tft.setTextColor(TEXT_COLOR);
  tft.println("Please Connect to the" );
  tft.setCursor(20, 180);
  tft.println("Adafruit Through" );
  tft.setCursor(20, 200);
  tft.setTextColor(ILI9341_BLUE);
  tft.println("your Bluefruit App" );

}

// Displays while sending data through bluefruit app
void sendingSDScreen2() {
  tft.fillScreen(BG_COLOR);
  screenHeader();
  tft.setCursor(20, 60);
  tft.setTextColor(TEXT_COLOR);
  tft.println("Sending Data" );
  tft.setCursor(20, 80);
  tft.setTextColor(ILI9341_RED);
  tft.println("Press Button to Stop");
}


// Start screen of the main program
void defaultScreen2(int option) {
  screenHeader();
  if (option <= ((4096/2) - 1)) { 
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
void screenHeader(){
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
  tft.print(hr);
  tft.setCursor(10, 74);
  
  tft.print("Avg QRSI (ms): ");
  tft.print(qrsi);
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
  if (pac) {
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
    if (adcValue <= ((4096/2) - 1)) {
      saveRaw = 1;
      tft.setCursor(10, screenHeight-20);
      tft.setTextColor(ILI9341_GREEN);
      tft.print("Save Raw");
      tft.setTextColor(TEXT_COLOR);
      tft.setCursor(screenWidth - 130, screenHeight-20);
      tft.print("Save Filt.");
    } else {
      saveRaw = 0;
      tft.setCursor(10, screenHeight-20);
      tft.setTextColor(TEXT_COLOR);
      tft.print("Save Raw");
      tft.setTextColor(ILI9341_GREEN);
      tft.setCursor(screenWidth - 130, screenHeight-20);
      tft.print("Save Filt.");
    }
    buttonState = onOff();
  }

  tft.setTextColor(TEXT_COLOR);
  tft.setFont(DEF_FONT);
  tft.fillScreen(BG_COLOR);
}

