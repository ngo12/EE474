// Create an IntervalTimer object
IntervalTimer myTimer;

// Pinouts
const int redPin = 20;
const int greenPin = 21;
const int bluePin = 22;

void setup(void) {

    // Set LED outputs
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    // Turn off LEDS
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);

    Serial.begin(9600);
    // Begin IntervalTimer
    myTimer.begin(blinkLED, 10000);  // blinkLED to run every 0.15 seconds

}

// The interrupt will blink the LED, and keep
// track of how many times it has blinked.
int redState = HIGH;
int greenState = HIGH;
int blueState = HIGH;
volatile unsigned long blinkCount = 255; // use volatile for shared variables

int fadeDir = 1;
int selectedPin = 0;
// functions called by IntervalTimer should be short, run as quickly as
// possible, and should avoid calling other functions if possible.
void blinkLED(void) {

    analogWrite(selectedPin, blinkCount);

    if (blinkCount >= 255) {
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
    else if (blinkCount <= 0) {
        fadeDir = fadeDir * -1;
    }
    blinkCount = blinkCount + 1 * fadeDir;

    /* switch (blinkCount) { */

    /* case 0: */
    /*     redState = LOW; */
    /*     greenState = HIGH; */
    /*     blueState = HIGH; */
    /*     break; */
    /* case 1: */
    /*     redState = HIGH; */
    /*     greenState = LOW; */
    /*     blueState = HIGH; */
    /*     break; */
    /* case 2: */
    /*     redState = HIGH; */
    /*     greenState = HIGH; */
    /*     blueState = LOW; */
    /*     break; */
    /* default: */
    /*     break; */

    /* } */

    /* blinkCount = blinkCount + 1;  // increase when LED turns on */
    /* digitalWrite(redPin, redState); */
    /* digitalWrite(greenPin, greenState); */
    /* digitalWrite(bluePin, blueState); */
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


    Serial.print("blinkCount = ");
    Serial.println(blinkCopy);
    delay(100);
}
