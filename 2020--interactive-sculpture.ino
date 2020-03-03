#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

WiFiClient espClient;

const char* ssid = "University of Washington"; // Wifi network SSID
const char* password = ""; // Wifi network password

String ip = "172.28.219.179"; // Sieg Master IP
String api_token = "rARKEpLebwXuW01cNVvQbnDEkd2bd56Nj-hpTETB"; // Sieg Master API Token

//String ip = "172.28.219.177"; // Red IP
//String api_token = "Lht3HgITYDN-96UYm5mkJ4CEjKj20d3siYidcSq-"; // Red API Token

int number = 1;

// ------------- ULTRASONIC SENSORS -------------
#define usTrig1     4   // orange striped
#define usEcho1     5   // orange solid

#define usTrig2     2   // green striped
#define usEcho2     15   // green solid

#define usTrig3     0   // blue striped
#define usEcho3     16   // blue solid

#define usTrig4    13    // brown striped
#define usEcho4    12    // brown solid

// ------------- ULTRASONIC SENSORS DATA STRUCTURE -------------
typedef struct {
  int one;
  int two;
  int three;
  int four;
} ultraSonic;

ultraSonic us;

int lowerLights[] = {10, 23, 11, 15, 7, 14, 22, 21, 16};

bool debug = true;
bool debugUs = false;
bool debugTouch = true;

int triggerDistance = 36;

// -------- CAPACITIVE TOUCH SENSOR --------
Adafruit_MPR121 cap = Adafruit_MPR121();
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

void setup() {
  Serial.begin(115200);
  Wire.pins(13, 12);
  if (!debug){
    setup_wifi();
  }
  //ultrasonicSetup();
  capacitiveTouchSetup();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED && !debug) {
      delay(1);
      Serial.print("WIFI Disconnected. Attempting reconnection.");
      setup_wifi();
      return; //End this loop cycle if WiFi disconnected
   }
   readSensors();
   //onOff();
   //iterateLights();
}

void iterateLights(){
  while(us.one < triggerDistance && us.two < triggerDistance && us.three < triggerDistance && us.four < triggerDistance){
    for (int i = 0; i < 9; i++) {
      String color = String(random(0, 65280));
      String brightness = String(random(150, 254));
      String saturation = String(random(180, 254));
      changeLight(lowerLights[i], 3, "on", "true", "bri", brightness, "hue", color, "sat", saturation);
      delay(150);
    }
    readSensors();
  }
}

void onOff(){
    if (us.two < triggerDistance){
      changeGroup(0, 6, "on", "false", "bri", "100", "hue", "40000", "sat", "254");
      delay(1000);
    }
    
}

void readSensors(){
  detectTouch();
//sensorOne(); //orange wire sensor
//sensorTwo(); //green wire sensor
//sensorThree(); //blue wire sensor
//sensorFour(); //brown wire sensor
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println("Connected to the WiFi network");
}

void detectTouch(){
  currtouched = cap.touched();
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
    }
  }
  lasttouched = currtouched;
  // comment out this line for detailed data from the sensor!
  return;
  // debugging info, what
  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
  Serial.print("Filt: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.filteredData(i)); Serial.print("\t");
  }
  Serial.println();
  Serial.print("Base: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.baselineData(i)); Serial.print("\t");
  }
  Serial.println();
  // put a delay so it isn't overwhelming
  delay(100);
}

// ------------- ULTRASONIC SENSORS -------------
void sensorOne(){
   long duration, distance;
   digitalWrite(usTrig1, LOW);  
   delayMicroseconds(2); 
   digitalWrite(usTrig1, HIGH);
   delayMicroseconds(10); 
   digitalWrite(usTrig1, LOW);
   duration = pulseIn(usEcho1, HIGH);
   distance = (duration/2) / 29.1;
   us.one = distance;
   debugUsSensors(1, distance);
}
void sensorTwo(){
   long duration, distance;
   digitalWrite(usTrig2, LOW);  
   delayMicroseconds(2);
   digitalWrite(usTrig2, HIGH);
   delayMicroseconds(10); 
   digitalWrite(usTrig2, LOW);
   duration = pulseIn(usEcho2, HIGH);
   distance = (duration/2) / 29.1;
   us.two = distance;
   debugUsSensors(2, distance);
}

void sensorThree(){
   long duration, distance;
   digitalWrite(usTrig3, LOW);  
   delayMicroseconds(2);
   digitalWrite(usTrig3, HIGH);
   delayMicroseconds(10); 
   digitalWrite(usTrig3, LOW);
   duration = pulseIn(usEcho3, HIGH);
   distance = (duration/2) / 29.1;
   us.three = distance;
   debugUsSensors(3, us.three);
}

void sensorFour(){
   long duration, distance;
   digitalWrite(usTrig4, LOW);  
   delayMicroseconds(2);
   digitalWrite(usTrig4, HIGH);
   delayMicroseconds(10); 
   digitalWrite(usTrig4, LOW);
   duration = pulseIn(usEcho4, HIGH);
   distance = (duration/2) / 29.1;
   us.four = distance;
   debugUsSensors(4, us.four);
}

void debugUsSensors(int sensorNumber, int sensorDistance){
  if (debugUs){
     Serial.print("Ultrasonic Distance " + String(sensorNumber) + ": ");
     Serial.print(sensorDistance);
     Serial.println(" cm");
   }
}

void ultrasonicSetup(){
  pinMode(usTrig1, OUTPUT);
  pinMode(usEcho1, INPUT);
  pinMode(usTrig2, OUTPUT);
  pinMode(usEcho2, INPUT);
  pinMode(usTrig3, OUTPUT);
  pinMode(usEcho3, INPUT);
  pinMode(usTrig4, OUTPUT);
  pinMode(usEcho4, INPUT);
}

void capacitiveTouchSetup(){
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
}
