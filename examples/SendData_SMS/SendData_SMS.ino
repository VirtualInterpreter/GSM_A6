#include <GSM_A6.h>

/*
  Ensure you are using a GSM with the correct firmware
  and GSM A6 only.
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

  String data = "Message From My GSM";
  String phoneNo = "07721356574";

  Serial.println(F("Switching On..."));
  setGsmOn(true);

  Serial.println(F("GSM Is On, Resetting..."));
  resetGSM();

  if (configureGSM()) {
    sendData(phoneNo, data);  // <- Direct approach
    sendData2(phoneNo, data); // <- Maybe more useful in certain situations
    gsm.deleteAllSMS();
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
