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
#include "SD.h"
//#include "SdFat.h"
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

// For SD Card
String initials = "KARLBN";
String sRate = "250";
const int chipSelect = 4;
int fileHeadingNumber = 0;

String fileName;
char filename[20];
int fileCount;
String directory[128];

//SdFat sd; // File system object.

File root;
File myFile;  

int optionCurr;
int optionPrev;
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
  //Serial.println(err);
  while (1);
}

/**************************************************************************/
/* Intializes Adafruit Blutooth module 
 */
/**************************************************************************/
void directoryPrint(File dir, int numTabs);
void blueInit() {
  Serial.begin(9600);
  //Serial.println(F("Adafruit Bluefruit Command Mode Example"));
  //Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  //Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  //Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    //Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  ble.sendCommandCheckOK(F("AT+GAPDEVNAME=KARLBN ECG"));

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  //Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  //Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  //Serial.println(F("Then Enter characters to send to Bluefruit"));
  //Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    //Serial.println(F("******************************"));
    //Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    //Serial.println(F("******************************"));
  }
 
}

// Bluetooth data transmission
#define BUFFERSIZE 8500
int bufferpos = 0; 
int readFile = 1; 
int count = 0;
char inputs[BUFFERSIZE+1];

// SD Card 
int n;

/**************************************************************************/
/* Intializes SD card 
 */
/**************************************************************************/
void sdInit() {
     // Setup SD Card
  if (!SD.begin(chipSelect)) {
    //Serial.println("initialization failed!");
    return;
  } else {
    //Serial.println("initialization done.");
  }
  root = SD.open("/ECGData");
}

/**************************************************************************/
/* Sets screen on LCD display requesting that the 
 * user connect to the Adafruit Bluetooth module 
 * through the Bluefruit App on their phone 
 */
/**************************************************************************/
void connectScreen() {
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
  int bufferCounter = 0;
  chooseFile(adcValue);
  //Serial.println(optionCurr);
  if (optionCurr != 0) {  
    fileName = "ECGData/" + fileName;
    myFile = SD.open(fileName, FILE_READ);
  
    // if the file opened okay, write to it:
    if (myFile) {
      //Serial.print("Reading...\n");
  
      // read from the file until there's nothing else in it:
      String received = "";
      char ch;
      
      myFile.parseInt();
      myFile.parseInt();
      
      ////Serial.println("INCREMENTING");
      while (myFile.available()) {
        // Reads data from file,
        // converts the value to a string,
        // and appends a comma and newline 
        // before sending over bluetooth
        ////Serial.println(value);
        myBuffer[bufferCounter] = myFile.parseInt();
        bufferCounter++;
        ////Serial.println(myBuffer[i]);
      }

      sendingSDScreen();
      for (int i = 0; i < bufferCounter; i++) { 
        buttonState = onOff();
        
        if (buttonState) {
          buttonState = 0;
          break;
        }
        
        data = String(myBuffer[i]) + ",\\n";
        
        // Clears the data buffer 
        memset(inputs, 0, BUFFERSIZE);
        
        appendToBuffer(inputs, data);
        ////Serial.println(inputs); 
        bufferpos = 0;
          
        ble.print("AT+BLEUARTTX=");
        ble.println(inputs);
        delay(30);
      }
    }
  }
}

/**************************************************************************/
/* Sets the LCD display informing the user that the 
 * ECG program is sending data from a file on the 
 * on the SD card to the Bluefruit App on a phone
 * during a file recall. 
 */
/**************************************************************************/
void sendingSDScreen() {
  tft.fillScreen(BG_COLOR);
  tft.setCursor(0, 60);
  tft.setTextColor(ILI9341_BLACK);
  tft.println("Sending Data" );
  tft.setCursor(0, 80);
  tft.setTextColor(ILI9341_RED);
  tft.println("Press Button to Stop");
}

