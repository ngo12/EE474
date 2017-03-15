/*
  fileRecall: Used for recalling a file during the 
  ECG Program. This file contains a mix of bluetooth
  and SD card actions. Bluetooth for sending the file 
  data over to a phone via bluetooth and SD card for 
  writing and reading a file. 
  
  Lab 8
  Course: EE 474

  Names and Student Numbers:
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375
*/

#include <Arduino.h>
#include "SD.h"
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

int cancelOption = 0;
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
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  while (1);
}

void directoryPrint(File dir, int numTabs);

// Intializes Adafruit Blutooth module 
void blueRecallInit() {
  connectFruitScreen();
  Serial.begin(9600);

  /* Initialise the module */
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  ble.sendCommandCheckOK(F("AT+GAPDEVNAME=KARLBN ECG"));

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  /* Print Bluefruit information */
  ble.info();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
    buttonState = onOff();
    
    if (buttonState) {
      buttonState = 0;
      cancelOption = 1;
      break;
    }
  }

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
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

// Intializes SD card 
void sdInit() {
     // Setup SD Card
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  } else {
    Serial.println("initialization done.");
  }
  root = SD.open("/ECGData");
}

// Sets screen on LCD display requesting that the 
// user connect to the Adafruit Bluetooth module 
// through the Bluefruit App on their phone 
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

// Appends data as a string into a character buffer. 
// The character buffer is used to send data via bluetooth.
//
// @param buffer[]: the buffer of data to be appended to 
// @param data: the data to be appended 
bool appendToBuffer(char buffer[], String data)
{   
  
  for (int i = 0; i < data.length(); i++) {
    buffer[bufferpos] = data.charAt(i); 
    bufferpos++;
  }
  
  return true;
}

// Sends the data in the SD card via bluetooth to a phone. 
// Allows scrolling of display data left and right after capture.
//
// @param filename: the name of the file in the SD card that will be sent
// and displayed on the phone via bluetooth
void sendSD(String filename) {
  int i = 0;
  int printCounter = 1;
  int printPercent = (7500/100);
  int value;
  String data;
  int bufferCounter = 0;
  chooseFile(adcValue);
  if (optionCurr != 0) {  
    fileName = "ECGData/" + fileName;
    myFile = SD.open(fileName, FILE_READ);
  
    // if the file opened okay, write to it:
    if (myFile) {
  
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
        myBuffer[bufferCounter] = myFile.parseInt();
        bufferCounter++;
      }

      for (int i = 0; i < bufferCounter; i++) {
        if (((printPercent*printCounter) - 1) == i && printCounter < 101) {
          printCounter++;
          sendingSDScreen(printCounter);
        } 
        buttonState = onOff();
        
        if (buttonState) {
          buttonState = 0;
          break;
        }
        
        data = String(myBuffer[i]) + ",\\n";
        
        // Clears the data buffer 
        memset(inputs, 0, BUFFERSIZE);
        
        appendToBuffer(inputs, data);
        bufferpos = 0;
          
        ble.print("AT+BLEUARTTX=");
        ble.println(inputs);
        delay(30);
      }
    }
  }
}

// Writes to the SD card the ECG measurement 
// data and Arythmia detection 
void writeCard(int* buff, int bufferLength) {
  String origFileName = "KARLBN" + (String)fileHeadingNumber + ".txt";
  String fileName = "ECGData/" + origFileName;
  
  tft.setCursor(90, 100);
  tft.setTextColor(ILI9341_BLUE);
  tft.println(origFileName);
  int fileNameLength = fileName.length() + 1;
  char file[fileNameLength + 8];
  fileName.toCharArray(file, fileNameLength);

  String header = initials + fileHeadingNumber;
  header = header + ", ";
  header = header + sRate;
  int headerLength = header.length() + 1;
  char headerCharArray[headerLength];
  header.toCharArray(headerCharArray, headerLength);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  SD.remove(file);
  myFile = SD.open(file, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println(headerCharArray);
    if (tachycardia || bradycardia) {
      myFile.println("Arythmia: Detected");
    } else {
      myFile.println("Arythmia: Not Detected");
    }
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
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
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
    
    if (entry.isDirectory()) {
      directoryPrint(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      fileName = entry.name();
      directory[fileCount] = fileName;
      fileCount++;
    }
    entry.close();
  }
}

// Determine which file is chosen during file recall
// based on the value of option
// @param int option: the adc value read from the ADC PDB
void chooseFile(int option) {
  int chosen = 1; 
  int mappedOption; 
  fileCount = 0;
  root.rewindDirectory();
  directoryPrint(root, 0);

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

  tft.fillScreen(BG_COLOR);
  tft.setTextSize(90);
  chooseFileScreen();
  while (chosen) {
    buttonState = onOff();
    if (buttonState) {
      buttonState = 0;
      chosen = 0;
    }
    
    mappedOption = map(adcValue, 0, 4096, 0, fileCount+1);
    optionCurr = constrain(mappedOption, 0, fileCount);

    if (optionCurr != optionPrev) {
      tft.fillScreen(BG_COLOR);
      tft.setTextSize(90);
      chooseFileScreen();
    }
    optionPrev = optionCurr; 
  }
}
