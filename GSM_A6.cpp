#include "GSM_A6.h"

/*
  �	Baud Rate 9600
  �	Requires 5V Power
  �	Only 3.3V logic for RX & TX, doesn't support 5V!!
  �	If in 'IP GPRSACT' then running configuration commands
      again will prevent the module from re-entering 'IP GPRSACT' state.
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
   Initialises the GSM Module and gets into sync with the GSM.
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

  if (!attemptSync("AT")) {
    #if defined( DEBUG_GSM )
      if (!attemptSync("AT")) {
        if (!attemptAutoTune()) return false;
      }
    #else
      return false;
    #endif
  }

  #if defined( DEBUG_GSM )
    if (myFile) {
      myFile.println(F("Success - In Sync"));
      myFile.println(F("Preparing"));
    }
  #endif
  
  sendAndWait("&F0"); // Reset Settings
  sendAndWait("E0"); // disable Echo
  sendAndWait("+CMEE=2"); // enable better error messages
  sendAndWait("+CPMS=\"SM\",\"SM\",\"SM\""); // Set SMS Storage for 3 memory areas
  sendAndWait("+CMGF=1");

  return true;
}

/*
   Tries to initailise communication with the GSM, using it's auto syncing ability.
   It sends the given command multiple times.
   @return true if the GSM could clearly respond to the command
*/
bool GSM_A6::attemptSync(const String & command) {
  for (uint8_t i = 0; i < 20; ++i) {
    Serial.print(command);
    Serial.print(GSM_END);
    Serial.flush();
    delay(40);
  }

  bool hasResponse = false;
  uint8_t counter = 0; // Time out counter

  while (!hasResponse && counter < 10) {
    delay(50);
    Serial.print(command);
    Serial.print(GSM_END);
    Serial.flush();
    hasResponse = waitFor("OK", 150) == 2;
    if (hasResponse) {
      delay(50);
      Serial.print(command);
      Serial.print(GSM_END);
      Serial.flush();
      hasResponse = waitFor("OK", 150) == 2;
    }
    ++counter;
  }

  return (counter < 10);
}

#if defined( DEBUG_GSM )
/*
  Loops through all the baud rates to see if it can find the current
  one the GSM is operating at.

  @return true if a line of communication has been setup with the GSM
*/
bool GSM_A6::attemptAutoTune() {

  uint8_t iterations = 0;
  uint8_t currentBaudRate = 0;
  long int baudRate[10] = { 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 74880, 115200};

  while ((!attemptSync("AT")) && iterations < 2) {
    ++currentBaudRate;
    if (currentBaudRate == 10) {
      currentBaudRate = 0;
      ++iterations;
    }
    Serial.flush();
    Serial.begin(baudRate[currentBaudRate]);
  }

  if (!(iterations == 2)) {
    myFile.println("Baud Rate Found at: ");
    myFile.println(baudRate[currentBaudRate]);
  } else {
    Serial.begin(9600);
    Serial.println(F("Baud Rate not Found"));
    Serial.println(F("Baud Rate not Found"));
  }

  return !(iterations == 2);
}

#endif

