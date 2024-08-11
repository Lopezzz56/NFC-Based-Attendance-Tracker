#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "WiFi.h"
#include <HTTPClient.h>
//----------------------------------------

// Defines SS/SDA PIN and Reset PIN for RFID-RC522.

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (18)
#define PN532_MOSI (23)
#define PN532_SS   (5)
#define PN532_MISO (19)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (33)  // Example: Using GPIO 33 for IRQ
#define PN532_RESET (32)  // Example: Using GPIO 32 for RESET


// Defines the button PIN.
#define UP_BTN_PIN 15  //left attendance
#define DOWN_BTN_PIN 4   //right reg
#define OK_BTN_PIN 13 
#define CANCEL_PIN 26

//----------------------------------------SSID and PASSWORD of your WiFi network.
const char* ssid = "Lopezzz";  //--> Your wifi name
const char* password = "TrAv2028"; //--> Your wifi password
//----------------------------------------

// Google script Web_App_URL.
String Web_App_URL = "";
//google app script api
String reg_Info = "";
String atc_Info = "";
String atc_Name = "";
String atc_Date = "";
String atc_Time_In = "";
String atc_Time_Out = "";

// Variables for the number of columns and rows on the LCD.
int lcdColumns = 16;
int lcdRows = 2;

// Variable to read data from RFID-RC522.


int readsuccess;
char str[32] = "";
String UID_Result = "--------";

String modes = "";

// Create LiquidCrystal_I2C object as "lcd" and set the LCD I2C address to 0x27 and set the LCD configuration to 20x4.
// In general, the address of a 16x2 I2C LCD is "0x27".
// However, if the address "0x27" doesn't work, you can find out the address with "i2c_scanner". Look here : https://playground.arduino.cc/Main/I2cScanner/
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  // (lcd_address, lcd_Columns, lcd_Rows)

// Create MFRC522 object as "mfrc522" and set SS/SDA PIN and Reset PIN.
//MFRC522 mfrc522(SS_PIN, RST_PIN);  //--> Create MFRC522 instance.
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

const int numCourses = 4;
String courses[numCourses] = {"SITS0601", "SITS0602", "SITS0603", "SITS0604"};
int selectedCourseIndex = 0;
int counter = 0;
void selectCourseAndReadCard() {
  counter = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select CourseID");
  displayCourse(selectedCourseIndex);
  delay(200); // Debouncing delay

  while (true) {
    if (digitalRead(UP_BTN_PIN) == LOW) {
      if (selectedCourseIndex > 0) {
        selectedCourseIndex--;
        displayCourse(selectedCourseIndex);
        delay(200); // Debouncing delay
      }
    }

    if (digitalRead(DOWN_BTN_PIN) == LOW) {
      if (selectedCourseIndex < numCourses - 1) {
        selectedCourseIndex++;
        displayCourse(selectedCourseIndex);
        delay(200); // Debouncing delay
      }
    }

    if (digitalRead(OK_BTN_PIN) == LOW) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selected Course:");
      lcd.setCursor(0, 1);
      lcd.print(courses[selectedCourseIndex] + " CARD");

      while (true) {
        // Call getUID();
        readsuccess = getUID();
        if (readsuccess == -1) {
          break;
        } else if (readsuccess) {
          counter++;
          lcd.clear();
          delay(500);
          lcd.setCursor(0,0);
          lcd.print("Getting  UID");
          lcd.setCursor(0,1);
          lcd.print("Please wait...");
          delay(1000);

          http_Req(modes, UID_Result, courses[selectedCourseIndex]);

        } else {
          modes="";
          lcd.clear();
          Serial.print("BREAK");
          break;
        }
        if (digitalRead(CANCEL_PIN) == LOW) {
          selectedCourseIndex = 0;
          return; // Exit the function
        }

        delay(500); // Display selected course for 2 seconds
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(courses[selectedCourseIndex]);
        lcd.setCursor(11, 0);
        lcd.print("DONE");
        lcd.setCursor(0, 1);
        lcd.print("TAP CARD ");
        lcd.setCursor(13, 1);
        lcd.print(counter);
      }
    }

    if (digitalRead(CANCEL_PIN) == LOW) {
      selectedCourseIndex = 0;
      return; // Exit the function
    }
  }
}



