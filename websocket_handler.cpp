#include <unistd.h>
#include "websocket_handler.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

std::unordered_map<std::string, int> Websocket_Handler::subscriptions;

Websocket_Handler::Websocket_Handler(int fd, Auth_base authentication):
		status_(WEBSOCKET_UNCONNECT),
		header_map_(),
		fd_(fd),
		authentication(authentication),
		subscribedChannels()
{
}

Websocket_Handler::~Websocket_Handler(){
}

int Websocket_Handler::process(uint8_t inbuff[], int bufflen) {
	if (status_ == WEBSOCKET_UNCONNECT) {
		return handshark(inbuff, bufflen);
	}
	if (bufflen < 2048)
	{
	wsMessage.handle(inbuff, buffer);
	json_error_t jerror;
	buffer[wsMessage.plength] = '\0';
	json_t *json = json_loadb((const char*)buffer, wsMessage.plength + 1, 0, &jerror);
	/*if (!json)
		DEBUG_LOG("Error parsing json data from server: %s\ndata was: %s",
			jerror.text, request_->get_payload().c_str());
	*/
	const std::string channelEvent = json_object_get(json, "event") ?
		json_string_value(json_object_get(json, "event")) : "";
	const std::string channel = json_object_get(json, "channel") ?
		json_string_value(json_object_get(json, "channel")) : "";
	const std::string sdata = json_object_get(json, "data") ?
		(json_is_string(json_object_get(json, "data")) ?
			json_string_value(json_object_get(json, "data")) : "")
		: "";

	if (channelEvent == "pusher:subscribe") {
		if (channel.compare(0, 9, "private-", 0, 9))
		{
			DEBUG_LOG("subcription to private channel %s, authentication required", channel.c_str());
			if (authentication.privateAuth(channel, sdata))
			{
				onSuccessfulSubscribe(channel, inbuff);
				DEBUG_LOG("authentication succeed, subscribed on private channel %s", channel.c_str());
			}
			else
				DEBUG_LOG("authentication failed");
		}
		else if (channel.compare(0, 10, "presence-", 0, 10))
		{
			DEBUG_LOG("subcription to presence channel %s, authentication required", channel.c_str());
			if (authentication.presenceAuth(channel, sdata))
			{
				onSuccessfulSubscribe(channel, inbuff);
				DEBUG_LOG("authentication succeed, subscribed on presence channel %s", channel.c_str());
			}
			else
				DEBUG_LOG("authentication failed");
		}
		else
		{
			DEBUG_LOG("subcription to channel %s", channel.c_str());
			onSuccessfulSubscribe(channel, inbuff);
			DEBUG_LOG("subscribed on channel %s", channel.c_str());
		}
	}
	else if (channelEvent == "pusher:unsubscribe")
	{
		//unsubcribe(channel);
		DEBUG_LOG("unsubscribed from channel %s", channel.c_str());
	}
	send_frame(buffer, wsMessage.plength, inbuff);
}
	else {
		std::unique_ptr<uint8_t[]> adbuf(new uint8_t[bufflen]);
		wsMessage.handle(inbuff, adbuf.get());
		json_error_t jerror;
		adbuf.get()[wsMessage.plength] = '\0';
		json_t *json = json_loadb((const char*)adbuf.get(), wsMessage.plength + 1, 0, &jerror);
		/*if (!json)
			DEBUG_LOG("Error parsing json data from server: %s\ndata was: %s",
				jerror.text, request_->get_payload().c_str());
		*/
		const std::string channelEvent = json_object_get(json, "event") ?
			json_string_value(json_object_get(json, "event")) : "";
		const std::string channel = json_object_get(json, "channel") ?
			json_string_value(json_object_get(json, "channel")) : "";
		const std::string sdata = json_object_get(json, "data") ?
			(json_is_string(json_object_get(json, "data")) ?
				json_string_value(json_object_get(json, "data")) : "")
			: "";

		if (channelEvent == "pusher:subscribe") {
			if (channel.compare(0, 9, "private-", 0, 9))
			{
				DEBUG_LOG("subcription to private channel %s, authentication required", channel.c_str());
				if (authentication.privateAuth(channel, sdata))
				{
					onSuccessfulSubscribe(channel, inbuff);
					DEBUG_LOG("authentication succeed, subscribed on private channel %s", channel.c_str());
				}
				else
					DEBUG_LOG("authentication failed");
			}
			else if (channel.compare(0, 10, "presence-", 0, 10))
			{
				DEBUG_LOG("subcription to presence channel %s, authentication required", channel.c_str());
				if (authentication.presenceAuth(channel, sdata))
				{
					onSuccessfulSubscribe(channel, inbuff);
					DEBUG_LOG("authentication succeed, subscribed on presence channel %s", channel.c_str());
				}
				else
					DEBUG_LOG("authentication failed");
			}
			else
			{
				DEBUG_LOG("subcription to channel %s", channel.c_str());
				onSuccessfulSubscribe(channel, inbuff);
				DEBUG_LOG("subscribed on channel %s", channel.c_str());
			}
		}
		else if (channelEvent == "pusher:unsubscribe")
		{
			//unsubcribe(channel);
			DEBUG_LOG("unsubscribed from channel %s", channel.c_str());
		}
		send_frame(adbuf.get(), wsMessage.plength, inbuff);
	}
	return 0;
}

