/* PWMtimer, Lab 2 Section C Part 1

   Course: EE 474 
   Names: Ryan Linden, Khalid Alzuhair, Brandon Ngo 
   
   This code modifies the tricolorTimer.ino 
   sketch from Lab 2 Section B 

   Fires the timer every 10 milliseconds so that a variable 
   for each LED in succession RED fades up (255 to 0) and
   then down (0 to 255), followed by Green fading up and down, 
   and then by Blue fading up and down, and the cycle repeats.  
*/


// Create an IntervalTimer object
IntervalTimer myTimer;

// Pinouts
const int redPin = 20;
const int greenPin = 21;
const int bluePin = 22;

void setup(void) {

    // Setup LEDs to be outputs
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    // Turn off LEDS at start
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);

    Serial.begin(9600);
    // Begin IntervalTimer
    myTimer.begin(fadeLED, 10000);  // fadeLED to run every 10ms

}

volatile unsigned long fadeCount = 255; // use volatile for shared variables
int fadeDir = 1; // 1 increments, -1 decrements
int selectedPin = 0; // 0 is no pin selected, 20=red, 21=green, 22=blue

// Fades the led and changes color when done fading
void fadeLED(void) {

    analogWrite(selectedPin, fadeCount);
    
    if (fadeCount >= 255) {
        fadeDir = fadeDir * -1;
        if (selectedPin == 0) {
            selectedPin = redPin;
        } else if (selectedPin == redPin) {
            selectedPin = greenPin;
            digitalWrite(redPin, HIGH);
        } else if (selectedPin == greenPin) {
            selectedPin = bluePin;
            digitalWrite(greenPin, HIGH);
        } else if (selectedPin == bluePin) {
            selectedPin = redPin;
            digitalWrite(bluePin, HIGH);
        }
    }
    else if (fadeCount <= 0) {
        fadeDir = fadeDir * -1;
    }
    
    fadeCount = fadeCount + 1 * fadeDir;

}

// The main program will print the fade count
// to the Arduino Serial Monitor
void loop(void) {
    unsigned long fadeCopy;  // holds a copy of the fadeCount

    // to read a variable which the interrupt code writes, we
    // must temporarily disable interrupts, to be sure it will
    // not change while we are reading.  To minimize the time
    // with interrupts off, just quickly make a copy, and then
    // use the copy while allowing the interrupt to keep working.
    noInterrupts();
    fadeCopy = fadeCount;
    interrupts();


    Serial.print("fadeCount = ");
    Serial.println(fadeCopy);
    delay(100);
}

