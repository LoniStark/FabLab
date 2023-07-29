
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

//maybe need this.
#include <NTPClient.h>
#include <WiFiUdp.h>

// Network Credentials
#define WIFI_SSID "MAKERSPACE"
#define WIFI_PASSWORD "12345678"
//#define WIFI_SSID "mariposa1052"
//#define WIFI_PASSWORD "acorn105"

// Firebase project API Key
#define API_KEY "AIzaSyCF6zm-Hlf90m8kcQNBth4EFZdiRuArbRc"

// RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://gardenhub-51953-default-rtdb.firebaseio.com/" 


//Define Firebase Data object
FirebaseData GardenDO;

FirebaseAuth auth;
FirebaseConfig config;


// log time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};


// Variables to save date and time
String formattedTime;
String dayStamp;
String hourStamp;
String minuteStamp;

String timeStamp;

unsigned long sendDataPrevMillis = 0;
unsigned long previousMillis = 0;
int count = 1;
bool signupOK = false;
bool found = false;
int defaultDry = 3325;
int defaultWet = 1250;
int moistureVal;

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
  int currentMoist; //current reading for moisture
} plant_profile;


plant_profile getPlantProfile(int PlantID){

 plant_profile plant_record;

 String plantLookup = String(PlantID);
 plant_record.p_ID = PlantID;
 Serial.print("GET FUNCTION");
   if (Firebase.RTDB.getString(&GardenDO, plantLookup + "/name")) {
        plant_record.name = GardenDO.stringData();
        Serial.print("Read Name: ");
        Serial.println(plant_record.name);
       
    }
    else {
      Serial.println(GardenDO.errorReason());
    }

    if (Firebase.RTDB.getString(&GardenDO, plantLookup + "/address")) {
        plant_record.pBudAddress = GardenDO.stringData();
        Serial.print("Read Address");
        Serial.println(plant_record.name);
       
    }
    else {
      Serial.println(GardenDO.errorReason());
    }

    if (Firebase.RTDB.getInt(&GardenDO, plantLookup + "/m_low")) {
        plant_record.moist_level_low = GardenDO.intData();
        Serial.print("Read Low: ");
        Serial.println(plant_record.moist_level_low);
       
    }
    else {
      Serial.println(GardenDO.errorReason());
    }

    if (Firebase.RTDB.getInt(&GardenDO, plantLookup + "/m_high")) {
        plant_record.moist_level_high = GardenDO.intData();
        Serial.print("Read High: ");
        Serial.println(plant_record.moist_level_high);
       
    }
    else {
      Serial.println(GardenDO.errorReason());
    }

    if (Firebase.RTDB.getInt(&GardenDO, plantLookup + "/w_time")) {
        plant_record.water_on_time_ms = GardenDO.intData();
        Serial.print("Read Water Time: ");
        Serial.println(plant_record.water_on_time_ms);
    }
    else {
      Serial.println(GardenDO.errorReason());
    }

      if (Firebase.RTDB.getInt(&GardenDO, plantLookup + "/soak")) {
        plant_record.water_soak_time_ms = GardenDO.intData();
        Serial.print("Read Water Time: ");
        Serial.println(plant_record.water_soak_time_ms);
    }
    else {
      Serial.println(GardenDO.errorReason());
    }

   if (Firebase.RTDB.getBool(&GardenDO, plantLookup + "/soilSensor")) {
        plant_record.status_sensor = GardenDO.boolData();
        Serial.print("Read Soil Sensor: ");
        Serial.println(plant_record.status_sensor);
    }
    else {
      Serial.println(GardenDO.errorReason());
    }

  if (Firebase.RTDB.getBool(&GardenDO, plantLookup + "/pumpOutput")) {
        plant_record.status_pump = GardenDO.boolData();
        Serial.print("Read Pump Output: ");
        Serial.println(plant_record.status_pump);
    }
    else {
      Serial.println(GardenDO.errorReason());
    }

      if (Firebase.RTDB.getBool(&GardenDO, plantLookup + "/wsourceSensor")) {
        plant_record.status_wsource = GardenDO.boolData();
        Serial.print("Read Water Source: ");
        Serial.println(plant_record.status_wsource);
    }
    else {
      Serial.println(GardenDO.errorReason());
    }
     if (Firebase.RTDB.getInt(&GardenDO, plantLookup + "/cMoist")) {
        plant_record.currentMoist = GardenDO.intData();
        Serial.print("Current Moisture: ");
        Serial.println(plant_record.currentMoist);
       
    }
    else {
      Serial.println(GardenDO.errorReason());
    }

 return plant_record;
}