int Websocket_Handler::process(uint8_t buff[], uint8_t adBuff[], int bufflen)
{
	return 0;
}

int Websocket_Handler::handshark(uint8_t* request, int datalen){

	status_ = WEBSOCKET_HANDSHARKED;
	request[datalen] = '\0';
	fetch_http_info((char*)request);
	memset(buffer, 0, 2048);
	parse_str((char*)buffer);
	return send_data(buffer, strlen((char*)buffer));
}

void Websocket_Handler::parse_str(char *request){  
	strcat(request, "HTTP/1.1 101 Switching Protocols\r\n");
	strcat(request, "Connection: upgrade\r\n");
	strcat(request, "Sec-WebSocket-Accept: ");
	std::string server_key = header_map_["Sec-WebSocket-Key"];
	server_key += MAGIC_KEY;

	SHA1 sha;
	unsigned int message_digest[5];
	sha.Reset();
	sha << server_key.c_str();

	sha.Result(message_digest);
	for (int i = 0; i < 5; i++) {
		message_digest[i] = htonl(message_digest[i]);
	}
	server_key = base64_encode(reinterpret_cast<const unsigned char*>(message_digest),20);
	server_key += "\r\n";
	strcat(request, server_key.c_str());
	strcat(request, "Upgrade: websocket\r\n\r\n");
}

int Websocket_Handler::fetch_http_info(char *message){
	std::istringstream s(message);
	std::string request;

	std::getline(s, request);
	if (request[request.size()-1] == '\r') {
		request.erase(request.end()-1);
	} else {
		return -1;
	}

	std::string header;
	std::string::size_type end;

	while (std::getline(s, header) && header != "\r") {
		if (header[header.size()-1] != '\r') {
			continue; //end
		} else {
			header.erase(header.end()-1);	//remove last char
		}

		end = header.find(": ",0);
		if (end != std::string::npos) {
			std::string key = header.substr(0,end);
			std::string value = header.substr(end+2);
			header_map_[key] = value;
		}
	}

	return 0;
}

int Websocket_Handler::send_data(uint8_t * buff, int datalen)
{
	return write(fd_, buff, datalen);
}

int Websocket_Handler::send_frame(uint8_t *frame, int datalen, uint8_t *Buf)
{
	if (datalen < 2048) {
		memset(Buf, 0, datalen);
		int framelen = make_frame(frame, datalen, Buf);
		return send_data(Buf, framelen);
	}
	else {
		std::unique_ptr<uint8_t[]> adbuf(new uint8_t[datalen + 20]);
		int framelen = make_frame(frame, datalen, adbuf.get());
		return send_data(adbuf.get(), framelen);
	}
}

void Websocket_Handler::onSuccessfulSubscribe(const std::string & channel, uint8_t *buffer)
{
	Websocket_Handler::subscriptions.insert(std::make_pair(channel, fd_));
	subscribedChannels.push_back(channel);
	std::stringstream tmp;
	tmp << "{\"event\":\"pusher:subscription_succeed\",\"channel\":\"" << channel << "\"}";
	send_frame((uint8_t*)tmp.str().c_str(), tmp.str().length(), buffer);
}

int Websocket_Handler::make_frame(uint8_t * msg, int msg_length, uint8_t * buffer)
{
	int pos = 0;
	buffer[pos++] = (unsigned char)0x81;// text frame
	if (msg_length <= 125) {
		buffer[pos++] = msg_length;
	}
	else if (msg_length <= 65535) {
		buffer[pos++] = 126; //16 bit length follows
		//uint16_t payload_len = msg_length
		uint16_t payload_len = htons(msg_length);
		//check this
		memcpy((void *)(buffer + pos), &payload_len, 2);
		pos += 2;
	}
	else { // >2^16-1 (65535)
		buffer[pos++] = 127; //64 bit length follows
		//same as previous
		uint64_t payload_len = ntohl(msg_length);
		memcpy((void *)(buffer + pos), &payload_len, 8);
		pos += 8;
	}
	memcpy((void *)(buffer + pos), (void *)msg, msg_length);
	return pos + msg_length;
}
