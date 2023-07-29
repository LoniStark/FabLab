
/*
Plant Bud IoT Module

by Loni Stark

This is for each plant, there will be a plant bud in the Home Garden System (HPG) system:

- Defines the soil sensor class. All functions are stateless, recording any state on the HPG hub.
- Defines the functions for this SoilSensor class:
    - SoilSensor(): constructor. Upon start, figures out identifier, and ensure this identifier is sent 
    in all communications with the HPG hub. 
    - Calibrate: This function is run when a soil sensor is first set up. It prompts the user to hold 
    it in normal air, then submerged in water to get the low and high water percentage. It also asks 
    the user to place it in the soil which is to be the idea moisture they want it to be so the % range 
    of soil moisture for the plant. This is recorded by sending the information to the HPG hub with unique 
    identifier and this information. 
      - Sensor unique identifier (SUI_ID)
      - Plant profile  - phase 2 for now, just PlantType - types will be looked up from a config file on hub
      - Water source - phase 2 (automated/manual)
      - Wet (100%) value
      - Dry (0%) value
      - Initial Soil (0-100%) value
      - Battery level - phase 2 - C3 does not have a pin to check battery level. There is an approach to
        take a look at here.

    - Update(int SUI_ID): 
       - if the SUI_ID equals the one for the particular sensor, then read the latest soil information and report 
        back to the HPG hub.

    Design question - should we have each sensor paired with a pump or have it be different pumps for each plant? 
    - are their mulitple pumps for each reservoir?
*/

//#define SOIL_MOIST D0

#include "WiFi.h"
#include <esp_now.h>

uint8_t broadcastAddress[] =  {0xE0, 0x5A, 0x1B, 0x0D, 0x0C, 0x8C};    

//Define structure for Plant Profile data to record plants in home garden
typedef struct plant_profile {
  String name;
  int p_ID;
  String pBudAddress;
  int moist_level_low;
  int moist_level_high;
  int water_on_time_ms;
  int water_soak_time_ms;
  bool status_sensor; //1 if present, 0 if not
  bool status_pump; //1 if present, 0 if not
  bool status_wsource; //1 if present, 0 if not
} plant_profile;

// Define variables to send soil sensor readings
int outgoing_PBsensor_moist; //percentage moisture of soil for a plant

typedef struct plantBud_sensor {
    int moist;
   // other recordings like time measure was taken
} plantBud_sensor;

//Structure for requests coming from Garden Hub to Plant Bud

typedef struct GHtoPB_input_req {
  int check_plant_ID;
  int get_soil_moist;
  int get_water_level;
} GHtoPB_input_req;

String success;

// Both GH request and PB incoming readings

GHtoPB_input_req gardenHubInputRequest;
plantBud_sensor outgoingPBReadings;
plant_profile plantData;

//byte incomingByte;
//byte outgoingByte;


esp_now_peer_info_t peerInfo;


// define full range from wet to dry based on calibration of sensor completely dry and immerse in water.
int defaultDry = 3325;
int defaultWet = 1250;
int moistureVal;
unsigned long previousMillis = 0;
long delayTime = 3000; 
int buzzerPin = D2;

 void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

  // This function will need to figure out what the request is and then take the necessary action. 
  Serial.println ("RECEIVING GARDEN HUB REQUEST");
  memcpy(&gardenHubInputRequest, incomingData, sizeof(gardenHubInputRequest));
  Serial.print("Bytes received: ");
  Serial.println(len);
  
  Serial.println("Information Received:");
  Serial.println(gardenHubInputRequest.check_plant_ID);
  Serial.println(gardenHubInputRequest.get_soil_moist);
  Serial.println(gardenHubInputRequest.get_water_level);
  
  
}

class SoilSensor
{ 
  // Class Member Variables
  // These are initialized at start up
  int moistPin;
  String SUI_ID;
  int PlantType;
  int wet;
  int dry;
  int batteryLife;
  //class PlantProfile

 public:
  SoilSensor(int pin)
  {
    moistPin = pin;
    wet = defaultWet;
    dry = defaultDry; 
    SUI_ID = WiFi.macAddress();

 }

int Update()
{
  
  int moistureVal = analogRead(moistPin);
  Serial.print("Capacitive Reading:");
  Serial.println(moistureVal);
  int percentageHumidity = map(moistureVal, wet, dry, 100, 0);
  Serial.print("Moisture Percentage:");
  Serial.print(percentageHumidity);
  Serial.println("%");
  //Serial.println(SUI_ID);
  return percentageHumidity;

 }

/*Calibrate function

{
- If SUI_ID equals card SUID then 
- prompt user to hold sensor in air -> record value
- prompt user to hold sensor in water -> record value
- prompt user to hold in plant -> record value
- do a check to ensure within the boundaries of what is possible for plants. For example, can be 100% moisture as ideal
- 
*/

};

SoilSensor soil(D0);
 
// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  pinMode(buzzerPin, OUTPUT);
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());
  //establish ESP Now?
  //soil.Calibrate();

  // Init ESP-NOW ***************************
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  Serial.print("Peer Address:");

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer.");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
// the loop function runs over and over again forever
void loop() {
  unsigned long currentMillis = millis();
   if(currentMillis-previousMillis>delayTime){
      outgoingPBReadings.moist = soil.Update();
      previousMillis = currentMillis;
 

 /*  if ((myData.b % 2) == 0){ 
      Serial.println("EVEN NUMBER");
      //tone(buzzerPin, 1000, 500);
      delay(3000);
      Serial.println(incomingByte);
      outgoingByte = incomingByte+1;
   }*/

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingPBReadings, sizeof(outgoingPBReadings));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

   }
//Serial.println(WiFi.macAddress());
 //delay(3000);
}
