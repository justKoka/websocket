#ifndef __WEBSOCKET_REQUEST__
#define __WEBSOCKET_REQUEST__

#include <stdint.h>
#include <arpa/inet.h>
#include "debug_log.h"
#include <string>


class Websocket_Request {
public:
	Websocket_Request();
	~Websocket_Request();
	int fetch_websocket_info(const char *msg);
	void print();
	void reset();
	const std::string&  get_payload();
private:
	int fetch_fin(const char *msg, int &pos);
	int fetch_opcode(const char *msg, int &pos);
	int fetch_mask(const char *msg, int &pos);
	int fetch_masking_key(const char *msg, int &pos);
	int fetch_payload_length(const char *msg, int &pos);
	int fetch_payload(const char *msg, int &pos);
private:
	uint8_t fin_;
	uint8_t opcode_;
	uint8_t mask_;
	uint8_t masking_key_[4];
	uint64_t payload_length_;
	/*int payload_length_;*/
	std::string payload_;
};

#endif
