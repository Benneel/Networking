/*
 * UDPSocket.h
 *
 *  Created on: Jul 5, 2011
 *      Author: ben
 */

#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include "Socket.h"

class UDPSocket: public virtual Socket {
protected:
	int CreateUDPServer();
	int CreateUDPClientReceiverMode();

public:
	UDPSocket();
	virtual ~UDPSocket();
};

#endif /* UDPSOCKET_H_ */
