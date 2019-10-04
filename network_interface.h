#ifndef __NETWORK_INTERFACE__
#define __NETWORK_INTERFACE__
#include "Auth_base.h"
#include "websocket_handler.h"

//constexpr int PORT = 9001;
constexpr int TIMEWAIT = 100;
constexpr int BUFFLEN = 2048;
constexpr int MAXEVENTSSIZE = 20;

typedef std::map<int, Websocket_Handler *> WEB_SOCKET_HANDLER_MAP;

class Network_Interface {
private:
	Network_Interface();
	~Network_Interface();
	int init();
	int epoll_loop();
	int set_noblock(int fd);
	void ctl_event(int fd, bool flag);
public:
	void run(Auth_base&, int PORT);
	static Network_Interface *get_share_network_interface();
private:
	uint16_t PORT;
	Auth_base authentication;
	int epollfd_;
	int listenfd_;
	WEB_SOCKET_HANDLER_MAP websocket_handler_map_;
	static Network_Interface *m_network_interface;
};

#define NETWORK_INTERFACE Network_Interface::get_share_network_interface()

#endif
