#include <GSM_A6.h>

/*
  This example transmits data to pushingbox using an ASDA sim card in the GSM
*/

GSM_A6 gsm = GSM_A6();

uint8_t resetPin = 5; // Can be any pin

void setup() {
  Serial.begin(9600);
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
   Triggers a software reset on the GSM,
   by connecting the reset Pin on the GSM
   to ground. (Used after switching on the GSM
   to prevent bugs occurring)
*/
void resetGSM() {
  digitalWrite(resetPin, HIGH);
  delay(2000);
  digitalWrite(resetPin, LOW);
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
