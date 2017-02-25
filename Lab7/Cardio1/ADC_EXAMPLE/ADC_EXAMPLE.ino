
#include "ADC.h"
#include <SPI.h>
#include <IntervalTimer.h>

// ADC constants
const int readPin = A0; // Buffered input goes here
const int timerInterval = 10; // us

// LCD constants
const int lcdpin_ss = 1;   // slave select
const int lcdpin_sck = 3;  // serial clk
const int lcdpin_si = 4;   // slave in
const int lcdpin_so = 2;   // slave out


ADC *adc = new ADC();
IntervalTimer myTimer; // timer used to get ADC readings

/// ADC variables ///
// max value of ADC, 12 bits -> 4095
int adcMax;
// threshold that considers a signal to be high, in ADC value range...not voltage range
float thresh;
// flag that tells us if the last threshold crossing was a pos edge
volatile int posEdgeTrigger = 0;
// adc reading and its copy variable, copy used in actual freq, volt calculations
volatile unsigned long adc_value;
unsigned long adc_value_copy;
// period of the signal and its copy, copy used in calculations
volatile unsigned long signalPeriod;
unsigned long signalPeriodCopy;
// ensures we record the time of last pos edge trigger for later period calculations
volatile unsigned long signalTimer;
// final volt measurement, converted to + - 6V
float volts;
// final freq measurement, converted to Hz
float freq;

void setup() {

  /// ADC INIT ///
  pinMode(readPin, INPUT); // setup for input signal pin
  adcInit();

  /// LCD INIT ///
  // 100ms delay needed for lcd to initialize
  delay(100);
  // Initialize display pins
  pinMode(lcdpin_ss, OUTPUT);
  pinMode(lcdpin_sck, OUTPUT);
  pinMode(lcdpin_si, OUTPUT);
  pinMode(lcdpin_so, INPUT);
  // Start SPI library
  SPI.begin();
  // Initializes display with proper text labels
  // See function definition below for details
  displayInit();

}

void loop() {

  // copy values while no interrupts can interfere
  noInterrupts();
  adc_value_copy = adc_value;
  signalPeriodCopy = signalPeriod; // us
  interrupts();

  // convert volts to + - 6V
  volts = ( (1.0 * adc_value_copy / adcMax) * 12.0 ) - 6.0;
  // convert frequency from the period (in microseconds) to Hz
  freq = 1.0 / ( signalPeriodCopy * 0.000001 );

  Serial.print("Freq: ");
  Serial.print(freq);
  Serial.println(" Hz");
  Serial.print("Volts: ");
  Serial.print(volts);
  Serial.println(" V");

}

//////////////////////////////////////////////////////////////////////////
// ADC
//////////////////////////////////////////////////////////////////////////

// ADC INITIALIZATION
void adcInit() {
  adc->setReference(ADC_REF_3V3, ADC_0); // make sure we are in right voltage range
  adc->setAveraging(4); // set number of averages
  adc->setResolution(12); // set bits of resolution
  // setup sampling speed and the speed of calculating the adc value
  adc->setConversionSpeed(ADC_HIGH_SPEED);
  adc->setSamplingSpeed(ADC_HIGH_SPEED);

  // start timer for the interrupt that starts reading the adc value
  myTimer.begin(adc0_start_reading, timerInterval);

  // start our first adc read
  adc->startSingleRead(readPin, ADC_0);
  // enable interrupts, adc0_isr() is triggered when adc value has been calculated
  adc->enableInterrupts(ADC_0);

  // use adc library function to get max value of the adc reading
  adcMax = adc->getMaxValue();
  // calculate our threshold to compare to adc values, threshold = 0.5V
  thresh = (0.5 / 3.3) * adcMax;
}

// This interrupt is a timed interrupt using the InvervalTimer object.
// We start the timer in the setup function to trigger at the specified
// rate to sample our data.
void adc0_start_reading() {
  adc->startSingleRead(readPin, ADC_0);
}

