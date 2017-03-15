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
}

/**************************************************************************/
/* Chooses the corresponding action according to the value of 
 * the potentiometer when the button is pressed.
 * 
 * When the default screen is showing, the potentiometer is used to 
 * scroll between the two available options. 
 */
/**************************************************************************/
void chooseOption(int option) {
  if (option <= ((4096/2) - 1)) { 
    progRunning = 1;  
    // Enable ADC interrupt, configure pin to ECG output 
    ADC0_SC1A = ADC_SC1_AIEN | 0;
    ADC0_SC1A = ADC_SC1_AIEN | channel2sc1a[0];
  } else {
    sendSD(fileName);
  }
}

