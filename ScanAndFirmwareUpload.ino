#include <WiFiManager.h>
#include <Update.h>
#include <HTTPClient.h>

#if defined(ESP32)
    #include <WiFi.h>
    #include <FirebaseESP32.h>
    #define WIFI_getChipId() (uint32_t)ESP.getEfuseMac()
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <FirebaseESP8266.h>
    #define WIFI_getChipId() ESP.getChipId()
#endif

#define FIREBASE_HOST "XXXXXXXXXX.firebaseio.com"
#define FIREBASE_AUTH "XXXXXXXXXX"

#define SLEEP_TIME_MS 3000
#define DEVICE_SLEEP ((millis() - oldTime) > SLEEP_TIME_MS) 

FirebaseData fbdo;
FirebaseJson json, firmwareJson;

String firebaseFirmwareVersion(""), firebaseFirmwareFilename("");
String currentFirmwareVersion(""), currentFirmwareFilename("");

uint16_t elapse_time = 0;
unsigned long oldTime;
String hostString;

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFiManager wm;

  bool res;

  res = wm.autoConnect();

  if (!res) {
    ESP.restart();
  }

  #if defined(ESP8266)
      fbdo.setBSSLBufferSize(2048, 512);
  #endif

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  setInitialValues();
}

void loop(void) {
  if (DEVICE_SLEEP)  {
    oldTime = millis();
    
    updateFirmwareCheck();

    scanStatusCheck();

    delay(2);
  }
}

void scanStatusCheck(void) {
  bool bVal;

  Serial.printf("Room Scan Status:  %s\n", Firebase.getBool(fbdo, "/" + hostString + "/SCAN/STATUS/running", &bVal) ? bVal ? "true" : "false" : fbdo.errorReason().c_str());

  if(bVal) {
    Firebase.getString(fbdo,  "/" + hostString + "/SCAN/STATUS/room");
    
    String roomName(fbdo.to<const char *>());

    Serial.printf("SCANING ROOM: ");
    Serial.println(roomName);

    scanRoom(roomName);
  }
}

void updateFirmware(String firmwareUrl) {
    HTTPClient http; 

    Serial.println("...Uploading...");
    
    http.begin(firmwareUrl);
    int httpCode = http.GET();
    if (httpCode <= 0) {
      return;
    }

    int contentLen = http.getSize();
    bool canBegin = Update.begin(contentLen);
    if (!canBegin) {
      return;
    }

    WiFiClient* client = http.getStreamPtr();
    size_t written = Update.writeStream(*client);

    if (written != contentLen) {
      return;
    }
    
    if (!Update.end()) {
      return;
    }
    
    if (Update.isFinished()) {
      Serial.println("Upload Finished");
      ESP.restart();
    } else {
      return;
    }
}

void updateFirmwareCheck(void) {
  FirebaseJson jVal;
  FirebaseJsonData resultFilename, resultVersion, resultUrl;

  Firebase.RTDB.getJSON(&fbdo,  "/" + hostString + "/FIRMWARE/WifiScan", &jVal);
  jVal.get(resultFilename, "filename");
  jVal.get(resultVersion, "version");

  if (resultFilename.success && resultVersion.success){
    firebaseFirmwareVersion = resultVersion.to<String>(); 
    firebaseFirmwareFilename = resultFilename.to<String>(); 
    
    if((firebaseFirmwareVersion != currentFirmwareVersion) ||
        firebaseFirmwareFilename != currentFirmwareFilename) {
      
      Serial.print("New Firmware: ");
      Serial.println(firebaseFirmwareFilename);
      Serial.print("Version: ");
      Serial.println(currentFirmwareVersion);
      
      jVal.get(resultUrl, "url");
      if (resultUrl.success) {
        Serial.printf("updating firmware.....");

        json.set("version", firebaseFirmwareVersion);
        json.set("filename", firebaseFirmwareFilename);
        Serial.printf("Saving new firmware version metadata to DB... %s\n", Firebase.updateNode(fbdo, "/"+hostString+"/FIRMWARE/CurrentFirmware/", json) ? "ok" : fbdo.errorReason().c_str());

        currentFirmwareVersion = firebaseFirmwareVersion; 
        currentFirmwareFilename = firebaseFirmwareFilename; 
  
        updateFirmware(resultUrl.to<String>().c_str());
      }
    }
  }
}

void scanRoom(String roomName) {
    int numNetworks = WiFi.scanNetworks();
    String output("{");
    output += "\"__location\"";
    output += ": ";
    output += '"' + roomName + '"';
    output += ", ";

    for (int i = 0; i < numNetworks; i++) {
        output += '"' + WiFi.SSID(i) + '"';
        output += ": ";
        output += WiFi.RSSI(i);
        output += (i == numNetworks - 1 ? "}\n" : ", ");
    }
  
  json.set("data", output);
  json.set("Ts/.sv", "timestamp");

  if (Firebase.RTDB.pushJSON(&fbdo, "/"+hostString+"/SCAN/data/"+roomName, &json)) {
    Serial.println(fbdo.dataPath());
    Serial.println(fbdo.pushName());
    Serial.println(fbdo.dataPath() + "/"+ fbdo.pushName());
  } else {
    Serial.println(fbdo.errorReason());
  } 
            
  Serial.print(output);
}

void setInitialValues(void) {
  hostString = String(WIFI_getChipId(), HEX);
  hostString.toUpperCase();
  
  json.set("STATUS/running", false);
  Serial.printf("Saving to FirebaseDB... %s\n", Firebase.updateNode(fbdo, "/"+hostString+"/SCAN/", json) ? "ok" : fbdo.errorReason().c_str());
  json.remove("STATUS");
  
  FirebaseJson jVal;
  FirebaseJsonData resultFilename, resultVersion;
  Firebase.RTDB.getJSON(&fbdo,  "/" + hostString + "/FIRMWARE/CurrentFirmware", &jVal);
  jVal.get(resultFilename, "filename");
  jVal.get(resultVersion, "version");

  if (resultFilename.success && resultVersion.success){
    currentFirmwareVersion = resultVersion.to<String>(); 
    currentFirmwareFilename = resultFilename.to<String>(); 

    Serial.print("Firmware: ");
    Serial.println(currentFirmwareFilename);
    Serial.print("Version: ");
    Serial.println(currentFirmwareVersion);
  }
}
