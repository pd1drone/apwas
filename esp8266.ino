#include <Wire.h>
#include <ESP8266WiFi.h> 
#include <FirebaseArduino.h>  
  
// Set these to run example.  
#define FIREBASE_HOST "apwas-c0fcb-default-rtdb.firebaseio.com"  
#define FIREBASE_AUTH "JmQbyQyx5J1JWv8OGZPfst88C2ChoQHevOj0oZIX"  
#define WIFI_SSID "WIFI_SSID"  // change this
#define WIFI_PASSWORD "WIFI_PASSWORD"  // change this

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
 Wire.requestFrom(8, 20); /* request & read data of size 13 from slave */
 
 Serial.println(Wire.available());
 while(Wire.available()>1){
    char c = Wire.read(); 
    Serial.println(c);
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