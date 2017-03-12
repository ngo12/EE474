

#include "font_AwesomeF200.h"
#include "font_Iceland-Regular.h"




void connectScreen2() {
  // LCD Screen Setup
  tft.begin();
  tft.setRotation(3);
  tft.setTextSize(2);
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(BG_COLOR);
  tft.fillScreen(BG_COLOR);

  tft.setCursor(0, 60);
  tft.setTextColor(ILI9341_BLACK);
  tft.println("Please Connect to the" );
  tft.setCursor(0, 80);
  tft.println("Adafruit Through" );
  tft.setCursor(0, 100);
  tft.setTextColor(ILI9341_BLUE);
  tft.println("your Bluefruit App" );

}

void sendingSDScreen2() {
  tft.fillScreen(BG_COLOR);
  tft.setCursor(0, 60);
  tft.setTextColor(ILI9341_BLACK);
  tft.println("Sending Data" );
  tft.setCursor(0, 80);
  tft.setTextColor(ILI9341_RED);
  tft.println("Press Button to Stop");
}

void defaultScreen2(int option) {
  tft.setFont(Iceland_12);
  if (option <= ((4096/2) - 1)) { 
    tft.setCursor(0, 60);
    tft.setTextColor(ILI9341_GREEN);
    tft.println("Measure");
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(0, 80);
    tft.println("Display File" );
  } else {
    tft.setCursor(0, 60);
    tft.setTextColor(ILI9341_BLACK);
    tft.println("Measure");
    tft.setTextColor(ILI9341_GREEN);
    tft.setCursor(0, 80);
    tft.println("Display File" );
  }
  tft.setFont(AwesomeF200_18);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(40);
  tft.println();
  tft.print((char)30); // from 0 to 127
  tft.setTextSize(2);
  tft.setFont(Iceland_20);
  
}
