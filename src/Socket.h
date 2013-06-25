/*
 * Socket.h
 *
 *  Created on: Jul 5, 2011
 *      Author: ben
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include "Graphics.h"
#include "Encryption.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h> //NonBlocking Sockets
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

struct UDPServiceInfo {
	sockaddr_storage RemoteAddr;
	char host[1024];
	char service[20];
	char UserName[20];
};

class Socket: public Graphics, public Encryption {
protected:
	int mySocketFD; //used in TCP/UDP server, listening socket descripter
	int ConnectionFD; //used in TCP/UDP client, connecting socket descriptor

	int Domain, Type, Protocol, Flag;
	char IPversion[10];
	char ListenAddress[INET6_ADDRSTRLEN], ListenPort[10];
	bool NonBlockingMode;

	timeval TWait; //time wait in nonblocking mode

	fd_set Master_FDs; //master file descriptor list
	fd_set Read_FDs; //temp file descriptor list for select()

	sockaddr_storage RemoteAddr; // client address
	sockaddr_storage RemoteAddrStorage[10]; // client addresses in TCP mode

	UDPServiceInfo UDPClientInfo[10];// client addresses in UDP mode

	addrinfo* GetAddrInfo();
	int GetBind(addrinfo*);
	int ListenOn();
	void ManageConnections();
	int Login();

	int SocketBlockingMode();

public:
	bool AppStat;

	char HostName[128];
	char UserName[20];

	void DisconnectClients();

	void ClientSendMode();
	void ClientReceiverMode();

	void FlushAll();

	Socket();
	virtual ~Socket();
};

#endif /* SOCKET_H_ */

