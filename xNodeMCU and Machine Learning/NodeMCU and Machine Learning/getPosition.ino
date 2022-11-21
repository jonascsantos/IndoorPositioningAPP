#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

#include "Converter.h"
#include "Classifier.h"

Eloquent::Projects::WifiIndoorPositioning positioning;
Eloquent::ML::Port::DecisionTree classifier;

#define FIREBASE_HOST "XXXXXXXXXX.firebaseio.com"
#define FIREBASE_AUTH "XXXXXXXXXX"

int16_t utc = 1; 

int lp = 0;
int lp_time = 15;            

char push_data[200]; 
unsigned long oldTime;
const char * ssid = "Vodafone-C4C6";
const char * password = "QCGDE7qAERMUdTA4";
String location;

FirebaseData fbdo;

void setup() {
    Serial.begin(115200); 
    delay(10);
    Serial.println('\n');

    startWIFI();

    oldTime = 0;
    
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
    if (WiFi.status() == WL_CONNECTED && (millis() - oldTime) > 1000)  {
        oldTime = millis();
        lp++;
        
        if(lp >= 6){
            positioning.scan();
            Serial.println(" ...");
            lp=0;
            
            FirebaseJson json;
            FirebaseJson json2;
            
            
            location = classifier.predictLabel(positioning.features);
            
            Serial.print("Location: ");
            Serial.println(location);

            json.set("room", location);
            json.set("Ts/.sv", "timestamp");
            
            if (Firebase.RTDB.pushJSON(&fbdo, "/USER2/ROOM", &json)) {
              Serial.println(fbdo.dataPath());
              Serial.println(fbdo.pushName());
              Serial.println(fbdo.dataPath() + "/"+ fbdo.pushName());
            } else {
              Serial.println(fbdo.errorReason());
            }
        } 
    } else if (WiFi.status() != WL_CONNECTED) {
        startWIFI();
    }
}

void startWIFI(void) {
    delay(100);              
    
    WiFi.begin(ssid, password);
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println(" ...");
    oldTime = 0;
    int i = 0;
    delay(100);         
    
    while (WiFi.status() != WL_CONNECTED) { 
        delay(2000);
        Serial.print(++i);
        Serial.print('.');
        delay(100);  
    }
    delay(2000);
    Serial.print('\n');
    Serial.print("Connected!\n");
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP()); 
}
