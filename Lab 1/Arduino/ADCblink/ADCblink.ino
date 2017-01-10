/* ADCblink, Teensyduino Tutorial #4
   This code combines the Blink and AnalogInput example code from 
   the Arduino Library

   The delay is now determined from the analogRead() function
   from the Arduino library so that the blink rate can be controlled from
   the potentiometer  
*/

// Teensy 2.0 has the LED on pin 11
// Teensy++ 2.0 has the LED on pin 6
// Teensy 3.x / Teensy LC have the LED on pin 13
const int ledPin = 13;
int val; 

// the setup() method runs once, when the sketch starts

void setup() {
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);
  Serial.begin(38400);
}

// the loop() methor runs over and over again,
// as long as the board has power

void loop() {
  val = analogRead(2);
  Serial.print("analog 2 is: ");
  Serial.println(val);
  digitalWrite(ledPin, HIGH);   // set the LED on
  delay(val);                  // wait for a second
  digitalWrite(ledPin, LOW);    // set the LED off
  delay(val);                  // wait for a second
}

