#include "SoftwareSerial.h"
#include "EngineProtocol.h"

#define ADDRESS "0003"

#define NETWORK_1_RX_PIN 2
#define NETWORK_1_TX_PIN 3
#define NETWORK_1_BAUD 28800

#define NETWORK_2_RX_PIN 4
#define NETWORK_2_TX_PIN 5
#define NETWORK_2_BAUD 28800

bool debug_send_message = false;

//timing
long current_time = 0;
long last_time = 0;

MessageBuilder mb;
Engine engine(28800,ADDRESS);
SoftwareSerial network_1(NETWORK_1_RX_PIN,NETWORK_1_TX_PIN);
SoftwareSerial network_2(NETWORK_2_RX_PIN,NETWORK_2_TX_PIN);

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
	network_1.begin(NETWORK_1_BAUD);
	network_2.begin(NETWORK_2_BAUD);
	engine.setCommPort1(network_1);
	engine.setCommPort2(network_2);

	pinMode(3,INPUT_PULLUP);

	mb.setFlags(FLAG_IMPORTANT);
	mb.setAddressFrom(ADDRESS);
	mb.setAddressTo("0002");
	mb.setMessageType('0');
	mb.setMessageSubType('1');
	mb.setData("data");
}

void loop(){
	engine.run();
	bool msg_pin = digitalRead(3);
	if (debug_send_message == false && msg_pin == LOW){
		engine.sendMessage(mb.getMessage());
		debug_send_message = true;
		Serial.print("Ram free: ");
		Serial.println(freeRam());
	}
	if (msg_pin == HIGH){
		debug_send_message = false;
	}
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}