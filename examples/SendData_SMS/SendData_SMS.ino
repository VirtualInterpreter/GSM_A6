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

  String data = "Message From My GSM";
  String phoneNo = "07721356574";

  resetGSM();

  if (configureGSM()) {
    sendData(phoneNo, data);
    sendData2(phoneNo, data);
  } else {
    Serial.println("Failed to configure GSM, check mobile network and connection to arduino");
    Serial.println("If error continues, comment out DEBUG_GSM statement in GSM_A6.h requires SD Card connection!");
  }

}

void loop() {

}

void sendData(const String & phoneNo, const String & data) {
  gsm.quickSMS(phoneNo, data);
}

void sendData2(const String & phoneNo, const String & data) {
  if (gsm.startSMS()) {
    Serial.print(phoneNo);
    gsm.enterSMSContent();
    Serial.print(data);
    gsm.sendSMS();
  }
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
