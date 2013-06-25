/*
 * TCPSocket.h
 *
 *  Created on: Jul 5, 2011
 *      Author: ben
 */

#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

#include "Socket.h"

class TCPSocket: public virtual Socket {
private:
	int GetConnect(addrinfo*);

protected:
	int CreateTCPServer();
	int CreateTCPClientReceiverMode();

public:
	TCPSocket();
	virtual ~TCPSocket();
};

#endif /* TCPSOCKET_H_ */
