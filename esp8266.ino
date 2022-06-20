#include <Wire.h>
#include <ESP8266WiFi.h> 
#include <FirebaseArduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>  

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

unsigned long epochTime; 

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};


// Set these to run example.  
#define FIREBASE_HOST "apwas-c0fcb-default-rtdb.firebaseio.com"  // firebase connection database
#define FIREBASE_AUTH "JmQbyQyx5J1JWv8OGZPfst88C2ChoQHevOj0oZIX"  //firebase authentication ((fingerprint.com))
#define WIFI_SSID "Inoueh"  // ssid-wifi name
#define WIFI_PASSWORD "c'estlavie002"  // wif-password

//#define FIREBASE_HOST "environmentcontrol-8e967-default-rtdb.firebaseio.com"  
//#define FIREBASE_AUTH "dgBDOnOQ7BLE1pAGqznL7gkJkvMVri4JXHQgBzPQ"  


bool startVolume = false;
bool startSoilMoisture = false;
String VolumeSTR;
String SoilMoistureSTR;
float Volume;
float SoilMoisture;
String soil;
bool GetDataFirst = false;
bool isFirstData = true;
void setup() {
 Serial.begin(9600); /* begin serial for debug */
 Wire.begin(D1, D2); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */
 timeClient.begin();
 while (!Serial) { // wait for serial port to connect. Needed for native USB port only
  }
  // connect to wifi.   
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  
  Serial.print("connecting");  
  while (WiFi.status() != WL_CONNECTED) {  
    Serial.print(".");  
    delay(500);  
  }  
  Serial.println();  
  Serial.print("connected: ");  
  Serial.println(WiFi.localIP());  

    
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 
  
  timeClient.setTimeOffset(28800);
}

unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

void loop() {

  if (Wire.available()==0){
   Wire.beginTransmission(8); /* begin with device address 8 */
   Serial.print("SoilMoisture: ");
   soil = Firebase.getString("MoistureSensitivity");
   delay(1000);
   soil.replace("\"","");
   Serial.println(soil);
   Wire.write(soil.c_str());  /* sends hello string */
   Wire.endTransmission();    /* stop transmitting */
   soil = 0.0;
   isFirstData = false;
 }
 Wire.requestFrom(8, 30); /* request & read data of size 13 from slave */
 
 Serial.println(Wire.available());
 while(Wire.available()>1){
    char c = Wire.read(); 
    Serial.println(c);
    if (c == 'Z'){      
      //epochTime = getTime();
      //Serial.print("Epoch Time: ");
      //Serial.println(epochTime);
      //int sendTime = (int)epochTime;
      timeClient.update();
    
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
      String currentDateTime = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay)+ " " + formattedTime;
      Serial.print("Current date: ");
      Serial.println(currentDateTime);

      Firebase.pushString("/timestamp",currentDateTime);
      // handle error   
      if (Firebase.failed()) {  
          Serial.print("setting /timestamp failed:");  
          Serial.println(Firebase.error());    
          return;  
      }  
      delay(1000);
    }
    if (c == 'E'){
      GetDataFirst = false;
    }
    if (c != '1'){
      if (c == 'V'){
        startVolume = true;
        startSoilMoisture = false;
        SoilMoisture = SoilMoistureSTR.toFloat();
        SoilMoistureSTR = "";
        
      }
      if (c =='M'){
        startVolume= false;
        startSoilMoisture = true;
        Volume = VolumeSTR.toFloat();
        VolumeSTR = "";
      }
      if (startVolume && c != 'V'){
        VolumeSTR +=c;
      }
      if (startSoilMoisture && c != 'M'){
        SoilMoistureSTR +=c;
      }
    }

    if(!GetDataFirst){
    Serial.println(Volume);
    Serial.println(SoilMoisture);

    // set value  
    Firebase.setFloat("SoilMoisture", SoilMoisture);  
    // handle error   
    if (Firebase.failed()) {  
        Serial.print("setting /SoilMoisture failed:");  
        Serial.println(Firebase.error());    
        return;  
    }  

    // set value  
    Firebase.setFloat("Volume", Volume);  
    // handle error   
    if (Firebase.failed()) {  
        Serial.print("setting /Volume failed:");  
        Serial.println(Firebase.error());    
        return;  
    }  

     Wire.beginTransmission(8); /* begin with device address 8 */
     Serial.print("SoilMoisture: ");
     soil = Firebase.getString("MoistureSensitivity");
     delay(1000);
     soil.replace("\"","");
     Serial.println(soil);
     Wire.write(soil.c_str());  /* sends hello string */
     Wire.endTransmission();    /* stop transmitting */
     soil = 0.0;
     

      GetDataFirst = true;
    }
 }
 delay(2000);

}
