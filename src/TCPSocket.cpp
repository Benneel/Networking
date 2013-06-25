/*
 * TCPSocket.cpp
 *
 *  Created on: Jul 5, 2011
 *      Author: ben
 */

#include "TCPSocket.h"

TCPSocket::TCPSocket() {
	// TODO Auto-generated constructor stub
	cout << "Constructing TCP Socket...\n";

}

TCPSocket::~TCPSocket() {
	// TODO Auto-generated destructor stub
	cout << "Destructing TCP Socket...\n";
}

int TCPSocket::CreateTCPServer() {
	sprintf(Message, "Server: Initiating TCP Server...\n");
	easyLog("bold");
	addrinfo *ai;
	if (AppStat)
		FlushAll();
	if ((ai = GetAddrInfo()) == NULL)
		return 1; //Error On Getting Address Information
	if ((mySocketFD = GetBind(ai)) == -1)
		return 2; //Error On Binding Socket
	if (SocketBlockingMode())
		return 3;
	if (ListenOn() == -1)
		return 4; //Error On Listener
	sprintf(Message, "Server Ready: TCP Connection Management started...\n");
	easyLog("bold");
	AppStat = true;
	ManageConnections();
	return 0;
}

int TCPSocket::GetConnect(addrinfo* ai) {
	// loop through all the results and connect to the first we can
	addrinfo *p;
	for (p = ai; p != NULL; p = p->ai_next) {
		if ((ConnectionFD
				= socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Client: socket");
			continue;
		}
		if (connect(ConnectionFD, p->ai_addr, p->ai_addrlen) == -1) {
			close(ConnectionFD);
			perror("Client: connect");
			continue;
		}
		break;
	}
	if (ConnectionFD < 0) {
		sprintf(Message, "Socket: Error on creating socket\n");
		easyLog("error");
	} else {
		sprintf(Message, "Socket: Successfully created with Num. %d\n",
				ConnectionFD);
		easyLog("success");
	}
	if (p == NULL) {
		sprintf(Message, "Client: Failed to connect to %s on socket %d\n",
				ListenAddress, ConnectionFD);
		easyLog("error");
		return 1;
	}
	if (SocketBlockingMode())
		return 2;
	sprintf(Message, "Client: Successfully connected to %s on socket %d\n",
			ListenAddress, ConnectionFD);
	easyLog("success");
	freeaddrinfo(p); // all done with this structure
	return 0;
}

int TCPSocket::CreateTCPClientReceiverMode() {
	sprintf(Message, "Client: Initiating TCP Client...\n");
	easyLog("bold");
	addrinfo *ai;
	if (AppStat)
		FlushAll();
	if ((ai = GetAddrInfo()) == NULL)
		return 1; //Error On Getting Address Information
	if (GetConnect(ai))
		return 2; //Error On Connectiong To Server
	if (Login())
		return 3;
	sprintf(Message, "Client Ready: TCP Receiver Mode started...\n");
	easyLog("bold");
	AppStat = true;
	ClientReceiverMode();
	return 0;
}
