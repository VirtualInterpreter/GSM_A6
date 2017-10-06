#include "GSM_A6.h"

/*
  �	Baud Rate 9600
  �	Requires 5V Power
  �	Only 3.3V logic for RX & TX, doesn�t support 5V!!
  �	If in �IP GPRSACT� then running configuration commands
      again will prevent the module from re-entering �IP GPRSACT� state.
      (Use Reset Pin solves this)
  �	Advisable to reset after powering on
  �	If connecting current directly to VCC5 via an external device,
      attach the second ground pin to the Arduino
  �	Some commands take a certain amount of time to complete,
      receiving OK does not necessarily mean that the command has been completed.
  �	<~ 300mA Power (When using functions in this library)
*/
GSM_A6::GSM_A6() : currentMessage(255) { }

/*
   Initailises the GSM Module and gets into sync with the GSM.
   If the GSM fails to communicate with the Arduino this method
   returns false.

   @return true if the GSM is ready, false if communication with the GSM fails
 */
bool GSM_A6::init() {
  #if defined( DEBUG_GSM ) // Enable or Disable debug logging
    if (!SD.begin(10)) {
      isDebugging = false;
      Serial.println(F("Failed - Debugging"));
    } else {
      Serial.println(F("Success - Debugging GSM"));
      isDebugging = true;
      myFile = SD.open("GSM_log.txt", FILE_WRITE);
      myFile.println(F("Starting: "));
    }
    if (myFile) {
      myFile.println(F("Initailising GSM..."));
    }
  #endif

  for (uint8_t i = 0; i < 20; ++i) {
    Serial.print(F("AT"));
    Serial.print(GSM_END);
    Serial.flush();
    delay(40);
  }

  bool hasResponse = false;
  uint8_t counter = 0; // Time out counter

  while (!hasResponse && counter < 10) {
    delay(50);
    sendAT();
    hasResponse = waitFor("OK", 150) == 2;
    if (hasResponse) {
      delay(50);
      sendAT();
      hasResponse = waitFor("OK", 150) == 2;
    }
    ++counter;
  }

  if (counter >= 10) return false;

  #if defined( DEBUG_GSM )
    if (myFile) {
      myFile.println(F("Success - In Sync"));
      myFile.println(F("Preparing"));
    }
  #endif

  sendAndWait("&F0"); // Reset Settings
  sendAndWait("E0"); // disable Echo
  sendAndWait("+CMEE=2"); // enable better error messages

  return true;
}

bool GSM_A6::setMobileNetwork(uint8_t networkProvider) {
  if (networkProvider == N_GIFFGAFF) {
    return connectToAPN(F("giffgaff.com"), F("giffgaff"), "");
  } else if (networkProvider == N_THREE) {
    return connectToAPN(F("three.co.uk"), "", "");
  } else if (networkProvider == N_ASDA) {
    return connectToAPN(F("everywhere"), F("eescure"), F("secure"));
  } else {
    return false;
  }
}

