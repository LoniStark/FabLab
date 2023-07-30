/*

For the Ear Sculpture

A proximity sensor at the front of the ear sculpture. If there is a person within different distances to
the sculpture, it will respond through a set of auditory and visual responses. 

Loni Stark

*/

#include <Adafruit_NeoPixel.h> //for LED lights

// defines pins numbers for the LED srip

#define PIN       11 
#define NUMPIXELS 9 
#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

const int trigPin = 10; //keep the proximity sensor the same.
const int echoPin = 6;
int buzzerPin = 7;

// defines variables
long duration;
int distance;
long waitTime = 1000; 

unsigned long previousMillis = 0;

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //declare an LED strip

class ProximitySensor
{
  // Class Member Variables
  // These are initialized at start up
  int pTrigPin;
  int pEchoPin;

  public:
  ProximitySensor(int tPin, int ePin)
  { 
    pTrigPin = tPin;
    pEchoPin = ePin;

 //   pinMode(pTrigPin, OUTPUT); // Sets the trigPin as an Output
 //   pinMode(pEchoPin, INPUT); // Sets the echoPin as an Input
  }

  void Update()
  { // Clears the trigPin
    digitalWrite(pTrigPin, LOW);
    delayMicroseconds(2);
      // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(pTrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pTrigPin, LOW);
   // Reads the echoPin, returns the sound wave travel time in microseconds
   duration = pulseIn(pEchoPin, HIGH);
  }
};

ProximitySensor distanceSensor(trigPin, echoPin); //declare a proximity sensor

void setup() {

  pinMode(buzzerPin, OUTPUT);
  strip.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(200); // Set BRIGHTNESS low to reduce draw (max = 255)
  Serial.begin(9600); // Starts the serial communication
}


void loop() {

  unsigned long currentMillis = millis();

  distanceSensor.Update();
  // Calculating the distance
  distance = duration * 0.034 / 2;

  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println (" cm.");

  strip.clear(); // Set all pixel colors to 'off'

if(currentMillis-previousMillis > waitTime){
  if (distance < 10) {
    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
       Serial.println ("buzzer");
       Serial.println (buzzerPin);
       tone(buzzerPin, 1000, 500);
      } 
    else if (distance < 20) {
       for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

         // strip.Color() takes RGB values, from 0,0,0 up to 255,255,255
         // Here we're using a moderately bright green color:
         strip.setPixelColor(i, strip.Color(150, 33, 33));
         strip.show();   // Send the updated pixel colors to the hardware.
        }
    }
else {
   for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

    // strip.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    strip.setPixelColor(i, strip.Color(0, 150, 0));
    strip.show();   // Send the updated pixel colors to the hardware.
  
  }
}
    previousMillis = currentMillis; 
 }
delay(3000);
}

 
