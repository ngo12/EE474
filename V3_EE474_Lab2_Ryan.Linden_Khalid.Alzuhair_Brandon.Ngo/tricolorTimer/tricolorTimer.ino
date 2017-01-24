/* tricolorTimer, Lab 2 Section B 

   Course: EE 474 
   Names: Ryan Linden, Khalid Alzuhair, Brandon Ngo 
   
   Modifies IntervalTimer sketch from Section A to blink each color for 
   one second in succession: Red, Green, Blue. 
   
   Also keeps track of the three states Arduino serial debug window
   with blinkCount used as a modulo-3 counter.
*/


// Create an IntervalTimer object 
IntervalTimer myTimer;

// The pin assignment of each LED color 
const int redPin = 20;
const int greenPin = 21;
const int bluePin = 22;

// The interrupt will blink the LED, and keep
// track of how many times it has blinked.
// States for each LED color 
int redState = HIGH;
int greenState = HIGH;
int blueState = HIGH;

void setup(void) {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin(9600);
  myTimer.begin(blinkLED, 1000000);  // blinkLED to run every 1 second

  // Sets initial state with red on, green off, and blue off 
  digitalWrite(redPin, redState);
  digitalWrite(greenPin, greenState);
  digitalWrite(bluePin, blueState);
}

volatile unsigned long blinkCount = 0; // use volatile for shared variables
volatile unsigned long blinkState = 0; // use volatile for shared variables

// functions called by IntervalTimer should be short, run as quickly as
// possible, and should avoid calling other functions if possible.
void blinkLED(void) {
  if (blinkState == 0) {
    redState = LOW;
    greenState = HIGH;
    blueState = HIGH;
  } else if (blinkState == 1) {
    redState = HIGH;
    greenState = LOW;
    blueState = HIGH;
  } else if (blinkState == 2){
    redState = HIGH;
    greenState = HIGH;
    blueState = LOW;
  }

  blinkCount = blinkCount + 1;  // increase when LED turns on
  
  // Writes corresponding state to each pin 
  digitalWrite(redPin, redState);
  digitalWrite(greenPin, greenState);
  digitalWrite(bluePin, blueState);
  
}

// The main program will print the blink count
// to the Arduino Serial Monitor
void loop(void) {
  unsigned long blinkCopy;  // holds a copy of the blinkCount

  // to read a variable which the interrupt code writes, we
  // must temporarily disable interrupts, to be sure it will
  // not change while we are reading.  To minimize the time
  // with interrupts off, just quickly make a copy, and then
  // use the copy while allowing the interrupt to keep working.
  noInterrupts();
  blinkCopy = blinkCount;
  interrupts();

  blinkState = blinkCopy % 3; // Keeps track of the number of rotations through red, green, blue
  Serial.print("blinkState = ");
  Serial.println(blinkState);
  delay(100);
}
