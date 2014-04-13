arduino-serial-message-protocol
===============================

A serial protocol to send and receive messages. Has checksum protection and can operate with ACK messages.

## Protocol Structure

| Start Flag | Msg. Flags | Address from | Address to | Msg. Type | Msg. Sub-Type | Data        | Checksum | Final Flag |
|:----------:|:----------:|:------------:|:----------:|:---------:|:-------------:|:-----------:|:--------:|:----------:|
|    &gt;    |      c     |     cccc     |    cccc    |     c     |       c       | c,c,...,c   |    cc    |      ;     |

Where "c" can be a char between '0' and '9', and 'a' and 'z'.

## Install

Copy the following files in your arduino/libraries/serial_protocol folder:

* EngineProtocol.h
* EngineProtocol.cpp
* Message.h
* MessageBuilder.h
* MessageBuilder.cpp

## Example file

There is an example file named "Example_1.ino". This file contains some basic demonstration of how to use this library.

## How to Use

A message has the structure showed in te aboce table. 
The flag of the message can be set to FLAG_IMPORTANT, to make the library wait for an acknowledge from the target device. In case the message is lost or becomes corrupted in any way, the sender will attempt to resend the message up to 3 times (with a timeout of 1 second).
Each device has an address composed of 4 chars. When you send a message you must know the address of the target device.
A message can include some data, in csv format. The target device knows how to parse the data based on the message type and sub-type.

The library needs some initialization in order to run:
```c
#define ADDRESS "0001"
Engine engine(28800,ADDRESS); //baud,address

void setup(){
	engine.setup(); //nedeed
	engine.setMessageProcessingFuction(&processMessage); // the function to receive the messages from the other devices
}

void loop(){
  engine.run();
}
```
The messages arrive at the function passed to setMessageProcessingFuction():
```c
void processMessage(char flags, String address_from, char type,char sub_type,String data){
	Serial.println("Received message: ");
	Serial.print("Flags: ");
	Serial.println((int)flags);
	Serial.print("From: ");
	Serial.println(address_from);
	Serial.print("Type: ");
	Serial.println(type);
	Serial.print("Sub-Type: ");
	Serial.println(sub_type);
	Serial.print("Data: ");
	Serial.println(data);
}
```
The message is constructed in this way:
``` c
#define ADDRESS "0001"
Engine engine(28800,ADDRESS); //baud,address
MessageBuilder mb;

void constructMessage(){
  mb.setFlags(FLAG_IMPORTANT);
  mb.setAddressFrom(ADDRESS);
  mb.setAddressTo("0002");
  mb.setMessageType('0');
  mb.setMessageSubType('1');
  mb.setData("data");
}
```
To send the message you only use `engine.sendMessage(mb.getMessage())`

The library can have a secondary virtual serial port:

```c
#include "SoftwareSerial.h"
#define BAUD_RATE 28800

#define NETWORK_1_RX_PIN 11
#define NETWORK_1_TX_PIN 6
#define NETWORK_1_BAUD BAUD_RATE

SoftwareSerial network_1(NETWORK_1_RX_PIN,NETWORK_1_TX_PIN);

void setup(){
	network_1.begin(NETWORK_1_BAUD);
	engine.setVirtualCommPort(&network_1);// set optional virtual serial port
}

```
