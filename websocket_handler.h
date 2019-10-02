#ifndef __WEBSOCKET_HANDLER__
#define __WEBSOCKET_HANDLER__

#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>
#include <list>
#include <sstream>
#include "base64.h"
#include "sha1.h"
#include "debug_log.h"
#include "websocket_request.h"
#include "websocket_message.h"
#include <jansson.h>
#include "Auth_base.h"

constexpr char MAGIC_KEY[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

enum WEBSOCKET_STATUS {
	WEBSOCKET_UNCONNECT = 0,
	WEBSOCKET_HANDSHARKED = 1,
};

typedef std::map<std::string, std::string> HEADER_MAP;

class Websocket_Handler{
public:
	Websocket_Handler(int fd, Auth_base authentication);
	~Websocket_Handler();
	int process(uint8_t buff[], int bufflen);
	int process(uint8_t buff[], uint8_t adBuff[], int bufflen);
private:
	int handshark(uint8_t* request, int datalen);
	void parse_str(char *request);
	int fetch_http_info(char *request);
	int send_data(uint8_t *buff, int datalen);
	int send_frame(uint8_t *frame, int datalen, uint8_t *Buf);
	void onSuccessfulSubscribe(const std::string &channel, uint8_t *buffer);
	int make_frame(uint8_t* msg, int msg_length, uint8_t* buffer);
	void unsubscribe(const std::string &channel/*, uint8_t *buffer*/);
private:
	uint8_t buffer[2048];
	websocket_message wsMessage;
	WEBSOCKET_STATUS status_;
	HEADER_MAP header_map_;
	int fd_;
	Auth_base authentication;
	static std::unordered_map<std::string, int> subscriptions;
	std::list<std::string> subscribedChannels;
	std::string socketId;
	//virtual void on_event();
};

#endif