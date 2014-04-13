#pragma once
#define response_length 80
#include "arduino.h"
#include "Message.h"
#include "SoftwareSerial.h"

#define COMM_HARDWARE_0 1
#define COMM_HARDWARE_1 2
#define COMM_HARDWARE_2 4
#define COMM_HARDWARE_3 8

class Engine{
private:
	//setup
	char* m_address;
	//timing
	long m_current_time;
	long m_last_time;
	//communication	
	char m_response_0[response_length + 1];
	int m_response_0_current_byte;
	SoftwareSerial* m_comm_1;
	uint8_t m_comms_enabled;
	char m_response_1[response_length + 1];
	int m_response_1_current_byte;

#ifdef __AVR_ATmega1280__
	char m_response_2[response_length + 1];
	int m_response_2_current_byte;
	char m_response_3[response_length + 1];
	int m_response_3_current_byte;
	char m_response_4[response_length + 1];
	int m_response_4_current_byte;
#endif // __AVR_ATmega1280__

	//process commands
	void (*m_processMessage)(char flags, String address_from, char type,char sub_type,String data);

	void blinckLed();
	void readComm0();
	void readComm1();

#ifdef __AVR_ATmega1280__
	void readComm2();
	void readComm3();
	void readComm4();
#endif //__AVR_ATmega1280__

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
	Engine(char address[4]);
	~Engine(void){}
	
	bool isCommEnabled(uint8_t flag_serial_comm);

	void setVirtualCommPort(SoftwareSerial* comm_1){
		m_comm_1 = comm_1;
	}


	void enableHardWareCommPort(uint8_t flags){
		m_comms_enabled = flags;
	}

	void transmitMessage(String message, uint8_t from_comm_port);

	void setMessageProcessingFuction(void (*processMessage)(char flags, String address_from, char type,char sub_type,String data)){
		m_processMessage = processMessage;
	}

	void sendMessage(String m);

	void run();
	void setup();
	bool repeatWithPeriod(int period_millis);
};

