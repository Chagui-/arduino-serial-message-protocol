#include "EngineProtocol.h"
#include "pins_arduino.h"
#include "arduino.h"
#include "MessageBuilder.h"

#define DEBUG 0

Engine::Engine(char address[4]):
	m_current_time(0),
	m_last_time(0),	m_response_0_current_byte(0),
	m_response_1_current_byte(0), m_comms_enabled(0)
#ifdef __AVR_ATmega1280__
	,m_response_2_current_byte(0),m_response_3_current_byte(0),
	m_response_4_current_byte(0){
	m_response_2[response_length] = '\0';
	m_response_3[response_length] = '\0';
	m_response_4[response_length] = '\0';
#else
{
#endif // __AVR_ATmega1280__
	m_response_0[response_length] = '\0';
	m_response_1[response_length] = '\0';

	m_comm_1 = NULL;
	m_address = address;
}

void Engine::setup(){          
	delay(1000);
}

void Engine::run(){
	m_current_time = millis();

	readComm0();
	readComm1();
	
#ifdef __AVR_ATmega1280__
	readComm2();
	readComm3();
	readComm4();
#endif //__AVR_ATmega1280__

	checkForResend(m_current_time);
	
	m_last_time = m_current_time;
}

void Engine::checkForResend(long time){
	for (char i = 0; i < MESSAGE_QUEUE_SIZE; i++){
		Message& m = m_message_queue[i]; 
		if (m.isNull()) continue;
		if (m.getResendAttempts() >= 3){
			m.eraseMessage();
			if (DEBUG){
				Serial.println("Resend Stopped.");
			}
		}else if (m.isTimeout(time)){
			resend(m.getMessage());
			m.attemptResend(time);
		}
	}
}

void Engine::readComm0(){
	if (isCommEnabled(COMM_HARDWARE_0) == false) return;
	while (Serial.available() > 0){
		//get current byte
		char code = addToBuffer(Serial.read(),0);
		if (code == 1){
			continue;
		}else if (code == 2){
			break;
		}
	}
}


bool Engine::isCommEnabled(uint8_t flag_serial_comm){
	return (m_comms_enabled & flag_serial_comm) != 0;
}

#ifdef __AVR_ATmega1280__

void Engine::readComm2(){
	if (isCommEnabled(COMM_HARDWARE_1) == false) return;
	while (Serial1.available() > 0){
		//get current byte
		char code = addToBuffer(Serial1.read(),2);
		if (code == 1){
			continue;
		}else if (code == 2){
			break;
		}
	}
}

void Engine::readComm3(){
	if (isCommEnabled(COMM_HARDWARE_2) == false) return;
	while (Serial2.available() > 0){
		//get current byte
		char code = addToBuffer(Serial2.read(),3);
		if (code == 1){
			continue;
		}else if (code == 2){
			break;
		}
	}
}

void Engine::readComm4(){
	if (isCommEnabled(COMM_HARDWARE_3) == false) return;
	while (Serial3.available() > 0){
		//get current byte
		char code = addToBuffer(Serial3.read(),4);
		if (code == 1){
			continue;
		}else if (code == 2){
			break;
		}
	}
}
#endif // __AVR_ATmega1280__

void Engine::readComm1(){
	if (m_comm_1 == NULL) return;
	
	m_comm_1->listen();
	while (m_comm_1->available() > 0){
		//get current byte	
		char code = addToBuffer(m_comm_1->read(),1);
		if (code == 1){
			continue;
		}else if (code == 2){
			break;
		}
	}
}

char Engine::addToBuffer(char byte,uint8_t comm_port){
	//codes
	// 0 = normal
	// 1 = continue
	// 2 = break 
	char* buffer;
	int* index;
	switch (comm_port){
	case 0:
		buffer = m_response_0;
		index = &m_response_0_current_byte;
		break;
	case 1:
		buffer = m_response_1;
		index = &m_response_1_current_byte;
		break;
#ifdef __AVR_ATmega1280__
	case 2:
		buffer = m_response_2;
		index = &m_response_2_current_byte;
		break;
	case 3:
		buffer = m_response_3;
		index = &m_response_3_current_byte;
		break;
	case 4:
		buffer = m_response_4;
		index = &m_response_4_current_byte;
		break;
#endif // __AVR_ATmega1280__
	default:
		return 2;
	}
	buffer[*index] = byte;	
	//Serial.print(byte);
	if (buffer[*index] == ';' || *index == response_length - 1){
		buffer[*index + 1] = '\0'; //null terminator
		processSerialCommand(String(buffer), comm_port);
		*index = 0;
		return 2;
	}
	//Validate
	if (*index == 0 && buffer[0] != '>'){
		//invalid message. reset.
		*index = 0;
		return 1;
	}
	if (buffer[*index] == '>'){
		*index = 1;
		buffer[0] = '>';
		return 1;
	}
	//End Validate

	(*index)++;
	return 0;
}

void Engine::processSerialCommand(String command, uint8_t from_comm_port){
	char error_code = MessageBuilder::validate(command);
	if (error_code != 0){
		if (DEBUG){			
			Serial.print("Error: ");
			switch (error_code){
			case 1:
				Serial.print("Checksum: should be: ");
				Serial.println(MessageBuilder::computeChecksum(command));
			break;
			case 2:
				Serial.println("Missing Fields");
				break;
			default:
				Serial.println("Unknown");
				break;
			}
		}
		return;
	}
	//Check address
	String address_to = MessageBuilder::decodeAddressTo(command);
	//String address_from = MessageBuilder::decodeAddressFrom(command);
	if (address_to == m_address){//for me. Process.
		char flags;
		String address_from;
		String address_to;
		char type;
		char sub_type;
		String data;
		MessageBuilder::decodeMessageFields(command,flags,address_from,address_to,type,sub_type,data);
		
		if ((flags & FLAG_IMPORTANT) != 0){
			sendACK(command,from_comm_port);
		}else if ((flags & FLAG_ACK) != 0){
			ACKArrived(command);
			return;
		}

		if (m_processMessage){
			m_processMessage(flags,address_from,type,sub_type,data);
		}
	}else {//for others. Transmit.
		transmitMessage(command,from_comm_port);
	}
	//transmitMessage(command,from_comm_port);
}

void Engine::ACKArrived(String message){
	for (char i = 0; i < MESSAGE_QUEUE_SIZE; i++){
		Message& m = m_message_queue[i]; 
		if (m.isNull()) continue;
		if (m.validate(message)){
			m.eraseMessage();
		}
	}
}

void Engine::resend(String m){
	if (m_comm_1){//virtual
		m_comm_1->print(m);
	}

	if (isCommEnabled(COMM_HARDWARE_0))
		Serial.print(m);

#ifdef __AVR_ATmega1280__
	if (isCommEnabled(COMM_HARDWARE_1))
		Serial1.print(m);
	if (isCommEnabled(COMM_HARDWARE_2))
		Serial2.print(m);
	if (isCommEnabled(COMM_HARDWARE_3))
		Serial3.print(m);
#endif // __AVR_ATmega1280__	
}

void Engine::sendMessage(String m){
	if ((MessageBuilder::decodeFlags(m) & FLAG_IMPORTANT) != 0){
		enqueMessage(m);
	}

	if (m_comm_1){//virtual
		m_comm_1->print(m);
	}
	
	if (isCommEnabled(COMM_HARDWARE_0))
		Serial.print(m);

#ifdef __AVR_ATmega1280__
	if (isCommEnabled(COMM_HARDWARE_1))
		Serial1.print(m);
	if (isCommEnabled(COMM_HARDWARE_2))
		Serial2.print(m);
	if (isCommEnabled(COMM_HARDWARE_3))
		Serial3.print(m);
#endif // __AVR_ATmega1280__	
}

bool Engine::enqueMessage(String m){
	char i = 0;
	for (; i < MESSAGE_QUEUE_SIZE; i++){
		if (m_message_queue[i].isNull()){
			break;//found empty space
		}
	}
	if (i == MESSAGE_QUEUE_SIZE){
		//no empty space found
		return false;
	}
	m_message_queue[i].setMessage(m,millis(),1000);
	return true;
}

void Engine::sendACK(String message, uint8_t to_comm_port){
	MessageBuilder::setFlags(message,FLAG_ACK);
	MessageBuilder::invertAddress(message);
	MessageBuilder::setChecksum(message);
	if (m_comm_1){
		m_comm_1->print("ACK sent: ");
		m_comm_1->println(message);
	}
	switch (to_comm_port)	{
	case 0:
		if (isCommEnabled(COMM_HARDWARE_0))
			Serial.print(message);
		break;
	case 1:
		if (m_comm_1)
			m_comm_1->print(message);
		break;
#ifdef __AVR_ATmega1280__		
	case 2:
		if (isCommEnabled(COMM_HARDWARE_1))
			Serial1.print(message);
		break;
	case 3:
		if (isCommEnabled(COMM_HARDWARE_2))
			Serial2.print(message);
		break;
	case 4:
		if (isCommEnabled(COMM_HARDWARE_3))
			Serial3.print(message);
		break;
#endif // __AVR_ATmega1280__	
	default:
		break;
	}
}

void Engine::transmitMessage(String message, uint8_t from_comm_port){
	switch (from_comm_port){
	case 0:
		if (m_comm_1)
			m_comm_1->print(message);
#ifdef __AVR_ATmega1280__
		if (isCommEnabled(COMM_HARDWARE_1))
			Serial1.print(message);
		if (isCommEnabled(COMM_HARDWARE_2))
			Serial2.print(message);
		if (isCommEnabled(COMM_HARDWARE_3))
			Serial3.print(message);
#endif // __AVR_ATmega1280__
		break;
	case 1:
		if (isCommEnabled(COMM_HARDWARE_0))
			Serial.print(message);
#ifdef __AVR_ATmega1280__
		if (isCommEnabled(COMM_HARDWARE_1))
			Serial1.print(message);
		if (isCommEnabled(COMM_HARDWARE_2))
			Serial2.print(message);
		if (isCommEnabled(COMM_HARDWARE_3))
			Serial3.print(message);
#endif // __AVR_ATmega1280__
		break;
#ifdef __AVR_ATmega1280__		
	case 2:
		if (m_comm_1)
			m_comm_1->print(message);
		if (isCommEnabled(COMM_HARDWARE_0))
			Serial.print(message);
		if (isCommEnabled(COMM_HARDWARE_2))
			Serial2.print(message);
		if (isCommEnabled(COMM_HARDWARE_3))
			Serial3.print(message);
		break;
	case 3:
		if (m_comm_1)
			m_comm_1->print(message);
		if (isCommEnabled(COMM_HARDWARE_1))
			Serial1.print(message);
		if (isCommEnabled(COMM_HARDWARE_0))
			Serial.print(message);
		if (isCommEnabled(COMM_HARDWARE_3))
			Serial3.print(message);
		break;
	case 4:
		if (m_comm_1)
			m_comm_1->print(message);
		if (isCommEnabled(COMM_HARDWARE_1))
			Serial1.print(message);
		if (isCommEnabled(COMM_HARDWARE_2))
			Serial2.print(message);
		if (isCommEnabled(COMM_HARDWARE_0))
			Serial.print(message);
		break;
#endif // __AVR_ATmega1280__
	default:
		break;
	}
}

bool Engine::repeatWithPeriod(int period_millis){
	period_millis = period_millis / 2;
	if ((int)(m_current_time / period_millis) % 2 == 0 && (int)(m_last_time / period_millis) % 2 == 1){
		return true;
	}
	return false;
}