#ifndef _GSM_A6_h
#define _GSM_A6_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//#define DEBUG_GSM

#if defined(DEBUG_GSM)
	#include <SdFat.h>
	#include <SPI.h>
#endif

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

  void quickSMS(const String & phoneNo, const String & message);
  bool startSMS();
  void enterSMSContent();
  void sendSMS();

  void startMessageCheck();
	// Does not work due to firmware problems
  bool hasNextMessage();
	// Does not work due to firmware problems
  SMS_Message getNextMessage(bool containDate = true, bool containSender = true, bool containContent = true);
	// Does not work due to firmware problems
  SMS_Message getSMS(uint8_t messageID);

	// Does not work due to firmware problems
  bool deleteSMS(uint8_t messageIndex);
	// Does not work due to firmware problems
  bool deleteAllSMS();
	// Does not work due to firmware problems
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

  uint8_t getMessageID(const String & message);

  #if defined( DEBUG_GSM )
    bool isDebugging;
    File myFile;
    SdFat SD;
  #endif
};

#endif