/*
  Set up the mobile network APN settings, this varies from network to network.
  If no username or password is present for the network provide ""

  @param apn The APN name of the network
  @param username The username of the network
  @param password The password to the network

  @return true if the network was setup correctly, or false if it could not set the network up.
*/
bool GSM_A6::connectToAPN(const String & apn, const String & username, const String & password) {
  #if defined( DEBUG_GSM )
    // Get network status for debugging
    if (myFile) {
      myFile.println(F("Getting Network Status:"));
    }
    sendAndWait("+CIPSTATUS", "IP INITIAL", 2);

    //Attach Network - Page 133 & 136
    if (!sendAndWait("+CGATT=1", 4)) {
      if (myFile) {
        myFile.println(F("Failed - Attach Network"));
      }
      return false;
    }
    delay(1000);

    // Define PDP Context page 134
    if (!sendAndWait("+CGDCONT=1,\"IP\",\"" + apn + "\"")) {
      if (myFile) {
        myFile.println(F("Failed - Define PDP Context"));
      }
      return false;
    }
    delay(1000);

    //Set APN Details - Page 159
    if (!sendAndWait("+CSTT=\"" + apn + "\",\"" + username + "\",\"" + password + "\"")) {
      if (myFile) {
        myFile.println(F("Failed - Set APN Settings"));
      }
      return false;
    }
    delay(1000);

    //Activate PDP Context - Page 136
    if (!sendAndWait("+CGACT=1,1")) {
      if (myFile) {
        myFile.println(F("Failed - Activate PDP Context"));
      }
      return false;
    }
    delay(1000);

    // Get network status for debugging
    if (myFile) {
      myFile.println(F("Getting Network Status:"));
    }
    sendAndWait("+CIPSTATUS", "IP GPRSACT", 2);

    //Get IP Address - Page 161
    if (!sendAndWait("+CIFSR", 4)) {
      if (myFile) {
        myFile.println(F("Failed - Get IP"));
      }
      return false;
    }

    if (myFile) {
      myFile.println(F("Success - APN Connection"));
    }
    return true;
  #else
    //Attach Network - Page 133 & 136
    if (!sendAndWait(F("+CGATT=1"), 4)) return false;
    delay(1000);
    // Define PDP Context page 134 01:25
    if (!sendAndWait("+CGDCONT=1,\"IP\",\"" + apn + "\"")) return false;
    delay(1000);
    //Set APN Details - Page 159
    if (!sendAndWait("+CSTT=\"" + apn + "\",\"" + username + "\",\"" + password + "\"")) return false;
    delay(1500);
    //Activate PDP Context - Page 136
    if (!sendAndWait(F("+CGACT=1,1"))) return false;
    delay(1500);
    //Get IP Address - Page 161
    if (!sendAndWait(F("+CIFSR"), 4)) return false;
    return true;
  #endif
}

Quality_Rating GSM_A6::getSignalStrength() {
  uint8_t signalStrengthRAW = getSignalStrengthRAW();
  if (signalStrengthRAW <= 10) {
    return UNUSABLE;
  } else if (signalStrengthRAW <= 12) {
    return VERY_BAD;
  } else if (signalStrengthRAW <= 15) {
    return NOT_GOOD;
  } else if (signalStrengthRAW <= 20) {
    return OKAY;
  } else if (signalStrengthRAW <= 21) {
    return GOOD;
  } else if (signalStrengthRAW <= 26) {
    return VERY_GOOD;
  } else if (signalStrengthRAW <= 31) {
    return EXCELLENT;
  } else if (signalStrengthRAW == 99) {
    return NOT_KNOWN;
  }
}

uint8_t GSM_A6::getSignalStrengthRAW() {
  for (uint8_t i = 0; i < 2; ++i) {
    sendCommand("+CSQ");
    long start = millis();

    while (millis() - start < 9000) {
      String temp = Serial.readString();

      #if defined( DEBUG_GSM )
        captureResponse(temp, start);
      #endif

      if (temp.indexOf("OK") > 0) {
        uint8_t numEnd = temp.indexOf(",");
        return temp.substring(numEnd - 2, numEnd).toInt();
      } else if (temp.indexOf("ERROR")) {
        if (temp.indexOf("Excute command failure") > 0) {
          break;
        } else if (temp.indexOf("invalid command line") > 0) {
          break;
        } else if (temp.indexOf("FATAL ERROR") > 0) {
          return 99;
        }
      }
    }

  }
}

Quality_Rating GSM_A6::getSignalBitErrorRate() {
  for (uint8_t i = 0; i < 2; ++i) {
    sendCommand("+CSQ");
    long start = millis();

    while (millis() - start < 9000) {
      String temp = Serial.readString();

      #if defined( DEBUG_GSM )
        captureResponse(temp, start);
      #endif

      if (temp.indexOf("OK") > 0) {
        uint8_t numEnd = temp.indexOf(",");
        return temp.substring(numEnd + 1, numEnd + 3).toInt();
      } else if (temp.indexOf("ERROR")) {
        if (temp.indexOf("Excute command failure") > 0) {
          break;
        } else if (temp.indexOf("invalid command line") > 0) {
          break;
        } else if (temp.indexOf("FATAL ERROR") > 0) {
          return 99;
        }
      }
    }

  }
}

