#define relayPin 8 // RELAY
// RELAY TERMINAL RIGHT  ----->  SOLENOID -
// RELAY TERMINAL MIDDLE ----->  GROUND OF 12V
// SOLENOID + -----> 12V SUPPLY
#define trigPin 2 //RX OF ULTRASONIC SENSOR
#define echoPin 3 //TX OF ULTRASONIC SENSOR
#include <NewPing.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
// I2C lcd SCL = A5 arduino
// I2C lcd SDA = A4 arduino
// I2C VCC = 5V arduino
// I2C GND = GND arduino
// NODEMCU D1 == A4 arduino
// NODEMCU D2 == A5 arduino
// NODEMCU GND == GND arduino

//

// Define maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500 cm:
#define MAX_DISTANCE 200
// NewPing setup of pins and maximum distance.
NewPing sonar = NewPing(trigPin, echoPin, MAX_DISTANCE);

long distance;
float AverageSoilMoisture;
int SoilMoisture1;
int SoilMoisture2;

String SoilMoistureStr;
float SoilMoistureSensitivity;
float GradualDecreaseOfWater;

//CHANGE THIS IF TOTAL WATER VOL IS DIFFERENT
float TotalVolume = 13273.23;

float CurrentVolume;
bool isValveOpen = false;

void setup() {
  // put your setup code here, to run once:
   Wire.begin(8);                /* join i2c bus with address 8 */
   Wire.onReceive(receiveEvent); /* register receive event */
   Wire.onRequest(requestEvent); /* register request event */ 
   Serial.begin(9600);           /* start serial for debug */
   pinMode(relayPin, OUTPUT);
   // initialize LCD
   lcd.init();
   // Print a message to the LCD.
   lcd.backlight();
}

void loop() {
  // put your main code here, to run repeatedly:
   
  

  // GET AND SEND WATER LEVEL VALUE TO NODE MCU
  //GetAndSendWaterLevel();
  // Get AVG Soil Moisture and Send to NODE MCU
  // GetAndSendSoilMoisture();
  

  SoilMoisture1 = floatMap(analogRead(A0), 0, 1023, 100, 0);
  lcd.print("SoilMoisture1: ");
  lcd.print(SoilMoisture1);
  lcd.println("%");

  SoilMoisture2 = floatMap(analogRead(A1), 0, 1023, 100, 0);
  lcd.print("SoilMoisture2: ");
  lcd.print(SoilMoisture2);
  lcd.println("%");

  AverageSoilMoisture = (SoilMoisture1 + SoilMoisture2)/2;
  
  lcd.print("Average SoilMoisture: ");
  lcd.print(AverageSoilMoisture);
  lcd.println("%");

  int dist = sonar.ping_cm();
  if (dist ==0 || dist >=25){
    dist = 25;
  }
  CurrentVolume = 13*13*PI*(50-dist);
  Serial.print("Current Volume: ");
  Serial.print(CurrentVolume);
  Serial.println("ml");


  lcd.setCursor(0,0);
  lcd.print("Environment Control");
  lcd.setCursor(0,1);
  lcd.print("SoilMoisture: "+String(AverageSoilMoisture)+"%");
  lcd.setCursor(0,2);
  lcd.print("Water Volume: ");
  lcd.setCursor(0,3);
  lcd.print(String(CurrentVolume,2)+"ml");

  if (AverageSoilMoisture > SoilMoistureSensitivity){
    digitalWrite(relayPin, HIGH); 
  }else{
    digitalWrite(relayPin, LOW);
    isValveOpen = true;
    delay(10000);
    digitalWrite(relayPin,HIGH);
  }


  delay(2000);
}

int floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


int GetAndSendWaterLevel(){
   // Measure distance and print to the Serial Monitor:
  // Send ping, get distance in cm and print result (0 = outside set distance range):
  distance = sonar.ping_cm();
  Serial.print(distance); 

  return distance;
}
void GetAndSendSoilMoisture(int sensitivity){


    //SEND to NODE MCU
}



void receiveEvent(int howMany) {
 while (0<Wire.available()) {
    char c = Wire.read();      /* receive byte as a character */
    SoilMoistureStr +=c;
  }
  SoilMoistureSensitivity = SoilMoistureStr.toFloat();
  Serial.println(SoilMoistureSensitivity);
  SoilMoistureStr="";
  

  
}

// function that executes whenever data is requested from master
void requestEvent() {

    if (isValveOpen){
      Wire.write("Z");
      Serial.println("Send Data that Valve is open");
      isValveOpen = false;
    }

    String volstr = "V"+String(CurrentVolume,2);
    
    Wire.write(volstr.c_str());  /*send string on request */
    
    float soil =  AverageSoilMoisture;
    String soilstr = "M"+String(AverageSoilMoisture,2)+"E";
    Wire.write(soilstr.c_str());  /*send string on request */


    Serial.println(volstr.c_str());
    Serial.println(CurrentVolume);
    Serial.println(soilstr.c_str());
    Serial.println(AverageSoilMoisture);


    

}

float CalculateWaterVolume(int distance){
    // FORMULA TO CALCULATE VOLUME RECTANGULAR PRISM SHAPE OF TANK
    // V = L * W * H
    // L = 23cm
    // W = 12cm
    // H = 22 (height of water level)
    // WE NEED TO PUT THE SENSOR 30cm from the full water tank 
    //( WATER ------(distance)-------sensor   ===== 30cm )

    //the total height from the bottom of the water tank up to the sensor is set to 52cm (change 52 if the total height from the bottom of the water tank is changed!
    // FULL WATER VOLUME
    float V = PI * 13 * 13 * (50 - distance);
    // 25    50 = 25
    //
    return V;
}
