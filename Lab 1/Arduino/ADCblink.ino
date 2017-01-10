/* Analog Input Example, Teensyduino Tutorial #4
   http://www.pjrc.com/teensy/tutorial4.html

   After uploading this to your board, use Serial Monitor
   to view the message.  When Serial is selected from the
   Tools > USB Type menu, the correct serial port must be
   selected from the Tools > Serial Port AFTER Teensy is
   running this code.  Teensy only becomes a serial device
   while this code is running!  For non-Serial types,
   the Serial port is emulated, so no port needs to be
   selected.

   This example code is in the public domain.
*/
int val;

/* LED Blink, Teensyduino Tutorial #1
   http://www.pjrc.com/teensy/tutorial.html
 
   This example code is in the public domain.
*/

// Teensy 2.0 has the LED on pin 11
// Teensy++ 2.0 has the LED on pin 6
// Teensy 3.x / Teensy LC have the LED on pin 13
const int ledPin = 13;

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

