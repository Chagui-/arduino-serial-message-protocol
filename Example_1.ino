#include "SoftwareSerial.h"
#include "EngineProtocol.h"

#define ADDRESS "0003"

#define BAUD_RATE 28800

#define NETWORK_1_RX_PIN 11
#define NETWORK_1_TX_PIN 6
#define NETWORK_1_BAUD BAUD_RATE

//timing
long current_time = 0;
long last_time = 0;

char msg_pin_old;

MessageBuilder mb;
Engine engine(BAUD_RATE,ADDRESS);
SoftwareSerial network_1(NETWORK_1_RX_PIN,NETWORK_1_TX_PIN);

void processMessage(char flags, String address_from, char type,char sub_type,String data){
	//Serial.println("Received message: ");
	//Serial.print("Flags: ");
	//Serial.println((int)flags);
	//Serial.print("From: ");
	//Serial.println(address_from);
	//Serial.print("Type: ");
	//Serial.println(type);
	//Serial.print("Sub-Type: ");
	//Serial.println(sub_type);
	//Serial.print("Data: ");
	//Serial.println(data);
}

void setup() {
	engine.setup();
	engine.setMessageProcessingFuction(&processMessage);
	network_1.begin(NETWORK_1_BAUD);// set optional virtual serial port
	engine.setVirtualCommPort(&network_1);

	pinMode(3,INPUT_PULLUP);//example button to send message

	//create example message
	mb.setFlags(0);
	mb.setAddressFrom(ADDRESS);
	mb.setAddressTo("0002");
	mb.setMessageType('1');
	mb.setMessageSubType('0');
	mb.setData("data");

}

void loop(){
	engine.run();
	char msg_pin = digitalRead(3);
	if (msg_pin == LOW && msg_pin_old == HIGH){//Send message by connecting pin 3 to GND
		mb.setData("" + String(random(0,100)) + ',' + String(random(0,16000)));
		engine.sendMessage(mb.getMessage());
		//Serial.print("Ram free: ");
		//Serial.println(freeRam());
	}
	msg_pin_old = msg_pin;
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}