/**
   Initailises a TCP Connection with the server.
   This needs to be called before a HTTP Header can be sent.

   @param server The domain name or IP Address of the server

   @return true if a TCP Connection could be established or false if it can't.
 */
bool GSM_A6::startTCPConnection(const String & server) {
  #if defined( DEBUG_GSM )
    if (myFile) {
      myFile.println(F("Making Get Request..."));
    }
  #endif

  if (!sendAndWait("+CIPSTART=\"TCP\",\"" + server + "\",80")) {
    #if defined( DEBUG_GSM )
      if (myFile) {
        myFile.println(F("Failed"));
      }
    #endif
    return false;
  }
  delay(150);

  if (!sendAndWait("+CIPSEND", ">")) {
    return false;
  }

  return true;
}

/**
   Closes the TCP Connection made to the server and
   awaits the servers response. Should be called straight after
   HTTP Header has been sent. An Example of a HTTP Header can be seen
   in the getRequest Method.

   @return true if the TCP Connection was successfully close, otherwise false.
 */
bool GSM_A6::closeTCPConnection() {
  Serial.write(0x1A);
  #if defined( DEBUG_GSM )
    if (!waitFor()) {
      if (myFile) {
        myFile.println(F("Failed - Waiting"));
      }
    } else {
      if (myFile) {
        myFile.println(F("Success - Waiting"));
      }
    }
  #else
    waitFor();
  #endif


  // Close connection
  #if defined( DEBUG_GSM )
    if (!sendAndWait("+CIPCLOSE")) {
      if (myFile) {
        myFile.println(F("Failed - Close Connection"));
      }
      return false;
    } else {
      if (myFile) {
        myFile.println(F("Success - Close Connection"));
      }
      return true;
    }
  #else
    if (!sendAndWait("+CIPCLOSE")) return false;
  #endif

  return true;
}

/*
   Makes a get request to the resource specified at server.
   This is not currently designed to return data from the server!

   Example:
     Server = "api.pushingbox.com"
     Resource = "/pushingbox?devid=??????????"

   Less Memory efficient way to making a get request compared to
   startTCPConnection and closeTCPConnection.

   @param server The server of the resource, this can be an IP Address or
                    a domain name.

   @param resource The URL location of the resource you want to reach.
*/
bool GSM_A6::getRequest(const String & server, const String & resource) {

  #if defined( DEBUG_GSM )
    if (myFile) {
      myFile.println(F("Making Get Request..."));
    }
  #endif

  if (!sendAndWait("+CIPSTART=\"TCP\",\"" + server + "\",80")) {
    #if defined( DEBUG_GSM )
      if (myFile) {
        myFile.println(F("Failed"));
      }
    #endif
    return false;
  }
  delay(150);

  if (!sendAndWait("+CIPSEND", ">")) {
    return false;
  }

  Serial.print(F("GET "));
  Serial.print(resource);
  Serial.print(F(" HTTP/1.1\r\n"));
  Serial.print(F("Host: "));
  Serial.print(server);
  Serial.print(F("\r\n"));
  Serial.print(F("Connection: close\r\n\r\n"));
  Serial.write(0x1A);

  #if defined( DEBUG_GSM )
    if (!waitFor()) {
      if (myFile) {
        myFile.println(F("Failed - Waiting"));
      }
    } else {
      if (myFile) {
        myFile.println(F("Success - Waiting"));
      }
    }
  #else
    waitFor();
  #endif


  // Close connection
  #if defined( DEBUG_GSM )
    if (!sendAndWait("+CIPCLOSE")) {
      if (myFile) {
        myFile.println(F("Failed - Close Connection"));
      }
      return false;
    } else {
      if (myFile) {
        myFile.println(F("Success - Connection Closed"));
      }
      return true;
    }
  #else
    if (!sendAndWait("+CIPCLOSE")) return false;
  #endif
}

#if defined( DEBUG_GSM )

