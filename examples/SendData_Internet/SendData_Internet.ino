#include <GSM_A6.h>

/*
  This example transmits data to pushingbox using an ASDA sim card in the GSM
  This requires understanding of how pushing box works. Which can be seen here:
  http://www.open-sensing.org/evaporometerblog/datalog (Using Google Sheets)
  http://www.instructables.com/id/Post-to-Google-Docs-with-Arduino/ (Using Google Forms)
*/

GSM_A6 gsm = GSM_A6();

#define RELAY_GSM_ON 15 // For Relay controlling GSM Power
#define RELAY_GSM_OFF 4 // For Relay controlling GSM Power
#define GSM_RESET_PIN 17 // Can be any pin linked to transister
#define C_GND 14

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_GSM_OFF, OUTPUT);
  pinMode(RELAY_GSM_ON, OUTPUT);
  pinMode(GSM_RESET_PIN, OUTPUT);
  pinMode(C_GND, OUTPUT); // Turn on CGND for SD Card access
  digitalWrite(C_GND, HIGH);
  while (!Serial) {
    ;
  }

  String data = "HELLO_WORLD";

  resetGSM();
  if (configureGSM()) {
    sendData(data);
    sendData2(data);
  } else {
    Serial.println("Failed to configure GSM, check mobile network and connection to arduino");
    Serial.println("If error continues, comment out DEBUG_GSM statement in GSM_A6.h requires SD Card connection!");
  }
}

void loop() {

}

// Due to the addition of string more memory is used
bool sendData(const String & data) {
  bool successful = false;
  uint8_t counter = 0;
  while (!successful && counter < 2) {
    successful = gsm.getRequest(F("api.pushingbox.com"), "/pushingbox?devid=vF79F2CC4B20841C&data=" + data);
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
      Serial.print(F("/pushingbox?devid=vF79F2CC4B20841C&")); // resource
      Serial.print("data=");
      Serial.print(data);

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
    digitalWrite(RELAY_GSM_ON, HIGH);
    delay(50);
    digitalWrite(RELAY_GSM_ON, LOW);
    delay(6000); // GSM Needs to initialise
  } else {
    digitalWrite(RELAY_GSM_OFF, HIGH);
    delay(50);
    digitalWrite(RELAY_GSM_OFF, LOW);
    delay(1000);
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
