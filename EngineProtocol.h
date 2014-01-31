#pragma once
#define response_length 50
#include "arduino.h"
#include "SoftwareSerial.h"
#include "Message.h"

class Engine{
private:
	char* m_address;
	//setup
	int m_serial_baud;
	//timing
	long m_current_time;
	long m_last_time;
	//communication	
	char m_response_0[response_length + 1];
	int m_response_0_current_byte;
	SoftwareSerial* m_comm_1;
	SoftwareSerial* m_comm_2;
	char m_response_1[response_length + 1];
	int m_response_1_current_byte;
	//process commands
	void (*m_processMessage)(char flags, String address_from, char type,char sub_type,String data);

	void blinckLed();
	void readComm0();
	void readComm1();
	void processSerialCommand(String command, uint8_t from_comm_port);
	void sendACK(String message, uint8_t to_comm_port);

	//Message queue
	#define MESSAGE_QUEUE_SIZE 10
	Message m_message_queue[MESSAGE_QUEUE_SIZE];
	bool enqueMessage(String m);
	void checkForResend(long time);
	void ACKArrived(String message);
	void resend(String m);

	//codes
	// 0 = normal
	// 1 = continue
	// 3 = break
	char addToBuffer(char byte,uint8_t comm_port);
public:
	Engine(int serial_baud, char address[4]);
	~Engine(void){}
	
	inline void setVirtualCommPort(SoftwareSerial* comm_1){
		m_comm_1 = comm_1;
	}

	void transmitMessage(String message, uint8_t from_comm_port);

	inline void setMessageProcessingFuction(void (*processMessage)(char flags, String address_from, char type,char sub_type,String data)){
		m_processMessage = processMessage;
	}

	void sendMessage(String m);

	void run();
	void setup();
	bool repeatWithPeriod(int period_millis);
};

