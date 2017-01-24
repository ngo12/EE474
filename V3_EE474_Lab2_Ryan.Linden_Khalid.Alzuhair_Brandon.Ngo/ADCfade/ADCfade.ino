/* ADCfade, Lab 2 Section C Part 2
   Course: EE 474 
   Names: Ryan Linden, Khalid Alzuhair, Brandon Ngo 
   
   This code modifies the PWMtimer.ino 
   sketch from Lab 2 Section C Part 1 
   
   We incorporated our potentiometer
   to change the fade rate
   
   This was done by mapping the value of the 
   potentiometer to a value between 0 and 10 
   using map(). Afterwards, the mapped value was 
   used to increment the the duty cycle: between 0 
   (always off) and 255 (always on). 

   The quicker or slower increment would change the 
   fade rate accordingly. 
*/

// Create an IntervalTimer object
IntervalTimer myTimer;

// Pinouts
const int redPin = 20;
const int greenPin = 21;
const int bluePin = 22;

// Value set by potentiometer
volatile int val; 
volatile int valCopy;

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
    myTimer.begin(fadeLED, 10000);  // fadeLED to run every 10us

}

volatile long fadeCount = 255; // use volatile for shared variables
volatile int fadeDir = 1; // 1 increments, -1 decrements
volatile int selectedPin = 0; // 0 is no pin selected, 20=red, 21=green, 22=blue

// Fades the led and changes color when done fading
void fadeLED(void) {
      
    // Sets delay between each loop 
    // Delay value is set according to potentiometer
    // and multiplied by 10 due to use of delayMicroseconds
    // this results in a fade rates similiar to PWMtimer
    // delayMicroseconds(val*10);
    
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

    val = analogRead(2);
    val = map(val, 0, 1023, 0, 10);
    fadeCount = fadeCount + (val * fadeDir);

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
    valCopy = val;
    interrupts();
    
    Serial.print("analog 2 is: ");
    Serial.println(valCopy*10);
    
    Serial.print("fadeCount = ");
    Serial.println(fadeCount);
}

