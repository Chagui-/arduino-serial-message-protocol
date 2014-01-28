#include "EngineProtocol.h"
#include "pins_arduino.h"
#include "arduino.h"
#include "MessageBuilder.h"

#define DEBUG 1

Engine::Engine(int serial_baud, char address[4]):
	m_serial_baud(serial_baud),m_current_time(0),
	m_last_time(0),	m_response_0_current_byte(0),
	m_response_1_current_byte(0),m_response_2_current_byte(0){
	m_response_0[response_length] = '\0';
	m_response_1[response_length] = '\0';
	m_response_2[response_length] = '\0';
	m_comm_1 = NULL;
	m_comm_2 = NULL;
	m_address = address;
}

void Engine::setup(){          
	delay(1000);
	Serial.begin(m_serial_baud);
}

void Engine::run(){
	m_current_time = millis() % 1000000;

	readComm0();
	readComm1();
	readComm2();

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

void Engine::readComm2(){
	if (m_comm_2 == NULL) return;
	
	m_comm_2->listen();
	while (m_comm_2->available() > 0){
		//get current byte	
		char code = addToBuffer(m_comm_2->read(),2);
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
	if (comm_port == 0){
		buffer = m_response_0;
		index = &m_response_0_current_byte;
	} else if (comm_port == 1){
		buffer = m_response_1;
		index = &m_response_1_current_byte;
	} else if (comm_port == 2){
		buffer = m_response_2;
		index = &m_response_2_current_byte;
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
	if (m_comm_1){
		m_comm_1->print(m);
	}
	if (m_comm_2){
		m_comm_2->print(m);
	}
	Serial.print(m);
}

void Engine::sendMessage(String m){
	if ((MessageBuilder::decodeFlags(m) & FLAG_IMPORTANT) != 0){
		enqueMessage(m);
	}
	if (m_comm_1){
		m_comm_1->print(m);
	}
	if (m_comm_2){
		m_comm_2->print(m);
	}
	Serial.print(m);
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
	switch (to_comm_port)	{
	case 0:
		Serial.print(message);
		break;
	case 1:
		m_comm_1->print(message);
		break;
	case 2:
		m_comm_2->print(message);
		break;
	default:
		break;
	}
}

void Engine::transmitMessage(String message, uint8_t from_comm_port){
	switch (from_comm_port){
	case 0:
		if (m_comm_1)
			m_comm_1->print(message);
		if (m_comm_2)
			m_comm_2->print(message);
		break;
	case 1:
		Serial.print(message);
		if (m_comm_2)
			m_comm_2->print(message);
		break;

	case 2:
		Serial.print(message);
		if (m_comm_1)
			m_comm_1->print(message);
		break;
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