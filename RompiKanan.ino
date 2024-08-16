#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Wire.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "iPhone"
#define WIFI_PASSWORD "apaaja123"

#define API_KEY "AIzaSyASxtsbff9kXX0wCEp2mDxGs7bynS3jwso"
#define DATABASE_URL "protel-7099e-default-rtdb.asia-southeast1.firebasedatabase.app"

#define SCK_PIN   D5  // SCK  -> D5 (GPIO14)
#define MISO_PIN  D6  // MISO -> D6 (GPIO12)
#define MOSI_PIN  D7  // MOSI -> D7 (GPIO13)
#define SS_PIN    D4  // SS   -> D8 (GPIO15)5z

PN532_SPI pn532spi(SPI, SS_PIN);
PN532 nfc(pn532spi);

const int piezoPin = A0;
int piezoValue = 0;
String kartuA = "e38bc415";
String tagA = "41c9312146f80";
String tagB = "4209312146f80";
String tagC = "4249312146f80";
String tagD = "4289312146f80";

String idnfc = "Body Kanan";

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiUDP ntpUDP;
// Initialize NTPClient with UTC+7 timezone offset (25200 seconds)
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200);


bool signupOK = false;

unsigned long counterk = 1;
unsigned long countert = 1;
unsigned long counter = 1;

void setup() {
  Serial.begin(115200);

  randomSeed(analogRead(0));

  nfc.begin();
   uint32_t versiondata = nfc.getFirmwareVersion();
   if (!versiondata) {
     Serial.print("Tidak menemukan modul PN53x");
     while (1);
        }
   nfc.SAMConfig();  // Configure to detect RFID cards
   Serial.println("Siap mendeteksi kartu RFID!");

  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected!");

  timeClient.begin();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

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
}

void loop() {
 uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  piezoValue = analogRead(piezoPin);

  timeClient.update();
  int randomNumber = random(200, 1025);

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success && Firebase.ready() && signupOK) {
    Serial.println("Found an NFC card!");

    Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < uidLength; i++) {
      Serial.print(" 0x");Serial.print(uid[i], HEX);
    }
    Serial.println("");

    // Construct a string from UID data
    String uidString = "";
    for (uint8_t i=0; i< uidLength; i++) {
      uidString += String(uid[i], HEX);
    }
    String hitid = "";
    
     if(uidString == tagA)
    {
      hitid = "Hit ke " + String (counterk++);
      hitid = hitid + " Kaki Kiri";
    }
    else if (uidString == tagB) {
      hitid = "Hit ke " + String (countert++);
      hitid = hitid + "Kaki kanan";
    }
    else if (uidString == tagC) {
      hitid = "Hit ke " + String (countert++);
      hitid = hitid + "Tangan kiri";
    }
    else if (uidString == tagD) {
      hitid = "Hit ke " + String (countert++);
      hitid = hitid + "Tangan kanan";
    }

    String idcount = String (counter++);
    
    // Send this UID data to Firebase

      if(Firebase.setString(fbdo,"timestamp",timeClient.getFormattedTime())){
      Serial.println("time data sent to Firebase successfully!");
    } else {
      Serial.println("Failed to send data to Firebase!");
      Serial.println(fbdo.errorReason());
      }

    if(Firebase.setString(fbdo,"hitfrom",hitid)){
      Serial.println("hit data sent to Firebase successfully!");
    } else {
      Serial.println("Failed to send data to Firebase!");
      Serial.println(fbdo.errorReason());
      }

    if(Firebase.setString(fbdo, "hitTo", idnfc)){
      Serial.println("target sent to Firebase successfully!");
    } else {
      Serial.println("Failed to send Piezoelectric data to Firebase!");
      Serial.println(fbdo.errorReason());
    }

    if(Firebase.setInt(fbdo,"piezoval",randomNumber))
    {
       Serial.println("piezo data sent to firebase!");
    } else {
      Serial.println("Failed to send Piezoelectric data to Firebase!");
      Serial.println(fbdo.errorReason());
    }


    delay(1000);
  }

}

