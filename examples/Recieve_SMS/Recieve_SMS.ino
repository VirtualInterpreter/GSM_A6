#include <GSM_A6.h>

/*
  Unable to recieve SMS messages due to commands not returning the message content
  with the rest of there data, so commands for this ascept do not seem to work.
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

  Serial.println("Reset GSM");
  delay(2000);
  resetGSM();

  if (configureGSM()) {
    Serial.println("AT+CMGF=?");
    delay(3000);
    String data = Serial.readString();
    
    Serial.println("ATOI");
    delay(3000);
    String data2 = Serial.readString();

    Serial.println("AT+CMGL=\"REC UNREAD\"");
    delay(3000);
    String data3 = Serial.readString();

    Serial.println("AT+CPMS?"); // Can be used to retrieve the total number of messages
    delay(3000);
    String data4 = Serial.readString();

    Serial.println("AT+CMGL=\"ALL\"");
    delay(4000);
    String data5 = Serial.readString();
    //gsm.getNextMessage();

    Serial.println("AT+CMGL=\"REC READ\"");
    delay(4000);
    String data6 = Serial.readString();

    Serial.println("AT+CMGR=1");
    delay(4000);
    String data7 = Serial.readString();

    Serial.println("AT+CMGR=2");
    delay(4000);
    String data8 = Serial.readString();

    Serial.println("AT+CMGR=3");
    delay(4000);
    String data9 = Serial.readString();

    Serial.println("====AT+CMGF=?");
    Serial.println(data);
    Serial.println("====AT+CMGR=?");
    Serial.println(data2);
    Serial.println("====AT+CMGL=\"REC UNREAD\"");
    Serial.println(data3);
    Serial.println("====AT+CPMS?");
    Serial.println(data4);
    Serial.println("====AT+CMGL=\"ALL\"");
    Serial.println(data5);
    Serial.println("====AT+CMGL=\"REC READ\"");
    Serial.println(data6);
    Serial.println("====AT+CMGR=1");
    Serial.println(data7);
    Serial.println("====AT+CMGR=2");
    Serial.println(data8);
    Serial.println("====AT+CMGR=3");
    Serial.println(data9);
    Serial.println("====AT+CMGR=4");

    
  } else {
    Serial.println("Failed to configure GSM, check mobile network and connection to arduino");
    Serial.println("If error continues, comment out DEBUG_GSM statement in GSM_A6.h requires SD Card connection!");
  }

}

void loop() {

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
