#include <GSM_A6.h>

/*
  This example transmits data to pushingbox using an ASDA sim card in the GSM
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

  String data = "Message From My GSM";
  String phoneNo = "07721356574";

  Serial.println(F("Switching On..."));
  setGsmOn(true);

  Serial.println(F("GSM Is On, Resetting..."));
  resetGSM();

  if (configureGSM()) {
    sendData(phoneNo, data);  // <- Direct approach
    sendData2(phoneNo, data); // <- Maybe more useful in certain situations
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