/*
  Sets the Network Provider

  @param networkProvider 0 == GIFFGAFF, 1 == THREE, 2 == ASDA

  @return true if the network provider was successfully set
*/
bool GSM_A6::setMobileNetwork(uint8_t networkProvider) {
  if (networkProvider == N_GIFFGAFF) {
    return connectToAPN(F("giffgaff.com"), F("giffgaff"), "");
  } else if (networkProvider == N_THREE) {
    return connectToAPN(F("three.co.uk"), "", "");
  } else if (networkProvider == N_ASDA) {
    return connectToAPN(F("everywhere"), F("eesecure"), F("secure"));
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

    if (!sendAndWait("+CGATT?", 4)) {
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
    delay(1500); 

    //Activate PDP Context - Page 136
    if (!sendAndWait(F("+CGACT=1,1"), 4)) {
      if (myFile) {
        myFile.println(F("Failed - Activate PDP Context"));
        myFile.print(F("GSM couldn't be attached to the mobile network "));
        myFile.println(F("(check APN settings and SIM Card is activated)"));
      }
      return false;
    }
    delay(1000);

    // Get network status for debugging
    if (myFile) {
      myFile.println(F("Getting Network Status:"));
    }

    // Check for old version first
    if (!sendAndWait(F("+CIPSTATUS"), "IP GPRSACT", 2)) {
      // The new version of the GSM A6 returns IP START here
      // And it requires and additional command to ready the internet connection
      if (sendAndWait(F("+CIPSTATUS"), "IP START" , 2)) {
        if (myFile) {
          myFile.println(F("Newer Version of GSM Detected, Firmware needs flash back"));
        }
        if (!sendAndWait(F("+CIICR"), 4)) return false;
        delay(2000);
      }
    }

    //Get IP Address - Page 161
    if (!sendAndWait("+CIFSR", 4)) {
      if (myFile) {
        myFile.println(F("Failed - Get IP"));
      }
      return false;
    }

    if (!sendAndWait(F("+CIPSTATUS"), "OK" , 4)) return false;

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
    if (!sendAndWait(F("+CGACT=1,1"), 4)) return false;
    delay(1500);

    // Check for older version first, as the GSM will then be quicker in the field
    if (!sendAndWait(F("+CIPSTATUS"), "IP GPRSACT", 2)) {
      // The new version of the GSM A6 returns IP START here
      // And it requires and additional command to ready the internet connection
      if (sendAndWait(F("+CIPSTATUS"), "IP START" , 2)) {
        if (!sendAndWait(F("+CIICR"), 4)) return false;
          delay(2000);
      }
    }
    //Get IP Address - Page 161
    if (!sendAndWait(F("+CIFSR"), 4)) return false;

    return true;
  #endif
}

/*
  Gets the Quality of the Signal Strength

  @return Quality_Rating of signal strength
*/
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

/*
  Returns the raw value of the signal strength

  @return signal strength
*/
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
        } else if (temp.indexOf("Unknown error") > 0) {
          break;
        } else if (temp.indexOf("FATAL ERROR") > 0) {
          return 99;
        }
      }
    }

  }
}

/*
  Gets the wireless communication error rate

  @return Quality_Rating of the Error rate
*/
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
        } else if (temp.indexOf("Unknown error") > 0) {
          break;
        } else if (temp.indexOf("FATAL ERROR") > 0) {
          return 99;
        }
      }
    }

  }
}

