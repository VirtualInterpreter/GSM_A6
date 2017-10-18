# GSM-A6 Readme
This library works for this GSM, it may or may not also work with other modules:
![Picture of GSM Module](https://github.com/MartinBKings/GSM-A6/blob/master/images/gsm.jpg)
This library is not able to receive SMS messages due to firmware problems, however, it can send SMS messages and connect to the internet.

## Basic GSM Information

* Baud Rate 9600, has been known to work with other rates too
* Requires 5V Power
* Only 3.3V logic for RX & TX, doesn’t support 5V
* If in ‘IP GPRSACT’ then running configuration commands such as ‘connectToAPN()’ again will prevent the module from re-entering ‘IP GPRSACT’ state.  (Use Reset Pin solves this)
* Advisable to reset after powering on using transistor as this prevents problems
* Some commands take a certain amount of time to complete, receiving OK does not necessarily mean that the command has been completed.
* <~ 350mA Power (When using functions in this library)
* There are other features in the GSM library which have not been explained here, see files.

### Connecting to GSM A6:
The GSM Module is wired as followed:

* RX of GSM to TX of Arduino
(The RX and TX pins of the GSM could also be connected to two other digital pins if the SoftwareSerial Library is used, however, the GSM library would have to be modified to use the software serial instead of the normal Serial.)
* TX of GSM to RX of Arduino
* RST to Collector of Transistor
* Base of Transistor to pin 5 of Arduino (Can be changed as needed)
* Emitter of Transistor to GND of GSM
* VCC5.0 to 5V Power Supply
* GND of GSM to both Arduino GND & 5V power supply GND
* PWK of GSM to VCC5.0 of GSM
* A 100k resistor is also needed between the VCC5.0 & the reset pin

#### Testing Connection

* After powering up the GSM, trigger the transistor to reset the GSM for 1-2 seconds. This step is necessary to prevent problems later.
* Using the GSM Library call the function ‘init()’, if this returns true communication with the device is possible, if it has returned false the device can’t be communicated.

## Making a GET Request (HTTP Request)

There are two ways in the GSM library to make a HTTP Request, the first can duplicate certain variables which may cause memory issues so if your resource URL or server name is quite large you may want to use the second approach to save RAM.
These approaches do not guarantee the integrity of the data received by the server as it may have got corrupted along the way. The data may also be intercepted/altered by a malicious user along the way.

Approach 1:

* Call the method ‘getRequest()’ with the name of your server and your resource for example: ‘api.pushingbox.com’ & ‘/pushingbox?devid=v0720sds45f’. If this method returns true then connection was established and the data was transmitted.

Approach 2: (more efficient)

* Use the methods ‘startTCPConnection()’ along with the server name to establish a TCP Connection that can be then used to transmit the HTTP Header manually via ‘Serial.print’ as can be seen in the ‘getRequest()’ method. After the HTTP Header has been sent the TCPConnection will need to be closed via ‘closeTCPConnection()’ before the TCP Connection times out, thus it is important not to use long delays before calling ‘closeTCPConnection()’.

## Sending a SMS

This can also be done in two different ways, the first approach as before may duplicate data causing memory problems when there is not enough memory left.
This method of sending data is also more secure than the last, however, less data can be transmitted this way.

Approach 1:

* Use the method ‘quickSMS()’ passing a phone number and a message.

Approach 2:

* Call startSMS() then ‘Serial.print’ the phone number.
* Then call enterSMSContent() and ‘Serial.print’ the sms message.
* Lastly call sendSMS()