void writeCard(int* buff, int bufferLength) {

  String fileName = "ECGData/KARLBN" + (String)fileHeadingNumber;
  fileName = fileName + ".txt";
  tft.setCursor(90, 100);
  tft.setTextColor(ILI9341_BLUE);
  tft.println(fileName);
  int fileNameLength = fileName.length() + 1;
  char file[fileNameLength + 8];
  fileName.toCharArray(file, fileNameLength);

  String header = initials + fileHeadingNumber;
  header = header + ", ";
  header = header + sRate;
  int headerLength = header.length() + 1;
  char headerCharArray[headerLength];
  header.toCharArray(headerCharArray, headerLength);

  ////Serial.println(file);
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  SD.remove(file);
  myFile = SD.open(file, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    //Serial.print("Writing...");
    myFile.println(headerCharArray);
    for (int i = 1; i <= bufferLength; i++) {
      myFile.print(buff[i - 1]);

      if (i % 8 == 0 && i != 1) {
        myFile.println();
        // if not last item, print comma
      } else if (bufferLength != i) {
        myFile.print(", ");
      }
    }
    myFile.println();
    myFile.println("EOF");
    // close the file:
    myFile.close();
    ////Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    //Serial.println("error opening file");
  }
  fileHeadingNumber++;
}

void directoryPrint(File dir, int numTabs) {
  while (true) {
    
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      //Serial.print('\t');
    }
    
    if (entry.isDirectory()) {
      ////Serial.println("/");
      directoryPrint(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      fileName = entry.name();
      ////Serial.print(fileName);
      directory[fileCount] = fileName;
      ////Serial.print("\t\t");
      ////Serial.println(entry.size(), DEC);
      fileCount++;
    }
    entry.close();
  }
}

void chooseFile(int option) {
  int chosen = 1; 
  int mappedOption; 
  fileCount = 0;
  //Serial.print("Before Count: ");
  //Serial.println(fileCount);
  root.rewindDirectory();
  directoryPrint(root, 0);
  //Serial.print("After Count: ");
  //Serial.println(fileCount);

  tft.fillScreen(BG_COLOR);
  tft.setCursor(0, 80);
  
  if (optionCurr == 0) {
    tft.setTextColor(ILI9341_RED);
    tft.println("CANCEL");
  } else {
    fileName = directory[optionCurr-1];
    tft.setTextColor(ILI9341_BLACK);
    tft.println(fileName);
  }
  
  while (chosen) {
    ////Serial.print("In Loop Count: ");
    ////Serial.println(fileCount);
    buttonState = onOff();
    ////Serial.println(buttonState);
    if (buttonState) {
      buttonState = 0;
      chosen = 0;
    }
    
    mappedOption = map(adcValue, 0, 4096, 0, fileCount+1);
    optionCurr = constrain(mappedOption, 0, fileCount);

    if (optionCurr != optionPrev) {
      tft.fillScreen(BG_COLOR);
      tft.setCursor(0, 80);
      if (optionCurr == 0) {
        tft.setTextColor(ILI9341_RED);
        tft.println("CANCEL");
      } else {
        //getFileName(optionCurr-1); 
        fileName = directory[optionCurr-1];
        tft.setTextColor(ILI9341_BLACK);
        tft.println(fileName);
      }
    }
    optionPrev = optionCurr; 
  }
}

void blueECGData() {
  // Check for user input
  String BPM = "HEART RATE (BPM): "; 
  String BPMVal = String(heartRate) + "\\n";  
  String QRS = "QRZ Int(ms): "; 
  String QRSVal = String(QRScopy) + "\\n";  
  String newpage = "\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n\\n"; 

  // Clears the data buffer 
  memset(inputs, 0, BUFFERSIZE);
  
  // Adds the desired data to be sent and displayed 
  // on the phone via bluetooth 
  appendToBuffer(inputs, BPM);
  appendToBuffer(inputs, BPMVal);
  appendToBuffer(inputs, QRS);
  appendToBuffer(inputs, QRSVal);
  appendToBuffer(inputs, newpage);

  // Send characters to Bluefruit
  //Serial.print("[Send] ");
  //Serial.println(inputs);
  
  ble.print("AT+BLEUARTTX=");
  ble.println(inputs);
  
  // Resets the buffer position for the new 
  // data in the next loop 
  bufferpos = 0; 
}


