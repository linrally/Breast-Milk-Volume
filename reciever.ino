#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "rl05"
#define WIFI_PASSWORD "aaaaaaaa"
#define API_KEY "AIzaSyD8tcXyA0qKaUPClM8IlGnD-_WaT1yYS3c"
#define DATABASE_URL "https://breast-milk-volume-default-rtdb.firebaseio.com/" 
#define USER_EMAIL "linrally@gmail.com"
#define USER_PASSWORD "aaaaaaaa"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String uid;
String usrPath;

const byte numChars = 32;
char receivedChars[numChars];  // an array to store the received data
boolean newData = false;
int dataInteger = 0;

void setup() {
  Serial.begin(9600);                       // Serial monitor
  Serial1.begin(9600, SERIAL_8N1, 16, 17);  // RX pin, TX pin
  initWifi();
  initFirebase();
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
  if (Serial1.available() > 0) {
    rc = Serial1.read();
    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0';  // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void readInteger() {
  if (newData == true) {
    dataInteger = 0;
    dataInteger = atoi(receivedChars);
    Serial.println(dataInteger);
    newData = false;
  }
}

void readAndPost(){
  if (Firebase.ready() && newData){
    dataInteger = atoi(receivedChars);
    Serial.println(dataInteger); //testing  
    //Firebase.RTDB.setIntAsync(&fbdo, usrPath + "/sensorRaw", dataInteger);
    newData = false;
  }
}

#define INTERVAL 1000  // Interval in milliseconds (adjust as needed)
#define WINDOW_SIZE 10
unsigned long lastPostTime = 0;
unsigned int movingAverageBuffer[WINDOW_SIZE]; // Buffer to store the latest values for the moving average
unsigned int movingAverageIndex = 0;
unsigned int movingAverageSum = 0;

void readAndPostMA(){
  if (Firebase.ready() && newData){
    dataInteger = atoi(receivedChars);

    movingAverageSum -= movingAverageBuffer[movingAverageIndex];
    movingAverageBuffer[movingAverageIndex] = dataInteger;
    movingAverageSum += dataInteger;
    movingAverageIndex = (movingAverageIndex + 1) % WINDOW_SIZE;

    unsigned long currentTime = millis();
    if (currentTime - lastPostTime >= INTERVAL) {
      unsigned int movingAverage = movingAverageSum / WINDOW_SIZE;
      Serial.println(movingAverage);
      Firebase.RTDB.setIntAsync(&fbdo, usrPath + "/sensorRaw", movingAverage);
      lastPostTime = currentTime;
    }

    newData = false;
  }
}

void initWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void initFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;
  
  Firebase.begin(&config, &auth);

  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.print(uid);
  
  usrPath = "users/" + uid;
}

void loop() {
  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
  recvWithEndMarker();
  //readInteger();
  readAndPostMA();
}