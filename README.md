# NFC-Based-Attendance-Tracker
 Developed an NFC-Based Attendance Tracker using ESP32 and PN532 NFC module. Enabled easy user registration and attendance marking via NFC ID cards with real-time LCD feedback. Securely stored attendance data in a cloud-based database, enhancing accuracy and efficiency in attendance management for institutions and organizations.

 
Features
Easy user registration and attendance marking using NFC ID cards.
Real-time feedback on an LCD display.
Cloud-based storage of attendance data using Google Sheets.

Hardware Requirements
ESP32 microcontroller
PN532 NFC module
LCD display
NFC ID cards
Push buttons
Software Requirements
Arduino IDE
Google Apps Script
Google Account (for Google Sheets)
Setup Guide
1. Arduino Setup
Install Arduino IDE: If you haven't already, download and install the Arduino IDE.

Connect the ESP32: Connect your ESP32 microcontroller to your computer using a USB cable.

Install Required Libraries:

Open the Arduino IDE.
Go to Sketch > Include Library > Manage Libraries....
Search for and install the following libraries:
Adafruit PN532
Wire
LiquidCrystal_I2C
WiFi (if not pre-installed)
Upload the Code:

Download the final.ino file from this repository.
Open the final.ino file in the Arduino IDE.
Adjust the WiFi credentials in the code:
cpp
Copy code
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
Select the correct board and port:
Go to Tools > Board and select ESP32 Dev Module.
Go to Tools > Port and select the appropriate COM port.
Click the Upload button to flash the code to your ESP32.
2. Google Apps Script Setup
Create a Google Sheet:

Create a new Google Sheet to store attendance data.
Create a Google Apps Script:

In your Google Sheet, go to Extensions > Apps Script.
Delete any existing code in the script editor.
Paste the Provided Google Apps Script:

Copy the provided Google Apps Script code from the repository and paste it into the script editor.
Deploy the Script as a Web App:

Go to Deploy > New deployment.
Select Web app.
Under Execute as, choose Me.
Under Who has access, choose Anyone.
Click Deploy and authorize the script.
Copy the Web App URL for use in the Arduino code.
Update the Web App URL in final.ino:

In the final.ino file, find the line where the script URL is defined:
cpp
Copy code
const char* serverName = "YOUR_WEB_APP_URL";
Replace "YOUR_WEB_APP_URL" with the URL of your deployed Google Apps Script.
Test the System:

Power on the ESP32 and tap an NFC ID card on the PN532 module.
Verify that the attendance is recorded in your Google Sheet.
Troubleshooting
No NFC Response: Check wiring connections between the ESP32 and PN532 module.
No Data in Google Sheets: Ensure the Web App URL is correctly entered in the Arduino code and that the script has the necessary permissions.
