/*  Soil Moisture Traffic Light
    This sketch was written starting from examples from SparkFun Electronics and the Arduino Projects Book. 
    Loni Stark
    July 1, 2023

    Checks soil moisture levels and alerts plant parent with possible 5 values:
    - too dry: red solid light
    - dry warning: red blink light
    - green: plant is at right moisture
    - wet warning: yellow blinking light
    - too wet: yellow solid light

*/

int val = 0;              //value for storing moisture value 
int soilPin = A0;         //Declare a variable for the soil moisture sensor 
int soilPower = 7;        //Variable for Soil moisture Power
int lightColor = 1;       //Variable for light color: RED = 0, YELLOW = 1, GREEN = 2;
int blink= 0;             // Variable for if light will blink. blink = 1, no blink = 0
int switchState = 0;
int redLightPin = 5;      //Variable for output pin for red light
int yellowLightPin = 4;   //Variable for output pin for red light
int greenLightPin = 3;    //Variable for output pin for red light
int switchPin= 2;

const int TOO_DRY = 200; //value equal or below is considered too dry
const int WARN_DRY = 300; //value less than or above TOO_DRY is considered a dry warning
const int WARN_WET = 800; //value greater than or below TOO_WET is considered a wet warning
const int TOO_WET = 900; //value equal or greater than is considered too wet. 
const int RED = 0;
const int GREEN = 1;
const int YELLOW = 2;
const int ERROR = -1;



//Rather than powering the sensor through the 3.3V or 5V pins, the digital pin 7 will
//be used to power hte sensor. This will prevent corrosion of the sensor as it is in soil. 
//The 3.3V power output will be used to power the red, yellow and green lights.

void setup() 
{

  Serial.begin(9600);   // open serial over USB
  pinMode(redLightPin, OUTPUT);
  pinMode(yellowLightPin, OUTPUT);
  pinMode(greenLightPin, OUTPUT);
  pinMode(switchPin, INPUT);
  pinMode(soilPower, OUTPUT);//Set D7 as an OUTPUT
  digitalWrite(soilPower, LOW);//Set to LOW so no power is flowing through the sensor
}

void loop() 
{

switchState = digitalRead(switchPin);
Serial.print("Button state:");
Serial.println(switchState);
if (switchState == LOW){
  //if switch is not pressed, set the lights off
  digitalWrite(greenLightPin, HIGH); 
  digitalWrite(redLightPin, HIGH);
  digitalWrite(yellowLightPin, HIGH);
}
else
{
 //if switch is not pressed, then check for moisture before setting right light

  Serial.println("===SOIL CONDITION====");

  Serial.print("Soil Moisture = ");    
  //get soil moisture value from the function below and print it
  Serial.println(readSoil());

  Serial.print("Light Color = "); 
  //set color indicator state for soil moisture.
  Serial.println(setLightColor());

  if (lightColor == RED){
   digitalWrite(greenLightPin, HIGH); 
   digitalWrite(redLightPin, LOW);
   digitalWrite(yellowLightPin, HIGH);
   Serial.println("Red");
    }
  else if (lightColor==GREEN){
    digitalWrite(greenLightPin, LOW); 
    digitalWrite(redLightPin, HIGH);
    digitalWrite(yellowLightPin, HIGH);
    Serial.println("Green");
  }
  else if (lightColor==YELLOW){
    digitalWrite(greenLightPin, HIGH); 
    digitalWrite(redLightPin, HIGH);
    digitalWrite(yellowLightPin, LOW);
    Serial.println("Yellow");
  }

  Serial.print("Light Blink = "); 
  //set blink indicator state for soil moisture.
  Serial.println(setLightBlink());

Serial.println(" ");

}

//This 2 second timefrme is used so you can test the state of the switch to determine if a read is needed.
delay(1000);//take a reading every second

}
//This is a function used to get the soil moisture content
int readSoil()
{

    digitalWrite(soilPower, HIGH);//turn D7 "On"
    delay(10);//wait 10 milliseconds 
    val = analogRead(soilPin);//Read the SIG value form sensor 
    digitalWrite(soilPower, LOW);//turn D7 "Off"
    return val;//send current moisture value
}

int setLightBlink () {
  //function to set the light color based on soilPin value
   if (val <= TOO_DRY){
      blink = 0;
   }
    else if (val <= WARN_DRY){
      blink = 1;
    }

    else if (val <= WARN_WET){
      blink = 0;
    }

    else if (val > WARN_WET){
      blink = 1;
    }

    else if (val > TOO_WET){
      blink = 0;
    }

    else{
      blink = ERROR;
    }


    return blink;
}

int setLightColor () {
  //function to set the light color based on soilPin value
   if (val <= TOO_DRY){
      lightColor = RED;
   }
    else if (val <= WARN_DRY){
      lightColor = RED;
    }

    else if (val <= WARN_WET){
      lightColor = GREEN;
    }

    else if (val > WARN_WET){
      lightColor = YELLOW;
    }

    else if (val > TOO_WET){
      lightColor = YELLOW;
    }
    
    else {
      lightColor = ERROR;
    }
    return lightColor;
}