//________________________________________________________________________________http_Req()
// Subroutine for sending HTTP requests to Google Sheets.
void http_Req(String str_modes, String str_uid, String courseName) {
  if (WiFi.status() == WL_CONNECTED) {
    String http_req_url = "";

    //----------------------------------------Create links to make HTTP requests to Google Sheets.
    if (str_modes == "atc") {
      http_req_url  = Web_App_URL + "?sts=atc";
      http_req_url += "&uid=" + str_uid;
      http_req_url += "&course=" + courseName; // Add course name to the URL
    }
    if (str_modes == "reg") {
      http_req_url = Web_App_URL + "?sts=reg";
      http_req_url += "&uid=" + str_uid;
    }
    //----------------------------------------

    //----------------------------------------Sending HTTP requests to Google Sheets.
    Serial.println();
    Serial.println("-------------");
    Serial.println("Sending request to Google Sheets...");
    Serial.print("URL : ");
    Serial.println(http_req_url);
    
    // Create an HTTPClient object as "http".
    HTTPClient http;

    // HTTP GET Request.
    http.begin(http_req_url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // Gets the HTTP status code.
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code : ");
    Serial.println(httpCode);

    // Getting response from google sheet.
    String payload;
    if (httpCode > 0) {
      payload = http.getString();
      Serial.println("Payload : " + payload);  
    }
    
    Serial.println("-------------");
    http.end();
    //----------------------------------------

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Example :                                                                                              //
    // Sending an http request to fill in "Time In" attendance.                                               //
    // User data :                                                                                            //
    // - Name : Adam                                                                                          //
    // - UID  : A01                                                                                           //
    // So the payload received if the http request is successful and the parameters are correct is as below : //
    // OK,Adam,29/10/2023,08:30:00 ---> Status,Name,Date,Time_In                                              //
    //                                                                                                        //
    // So, if you want to retrieve "Status", then getValue(payload, ',', 0);                                  //
    // String sts_Res = getValue(payload, ',', 0);                                                            //
    // So the value of sts_Res is "OK".                                                                       //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    String sts_Res = getValue(payload, ',', 0);

    //----------------------------------------Conditions that are executed are based on the payload response from Google Sheets (the payload response is set in Google Apps Script).
    if (sts_Res == "OK") {
      //..................
      if (str_modes == "atc") {
        atc_Info = getValue(payload, ',', 1);
        
        if (atc_Info == "TI_Successful") {
          atc_Name = getValue(payload, ',', 2);
          atc_Date = getValue(payload, ',', 3);
          atc_Time_In = getValue(payload, ',', 4);

          //::::::::::::::::::Create a position value for displaying "Name" on the LCD so that it is centered.
          int name_Lenght = atc_Name.length();
          int pos = 0;
          if (name_Lenght > 0 && name_Lenght <= lcdColumns) {
            pos = map(name_Lenght, 1, lcdColumns, 0, (lcdColumns / 2) - 1);
            pos = ((lcdColumns / 2) - 1) - pos;
          } else if (name_Lenght > lcdColumns) {
            atc_Name = atc_Name.substring(0, lcdColumns);
          }
          //::::::::::::::::::

          lcd.clear();
          delay(500);
          lcd.setCursor(3,0);
          lcd.print(atc_Name);
          lcd.setCursor(5,1);
          lcd.print("DONE");
          delay(3000);
          lcd.clear();
          delay(500);
        }

        if (atc_Info == "P_Successful") {
          atc_Name = getValue(payload, ',', 2);
          atc_Date = getValue(payload, ',', 3);
          atc_Time_In = getValue(payload, ',', 4);
          atc_Time_Out = getValue(payload, ',', 5);

          //::::::::::::::::::Create a position value for displaying "Name" on the LCD so that it is centered.
          int name_Lenght = atc_Name.length();
          int pos = 0;
          if (name_Lenght > 0 && name_Lenght <= lcdColumns) {
            pos = map(name_Lenght, 1, lcdColumns, 0, (lcdColumns / 2) - 1);
            pos = ((lcdColumns / 2) - 1) - pos;
          } else if (name_Lenght > lcdColumns) {
            atc_Name = atc_Name.substring(0, lcdColumns);
          }
          //::::::::::::::::::

          lcd.clear();
          delay(500);
          lcd.setCursor(3,0);
          lcd.print(atc_Name);
          lcd.setCursor(5,1);
          lcd.print("DONE ");
          delay(3000);
          lcd.clear();
          delay(500);
        }

        if (atc_Info == "atcErr02") {
          counter--;
          lcd.clear();
          delay(500);
          lcd.setCursor(0,0);
          lcd.print("You have completed");
          lcd.setCursor(0,1);
          lcd.print("your  attendance");
          delay(5000);
          lcd.clear();
          delay(500);
        }

        if (atc_Info == "atcErr01") {
          counter--;
          lcd.clear();
          delay(500);
          lcd.setCursor(6,0);
          lcd.print("Error !");
          lcd.setCursor(1,1);
          lcd.print("Not Registered.");
          delay(5000);
          lcd.clear();
          delay(500);
        }

        atc_Info = "";
        atc_Name = "";
        atc_Date = "";
        atc_Time_In = "";
        atc_Time_Out = "";
      }
      //..................

      //..................
      if (str_modes == "reg") {
        reg_Info = getValue(payload, ',', 1);
        
        if (reg_Info == "R_Successful") {
          lcd.clear();
          delay(500);
          lcd.setCursor(2,0);
          lcd.print("Registration");
          lcd.setCursor(3,1);
          lcd.print("Successful!");
          delay(5000);
          lcd.clear();
          delay(500);
        }

        if (reg_Info == "regErr01") {
          lcd.clear();
          delay(500);
          lcd.setCursor(6,0);
          lcd.print("Error !");
          lcd.setCursor(0,1);
          lcd.print("Already Registered");
          delay(5000);
          lcd.clear();
          delay(500);
        }

        reg_Info = "";
      }
      //..................
    }
    //----------------------------------------
  } else {
    lcd.clear();
    delay(500);
    lcd.setCursor(0,0);
    lcd.print("Error !");
    lcd.setCursor(1,1);
    lcd.print("WiFi disconnected");
    delay(3000);
    lcd.clear();
    delay(500);
  }
}
//________________________________________________________________________________


//________________________________________________________________________________getValue()
// String function to process the data (Split String).
// I got this from : https://www.electroniclinic.com/reyax-lora-based-multiple-sensors-monitoring-using-arduino/
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//________________________________________________________________________________ 

int getUID() {
  uint8_t success; 
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };// Buffer to store UID
  uint8_t uidLength; // Length of UID (4 or 7 bytes depending on tag type)
 Serial.print(" nfc scanning: ");

  delay(3500);//PRESS CANCEL BUTTON WITHIN THIS TIME OR SCAN A CARD.
  // Continuous check for cancel button before and during card reading
  while (true) {
    if (digitalRead(CANCEL_PIN) == LOW) {
      return 0; // Exit the loop
    }

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

   if (success) {
    Serial.print(" loopcheck: ");

     UID_Result = "";
    
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) {
        // If the byte is less than 0x10, pad it with a leading zero
        UID_Result += "0";
      }
      UID_Result += String(uid[i], HEX); // Convert byte to hexadecimal string
      if (i < uidLength - 1) {
        UID_Result += "-"; // Add a dash as separator between bytes
      }
    }
    
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
Serial.println(UID_Result);
    if (uidLength == 4) {
      // We probably have a Mifare Classic card ...
      uint32_t cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];
      cardid <<= 8;
      cardid |= uid[3];
      
      Serial.print("Seems to be a Mifare Classic card #");
      Serial.println(cardid);
    }
    // Add a short delay after updating the LCD display
    delay(1000);
    return 1; //SUCCESS
  }
  else{
    return 0;  //FAILURE
  }
  }
}

