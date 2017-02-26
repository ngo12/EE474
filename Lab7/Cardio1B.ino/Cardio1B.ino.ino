/*
  SD card read/write
 
 This example shows how to read and write data to and from an SD card file   
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11, pin 7 on Teensy with audio board
 ** MISO - pin 12
 ** CLK - pin 13, pin 14 on Teensy with audio board
 ** CS - pin 4, pin 10 on Teensy with audio board
 
 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 This example code is in the public domain.
   
 */
 
#include <SD.h>
#include <SPI.h>
#include <ILI9341_t3.h>

#define TFT_DC  9
#define TFT_CS 10
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

File myFile;
String initials = "KARLBN";
String sRate = "250";

const int chipSelect = 4;

int bufferLength = 20;
int tempBuffer[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

int fileHeadingNumber = 0;

void setup()
{
 //UNCOMMENT THESE TWO LINES FOR TEENSY AUDIO BOARD:
 //SPI.setMOSI(7);  // Audio shield has MOSI on pin 7
 //SPI.setSCK(14);  // Audio shield has SCK on pin 14
  
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  tft.begin();
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  tft.fillScreen(ILI9341_RED);

  writeCard(tempBuffer, bufferLength);
  writeCard(tempBuffer, bufferLength);

}

void loop()
{
  // nothing happens after setup
}

void writeCard(int* buff, int bufferLength) {

  String fileName = "KARLBN" + (String)fileHeadingNumber;
  fileName = fileName + ".txt";

  int fileNameLength = fileName.length()+1;
  char file[fileNameLength];
  fileName.toCharArray(file, fileNameLength);

  String header = initials + fileHeadingNumber;
  header = header + ", ";
  header = header + sRate;
  int headerLength = header.length()+1;
  char headerCharArray[headerLength];
  header.toCharArray(headerCharArray, headerLength);
  
  Serial.println(file);
    // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  SD.remove(file);
  myFile = SD.open(file, FILE_WRITE);

  //int bufferLength = 20;
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing...");
    myFile.println(headerCharArray);
    for (int i=0; i < bufferLength; i++) {
      myFile.print(buff[i]);

      if (i % 8 == 0 && i != 0) {
        myFile.println();
      // if not last item, print comma
      } else if (bufferLength != i+1) {
        myFile.print(", ");
      }
    }
    myFile.println();
    myFile.println("EOF");
  // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
  
  // re-open the file for reading:
  myFile = SD.open(file);
  if (myFile) {
    Serial.print(file);
    Serial.println(": ");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
  fileHeadingNumber++;
}