/**
   Initialises a TCP Connection with the server.
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

  if (!sendAndWait("+CIPSTART=\"TCP\",\"" + server + "\",80", 2)) {
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

  return true;
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
    if (myFile) myFile.println(F("Connecting To Network..."));
    
    uint8_t counter = 0;
    
    while (counter < 40) {
      ++counter;

      sendCommand("+CREG?");
      long start = millis();
      while (millis() - start < timeout) {
        String temp = Serial.readString();
    
        if (myFile) captureResponse(temp, start);
        
        int indexOf = temp.indexOf("+CREG: 1,1");

        if (indexOf > 0 && isDigit(temp.charAt(indexOf + 10))) {
          delay(1000);
          break;
        } else if (indexOf > 0) {
          if (myFile) myFile.println(F("Success - Connected"));
          return true;
        } else if (temp.indexOf("ERROR") > 0) {
          Serial.print(F("AT"));
          Serial.print(GSM_END);
          Serial.flush();
          delay(1000);
          Serial.readString();
          break;
        }
      }
    }
    
    if (myFile) myFile.println(F("Failed - Not Connected"));
    return false;
  #else
    
    uint8_t counter = 0;
    
    while (counter < 40) { // 5 seconds
      ++counter;

      sendCommand("+CREG?");
      long start = millis();
      while (millis() - start < timeout) {
        String temp = Serial.readString();
    
        if (temp.indexOf("+CREG: 1,10") > 0) {
          delay(100);
          break;
        } else if (temp.indexOf("+CREG: 1,1") > 0) {
          return true;
        } else if (temp.indexOf("ERROR") > 0) {
          Serial.print(F("AT"));
          Serial.print(GSM_END);
          Serial.flush();
          delay(100);
          Serial.readString();
          break;
        }
      }
    }
    
    return false;
  #endif
}

/*
   Waits for a response from the GSM. The lower the return number
   the more fatal of the error. The result is recorded to an SD Card if
   Debugging is Enabled, in the header file.

   @param expected Part of the desired response from the GSM Module
   @param timeout The time in milliseconds to timeout after.

   @return 0 for a fatal error, 1 for a minor error (try resending command),
              2 desired response was recieved from the GSM.
*/
uint8_t GSM_A6::waitFor(String expected = "OK", unsigned long timeout = 15000L) {
  long start = millis();
  while (millis() - start < timeout) {
    String temp = Serial.readString();

  #if defined( DEBUG_GSM )
    captureResponse(temp, start);
  #endif

    if (temp.indexOf(expected) > 0) {
      return 2;
    } else if (temp.indexOf("ERROR") > 0) {
      if (temp.indexOf("Excute command failure") > 0) {
        Serial.print(F("AT"));
        Serial.print(GSM_END);
        Serial.flush();
        delay(100);
        Serial.readString();
        return 1;
      } else if (temp.indexOf("Unknown error") > 0) {
        Serial.print(F("AT"));
        Serial.print(GSM_END);
        Serial.flush();
        delay(100);
        Serial.readString();
        return 1;
      } else if (temp.indexOf("FATAL ERROR") > 0) {
        return 0;
      } else {
        Serial.print(F("AT"));
        Serial.print(GSM_END);
        Serial.flush();
        delay(100);
        Serial.readString();
        return 1;
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

/*
  Used to start a SMS Message, should be followed by a phone number

  @return true if the SMS Message could be successfully started
*/
bool GSM_A6::startSMS() {
  if (!sendAndWait("+CMGF=1")) return false;
  delay(2000);
  Serial.print("AT+CMGS=\"");
  return true;
}

/*
  Marks the end of the phone number and the beginning of the SMS Body
*/
void GSM_A6::enterSMSContent() {
  Serial.write(0x22);
  Serial.print(GSM_END);
  delay(2000);
}

/*
  Finishes and sends the SMS Message
*/
void GSM_A6::sendSMS() {
  delay(500);
  Serial.println(char(26));
  Serial.print(GSM_END);
  Serial.flush();
  delay(1000);
}

/*
  Sends a SMS Message

  @param phoneNo The phone number to text
  @param message The message to send
*/
void GSM_A6::quickSMS(const String & phoneNo, const String & message) {
  startSMS();
  Serial.print(phoneNo);
  enterSMSContent();
  Serial.print(message);
  sendSMS();
}

/*
  Gets the total number of messages held on the SIM card.
  This only retrieves messages held on the SIM Card itself,
  as the GSM A6 has no other means of storage.

  Messages maybe recieved straight after calling this method,
  so it is useful to use 'hasNextMessage()' when iterating 
  through the list of messages.

  @return number of messages, 0 if none are present.
*/
uint8_t GSM_A6::totalMessages() {

  for (uint8_t i = 0; i < 2; ++i) {

    sendCommand("+CPMS?");

    #if defined( DEBUG_GSM )
      if (myFile) {
        myFile.println(F("Response:"));
      }
      uint8_t counter = 0;
    #endif

    long start = millis();
    while (millis() - start < 20000L) {
      String data = Serial.readStringUntil(',');

      #if defined( DEBUG_GSM )
        if (myFile) {
          ++counter;
          if (data.length() > 1) {
            myFile.print("Counter is: ");
            myFile.println(counter);
            myFile.print(data);
            myFile.print(F(","));
          }
        }
      #endif

      if (data.indexOf("+CPMS: ") > -1) {
        String data = Serial.readStringUntil(',');
        uint8_t noOfMessages = data.charAt(data.length() - 1) -'0'; // Last character

        #if defined( DEBUG_GSM )
          if (myFile) {
            myFile.print(data);
            myFile.print(F(","));
            data = Serial.readString();
            myFile.println(data);
            long currentTime = millis();
            myFile.print(F("Time Taken (ms): "));
            myFile.println(currentTime - start);
          }
        #else
          Serial.readString();
        #endif

        return noOfMessages;
      }
    }
  }
  return 0;
}

/*
  Start retrieving SMS messages from the beginning of the SMS Message List
  stored on the SIM Card
*/
void GSM_A6::startMessageCheck() {
  currentMessage = 0;
}

/*
  Checks to see if there is another message in the list.
  @return true if there is another message after the current message
*/
bool GSM_A6::hasNextMessage() {
  return currentMessage < totalMessages();
}

/*
  Gets the next SMS message from the sim card

  @return SMS_Message or {} if no message is present
*/
SMS_Message GSM_A6::getNextMessage() {
  return getSMS(++currentMessage);
}

/*
  Retrieves the message with the given ID, ID's start at 0.
  Sender phone number is formatted to remove the area code
  so +44... is 0...
*/
SMS_Message GSM_A6::getSMS(uint8_t messageID) {
  SMS_Message newMessage;

  for (uint8_t i = 0; i < 2; ++i) {
    //AT + CMGR = [messageID]
    sendCommand("+CMGR=" + String(messageID));

    #if defined( DEBUG_GSM )
      if (myFile) {
        myFile.println(F("Response:"));
      }
      uint8_t counter = 0;
    #endif
    
    long start = millis();
    while (millis() - start < 20000L) {
      String data = Serial.readStringUntil(',');

      #if defined( DEBUG_GSM )
        if (myFile) {
          ++counter;
          if (data.length() > 1) {
            myFile.print("Counter is: ");
            myFile.println(counter);
            myFile.print(data);
          }
        }
      #endif

      if (data.indexOf("+CMGR: ") > -1) {
        //uint8_t indexBeforeNo = data.lastIndexOf("+CMGR: ") + 6;
        //newMessage.id = getMessageID(data);
        newMessage.id = messageID;
        newMessage.status = data.indexOf("UNREAD") > -1 ? 1 : 0;

        Serial.read();
        data = Serial.readStringUntil(',');

        #if defined( DEBUG_GSM )
          if (myFile) {
            myFile.print(F(","));
            myFile.print(data);
          }
        #endif
        
        newMessage.sender = "0" + data.substring(4, 14);

        // Prints ,, - and any contents present between the comma's
        Serial.read();
        data = Serial.readStringUntil(',');
        Serial.read();
        
        #if defined( DEBUG_GSM )
          if (myFile) {
            myFile.print(F(","));
            myFile.print(data);
            myFile.print(F(","));
          }
        #endif

        data = Serial.readStringUntil(',');
        newMessage.timeReceived = data.substring(1, data.length());
        Serial.read();
        
        #if defined( DEBUG_GSM )
          if (myFile) {
            myFile.print(data);
            myFile.print(F(","));
          }
        #endif

        data = Serial.readStringUntil('"');
        newMessage.timeReceived = newMessage.timeReceived + "," + data;
        Serial.read(); // "

        #if defined( DEBUG_GSM )
          if (myFile) {
            myFile.print(data);
            myFile.println(F("\""));
          }
        #endif
        
        data = Serial.readString();
        newMessage.content = data.substring(2, data.lastIndexOf("OK")-4);

        #if defined( DEBUG_GSM )
          if (myFile) {
            myFile.println(newMessage.content);
            long currentTime = millis();
            myFile.println(F("OK"));
            myFile.print(F("Time Taken (ms): "));
            myFile.println(currentTime - start);
          }
        #endif

        delay(500);
        return newMessage;

      }

    }
  }

  delay(500);
  return {};
}


bool GSM_A6::deleteAllSMS() {
  return sendAndWait("+CMGD=1,4");
}

/*
bool GSM_A6::deleteSMS(uint8_t message) {
return sendAndWait("+CMGD=" + String(message) + ",0");
}

bool GSM_A6::deleteAllReadSMS() {
  return sendAndWait("+CMGD=1,1");
}
*/