String getTimeLogStamp(){
  Serial.println("GET TIME LOG STARTED");
  timeClient.update();
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  Serial.println("Formated Time**************");
  formattedTime = timeClient.getFormattedTime();
  Serial.println(formattedTime); 
  Serial.println("Formatted Day**************");
    // Extract date


  time_t epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  

  int currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);  

  int currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute); 
   
  int currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);  

  String weekDay = weekDays[timeClient.getDay()];
  Serial.print("Week Day: ");
  Serial.println(weekDay);    

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  String currentMonthName = months[currentMonth-1];
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  int currentYear = ptm->tm_year+1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.print("Current date: ");
  Serial.println(currentDate);

  String logTime = currentDate + " " + formattedTime;
  
  Serial.println("LOG TIME**************");
  Serial.println(logTime);
  return(logTime);
  delay(2000);
}
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
 /* Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();*/
}

plant_profile pBudInfo;
plant_profile readProfile;

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  pinMode(D0, INPUT);
  pinMode(D1, OUTPUT);

// Initialize Serial Monitor
// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(-14400); // set for PST zone
  
  String logTimeStamp = getTimeLogStamp();

  delay(10000);
  //set the plant bud's profile
  pBudInfo.pBudAddress = WiFi.macAddress();
  Serial.println(pBudInfo.pBudAddress);
  delay(1000);
  while ((pBudInfo.pBudAddress != readProfile.pBudAddress) && (count<6)){
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 3000 || sendDataPrevMillis == 0)){
        sendDataPrevMillis = millis();
        pBudInfo.p_ID = count;
        String plantLookup = String(pBudInfo.p_ID);
        //get the plantlook up for the address
        Serial.println ("LOOKUP");
        Serial.println (plantLookup);
         if (Firebase.RTDB.getString(&GardenDO, plantLookup + "/address")) {
              readProfile.pBudAddress = GardenDO.stringData();
              Serial.print("Read Address");
              Serial.println(readProfile.pBudAddress);
              if (pBudInfo.pBudAddress == readProfile.pBudAddress){
                pBudInfo = getPlantProfile(count); //found the record for the plant bud in Garden Hub
                found = true;
                Serial.println("FOUND IS TRUE:");
                Serial.println (count);
                count = 100;
                //exit while loop criteria
              }  
             
            }
         else {
              Serial.println(GardenDO.errorReason());
            }

               Serial.println("SET VALUE OF PLANT BUD ID");
                Serial.println(pBudInfo.p_ID);
     
        count++;
      }
             
  }

  if (found == false){
     count = 1; //start at record one again
    Serial.println("FOUND IS FALSE. Find empty record.");
    while ((pBudInfo.pBudAddress != readProfile.pBudAddress) && (count<6)){
       if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 3000 || sendDataPrevMillis == 0)){
        sendDataPrevMillis = millis();
        pBudInfo.p_ID = count;
        String plantLookup = String(pBudInfo.p_ID);
        Serial.println ("FINDING A BLANK RECORD:");
        Serial.println (plantLookup);

        //get the plantlook up for the address
         if (Firebase.RTDB.getString(&GardenDO, plantLookup + "/address")) {
              readProfile.pBudAddress = GardenDO.stringData();
              Serial.print("Read Address");
              Serial.println(readProfile.pBudAddress);
              if (readProfile.pBudAddress == "none"){
                //set the adddress of the record to the Mac Address here
                Serial.println ("FOUND BLANK RECORD....");
                if (Firebase.RTDB.setString(&GardenDO, plantLookup + "/address", pBudInfo.pBudAddress)){  
                    Serial.println (GardenDO.stringData());
                    Serial.println("PASSED");
                    Serial.println("PATH: " + GardenDO.dataPath());
                    Serial.println("TYPE: " + GardenDO.dataType());
                  }
                 else {
                    Serial.println("FAILED");
                    Serial.println("REASON: " + GardenDO.errorReason());
                   }
                Serial.println("GETTING PLANT PROFILE DATA....");
                pBudInfo = getPlantProfile(count); //assign the  plant id to pBudInfo
                Serial.println("SET VALUE OF PLANT BUD ID");
                Serial.println(pBudInfo.p_ID);
                found = true;
          
                //exit while loop criteria
              }  
              else{
                count++;
              }
            }
         else {
              Serial.println(GardenDO.errorReason());
            }
      }
  }
  
}
// Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

    //disconnect WiFi as it's no longer needed
 // WiFi.disconnect(true);
 // WiFi.mode(WIFI_OFF);
  Serial.println ("EXIT SET UP. PLANT ID:");
  Serial.print(pBudInfo.p_ID);

}

