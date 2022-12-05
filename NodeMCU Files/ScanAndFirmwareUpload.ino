#include <WiFiManager.h>

#if defined(ESP32)
    #include <WiFi.h>
    #include <FirebaseESP32.h>
    #include <Update.h>
    #include <HTTPClient.h>
    #define WIFI_getChipId() (uint32_t)ESP.getEfuseMac()
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <FirebaseESP8266.h>
    #include <WiFiClient.h>
    #include <ESP8266HTTPClient.h>
    #include <ESP8266httpUpdate.h>
    #define WIFI_getChipId() ESP.getChipId()
#endif

#define FIREBASE_HOST "FIREBASE_HOST_ENV"
#define FIREBASE_AUTH "FIREBASE_AUTH_ENV"

#define SLEEP_TIME_MS 5000
#define DEVICE_SLEEP ((millis() - oldTime) > SLEEP_TIME_MS) 

FirebaseData fbdo, fbdoPositioning;
FirebaseJson json, json2, jsonPositioning;

String firebaseFirmwareVersion(""), firebaseFirmwareFilename("");
String currentFirmwareVersion(""), currentFirmwareFilename("");

unsigned long oldTime;
String hostString, location;
 
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
      fbdoPositioning.setBSSLBufferSize(2048, 512);
  #endif

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  setInitialValues();
}

void loop(void) {
  if (DEVICE_SLEEP)  {
    oldTime = millis();
    
    updateFirmwareCheck();

    bool isScanning = scanStatusCheck();
    
    delay(2);
  }
}

bool isTrackPositionOn(void) {
  bool bVal;

  Serial.printf("Track Position Status:  %s\n", Firebase.getBool(fbdo, "/" + hostString + "/TRACK_POSITION/STATUS/running", &bVal) ? bVal ? "true" : "false" : fbdo.errorReason().c_str());

  return bVal;
}

bool scanStatusCheck(void) {
  bool bVal;

  Serial.printf("Room Scan Status:  %s\n", Firebase.getBool(fbdo, "/" + hostString + "/SCAN/STATUS/running", &bVal) ? bVal ? "true" : "false" : fbdo.errorReason().c_str());

  if(bVal) {
    Firebase.getString(fbdo,  "/" + hostString + "/SCAN/STATUS/room");
    
    String roomName(fbdo.to<const char *>());

    Serial.printf("SCANING ROOM: ");
    Serial.println(roomName);

    scanRoom(roomName);
  }

  return bVal;
}

void updateFirmware(String firmwareUrl) {
    #if defined(ESP32)
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
        
        save_new_firmware();
  
        delay(1000); 
          
        ESP.restart();
      } else {
        return;
      }
    #else
      WiFiClientSecure client;
      client.setInsecure();
    
      ESPhttpUpdate.onStart(update_started);
      ESPhttpUpdate.onEnd(update_finished);
      ESPhttpUpdate.onProgress(update_progress);
      ESPhttpUpdate.onError(update_error);
      
      ESPhttpUpdate.followRedirects(true);
      ESPhttpUpdate.rebootOnUpdate(false);
      
      Serial.print(firmwareUrl);
      
      t_httpUpdate_return ret = ESPhttpUpdate.update(client, firmwareUrl);

      switch (ret) {
        case HTTP_UPDATE_FAILED:
          Serial.print(F("Update error "));
          Serial.print(ESPhttpUpdate.getLastError());
          Serial.print(" = ");
          Serial.print(ESPhttpUpdate.getLastErrorString());
          Serial.println();
          delay(5000); 
          break;
   
        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
   
        case HTTP_UPDATE_OK:
          Serial.println("Update OK. Restarting ESP");
          
          save_new_firmware();
  
          delay(1000); 
          ESP.restart();
          break;
      }
    #endif
    
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

  if (Firebase.RTDB.pushJSON(&fbdo, "/SCAN/data/" + roomName, &json)) {
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

  #if defined(ESP8266)
    jsonPositioning.set("board", "ESP8266");
  #elif defined(ESP32)
    jsonPositioning.set("board", "ESP32");
  #endif
        
  json.set("running", false);
  json2.set("running", true);
  
  Serial.printf("Saving to FirebaseDB 1/3... %s\n", Firebase.updateNode(fbdo, "/"+hostString+"/SCAN/STATUS/", json) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Saving to FirebaseDB.2/3.. %s\n", Firebase.updateNode(fbdo, "/"+hostString+"/TRACK_POSITION/STATUS/", json2) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Saving to FirebaseDB.3/3.. %s\n", Firebase.updateNode(fbdo, "/"+hostString+"/FIRMWARE/", jsonPositioning) ? "ok" : fbdo.errorReason().c_str());
  
  json.remove("SCAN");
  json2.remove("TRACK_POSITION");
  jsonPositioning.remove("board");
  
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

void update_started() {
  Serial.println("Firmware upload started");
}
 
void update_finished() {
  Serial.println("Firmware Uploaded!");
}
 
void update_progress(int cur, int total) {
  Serial.printf("Upload process at %d of %d bytes...\n", cur, total);
}
 
void update_error(int err) {
  Serial.printf("Firmware upload error code %d\n", err);
}

void save_new_firmware () {
    json2.set("version", firebaseFirmwareVersion);
    json2.set("filename", firebaseFirmwareFilename);
    
    Serial.printf("Saving new firmware version metadata to DB... %s\n", Firebase.updateNode(fbdo, "/"+hostString+"/FIRMWARE/CurrentFirmware/", json2) ? "ok" : fbdo.errorReason().c_str());

    currentFirmwareVersion = firebaseFirmwareVersion; 
    currentFirmwareFilename = firebaseFirmwareFilename; 
}