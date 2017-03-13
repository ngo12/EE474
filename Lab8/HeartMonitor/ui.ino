

#include "font_AwesomeF200.h"
#include "font_RussoOne-Regular.h"




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

void defaultScreen2(int option) {
  screenHeader();
  if (option <= ((4096/2) - 1)) { 
    tft.setCursor(20, 160);
    tft.setTextColor(TEXT_HI);
    tft.println("Measure");
    tft.setTextColor(TEXT_COLOR);
    tft.setCursor(140, 160);
    tft.println("Display File" );
  } else {
    tft.setCursor(20, 160);
    tft.setTextColor(TEXT_COLOR);
    tft.println("Measure");
    tft.setTextColor(TEXT_HI);
    tft.setCursor(140, 160);
    tft.println("Display File" );
  }
}

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


void displayResults() {
  
}

