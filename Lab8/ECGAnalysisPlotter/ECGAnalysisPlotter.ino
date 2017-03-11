/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         1
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(Serial1, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
const int chipSelect = 4;

void setup(void)
{
     // Setup SD Card
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  } else {
    Serial.println("initialization done.");
  }
  
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Command Mode Example"));
  Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("******************************"));
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    Serial.println(F("******************************"));
  }
 
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
// Bluetooth data transmission
int bufferpos = 0; 
int readFile = 1; 
int BUFFERSIZE = 8500;
int count = 0;

// SD Card 
int fileHeadingNumber = 0;
String fileName = "KARLBN0.txt";
File myFile;
char line[1000];
int n;

void loop(void)
{ 
  char lineChar;
  
  if (readFile) {
    delay(5000);
    sendSD(fileName);
    
//    int i = 0;
//    //readSD(fileName);
//    myFile = SD.open(fileName, FILE_READ);
//
//    // if the file opened okay, write to it:
//    if (myFile) {
//      while (myFile.available()) {
//        while (lineChar!='\n') {
//          lineChar = myFile.read();
//          line[i]=lineChar;
//          i++;
//        }
//        Serial.println(line);
//      }
//       
//    }
    
    readFile = 0;
  }
//  if (readFile) {
//    int number = random(100);
//    
//    String data1 = String(number) + ",\\n";
//      
//    // Adds the desired data to be sent and displayed 
//    // on the phone via bluetooth 
//    appendToBuffer(inputs, data1, BUFFERSIZE);
//    
//    // Send characters to Bluefruit
//    Serial.print("[Send] ");
//    Serial.println(inputs);
//  
//    ble.print("AT+BLEUARTTX=");
//    ble.println(inputs);
//  
//    // check response stastus
//    if (! ble.waitForOK() ) {
//      Serial.println(F("Failed to send?"));
//    }
//    count++;
//    if (count == 8000) {
//      readFile = 0;
//    }
//
//}


  
}

/**************************************************************************/
/* Appends data as a string into a character buffer. 
 * The character buffer is used to send data via bluetooth.
 * 
 * @param buffer[]: the buffer of data to be appended to 
 * @param data: the data to be appended 
 */
/**************************************************************************/
bool appendToBuffer(char buffer[], String data)
{   
  // Clears the data buffer 
  memset(buffer, 0, BUFFERSIZE);
  
  for (int i = 0; i < data.length(); i++) {
    buffer[bufferpos] = data.charAt(i); 
    bufferpos++;
  }
  
  return true;
}

/**************************************************************************/
/* Sends the data in the SD card via bluetooth to a phone. 
 * Allows scrolling of display data left and right after capture.
 * 
 * @param filename: the name of the file in the SD card that will be sent
 * and displayed on the phone via bluetooth
 */
/**************************************************************************/
void sendSD(String filename) {
  int i = 0;
  int value;
  String data;
  char inputs[BUFFERSIZE+1];
  
  myFile = SD.open(filename, FILE_READ);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Reading...\n");

    // read from the file until there's nothing else in it:
    String received = "";
    char ch;
    
    myFile.parseInt();
    myFile.parseInt();
    
    while (myFile.available()) {
      
      // Reads data from file,
      // converts the value to a string,
      // and appends a comma and newline 
      // before sending over bluetooth
      value = myFile.parseInt();
      data = String(value) + ",\\n";    
      appendToBuffer(inputs, data);
      bufferpos = 0;
      
      // Send characters to Bluefruit
      // Serial.print("[Send] ");
      // Serial.println(inputs);
      
      ble.print("AT+BLEUARTTX=");
      ble.println(inputs);
      delay(30);
    }   
  }
}