byte down[] = {
B00100,
B00100,
B00100,
B00100,
B00100,
B11111,
B01110,
B00100,
};

byte up[] = {
B00100,
B01110,
B11111,
B00100,
B00100,
B00100,
B00100,
B00100,
};

    

//setup--------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(1000);
  pinMode(UP_BTN_PIN, INPUT_PULLUP); // Set the ATC button pin as input with pull-up resistor
  pinMode(DOWN_BTN_PIN, INPUT_PULLUP); 
  pinMode(OK_BTN_PIN, INPUT_PULLUP);
  pinMode(CANCEL_PIN, INPUT_PULLUP);  //by default HIGH

  
 // Initialize NFC module
nfc.begin();

// Retrieve firmware version from the PN532 module
uint32_t versiondata = nfc.getFirmwareVersion();
Serial.println(versiondata);

// Check if firmware version retrieval was successful
if (!versiondata) {
  Serial.println("Didn't find PN53x board");
  while (1); // Halt
}

// Print out the retrieved firmware version
Serial.print("Found chip PN5");
Serial.println((versiondata >> 24) & 0xFF, HEX); // IC version
Serial.print("Firmware ver. ");
Serial.print((versiondata >> 16) & 0xFF, DEC); // Major version
Serial.print(".");
Serial.println((versiondata >> 8) & 0xFF, DEC); // Minor version

