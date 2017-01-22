/* ADCfade, Lab 2 Section C Part 2

   Course: EE 474 
   Names: Ryan Linden, Khalid Alzuhair, Brandon Ngo 
   
   This code modifies the PWMtimer.ino 
   sketch from Lab 2 Section C Part 1 

   Incorporated our potentiometer and ADC code from Lab 1
   to change the fade rate

   We used the delay function to set the timing of our analogWrite() 
   function calls similiar to blink sketch in Lab 1
*/

// Create an IntervalTimer object
IntervalTimer myTimer;

// Pinouts
const int redPin = 20;
const int greenPin = 21;
const int bluePin = 22;

// Value set by potentiometer
int val; 

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
    
}

volatile unsigned long fadeCount = 255; // use volatile for shared variables
int fadeDir = 1; // 1 increments, -1 decrements
int selectedPin = 0; // 0 is no pin selected, 20=red, 21=green, 22=blue

// The main program will print the fade count
// to the Arduino Serial Monitor
void loop(void) {

    // to read a variable which the interrupt code writes, we
    // must temporarily disable interrupts, to be sure it will
    // not change while we are reading.  To minimize the time
    // with interrupts off, just quickly make a copy, and then
    // use the copy while allowing the interrupt to keep working.
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

    val = analogRead(2);
    Serial.print("analog 2 is: ");
    Serial.println(val*10);
    
    Serial.print("fadeCount = ");
    Serial.println(fadeCount);

    // Sets delay between each loop 
    // Delay value is set according to potentiometer
    // and multiplied by 10 due to use of delayMicroseconds
    // this results in a fade rates similiar to PWMtimer
    delayMicroseconds(val*10);
}

