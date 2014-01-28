#pragma once
#include "MessageBuilder.h"
class Message{
private:
	String m_message;
	char m_flags;
	long m_time_start;//millis
	long m_timeout;//millis
	char m_resend_attempts;

	bool m_null;

public:

	inline bool isTimeout(long current_time){
		long delta = (current_time - m_time_start);
		if (delta < 0){
			delta += 0xffff;
		}
		if (delta >= m_timeout){
			//Serial.println("timeout");
			return true;
		}
		return false;
	}

	inline char getResendAttempts(){
		return m_resend_attempts;
	}

	inline void attemptResend(long time_start){
		m_time_start = time_start;
		++m_resend_attempts;
	}

	inline void setMessage(String message, long time_start, long timeout){
		m_message = message;
		m_time_start = time_start;
		m_timeout = timeout;
		m_flags = MessageBuilder::decodeFlags(message);
		m_null = false;
	}

	inline bool validate(String m){
		char flags;
		String address_from;
		String address_to;
		char type;
		char sub_type;
		String data;
		MessageBuilder::decodeMessageFields(m,flags,address_from,address_to,type,sub_type,data);
		String m_address_from;
		String m_address_to;
		char m_type;
		char m_sub_type;
		String m_data;
		MessageBuilder::decodeMessageFields(m_message,flags,m_address_from,m_address_to,m_type,m_sub_type,m_data);
		if (address_from == m_address_to &&
			address_to == m_address_from &&
			type == m_type && 
			sub_type == m_sub_type &&
			data == m_data)
			return true;
		return false;
	}

	inline void eraseMessage(){
		m_null = true;
		m_resend_attempts = 0;
		m_flags = 0;
	}
	
	inline bool isNull(){
		return m_null;
	}

	inline bool hasFlag(char flag){
		if (m_flags & flag == 1)
			return true;
		return false;
	}

	inline String getMessage(){
		return m_message;
	}

	Message(void): m_null(true),m_flags(0),
		m_resend_attempts(0){
	}
	~Message(void){}
};

