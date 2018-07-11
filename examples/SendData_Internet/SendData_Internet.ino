#include <GSM_A6.h>

/*
  This example transmits data to pushingbox using an ASDA sim card in the GSM
  This requires understanding of how pushing box works. Which can be seen here:
  http://www.open-sensing.org/evaporometerblog/datalog (Using Google Sheets)
  http://www.instructables.com/id/Post-to-Google-Docs-with-Arduino/ (Using Google Forms)
*/

// To enable debugging you must open the GSM_A6.h header file
// and uncomment #define DEBUG_GSM on line 10, or comment it 
// out to disable it. Debug mode requires an SD Card connection
// and for you to call gsm.stopDebugging(). Using Debug Mode uses
// much more memory and program space.
// The GSM_A6.h is normally located in your Arduino libraries
// For example: Documents/Arduino/libraries/GSM_A6/GSM_A6.h


GSM_A6 gsm = GSM_A6();

#define GSM_GND 4
#define GSM_RESET_PIN 17
#define C_GND 14

void setup() {
  Serial.begin(9600);
  pinMode(GSM_GND, OUTPUT);
  pinMode(GSM_RESET_PIN, OUTPUT);
  pinMode(C_GND, OUTPUT); // Turn on CGND for SD Card access
  digitalWrite(C_GND, HIGH);
  while (!Serial) {
    ;
  }

  String data = "HELLO_WORLD";

  setGsmOn(true);
  resetGSM();
  
  if (configureGSM()) {
    sendData(data);
    sendData2(data);
  } else {
    Serial.println("Failed to configure GSM, check mobile network and connection to arduino");
    Serial.println("If error continues, comment out DEBUG_GSM statement in GSM_A6.h requires SD Card connection!");
  }


  setGsmOn(false);

  #if defined( DEBUG_GSM )
    gsm.stopDebugging();
    gsm.printDebugFile();
  #endif

}

void loop() {

}

// Due to the addition of string more memory is used
bool sendData(const String & data) {
  bool successful = false;
  uint8_t counter = 0;
  while (!successful && counter < 2) {
    successful = gsm.getRequest(F("api.pushingbox.com"), "/pushingbox?devid=vB5C666821EA7EAF&ID=2&T=24.2&H=14.8&dt=09:10:00%2002/07/2018");
    ++counter;
  }
  return successful;
}

// Saves memory
bool sendData2(const String & data) {
  bool successful = false;
  uint8_t counter = 0;
  while (!successful && counter < 2) {
    successful = gsm.startTCPConnection(F("api.pushingbox.com")); // Server
    if (successful) {
      Serial.print(F("GET "));
      Serial.print(F("/pushingbox?devid=vB5C666821EA7EAF&")); // resource
      Serial.print("ID=");
      Serial.print(2);
      Serial.print("&T=");
      Serial.print(24.2);
      Serial.print("&H=");
      Serial.print(14.8);
      Serial.print("&dt=09:10:00%2002/07/2018");

      Serial.print(F(" HTTP/1.1\r\n"));
      Serial.print(F("Host: "));
      Serial.print(F("api.pushingbox.com"));
      Serial.print(F("\r\n"));
      Serial.print(F("Connection: close\r\n\r\n"));

      successful = gsm.closeTCPConnection();
    }
    ++counter;
  }
  return successful;
}

/*
  Turns the power on/off to the GSM,
  by providing power to relay pins
*/
void setGsmOn(bool isOn) {
  if (isOn) {
    digitalWrite(GSM_GND, HIGH);
    delay(6000); // GSM Needs to initialise
  } else {
    digitalWrite(GSM_GND, LOW);
    delay(100);
  }
}

/*
  Triggers a software reset on the GSM,
  by connecting the reset Pin on the GSM
  to ground. (Used after switching on the GSM
  to prevent bugs occurring)
*/
void resetGSM() {
  digitalWrite(GSM_RESET_PIN, HIGH);
  delay(2000);
  digitalWrite(GSM_RESET_PIN, LOW);
  delay(6000); // Give the GSM sufficient time to reinitialise
}

/*
  Initialises the GSM Module by
  configuring it's APN Settings
*/
bool configureGSM() {
  if (!gsm.init()) return false;

  if (!gsm.waitForNetwork()) return false;
  delay(1000);

  return gsm.setMobileNetwork(N_ASDA);
}
