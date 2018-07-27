# GSM-A6 Readme

This library works for this GSM, it may or may not also work with other modules:

Pin setup has been optimised to work with the 2G [FreeStation](http://www.freestation.org).

![Picture of GSM Module](https://github.com/VirtualInterpreter/GSM_A6/blob/master/images/gsm_small.jpg)

Ensure you have the correct firmware on your GSM, before trying to use features of it.

## Basic GSM Information

* Baud Rate 9600, has been known to work with other rates too
* Requires 5V Power
* Only 3.3V logic for RX & TX, doesn’t support 5V
* If in ‘IP GPRSACT’ then running configuration commands such as ‘connectToAPN()’ again will prevent the module from re-entering ‘IP GPRSACT’ state.  (Use Reset Pin solves this)
* Advisable to reset after powering on using transistor as this prevents problems
* Some commands take a certain amount of time to complete, receiving OK does not necessarily mean that the command has been completed.
* Power consumption can spike to around	~700mA, average power use is much lower
* There are other features in the GSM library which have not been explained here, see files.

### Recommended Components

* GSM A6
* 5V Step Up
* 2 AA Batteries (2-3.5v combined)
* 2x 680 Resistor
* 1x 1K Resistor
* 2x 10K Resistor
* MOSFET (RS-Code: 325-7580) [Link](https://uk.rs-online.com/web/p/mosfet-transistors/3257580/?sra=pstk)
* 2x Transistor (RS-Code: 806-4548) [Link](https://uk.rs-online.com/web/p/bipolar-transistors/8064548/?sra=pstk)

### Connecting to GSM A6:
The GSM Module is wired as followed: (See image folder for pictures, it does not matter which way around resistors are)

* Connect VCC5.0 of GSM to PWR of GSM
* RX of GSM to TX of Arduino
(The RX and TX pins of the GSM could also be connected to two other digital pins if the SoftwareSerial Library is used, however, the GSM library would have to be modified to use the software serial instead of the normal Serial.)
* TX of GSM to RX of Arduino
* RST pin of GSM to Collector Pin of first Transistor
* Base of first Transistor to pin 17 (A3) of Arduino with a 680 Resistor in between
* Emitter of first Transistor to GND of GSM
* Base of second Transistor to pin 4 of Arduino with a 680 Resistor in between
* Collector of second Transistor to GND pin of Arduino
* Emitter of second Transistor GND of GSM
* Gate of MOSFET to pin 4 of Arduino with 1k resistor in between
* 10k Resistor between Gate and Source of MOSFET
* Drain of MOSFET to Ground Input of Step Up Converter
* Source of MOSFET to Ground of Power Supply
* Postive of Power Supply to Postive Input of Step Up Converter
* Output side Ground of Step Up Converter to GND of GSM
* Output side Postive/5v of Step Up Converter to VCC5.0 of GSM
* A 10k resistor is also needed between the VCC5.0 or PWR to the RST Pin of the GSM

#### Testing Connections
Try using some of the example sketches or:

* After powering up the GSM, trigger the transistor to reset the GSM for 1-2 seconds. This step is necessary to prevent problems later.
* Using the GSM Library call the function ‘init()’, if this returns true communication with the device is possible, if it has returned false the device can’t be communicated with.

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
