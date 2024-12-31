#include <ESP8266WiFi.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "redmi"        // input your home or public wifi name
#define WIFI_PASSWORD "12345678" // password for WiFi

#define API_KEY "AIzaSyCGWIkcgdr03SkgSItKoprpvV7Pl6Jpeq8"
#define DATABASE_URL "https://ledtest-f6186-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo_new, fbdo_avb;
FirebaseAuth auth;
FirebaseConfig config;

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C display address 27 and 16x2 LCD display

bool signupOK = false;

String msg1 = "Occupied";
String msg2 = "available";

void setup() {
  delay(1000);
  Serial.begin(115200); // Serial debugging
  
  Wire.begin(D2, D1);   // I2C start

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected to IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  if (Firebase.signUp(&config, &auth, "", "")) { // Anonymous sign up
    Serial.println("SignUp OK");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  
  config.token_status_callback = tokenStatusCallback; // See addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.RTDB.beginStream(&fbdo_avb, "/ParkingStatus/Available")) {
    Serial.printf("stream available begin error: %s\n\n", fbdo_avb.errorReason().c_str());
  }

  if (!Firebase.RTDB.beginStream(&fbdo_new, "/ParkingStatus1/Available")) {
    Serial.printf("stream available begin error: %s\n\n", fbdo_new.errorReason().c_str());
  }

  lcd.begin(16, 2); // Begin LCD with the specified number of columns and rows
  lcd.backlight();
  //lcd.home();
  lcd.clear();
  //lcd.setCursor(0, 0); // 0th row and 0th column
  //lcd.print("Smart Parking");
  //lcd.clear();
}

void loop() {
  if (Firebase.ready() && signupOK) {
    //lcd.clear();

    if (!Firebase.RTDB.readStream(&fbdo_avb)) {
      Serial.print("stream available read error: ");
      Serial.println(fbdo_avb.errorReason().c_str());
    }
    if (fbdo_avb.streamAvailable()) {
      if (fbdo_avb.dataType() == "string") {
        String data1 = fbdo_avb.stringData();
        Serial.println("successful read from " + fbdo_avb.dataPath() + ": " + data1);
        
        lcd.setCursor(0, 0);
        lcd.print(data1+"-->");
        // Update the LCD or perform other actions based on `data`
      }
    }


    if (!Firebase.RTDB.readStream(&fbdo_new)) {
      Serial.print("stream available read error: ");
      Serial.println(fbdo_new.errorReason().c_str());
    }
    if (fbdo_new.streamAvailable()) {
      if (fbdo_new.dataType() == "string") {
        String data2 = fbdo_new.stringData();
        Serial.println("successful read from " + fbdo_new.dataPath() + ": " + data2);

        lcd.setCursor(0, 1);
        lcd.print("<--"+data2);
        // Update the LCD or perform other actions based on `data`
      }
    }

}

  delay(1000); // Add a delay to avoid flooding the server with requests
}
