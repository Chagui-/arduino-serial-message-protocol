#include "MessageBuilder.h"

MessageBuilder::MessageBuilder(void):
	m_flags('-'),m_message_type('-'),m_message_sub_type('-'){}

MessageBuilder::~MessageBuilder(void){

}

void MessageBuilder::setFlags(char codes){
	m_flags = intToBase36(codes).charAt(0);
}

void MessageBuilder::setAddressFrom(String address){
	m_address_from = address;
}

void MessageBuilder::setAddressTo(String address){
	m_address_to = address;
}

void MessageBuilder::setMessageType(char type){
	m_message_type = type;
}

void MessageBuilder::setMessageSubType(char sub_type){
	m_message_sub_type = sub_type;
}

void MessageBuilder::setData(String data){
	m_data = data;
}

void MessageBuilder::setFlags(String& message, char codes){
	char flags = intToBase36(codes).charAt(0);
	message.setCharAt(1,flags);
}

void MessageBuilder::invertAddress(String& message){
	String address_from = decodeAddressFrom(message);
	String address_to = decodeAddressTo(message);
	setAddressFrom(message,address_to);
	setAddressTo(message,address_from);
}


void MessageBuilder::setAddressFrom(String& message, String address){
	message.setCharAt(2,address.charAt(0));
	message.setCharAt(3,address.charAt(1));
	message.setCharAt(4,address.charAt(2));
	message.setCharAt(5,address.charAt(3));
}

void MessageBuilder::setAddressTo(String& message, String address){
	message.setCharAt(6,address.charAt(0));
	message.setCharAt(7,address.charAt(1));
	message.setCharAt(8,address.charAt(2));
	message.setCharAt(9,address.charAt(3));
}

String MessageBuilder::getMessage(){
	String output = String('>') + m_flags + 
		m_address_from + m_address_to + 
		m_message_type + m_message_sub_type + 
		m_data + "00;";
	setChecksum(output);
	return output;
}

String MessageBuilder::decodeAddressTo(String message){
	return message.substring(6,10);
}

String MessageBuilder::decodeAddressFrom(String message){
	return message.substring(2,6);
}

void MessageBuilder::decodeMessageFields(String message,
	char& flags, String& address_from, String& address_to,
	char& type,char& sub_type,String& data){
		flags = decodeFlags(message);
		address_from =decodeAddressFrom(message);
		address_to =decodeAddressTo(message);
		type = decodeType(message);
		sub_type = decodeSubType(message);
		data = decodeData(message);
}

String MessageBuilder::decodeData(String message){
	return message.substring(12,message.length() - 3);
}

char MessageBuilder::decodeType(String message){
	return message.charAt(10);
}

char MessageBuilder::decodeSubType(String message){
	return message.charAt(11);
}

void MessageBuilder::setChecksum(String& message){
	String checksum = computeChecksum(message);
	message.setCharAt(message.length() - 3 , checksum.charAt(checksum.length() - 2));
	message.setCharAt(message.length() - 2 , checksum.charAt(checksum.length() - 1));
}

char MessageBuilder::decodeFlags(String message){
	return base36ToInt(String(message.charAt(1)));
}

String MessageBuilder::intToBase36(int i) {
	String output("");
	if(i == 0) {
		return "0";
	}
	while (i >= 1) {
		int mod = i % 36;
		if (mod > 9) {
			//output += (char) ((mod - 10) + 'a');
			output = String((char) ((mod - 10) + 'a')) + output;
		} else {
			//output += (char) (mod + '0');
			output = String((char) (mod + '0')) + output;
		}
		i = i / 36;
	}
	return output;
}

int MessageBuilder::base36ToInt(String str) {
	int r = 0;
	int power = 1;
	for (int i = str.length() - 1; i >= 0; i--) {
		char c = str.charAt(i);
		if (c >= '0' && c <= '9') {
			r += (c - '0') * power;
		} else {
			r += ((c - 'a') + 10) * power;
		}
		power *= 36;
	}
	return r;
}
	
String MessageBuilder::computeChecksum(String message) {
	int sum = 0;
	for (int i = 1; i < message.length() - 3; i++) {
		sum += message.charAt(i);
	}
	return intToBase36(sum);
}

char MessageBuilder::validate(String message){
	if (message.length() < 15)//fields missing
		return 2;
	String chechsum = computeChecksum(message);
	if (chechsum.charAt(chechsum.length() - 2) != message.charAt(message.length() - 3))
		return 1;
	if (chechsum.charAt(chechsum.length() - 1) != message.charAt(message.length() - 2))
		return 1;
	return 0;
}