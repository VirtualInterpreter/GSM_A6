#include <GSM_A6.h>

/*
  Ensure you are using a GSM with the correct firmware
  and GSM A6 only. Debug mode must be active in GSM_A6.h file.
*/

#define RELAY_GSM_ON 15
#define RELAY_GSM_OFF 4
#define GSM_RESET_PIN 17
#define C_GND 14

GSM_A6 gsm = GSM_A6();

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
  
  Serial.println(F("Switching On..."));
  setGsmOn(true);

  Serial.println(F("GSM Is On, Resetting..."));
  resetGSM();

  if (gsm.init()) { // Can causes baud rate to change (Only happens in debug mode)
    gsm.sendAndWait("OI",2); // Ask for Version Information
  }

  gsm.stopDebugging();
  setGsmOn(false);

  Serial.begin(9600);
  Serial.println("");
  Serial.println("Finished");
  gsm.printDebugFile();
}

void loop() {

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
