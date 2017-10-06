#ifndef _GSM_A6_h
#define _GSM_A6_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <SdFat.h>
#include <SPI.h>

#define GSM_END "\r\n"
#define GSM_OK "OK" + GSM_END
#define GSM_ERROR "ERROR" + GSM_END;

#define N_GIFFGAFF 0
#define N_THREE 1
#define N_ASDA 2

// Low the code number the greater the error
enum CommandExectuionStatus : uint8_t {
  FATAL_ERROR = 0,  // Serve Error
  FAILED = 1,       // Minor Error
  SUCCESS = 2,
};

enum Message_Status : uint8_t {
  READ = 0,
  UNREAD = 1,
};

struct SMS_Message {
  uint8_t id;
  Message_Status status;
  String timeRecieved;
  String sender;
  String content;
};

enum Quality_Rating : uint8_t {
  EXCELLENT = 0,
  VERY_GOOD = 1,
  GOOD      = 2,
  OKAY      = 3,
  NOT_GOOD  = 4,
  VERY_BAD  = 5,
  UNUSABLE  = 7,
  NOT_KNOWN = 99,
};

#define DEBUG_GSM

class GSM_A6 {
public:
  GSM_A6();

  bool init();
  bool setMobileNetwork(uint8_t networkProvider);
  bool connectToAPN(const String & apn, const String & username, const String & password);

  Quality_Rating getSignalStrength();
  Quality_Rating getSignalBitErrorRate();
  uint8_t getSignalStrengthRAW();

  bool getRequest(const String & server, const String & resource);
  bool startTCPConnection(const String & server);
  bool closeTCPConnection();

  bool waitForNetwork(unsigned long timeout = 20000L);
  bool sendAndWait(const String & command, uint8_t repeatAmountOnMinorError = 2);
  bool sendAndWait(const String & command, const String expected, uint8_t repeatAmountOnMinorError = 2);
  uint8_t waitFor(const String expected = "OK", unsigned long timeout = 20000L);
  void sendCommand(const String & command);
  void sendAT();

  bool quickSMS(const String & phoneNo, const String & message);
  bool startSMS();
  bool enterSMSContent();
  bool sendSMS();

  bool setSMSStorage();
  bool hasNextMessage(bool rollover = false);
  SMS_Message getNextMessage(bool containDate = true, bool containSender = true, bool containContent = true);
  SMS_Message getSMS(uint8_t message);

  bool deleteSMS(uint8_t messageIndex);
  bool deleteAllSMS();
  bool deleteAllReadSMS();

  #if defined( DEBUG_GSM )
    void captureResponse(String &temp, long & start);
    void stopDebugging();
    void printDebugFile();
  #endif

private:
  //SoftwareSerial& serialA6;
  uint8_t currentMessage;
  bool isSmsStorageSet;

  #if defined( DEBUG_GSM )
    bool isDebugging;
    File myFile;
    SdFat SD;
  #endif
};

#endif
