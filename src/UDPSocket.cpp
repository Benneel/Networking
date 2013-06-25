/*
 * UDPSocket.cpp
 *
 *  Created on: Jul 5, 2011
 *      Author: ben
 */

#include "UDPSocket.h"

void *get_in_addr(sockaddr*); // get sockaddr: IPv4 or IPv6:

UDPSocket::UDPSocket() {
	// TODO Auto-generated constructor stub
	cout << "Constructing UDP Socket...\n";
}

UDPSocket::~UDPSocket() {
	// TODO Auto-generated destructor stub
	cout << "Destructing UDP Socket...\n";
}

int UDPSocket::CreateUDPServer() {
	sprintf(Message, "Server: Initiating UDP Server...\n");
	easyLog("bold");
	struct addrinfo *ai;
	if (AppStat)
		FlushAll();
	if ((ai = GetAddrInfo()) == NULL)
		return 1; //Error On Getting Address Information
	if ((mySocketFD = GetBind(ai)) == -1)
		return 2; //Error On Binding Socket
	if (SocketBlockingMode())
		return 3;
	sprintf(Message, "Server Ready: UDP Connection Management started...\n");
	easyLog("bold");
	AppStat = true;
	ManageConnections();
	return 0;
}

int UDPSocket::CreateUDPClientReceiverMode() {
	sprintf(Message, "Client: Initiating UDP Client...\n");
	easyLog("bold");
	struct addrinfo *ai;
	if (AppStat)
		FlushAll();
	if ((ai = GetAddrInfo()) == NULL)
		return 1; //Error On Getting Address Information
	for (; ai != NULL; ai = ai->ai_next) {
		ConnectionFD = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (ConnectionFD < 0) {
			continue;
		} else {
			if (SocketBlockingMode())
				continue;
			memcpy(&RemoteAddr, ai->ai_addr, sizeof RemoteAddr);
			break;
		}
	}
	if (ConnectionFD < 0) {
		sprintf(Message, "Socket: Error on creating socket\n");
		easyLog("error");
		return 2;
	}
	if (Login())
		return 3;
	sprintf(Message, "Client Ready: UDP Receiver Mode started...\n");
	easyLog("bold");
	AppStat = true;
	ClientReceiverMode();
	return 0;
}