void loop(){
  
  Serial.println ("START THE LOOP. PLANT ID:");
  Serial.print(pBudInfo.p_ID);
  //periodically check sensor for moisture levels
  unsigned long currentMillis = millis();
  struct tm timeinfo;
   //time_t t = worldTime.toInt();

  
   if(currentMillis-previousMillis>10000){
     moistureVal = analogRead(D0);
 // Serial.print("Capacitive Reading:");
 // Serial.println(moistureVal);
  int percentageHumidity = map(moistureVal, defaultWet, defaultDry, 100, 0);
  Serial.print("Moisture Percentage:");
  Serial.print(percentageHumidity);
  Serial.println("%");

  //Serial.println(SUI_ID);
  previousMillis = currentMillis;

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  // log humidity reading
  String lookup = String(pBudInfo.p_ID);
  Serial.println("STRING LOG ID");
  Serial.println (lookup);

  // update current moist and log it

 if (Firebase.RTDB.setInt(&GardenDO, lookup + "/cMoist", percentageHumidity)){
      Serial.println("PASSED");
      Serial.println("PATH: " + GardenDO.dataPath());
      Serial.println("TYPE: " + GardenDO.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + GardenDO.errorReason());
    }
   timeStamp = getTimeLogStamp();
   Serial.println(timeStamp);
  if (Firebase.RTDB.setInt(&GardenDO, lookup + "/log/" + timeStamp, percentageHumidity)){
      Serial.println("PASSED");
      Serial.println("PATH: " + GardenDO.dataPath());
      Serial.println("TYPE: " + GardenDO.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + GardenDO.errorReason());
    }

  //check to see if needs water
  if (percentageHumidity<pBudInfo.moist_level_low){
    Serial.print("Needs water: ");
    Serial.println(pBudInfo.name);
    

    while (percentageHumidity<pBudInfo.moist_level_high){
      Serial.println("Start pump");
      digitalWrite(D1, HIGH);
      delay(pBudInfo.water_soak_time_ms);
      Serial.println("Stop pump");
       digitalWrite(D1, LOW);
      Serial.println ("Take moisture measure");
      delay(pBudInfo.water_soak_time_ms);
      moistureVal = analogRead(D0);
      percentageHumidity = map(moistureVal, defaultWet, defaultDry, 100, 0);
      Serial.println(percentageHumidity);
   
    }
    
  }
   printLocalTime();
 }
}