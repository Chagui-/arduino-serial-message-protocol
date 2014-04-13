#include "SoftwareSerial.h"
#include "EngineProtocol.h"

#define ADDRESS "0001"

#define BAUD_RATE 28800

#define NETWORK_1_RX_PIN 11
#define NETWORK_1_TX_PIN 6
#define NETWORK_1_BAUD BAUD_RATE

MessageBuilder mb;
Engine engine(ADDRESS);
SoftwareSerial network_1(NETWORK_1_RX_PIN,NETWORK_1_TX_PIN);

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

void setup() {
	engine.setup();
	engine.setMessageProcessingFuction(&processMessage);

	Serial.begin(BAUD_RATE);

	engine.enableHardWareCommPort(COMM_HARDWARE_0);
	
	network_1.begin(NETWORK_1_BAUD);// set optional virtual serial port
	engine.setVirtualCommPort(&network_1);

	//create example message
	mb.setFlags(0);
	mb.setAddressFrom(ADDRESS);
	mb.setAddressTo("0002");
	mb.setMessageType('1');
	mb.setMessageSubType('5');
	mb.setData("data");
}

void loop(){
	engine.run();
	//char msg_pin = digitalRead(3);
	//if (msg_pin == LOW && msg_pin_old == HIGH){//Send message by connecting pin 3 to GND
	//	//mb.setData("" + String(random(0,100)) + ',' + String(random(0,8000)));
	//	mb.setData("" + String(random(0,2)));
	//	engine.sendMessage(mb.getMessage());
	//	//Serial.print("Ram free: ");
	//	//Serial.println(freeRam());
	//}
	//msg_pin_old = msg_pin;
}
