#pragma once
#include "Arduino.h"
//Flags
#define FLAG_IMPORTANT 1
#define FLAG_ACK 2

class MessageBuilder{
private:
	char m_flags;
	String m_address_from;
	String m_address_to;
	char m_message_type;
	char m_message_sub_type;
	String m_data;

	static String intToBase36(int i);
	static int base36ToInt(String str);
public:
	static void setChecksum(String& message);
	//Error codes
	// 0 = Ok.
	// 1 = checksum error
	// 2 = fields missing
	static char validate(String message);
	static String computeChecksum(String message);

	static char decodeFlags(String message);
	static String decodeAddressTo(String message);
	static String decodeAddressFrom(String message);
	static char decodeType(String message);
	static char decodeSubType(String message);
	static String decodeData(String message);
	static void decodeMessageFields(String message,
		char& flags, String& address_from, String& address_to,
		char& type,char& sub_type,String& data);

	static void setFlags(String& message, char codes);
	static void setAddressFrom(String& message, String address);
	static void setAddressTo(String& message, String address);
	static void invertAddress(String& message);

	void setFlags(char codes);
	void setAddressFrom(String address);
	void setAddressTo(String address);
	void setMessageType(char type);
	void setMessageSubType(char sub_type);
	void setData(String data);
	String getMessage();


	MessageBuilder(void);
	~MessageBuilder(void);
};

