#include "network_interface.h"

int main(int argc, char **argv){
	Auth_base auth;
	NETWORK_INTERFACE->run(auth, 9001);
	return 0;
}