void GSM_A6::captureResponse(String &temp, long & start) {
  if (temp.length() > 1) {
    long currentTime = millis();
    if (myFile) {
      myFile.println(F("Response:"));
      myFile.println(temp);
      myFile.print(F("Time Taken (ms): "));
      myFile.println(currentTime - start);
    }
  }
}

/*
   Prints the contents recording from the debugging.
*/
void GSM_A6::printDebugFile() {
  if (myFile) {
    myFile.close();
  }
  myFile = SD.open("GSM_log.txt");
  if (myFile) {
    Serial.println(F("Reading GSM Log"));
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close();
  }
}

/*
   Ends the debugging session.
*/
void GSM_A6::stopDebugging() {
  if (myFile) {
    myFile.close();
  }
}
#endif

/*
   Connects to the mobile network.

   @param timeout The time in milliseconds to timeout after.

   @return true if the GSM successfully connected to the network else false.
*/
bool GSM_A6::waitForNetwork(unsigned long timeout = 20000L) {
  #if defined( DEBUG_GSM )
    if (myFile) {
      myFile.println(F("Connecting To Network..."));
      if (sendAndWait("+CREG?", "+CREG: 1,1")) {
        myFile.println(F("Success - Connected"));
        return true;
      } else {
        myFile.println(F("Failed - Not Connected"));
        return false;
      }
    } else {
      return sendAndWait("+CREG?", "+CREG: 1,1");
    }
  #else
    return sendAndWait("+CREG?", "+CREG: 1,1");
  #endif
}

/*
   Waits for a response from the GSM. The lower the return number
   the more fatal of the error.

   @param expected Part of the desired response from the GSM Module
   @param timeout The time in milliseconds to timeout after.

   @return 0 for a fatal error, 1 for a minor error (try resending command),
              2 desired response was recieved from the GSM.
*/
uint8_t GSM_A6::waitFor(String expected = "OK", unsigned long timeout = 20000L) {
  long start = millis();
  while (millis() - start < timeout) {
    String temp = Serial.readString();

    #if defined( DEBUG_GSM )
      captureResponse(temp, start);
    #endif

    if (temp.indexOf(expected) > 0) {
      return 2;
    } else if (temp.indexOf("ERROR")) {
      if (temp.indexOf("Excute command failure") > 0) {
        return 1;
      } else if (temp.indexOf("invalid command line") > 0) {
        return 1;
      } else if (temp.indexOf("FATAL ERROR") > 0) {
        return 0;
      }
    }
  }
}

/*
   Sends the given command to the GSM. Do not include
   AT in the command at the beginning of the command,
   as this is done by default.

   Example: command = "+CREG"
   sent to GSM = "AT+CREG"

   @param command The command to send to the GSM.
*/
void GSM_A6::sendCommand(const String & command) {
  #if defined( DEBUG_GSM )
    if (myFile) {
      myFile.print(F("Command: AT"));
      myFile.print(command);
      myFile.print(GSM_END);
    }
  #endif

  Serial.print(F("AT"));
  Serial.print(command);
  Serial.print(GSM_END);
  Serial.flush();
}

/*
   Sends a basic AT command.
*/
void GSM_A6::sendAT() {
  #if defined( DEBUG_GSM )
    if (myFile) {
      myFile.print(F("Command: AT\r\n"));
    }
  #endif

  Serial.print(F("AT"));
  Serial.print(GSM_END);
  Serial.flush();
}

/*
   Sends the given command to the GSM and awaits an OK response.

   @param command The command to send to the GSM. This should not include AT at the start.
   @param repeatAmountOnMinorError The amount of times to retry and send the command
                                    if the GSM returns a minor error as a response.

   @return true if the GSM responsed with OK otherwise false.
*/
bool GSM_A6::sendAndWait(const String & command, uint8_t repeatAmountOnMinorError = 2) {
  return sendAndWait(command, "OK", repeatAmountOnMinorError);
}

