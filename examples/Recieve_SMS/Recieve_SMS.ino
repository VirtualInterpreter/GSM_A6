#include <GSM_A6.h>

/*
  Ensure you are using a GSM with the correct firmware
  and GSM A6 only. Debug mode must be active in GSM_A6.h file.
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

  Serial.println(F("Switching On..."));
  setGsmOn(true);

  Serial.println(F("GSM Is On, Resetting..."));
  resetGSM();

  if (configureGSM()) { // Can causes baud rate to change (Only happens in debug mode)
    readAllMessages();
    gsm.deleteAllReadSMS();
  } else {
    Serial.println("Failed to configure GSM, check mobile network and connection to arduino");
  }

  gsm.stopDebugging(); // Ensure debug file is closed
  setGsmOn(false);

  Serial.begin(9600);
  Serial.println("");
  Serial.println("Finished");
  gsm.printDebugFile();
}

void loop() {

}


/*
  Loops through all the available SMS Messages on the GSM,
  returning one SMS message at a time.
*/
void readAllMessages() {
  gsm.startMessageCheck();
  while (gsm.hasNextMessage()) {
    SMS_Message m1 = gsm.getNextMessage();
    printMessage(m1);
  }
}

/*
  Prints out an SMS Message
*/
void printMessage(SMS_Message &m) {

  Serial.println("Printing SMS Message...");
  Serial.print("Message ID: ");
  Serial.println(m.id);
  Serial.print("Status: ");
  
  if (m.status == Message_Status::READ) {
    Serial.println("Read");
  } else {
    Serial.println("Unread");
  }
  
  Serial.print("Time Received: ");
  Serial.println(m.timeReceived);
  Serial.print("From: ");
  Serial.println(m.sender);
  Serial.println("Message:");
  Serial.println(m.content);

  Serial.print("\r\n"); // Tells GSM that recent Serial print was not a command
  Serial.readString(); // Now Read input returned by GSM and discard
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
