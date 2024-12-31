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

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C display address 27 and 16x2 LCD display
Servo myservo;                     // Servo as gate for entry
Servo myservos;                    // Servo as gate for exit

int carEnter = D0;   // Entry sensor
int carExited = D3;  // Exit sensor
int slot1 = D7;       // slot 1
int slot2 = D8;       // slot 2
int slot3 = D4;        // Slot 3

boolean carEntry, carExit, s1, s2, s3;  //bool temp variables
int CLOSE_ANGLE = 90;  // The closing angle of the servo motor arm
int OPEN_ANGLE = 0;  // The opening angle of the servo motor arm 

long duration, distance;

int countYes = 0;
int allSpace = 3;
int Empty;

boolean s1_occupied = false;
boolean s2_occupied = false;
boolean s3_occupied = false;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

String msg1 = "Occupied";
String msg2 = "available";

void setup() {
  delay(1000);
  Serial.begin(115200); // Serial debugging
  
  Wire.begin(D2, D1);   // I2C start
  
  myservo.attach(D6);   // Servo pin to D6
  myservos.attach(D5);  // Servo pin to D5
  
  pinMode(slot1, INPUT); // IR sensor for slot1
  pinMode(slot2, INPUT);  // IR sensor for slot2
  pinMode(slot3, INPUT);  // IR sensor for slot3
  pinMode(carExited, INPUT); // IR sensor for exit as input
  pinMode(carEnter, INPUT);  // IR sensor for entry as input

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

  lcd.begin(16, 2); // Begin LCD with the specified number of columns and rows
  lcd.backlight();
  //lcd.home();
  lcd.clear();
  //lcd.setCursor(0, 0); // 0th row and 0th column
  lcd.print("Smart Parking");
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    //delayMicroseconds(2);

   carEntry= !digitalRead(carEnter);
   carExit = !digitalRead(carExited);
   s1 = !digitalRead(slot1);
   s2 = !digitalRead(slot2);
   s3 = !digitalRead(slot3);

    
    if (carEntry == 1) {               // If high, then count and send data
      countYes++;                         // Increment count
      if(countYes>3){
        Serial.print("Slots are Full\n");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Slots are Full");
        countYes--;
      }else{
      Serial.print("\nCar Entered \n");
      Serial.print("Total car =");
      Serial.println(countYes);
      lcd.setCursor(0, 1);
      lcd.print("Car Entered");
      myservos.write(OPEN_ANGLE);
      delay(3000);
      myservos.write(CLOSE_ANGLE);

      // Send updated count to Firebase
      if (Firebase.RTDB.setInt(&fbdo, "/ParkingStatus1/countYes", countYes)) {
        Serial.print("Count successfully saved to: ");
        Serial.println(fbdo.dataPath());
      } else {
        Serial.println("FAILED: " + fbdo.errorReason());
      }
      lcd.clear();
    }
    }

    if (carExit == 1) {                // If high, then count and send data
      countYes--;                         // Decrement count
      if(countYes<0){
        countYes++;
      }else{
      Serial.print("Car Exit\n");
      Serial.print("Total car =");
      Serial.println(countYes);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Car Exit");
      myservo.write(OPEN_ANGLE);
      delay(3000);
      myservo.write(CLOSE_ANGLE);

      // Send updated count to Firebase
      if (Firebase.RTDB.setInt(&fbdo, "/ParkingStatus1/countYes", countYes)) {
        Serial.print("Count successfully saved to: ");
        Serial.println(fbdo.dataPath());
      } else {
        Serial.println("FAILED: " + fbdo.errorReason());
      }
      lcd.clear();
    }
  }

    Empty = allSpace - countYes; // Calculate available spaces


    if (s1 == 1 && s1_occupied == false) {                     
          Serial.println("Occupied 1");
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Occupied 1");
          s1_occupied = true;

          if (Firebase.RTDB.setString(&fbdo, "/ParkingStatus1/slot1", msg1)) {
          Serial.print("parking slot successfully saved to: ");
          Serial.println(fbdo.dataPath());
          } else {
            Serial.println("FAILED: " + fbdo.errorReason());
          }
      }
    if(s1 == 0 && s1_occupied == true) {
       Serial.println("Available 1");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Available 1");
        s1_occupied = false;

        if (Firebase.RTDB.setString(&fbdo, "/ParkingStatus1/slot1", msg2)) {
        Serial.print("parking slot successfully saved to: ");
        Serial.println(fbdo.dataPath());
        } else {
          Serial.println("FAILED: " + fbdo.errorReason());
        }
    }


  if (s2 == 1 && s2_occupied == false) {                     
        Serial.println("Occupied 2 ");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Occupied 2");
        s2_occupied = true;

        if (Firebase.RTDB.setString(&fbdo, "/ParkingStatus1/slot2", msg1)) {
        Serial.print("parking slot successfully saved to: ");
        Serial.println(fbdo.dataPath());
        } else {
          Serial.println("FAILED: " + fbdo.errorReason());
        }
    }
  if(s2 == 0 && s2_occupied == true) {
       Serial.println("Available 2");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Available 2");
        s2_occupied = false;

        if (Firebase.RTDB.setString(&fbdo, "/ParkingStatus1/slot2", msg2)) {
        Serial.print("parking slot successfully saved to: ");
        Serial.println(fbdo.dataPath());
        } else {
          Serial.println("FAILED: " + fbdo.errorReason());
        }
  }


  if (s3 == 1 && s3_occupied == false) {                     
        Serial.println("Occupied 3");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Occupied 3");
        s3_occupied = true;

        if (Firebase.RTDB.setString(&fbdo, "/ParkingStatus1/slot3", msg1)) {
        Serial.print("parking slot successfully saved to: ");
        Serial.println(fbdo.dataPath());
        } else {
          Serial.println("FAILED: " + fbdo.errorReason());
        }
    }
  if(s3 == 0 && s3_occupied == true) {
       Serial.println("Available 3");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Available 3");
        s3_occupied = false;

        if (Firebase.RTDB.setString(&fbdo, "/ParkingStatus1/slot3", msg2)) {
        Serial.print("parking slot successfully saved to: ");
        Serial.println(fbdo.dataPath());
        } else {
          Serial.println("FAILED: " + fbdo.errorReason());
        }
    }


    String Available = String("Available= ") + String(Empty) + String("/") + String(allSpace);
    String fireAvailable = String("Available=") + String(Empty) + String("/") + String(allSpace);

    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(Available); // Print available spaces on LCD

    // Send available spaces to Firebase
    if (Firebase.RTDB.setString(&fbdo, "/ParkingStatus1/Available", fireAvailable)) {
      Serial.print("Availability successfully saved to: ");
      Serial.println(fbdo.dataPath());
    } else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }
  }
 
  //delay(500); // Add a delay to avoid flooding the server with requests
}