/*
   Sends the given command to the GSM and awaits the expected response.

   @param command The command to send to the GSM. This should not include AT at the start.
   @param expected The response to be expected from the GSM.
   @param repeatAmountOnMinorError The amount of times to retry and send the command
                                    if the GSM returns a minor error as a response.

   @return true if the GSM responsed with the expected response otherwise false.
*/
bool GSM_A6::sendAndWait(const String & command, const String expected, uint8_t repeatAmountOnMinorError = 2) {
  for (signed char i = 0; i < repeatAmountOnMinorError; ++i) {
    sendCommand(command);
    uint8_t status = waitFor(expected);
    if (status == 2) {
      return true;
    } else if (status == 0) {
      return false;
    }
  }
  return false;
}

bool GSM_A6::startSMS() {
  if (!sendAndWait("+CMGF=1")) return false;
  delay(2000);
  if (!sendAndWait("+CMGS=\"")) return false;
  return true;
}

bool GSM_A6::enterSMSContent() {
  Serial.write(0x22);
  Serial.print(GSM_END);
  delay(2000);
}

bool GSM_A6::sendSMS() {
  delay(500);
  Serial.println(char(26));
}

bool GSM_A6::quickSMS(const String & phoneNo, const String & message) {
  startSMS();
  Serial.print(phoneNo);
  enterSMSContent();
  Serial.print(message);
  sendSMS();
}

bool GSM_A6::setSMSStorage() {
  if (!isSmsStorageSet) {
    // AT + CPMS = "SM" - Set Storage
    isSmsStorageSet = true;
    return sendAndWait("+CPMS=\"SM\"");
  }
}

bool GSM_A6::hasNextMessage(bool rollover = false) {
  if (!setSMSStorage()) return false;

  for (uint8_t i = 0; i < 2; ++i) {
    sendCommand("+CMGL=\"ALL\"");

    long start = millis();
    while (millis() - start < 9000) {
      String temp = Serial.readString();

      #if defined( DEBUG_GSM )
        captureResponse(temp, start);
      #endif

      if (temp.indexOf("OK") > 0) {
        if (currentMessage == 255) { // Is currentMessage not set
          return temp.length() > 10; // Must be SMS
        } else {
          // Last SMS has highest index
          uint8_t indexof = temp.charAt(temp.lastIndexOf("+CMGL: "));
          String number = String(indexof + 1);
          if (temp.charAt(indexof + 2) != ',') {
            number = number + temp.charAt(temp.lastIndexOf("+CMGL: ") + 2);
          }
          return currentMessage < number.toInt();
        }
      } else if (temp.indexOf("ERROR")) {
        if (temp.indexOf("Excute command failure") > 0) {
          break;
        } else if (temp.indexOf("invalid command line") > 0) {
          break;
        } else if (temp.indexOf("FATAL ERROR") > 0) {
          return false;
        }
      }
    }
  }

  return false;
}

/*
   Not fully implemented yet, does not work!!
*/
SMS_Message GSM_A6::getNextMessage(bool containDate = true, bool containSender = true, bool containContent = true) {
  if (!setSMSStorage()) return {};

}
/*
  Not fully implemented yet, does not work!!
*/
SMS_Message GSM_A6::getSMS(uint8_t message) {
  if (!setSMSStorage()) return {};
  for (uint8_t i = 0; i < 2; ++i) {
    //AT + CMGR = [messageID]
    sendCommand("+CMGR=" + String(message));

    long start = millis();
    while (millis() - start < 9000) {
      String temp = Serial.readString();

      #if defined( DEBUG_GSM )
        captureResponse(temp, start);
      #endif

      if (temp.indexOf("OK") > 0) {

      } else if (temp.indexOf("ERROR")) {
        if (temp.indexOf("Excute command failure") > 0) {
          break;
        } else if (temp.indexOf("invalid command line") > 0) {
          break;
        } else if (temp.indexOf("FATAL ERROR") > 0) {
          return {};
        }
      }
    }
  }
  return {};
}

bool GSM_A6::deleteSMS(uint8_t message) {
  return sendAndWait("+CMGD=" + String(message) + ",0");
}

bool GSM_A6::deleteAllSMS() {
  return sendAndWait("+CMGD=1,4");
}

bool GSM_A6::deleteAllReadSMS() {
  return sendAndWait("+CMGD=1,1");
}
