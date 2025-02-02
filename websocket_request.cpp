#include "websocket_request.h"
#include <memory>
#include <iostream>

Websocket_Request::Websocket_Request():
		fin_(),
		opcode_(),
		mask_(),
		masking_key_(),
		payload_length_(),
		payload_()
{
}

Websocket_Request::~Websocket_Request(){

}

int Websocket_Request::fetch_websocket_info(const char *msg){
	int pos = 0;
	fetch_fin(msg, pos);
	fetch_opcode(msg, pos);
	fetch_mask(msg, pos);
	fetch_payload_length(msg, pos);
	fetch_masking_key(msg, pos);
	return fetch_payload(msg, pos);
}

void Websocket_Request::print(){
	DEBUG_LOG("WEBSOCKET PROTOCOL\n"
				"FIN: %d\n"
				"OPCODE: %d\n"
				"MASK: %d\n"
				"PAYLOADLEN: %d\n"
				"PAYLOAD: %s",
				fin_, opcode_, mask_, payload_length_, payload_.c_str());
}

void Websocket_Request::reset(){
	fin_ = 0;
	opcode_ = 0;
	mask_ = 0;
	memset(masking_key_, 0, sizeof(masking_key_));
	payload_length_ = 0;
	payload_ = "";
}

const std::string & Websocket_Request::get_payload()
{
	return payload_;
}

int Websocket_Request::fetch_fin(const char *msg, int &pos){
	fin_ = (unsigned char)msg[pos] >> 7;
	return 0;
}

int Websocket_Request::fetch_opcode(const char *msg, int &pos){
	opcode_ = msg[pos] & 0x0f;
	pos++;
	return 0;
}

int Websocket_Request::fetch_mask(const char *msg, int &pos){
	mask_ = (unsigned char)msg[pos] >> 7;
	return 0;
}

int Websocket_Request::fetch_masking_key(const char *msg, int &pos){
	if(mask_ != 1)
		return 0;
	for(int i = 0; i < 4; i++)
		masking_key_[i] = (unsigned char)msg[pos + i];
	pos += 4;
	return 0;
}

int Websocket_Request::fetch_payload_length(const char *msg, int &pos){
	payload_length_ = msg[pos] & 0x7f;
	pos++;
	if (payload_length_ == 126) {
		uint16_t length = 0;
		memcpy(&length, msg + pos, 2);
		pos += 2;
		payload_length_ = ntohs(length);
	}
	else if (payload_length_ == 127) {
		uint32_t length = 0;
		memcpy(&length, msg + pos, 4);
		pos += 4;
		payload_length_ = ntohl(length);
	}
	std::cout << std::endl<<"PAYLOAD LENGTH: " << payload_length_ << std::endl;
	return 0;
}

int Websocket_Request::fetch_payload(const char *msg, int &pos){
	std::unique_ptr<unsigned char[]> pay_(new unsigned char[payload_length_ + 1]);
	memset(pay_.get(), 0, payload_length_ + 1);
	if(mask_ != 1){
		memcpy(pay_.get(), msg + pos, payload_length_);
	}
	else {
		for(uint i = 0; i < payload_length_; i++){
			pay_.get()[i] = (unsigned char)msg[pos + i] ^ masking_key_[i % 4];
		}
		pay_.get()[payload_length_] = '\0';
		printf("payload: %s", pay_.get());
		if (!payload_.empty())
			payload_.clear();
		payload_.assign(std::move((const char*)pay_.get()));
	}
	pos += payload_length_;
	return 0;
}