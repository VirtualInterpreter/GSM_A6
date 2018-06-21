# GSM-A6 Readme

This library works for this GSM, it may or may not also work with other modules:

Pin setup has been optimised to work with the 2G FreeStation.

![Picture of GSM Module](https://github.com/MartinBKings/GSM-A6/blob/master/images/gsm_small.jpg)

Ensure you have the correct firmware on your GSM, before trying to use features of it.

## Basic GSM Information

* Baud Rate 9600, has been known to work with other rates too
* Requires 5V Power
* Only 3.3V logic for RX & TX, doesn’t support 5V
* If in ‘IP GPRSACT’ then running configuration commands such as ‘connectToAPN()’ again will prevent the module from re-entering ‘IP GPRSACT’ state.  (Use Reset Pin solves this)
* Advisable to reset after powering on using transistor as this prevents problems
* Some commands take a certain amount of time to complete, receiving OK does not necessarily mean that the command has been completed.
* <~ 350mA Power (When using functions in this library)
* There are other features in the GSM library which have not been explained here, see files.
* Even when the GSM is powered off, the LED onboard may still draw power from RX & TX and light up.

### Recommended Components

* GSM A6
* 3V latching relay (EC2-3TNU) [datasheet](https://www.mouser.co.uk/datasheet/2/212/KEM_R7002_EC2_EE2-1104574.pdf)
* Transistor (KSP44BU)

### Connecting to GSM A6:
The GSM Module is wired as followed:

* RX of GSM to TX of Arduino
(The RX and TX pins of the GSM could also be connected to two other digital pins if the SoftwareSerial Library is used, however, the GSM library would have to be modified to use the software serial instead of the normal Serial.)
* TX of GSM to RX of Arduino
* RST to Collector of Transistor
* Base of Transistor to pin 17 (A3) of Arduino (Can be changed as needed)
* Emitter of Transistor to GND of GSM
* Using the datasheet of the 3V relay, connect pin 15 (A1) of the arduino to pin 4 or pin 9 of the relay.
* Likewise connect pin 4 of the arduino to pin 5 or 8 of the relay. If the former was chosen was then the former must be chosen here aswell.
* GND of GSM to both Arduino GND & 5V power supply GND
* PWK of GSM to VCC5.0 of GSM
* A 10k resistor is also needed between the VCC5.0 & the reset pin (It does not matter which way around the resistor is)

#### Testing Connection

* After powering up the GSM, trigger the transistor to reset the GSM for 1-2 seconds. This step is necessary to prevent problems later.
* Using the GSM Library call the function ‘init()’, if this returns true communication with the device is possible, if it has returned false the device can’t be communicated.

## Checking Firmware

Consult the firmware guide in the repo, software is included.

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
