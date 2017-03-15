
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
    blueHBMInit();
    if (!cancelOption) {
      progRunning = 1;  
      // Enable ADC interrupt, configure pin to ECG output 
      ADC0_SC1A = ADC_SC1_AIEN | 0;
      ADC0_SC1A = ADC_SC1_AIEN | channel2sc1a[0];
    }
  } else {
    blueRecallInit();
    if (!cancelOption) {
      sendSD(fileName);
    } 
  }
  cancelOption = 0;
}