// Configure PN532 module (assuming this is part of the initialization)
nfc.SAMConfig();
  Serial.print(versiondata);

  //Serial.println("Please tap your NFC tag to the PN532 module.");

  // Initialize the LCD display
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Attendance");
  lcd.setCursor(5,1);
  lcd.print("System");


  delay(3000);
  lcd.clear();
  lcd.createChar(0, up);
  lcd.createChar(1, down);

  //----------------------------------------Set Wifi to STA mode
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");
  //---------------------------------------- 

  //----------------------------------------Connect to Wi-Fi (STA).
  Serial.println();
  Serial.println("------------");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

 //:::::::::::::::::: The process of connecting ESP32 with WiFi Hotspot / WiFi Router.
  // The process timeout of connecting ESP32 with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 has not been successfully connected to WiFi, the ESP32 will restart.

  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");

    lcd.setCursor(0,0);
    lcd.print("Connecting to");
    lcd.setCursor(0,1);
    lcd.print("Wifi...");
    delay(250);

    lcd.clear();
    delay(250);
    
    if (connecting_process_timed_out > 0) connecting_process_timed_out--;
    if (connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("------------");

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WiFi connected");
  delay(2000);
  //::::::::::::::::::
  //----------------------------------------

  lcd.clear();
  delay(500);

}


void loop() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Select mode");
  lcd.setCursor(0, 1);
  lcd.write((byte)0); //up - reg
  lcd.setCursor(1, 1);
  lcd.print("-REG");
  lcd.setCursor(11, 1);
  lcd.print("ATC-");
  lcd.setCursor(15, 1);
  lcd.write((byte)1); //down - atc
  delay(500);

   if (digitalRead(DOWN_BTN_PIN) == LOW) {
  Serial.println("loop1");
  modes = "atc";
  selectCourseAndReadCard();
  }

  // Check the state of the REG button
  if (digitalRead(UP_BTN_PIN) == LOW) {
    Serial.println("loop reg");
      modes = "reg";

   while(modes=="reg"){
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("REGISTRATION");
    lcd.setCursor(3,1);
    lcd.print("Tap card");
    delay(1000);

    readsuccess = getUID();
    if (readsuccess){
      lcd.clear();
      delay(500);
      lcd.setCursor(0,0);
      lcd.print("Getting  UID");
      lcd.setCursor(0,1);
      lcd.print("Please wait...");
      delay(1000);

      http_Req(modes, UID_Result, "");

      pinMode(CANCEL_PIN, INPUT_PULLUP);
    }
    else{
      modes="";
      lcd.clear();
      Serial.print("BREAK");
      break;
    }

    if (digitalRead(CANCEL_PIN) == LOW) {
          break; // Exit the function
        }
    delay(1000); // Delay for button debouncing     
   } //while
  }  //reg

  delay(10);
}


void displayCourse(int index) {
  lcd.setCursor(0, 1);
  lcd.print(" ");
  
  if (index == 0) {
    lcd.print(" ");
  } else {
    lcd.write((byte)0); // Left arrow
  }
  
  lcd.print(courses[index]);
  
  if (index == numCourses - 1) {
    lcd.print(" ");
  } else {
    lcd.write((byte)1); // Right arrow
  }
}


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