// This interrupt is automatically called once the reading from adc0_start_reading()
// has finished calculations.
//
// We get the ADC reading and check for a positive edge by making sure it passes
// the threshold value and that our posEdgeTrigger flag was previously 0.
// Using the micros() function we time the interval in between the posEdgeTriggers
// and that is the period of the signal
void adc0_isr() {
  if(!adc->adc0->isConverting()) {
    adc_value = adc->readSingle(ADC_0);
    if (posEdgeTrigger == 0 && adc_value > (int)thresh) {
      posEdgeTrigger == 1;
      signalPeriod = micros() - signalTimer;
      signalTimer = micros();
    } else if ( posEdgeTrigger == 1 && (adc_value < (int)thresh) ){
      posEdgeTrigger = 0;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// LCD
//////////////////////////////////////////////////////////////////////////

// Initialize the display by writing the Freq and Volts labels
void displayInit(){
  // Set chip select low to enable transfer
  digitalWrite(lcdpin_ss, LOW);
  // Clear screen and set cursor to home
  SPI.transfer(0xFE);
  SPI.transfer(0x51);
  delay(1.5);

  // Write "Freq:" to display
  // Each character written is followed by a move cursor right command
  LCD_writeChar(0x46); // F
  LCD_writeChar(0x72); // r
  LCD_writeChar(0x65); // e
  LCD_writeChar(0xF1); // q
  LCD_writeChar(0x3A); // :

  // Set cursor to beginning of second row
  SPI.transfer(0xFE);
  SPI.transfer(0x45);
  SPI.transfer(0x40);
  delayMicroseconds(100);

  // Write "Volts:" to display on second row
  // Each character written is followed by a move cursor right command
  LCD_writeChar(0x56); // V
  LCD_writeChar(0x6F); // o
  LCD_writeChar(0x6C); // l
  LCD_writeChar(0x74); // t
  LCD_writeChar(0x73); // s
  LCD_writeChar(0x3A); // :

  // Done with LCD setup
  digitalWrite(lcdpin_ss, HIGH);
}

// Write voltage to LCD, e.g. 2.00V
void LCD_writeVolts(float v){
  // v*100 lets us easily get the decimal digits into their own variable
  int v_int = (int)(v*100.0);
  // Get each individual digit from the volt reading
  int digit_ones = (v_int / 100) % 10;
  int digit_decimal1 = (v_int / 10) % 10;
  int digit_decimal2 = v_int % 10;

  // Add 0x40 to get the value the LCD needs to display the digit
  digit_ones = 0x40 + digit_ones;
  digit_decimal1 = 0x40 + digit_decimal1;
  digit_decimal2 = 0x40 + digit_decimal2;

  // Set chip select low to enable transfer
  digitalWrite(lcdpin_ss, LOW);

  // Set cursor to proper position
  SPI.transfer(0xFE);
  SPI.transfer(0x45);
  SPI.transfer(0x4A);
  delayMicroseconds(100);

  if (v < 0.0) {
  LCD_writeChar(0xB0); // -
  } else {
  LCD_writeChar(0x10); // blank
  }

  LCD_writeChar(digit_ones);
  LCD_writeChar(0x2E); // .
  LCD_writeChar(digit_decimal1);
  LCD_writeChar(digit_decimal2);
  LCD_writeChar(0x56); // V

  digitalWrite(lcdpin_ss, HIGH);
}

// Write frequency to the LCD, e.g 10000.00Hz
void LCD_writeFreq(float f){
  // Get each single digit of the freq value
  int digit_tenthousand = (f / 10000) % 10;
  int digit_thousand = (f / 1000) % 10;
  int digit_hundred = (f / 100) % 10;
  int digit_tens = (f / 10) % 10;
  int digit_ones = (f / 1) % 10;
  f = f * 100; // move first two decimal digits over for easy retreival
  int digit_decimal1 = (f / 10) % 10;
  int digit_decimal2 = f % 10;

  // Add 0x40 to get the value the LCD needs to display the digit
  digit_tenthousand = 0x40 + digit_tenthousand;
  digit_thousand = 0x40 + digit_thousand;
  digit_hundred = 0x40 + digit_hundred;
  digit_tens = 0x40 + digit_tens;
  digit_ones = 0x40 + digit_ones;
  digit_decimal1 = 0x40 + digit_decimal1;
  digit_decimal2 = 0x40 + digit_decimal2;

  // Set chip select low to enable transfer
  digitalWrite(lcdpin_ss, LOW);

  // Set cursor to proper position
  SPI.transfer(0xFE);
  SPI.transfer(0x45);
  SPI.transfer(0x06);
  delayMicroseconds(100);

  LCD_writeChar(digit_tenthousand);
  LCD_writeChar(digit_thousand);
  LCD_writeChar(digit_hundred);
  LCD_writeChar(digit_tens);
  LCD_writeChar(digit_ones);
  LCD_writeChar(0x2E); // .
  LCD_writeChar(digit_decimal1);
  LCD_writeChar(digit_decimal2);
  LCD_writeChar(0x48); // H
  LCD_writeChar(0x7A); // z

  // Done with LCD setup
  digitalWrite(lcdpin_ss, HIGH);
}

// Only if ssPin is low, it writes letter to LCD and shifts cursor right
void LCD_writeChar(int letterCode) {

  if (digitalRead(lcdpin_ss) == 1) {
    // Slave select needs to be low...
    return;
  }

  // delays needed to make sure signals arent sent too fast
  SPI.transfer(letterCode);
  delayMicroseconds(100);
  SPI.transfer(0xFE); // move cursor right
  SPI.transfer(0x4A);
  delayMicroseconds(100);

}
