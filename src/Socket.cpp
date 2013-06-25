/*
 * Socket.cpp
 *
 *  Created on: Jul 5, 2011
 *      Author: ben
 */

#include "Socket.h"

#define Specialize(str)				\
	for (int z = 0; z <= 255; z++)	\
		if (str[z] == 0)			\
				str[z] = 0x01;

#define unSpecialize(str)			\
	for (int z = 0; z <= 255; z++)	\
		if (str[z] == 0x01)			\
				str[z] = 0;

int sendall(int, char*, int*);
void *get_in_addr(sockaddr*); // get sockaddr: IPv4 or IPv6:

Socket::Socket() {
	// TODO Auto-generated constructor stub
	cout << "Constructing Socket...\n";

	AppStat = false;
	gethostname(HostName, sizeof HostName);
	FD_ZERO(&Master_FDs); // clear the master and temp sets
	FD_ZERO(&Read_FDs);
}

Socket::~Socket() {
	// TODO Auto-generated destructor stub
	cout << "Destructing Socket...\n";
}

void Socket::FlushAll() {
	sprintf(Message, "Program: Reset mode activated\n");
	easyLog("italic");
	AppStat = false;
	if (!strcmp(ProgramMode, "Server")) {
		if (Type == SOCK_STREAM) {
			DisconnectClients();
			sleep(1);
			if (mySocketFD > 0) {
				shutdown(mySocketFD, SHUT_RDWR);
				close(mySocketFD);
				mySocketFD = -1;
			}
		} else if (Type == SOCK_DGRAM) {
			DisconnectClients();
			sleep(1);
			if (mySocketFD > 0) {
				close(mySocketFD);
				mySocketFD = -1;
			}
		}
		sprintf(Message, "Program: Server Stopped\n");
		easyLog("error");
	} else if (!strcmp(ProgramMode, "Client")) {
		if (Type == SOCK_STREAM) {
			if (ConnectionFD > 0) {
				shutdown(ConnectionFD, SHUT_WR);
				sleep(1);
				shutdown(ConnectionFD, SHUT_RD);
				close(ConnectionFD);
				ConnectionFD = -1;
			}
		} else if (Type == SOCK_DGRAM) {
			if (ConnectionFD > 0) {
				char *ConnInfo = new char();
				strcpy(ConnInfo, "00000000000000000000");
				ClientCheckUpdate(ConnInfo);
				char sendBuffer[256] = "";
				strcpy(sendBuffer, "%DISCONNECTED%");
				strcpy(sendBuffer, md5(sendBuffer).c_str());
				sendto(ConnectionFD, sendBuffer, sizeof sendBuffer, 0,
						(sockaddr *) &RemoteAddr, sizeof RemoteAddr);
				sleep(1);
				close(ConnectionFD);
				ConnectionFD = -1;
			}
		}
		sprintf(Message, "Program: Client Stopped\n");
		easyLog("error");
	}
	FD_ZERO(&Master_FDs); // clear the master and temp sets
	FD_ZERO(&Read_FDs);
	AppStat = false;
}

int Socket::SocketBlockingMode() {
	if (NonBlockingMode) {
		TWait.tv_sec = 0;
		TWait.tv_usec = 750000;
		if (!strcmp(ProgramMode, "Server")) {
			int fdflags = fcntl(mySocketFD, F_GETFL);
			if (fcntl(mySocketFD, F_SETFL, fdflags | O_NONBLOCK) < 0) {
				perror("fcntl");
				sprintf(
						Message,
						"NonBlocking: Error on setting socket %d to NonBlock.\n",
						mySocketFD);
				easyLog("error");
				return 1;
			} else {
				sprintf(
						Message,
						"NonBlocking: Socket %d successfully set to NonBlock.\n",
						mySocketFD);
				easyLog("success");
			}
		} else if (!strcmp(ProgramMode, "Client")) {
			int fdflags = fcntl(ConnectionFD, F_GETFL);
			if (fcntl(ConnectionFD, F_SETFL, fdflags | O_NONBLOCK) < 0) {
				perror("fcntl");
				sprintf(
						Message,
						"NonBlocking: Error on setting socket %d to NonBlock.\n",
						ConnectionFD);
				easyLog("error");
				return 1;
			} else {
				sprintf(
						Message,
						"NonBlocking: Socket %d successfully set to NonBlock.\n",
						ConnectionFD);
				easyLog("success");
			}
		}
	} else {
		memset(&TWait, 0, sizeof TWait);
	}
	return 0;
}

addrinfo* Socket::GetAddrInfo(void) {
	struct addrinfo hints, *ai;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = Domain;
	hints.ai_socktype = Type;
	hints.ai_protocol = Protocol;
	hints.ai_flags = Flag;

	if ((rv = getaddrinfo(ListenAddress, ListenPort, &hints, &ai)) != 0) {
		sprintf(Message,
				"Error On Getting Network Address Information!! (%s)\n",
				gai_strerror(rv));
		easyLog("error");
		return NULL;
	}
	return ai;
}

int Socket::GetBind(addrinfo* ai) {
	addrinfo *p;
	int Listener = -1;
	int yes = 1; // for setsockopt() SO_REUSEADDR, below
	for (p = ai; p != NULL; p = p->ai_next) {
		Listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (Listener < 0) {
			continue;
		}
		// lose the pesky "address already in use" error message
		setsockopt(Listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		if (bind(Listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(Listener);
			continue;
		}
		break;
	}
	if (Listener < 0) {
		sprintf(Message, "Socket: Error on creating socket\n");
		easyLog("error");
	} else {
		sprintf(Message, "Socket: Successfully created with Num. %d\n",
				Listener);
		easyLog("success");
	}
	// if we got here, it means we didn't get bound
	if (p == NULL) {
		sprintf(Message, "Bind Failed: Server binding failed on %s:%s (%s).\n",
				ListenAddress, ListenPort, IPversion);
		easyLog("error");
		return -1;
	}
	sprintf(Message,
			"Bind Successful: Server binded on %s:%s (%s) socket %d.\n",
			ListenAddress, ListenPort, IPversion, Listener);
	easyLog("success");
	freeaddrinfo(ai); // all done with this
	return Listener;
}

int Socket::ListenOn() {
	if (listen(mySocketFD, 10) == -1) {
		sprintf(Message,
				"Listen Error: Server listening on socket %d failed.\n",
				mySocketFD);
		easyLog("error");
		return -1;
	}
	FD_SET(mySocketFD, &Master_FDs); // add the listener to the master set
	sprintf(
			Message,
			"Listen Successful: Server started listening on socket %d for incoming connections.\n",
			mySocketFD);
	easyLog("success");
	return 1;
}

void Socket::ManageConnections() {
	int fdmax; // maximum file descriptor number
	int newfd; // newly accept()ed socket descriptor
	int cntConn = 0;//get track of the number of connections
	int SocketFD = 0; //loop through the socketfds
	const int Floorfd = mySocketFD + 1; // connection FDs start at 0
	socklen_t addr_len;
	char buf[256]; // buffer for client data
	char ConnectionInfo[10] = "0";//used mask to set check boxes
	char sendConnectionInfo[10] = "0";//mask used to send data
	char ClientUserName[10][20] = { "" };
	char getClientUserName[20] = "";
	char getClientPass[15] = "";
	int nbytes, nbytesall;
	char remoteIP[INET6_ADDRSTRLEN];
	int j;
	bool Authenticate[10] = { 0 };
	fdmax = mySocketFD; // so far, it's this one
	char host[1024];
	char service[20];
	for (int i = 0; i <= 9; i++) {
		strcpy(UDPClientInfo[i].host, "");
		strcpy(UDPClientInfo[i].service, "");
		strcpy(UDPClientInfo[i].UserName, "");
		bzero(&UDPClientInfo[i].RemoteAddr, sizeof UDPClientInfo[i].RemoteAddr);
		ClientCheck(-(i + 1), "");
	}
	addr_len = sizeof RemoteAddr;

	// main loop
	if (Type == SOCK_STREAM) {
		for (;;) {
			strcpy(buf, "");
			Read_FDs = Master_FDs; // copy it
			if (NonBlockingMode) {
				TWait.tv_sec = 0;
				TWait.tv_usec = 750000;
				if (select(fdmax + 1, &Read_FDs, NULL, NULL, &TWait) == -1) {
					sprintf(Message,
							"Internal Server Error!: Error on socket.\n");
					easyLog("error");
					if (AppStat)
						FlushAll();
					return;
				}
			} else {
				if (select(fdmax + 1, &Read_FDs, NULL, NULL, NULL) == -1) {
					sprintf(Message,
							"Internal Server Error!: Error on socket.\n");
					easyLog("error");
					if (AppStat)
						FlushAll();
					return;
				}
			}
			// run through the existing connections looking for data to read
			for (SocketFD = mySocketFD; SocketFD <= fdmax; SocketFD++) {
				if (FD_ISSET(SocketFD, &Read_FDs)) { // we got one!!
					if (SocketFD == mySocketFD) {
						// handle new connections
						if (cntConn == 10) {
							//reached maximum of connection
							newfd = accept(mySocketFD,
									(struct sockaddr *) &RemoteAddr, &addr_len);
							close(newfd);
							sprintf(
									Message,
									"Connection: Maximum connections reached\n\tNew connection from %s refused!\n",
									remoteIP);
							easyLog("italic");
							continue;
						}
						//accept new connection
						addr_len = sizeof RemoteAddr;
						newfd = accept(mySocketFD,
								(struct sockaddr *) &RemoteAddr, &addr_len);
						if (newfd == -1) {
							sprintf(Message,
									"Server: Error on accepting connection\n");
							easyLog("error");
						} else {
							if (NonBlockingMode) {
								int fdflags = fcntl(newfd, F_GETFL);
								if (fcntl(newfd, F_SETFL, fdflags | O_NONBLOCK)
										< 0) {
									perror("fcntl");
									sprintf(
											Message,
											"NonBlocking: Error on setting socket %d to NonBlock.\n",
											newfd);
									easyLog("error");
								} else {
									sprintf(
											Message,
											"NonBlocking: Socket %d successfully set to NonBlock.\n",
											newfd);
									easyLog("success");
								}
							}
							cntConn++;
							FD_SET(newfd, &Master_FDs); // add to master set
							if (newfd > fdmax) { // keep track of the max
								fdmax = newfd;
							}
							RemoteAddrStorage[newfd - Floorfd] = RemoteAddr;
							inet_ntop(RemoteAddr.ss_family, get_in_addr(
									(struct sockaddr*) &RemoteAddr), remoteIP,
									INET6_ADDRSTRLEN);
							sprintf(Message,
									"Connection: new connection from %s on "
										"socket %d\n", remoteIP, newfd);
							easyLog("bold");
						}
					} else {
						// handle data from a client
						inet_ntop(
								RemoteAddrStorage[SocketFD - Floorfd].ss_family,
								get_in_addr(
										(struct sockaddr*) &RemoteAddrStorage[SocketFD
												- Floorfd]), remoteIP,
								INET6_ADDRSTRLEN);
						if ((nbytes = recv(SocketFD, buf, sizeof buf, 0)) <= 0) {
							// got error or connection closed by client
							if (nbytes == 0) {
								// connection closed
								sprintf(
										Message,
										"Connection Lost: connection %s on socket %d hung up\n",
										remoteIP, SocketFD);
								easyLog("lost");
							} else {
								sprintf(
										Message,
										"Receive Error: cannot get data from %s on socket %d\n",
										remoteIP, SocketFD);
								easyLog("error");
							}
							cntConn--;
							strncpy(&ConnectionInfo[SocketFD - Floorfd], "0", 1);
							ClientCheck(-(SocketFD - Floorfd + 1), "");
							Authenticate[SocketFD - Floorfd] = false;
							strcpy(ClientUserName[SocketFD - Floorfd], "");
							shutdown(SocketFD, SHUT_RDWR);
							close(SocketFD); // bye!
							FD_CLR(SocketFD, &Master_FDs); // remove from master set
							strcpy(buf, "%CONF%");
							strcpy(&buf[10], ConnectionInfo);
							for (int z = 1; z <= 10; z++) {
								strcpy(&buf[20 * z], ClientUserName[z - 1]);
							}
							Specialize(buf);
							Encrypt(buf, buf, EncryptionType);
							for (j = Floorfd; j <= fdmax; j++) {
								// send to everyone!
								// except me{mySocketFD}
								if (!strncmp(&ConnectionInfo[j - Floorfd], "1",
										1)) {
									int tmp = sizeof buf;
									inet_ntop(
											RemoteAddrStorage[SocketFD
													- Floorfd].ss_family,
											get_in_addr(
													(struct sockaddr*) &RemoteAddrStorage[SocketFD
															- Floorfd]),
											remoteIP, INET6_ADDRSTRLEN);
									if (sendall(j, buf, &tmp) == -1) {
										sprintf(
												Message,
												"Configuration: Error on sending ClientList to %s on socket %d!\n",
												remoteIP, j);
										easyLog("error");
									} else {
										sprintf(
												Message,
												"Configuration: ClientList successfully sent to %s on socket %d!\n",
												remoteIP, j);
										easyLog("success");
									}
								}
							}
						} else {
							nbytesall = nbytes;
							// we got some data from a client
							Decrypt(buf, buf, EncryptionType);
							unSpecialize(buf);
							if (!Authenticate[SocketFD - Floorfd]) {
								strncpy(ClientUserName[SocketFD - Floorfd],
										buf, 20);
								strncpy(getClientPass, &buf[20], 15);
								if (!strcmp(getClientPass, Password)) {
									Authenticate[SocketFD - Floorfd] = true;
									sprintf(
											Message,
											"Login Successful: connection from %s on socket %d\n",
											remoteIP, SocketFD);
									easyLog("success");
									strncpy(
											&ConnectionInfo[SocketFD - Floorfd],
											"1", 1);
									ClientCheck(SocketFD - Floorfd + 1,
											ClientUserName[SocketFD - Floorfd]);
									strcpy(buf, "%CONF%");
									strcpy(&buf[10], ConnectionInfo);
									for (int z = 1; z <= 10; z++) {
										strcpy(&buf[20 * z], ClientUserName[z
												- 1]);
									}
									Encrypt(buf, buf, EncryptionType);
									for (j = Floorfd; j <= fdmax; j++) {
										// send to everyone! except me{mySocketFD}
										if (!strncmp(&ConnectionInfo[j
												- Floorfd], "1", 1)) {
											int tmp = sizeof buf;
											inet_ntop(
													RemoteAddrStorage[j
															- Floorfd].ss_family,
													get_in_addr(
															(struct sockaddr*) &RemoteAddrStorage[j
																	- Floorfd]),
													remoteIP, INET6_ADDRSTRLEN);
											if (sendall(j, buf, &tmp) == -1) {
												sprintf(
														Message,
														"Configuration: Error on sending ClientList to %s on socket %d!\n",
														remoteIP, j);
												easyLog("error");
											} else {
												sprintf(
														Message,
														"Configuration: ClientList successfully sent to %s on socket %d!\n",
														remoteIP, j);
												easyLog("success");
											}
										}

									}
									continue;
								} else {
									sprintf(
											Message,
											"Login Failed: connection from %s on socket %d\n",
											remoteIP, SocketFD);
									easyLog("error");
									//*************************************************
									strcpy(buf, "%LOGIN_FAILED%");
									strcpy(buf, md5(buf).c_str());
									send(SocketFD, buf, sizeof buf, 0);
									continue;
								}
							}
							sprintf(
									Message,
									"Receive Successful: %d bytes received from %s on socket %d\n",
									nbytes, remoteIP, SocketFD);
							easyLog("success");
							strncpy(sendConnectionInfo, buf, 10);
							Specialize(buf);
							strcpy(buf, &buf[10]);
							Encrypt(buf, buf, EncryptionType);
							for (j = Floorfd; j <= fdmax; j++) {
								// send to everyone!
								if (FD_ISSET(j, &Master_FDs)) {
									// except me{mySocketFD} and sender{SocketFD}
									if (j != mySocketFD && j != SocketFD
											&& Authenticate[j - Floorfd]
											&& !strncmp(&sendConnectionInfo[j
													- Floorfd], "1", 1)) {
										inet_ntop(
												RemoteAddrStorage[j - Floorfd].ss_family,
												get_in_addr(
														(struct sockaddr*) &RemoteAddrStorage[j
																- Floorfd]),
												remoteIP, INET6_ADDRSTRLEN);
										if (sendall(j, buf, &nbytesall) == -1) {
											sprintf(
													Message,
													"Send Error: %d bytes of data sent to %s on socket %d!\n",
													nbytesall, remoteIP, j);
											easyLog("error");
										} else {
											sprintf(
													Message,
													"Send Successful: %d bytes sent to %s on socket %d!\n",
													nbytesall, remoteIP, j);
											easyLog("success");
										}
									}
								}
							}
						}
					} // END handle data from client
				} else { // END got new incoming connection
					//cout << "********time out*******\n";
				}
			} // END looping through file descriptors
		} // END for(;;)--and you thought it would never end!
	} else if (Type == SOCK_DGRAM) {
		for (;;) {
			strcpy(buf, "");
			if (NonBlockingMode) {
				TWait.tv_sec = 0;
				TWait.tv_usec = 750000;
				select(NULL, NULL, NULL, NULL, &TWait);
			}
			if ((nbytes = recvfrom(mySocketFD, buf, sizeof buf, 0,
					(sockaddr *) &RemoteAddr, &addr_len)) <= 0) {
				inet_ntop(RemoteAddr.ss_family, get_in_addr(
						(sockaddr*) &RemoteAddr), remoteIP, INET6_ADDRSTRLEN);
				getnameinfo((sockaddr *) &RemoteAddr, sizeof RemoteAddr, host,
						sizeof host, service, sizeof service, 0);
				if (NonBlockingMode) {
					//printf(
					//		"Receive Error: Maybe error happened due to time out\n");
					//sprintf(
					//		Message,
					//		"Receive Error: Maybe error happened due to time out\n");
					//BufferInsert();
				} else {
					perror("recvfrom");
					sprintf(
							Message,
							"Receive Error: cannot get data from %s with %s,%s\n",
							remoteIP, host, service);
					easyLog("error");
				}
			} else if (!strcmp(buf, md5("%DISCONNECTED%").c_str())) {
				inet_ntop(RemoteAddr.ss_family, get_in_addr(
						(sockaddr*) &RemoteAddr), remoteIP, INET6_ADDRSTRLEN);
				getnameinfo((sockaddr *) &RemoteAddr, sizeof RemoteAddr, host,
						sizeof host, service, sizeof service, 0);
				sprintf(Message,
						"Connection Removed: connection from %s with %s,%s\n",
						remoteIP, host, service);
				easyLog("lost");
				for (int z = 0; z <= 9; z++) {
					if (!strcmp(UDPClientInfo[z].host, host) && !strcmp(
							UDPClientInfo[z].service, service)) {
						strncpy(&ConnectionInfo[z], "0", 1);
						ClientCheck(-(z + 1), "");
						Authenticate[z] = false;
						strcpy(UDPClientInfo[z].host, "");
						strcpy(UDPClientInfo[z].service, "");
						strcpy(UDPClientInfo[z].UserName, "");
						bzero(&UDPClientInfo[z].RemoteAddr,
								sizeof UDPClientInfo[z].RemoteAddr);
						break;
					}
				}
				//Send Update List**********************************************
				strcpy(buf, "%CONF%");
				strcpy(&buf[10], ConnectionInfo);
				for (int z = 1; z <= 10; z++) {
					strcpy(&buf[20 * z], UDPClientInfo[z - 1].UserName);
				}
				Specialize(buf);
				Encrypt(buf, buf, EncryptionType);
				for (int i = 0; i <= 9; i++) {
					inet_ntop(UDPClientInfo[i].RemoteAddr.ss_family,
							get_in_addr(
									(sockaddr*) &UDPClientInfo[i].RemoteAddr),
							remoteIP, INET6_ADDRSTRLEN);
					if (strcmp(UDPClientInfo[i].host, "")) {
						if ((nbytes = sendto(mySocketFD, buf, strlen(buf), 0,
								(sockaddr *) &UDPClientInfo[i].RemoteAddr,
								sizeof UDPClientInfo[i].RemoteAddr)) <= 0) {
							sprintf(
									Message,
									"Error on sending ClientList to %s with %s,%s\n",
									remoteIP, UDPClientInfo[i].host,
									UDPClientInfo[i].service);
							easyLog("error");
						} else {
							sprintf(
									Message,
									"ClientList successfully sent to %s with %s,%s\n",
									remoteIP, UDPClientInfo[i].host,
									UDPClientInfo[i].service);
							easyLog("success");
						}
					}
				}
				continue;
			} else {
				Decrypt(buf, buf, EncryptionType);
				unSpecialize(buf);
				inet_ntop(RemoteAddr.ss_family, get_in_addr(
						(sockaddr*) &RemoteAddr), remoteIP, INET6_ADDRSTRLEN);
				getnameinfo((sockaddr *) &RemoteAddr, sizeof RemoteAddr, host,
						sizeof host, service, sizeof service, 0);
				strncpy(getClientUserName, &buf[10], 20);
				bool found = false;
				for (int z = 0; z <= 9; z++) {
					if (!strcmp(UDPClientInfo[z].host, host) && !strcmp(
							UDPClientInfo[z].service, service)) {
						found = true;
						sprintf(
								Message,
								"Receive Successful: %d bytes received from %s with %s,%s\n",
								nbytes, remoteIP, host, service);
						easyLog("success");
						strncpy(sendConnectionInfo, buf, 10);
						Specialize(buf);
						strcpy(buf, &buf[10]);
						Encrypt(buf, buf, EncryptionType);
						for (int i = 0; i <= 9; i++) {
							inet_ntop(
									UDPClientInfo[i].RemoteAddr.ss_family,
									get_in_addr(
											(sockaddr*) &UDPClientInfo[i].RemoteAddr),
									remoteIP, INET6_ADDRSTRLEN);
							if ((strcmp(UDPClientInfo[i].host, "") && !strncmp(
									&sendConnectionInfo[i], "1", 1))) {
								if (strcmp(UDPClientInfo[i].host, host)
										|| strcmp(UDPClientInfo[i].service,
												service)) {
									if ((nbytes
											= sendto(
													mySocketFD,
													buf,
													strlen(buf),
													0,
													(sockaddr *) &UDPClientInfo[i].RemoteAddr,
													sizeof UDPClientInfo[i].RemoteAddr))
											<= 0) {
										sprintf(
												Message,
												"Send Failed: Sending data to %s with %s,%s failed\n",
												remoteIP,
												UDPClientInfo[i].host,
												UDPClientInfo[i].service);
										easyLog("error");
									} else {
										sprintf(
												Message,
												"Send Successful: %d bytes send to %s with %s,%s\n",
												nbytes, remoteIP,
												UDPClientInfo[i].host,
												UDPClientInfo[i].service);
										easyLog("success");
									}
								}
							}
						}
						break;
					}
				}
				if (!found) {
					strncpy(getClientUserName, buf, 20);
					strncpy(getClientPass, &buf[20], 15);
					sprintf(
							Message,
							"Connection: New connection as %s from %s with %s,%s\n",
							getClientUserName, remoteIP, host, service);
					easyLog("bold");
					if (!strcmp(getClientPass, Password)) {
						Authenticate[cntConn] = true;
						sprintf(Message,
								"Login Successful: %s from %s with %s,%s\n",
								getClientUserName, remoteIP, host, service);
						easyLog("success");
						strcpy(UDPClientInfo[cntConn].host, host);
						strcpy(UDPClientInfo[cntConn].service, service);
						strcpy(UDPClientInfo[cntConn].UserName,
								getClientUserName);
						UDPClientInfo[cntConn].RemoteAddr = RemoteAddr;
						strncpy(&ConnectionInfo[cntConn], "1", 1);
						ClientCheck(cntConn + 1, getClientUserName);
						cntConn = (cntConn + 1) % 10;
						//Send Update List**********************************************
						strcpy(buf, "%CONF%");
						strcpy(&buf[10], ConnectionInfo);
						for (int z = 1; z <= 10; z++) {
							strcpy(&buf[20 * z], UDPClientInfo[z - 1].UserName);
						}
						Specialize(buf);
						Encrypt(buf, buf, EncryptionType);
						for (int i = 0; i <= 9; i++) {
							inet_ntop(
									UDPClientInfo[i].RemoteAddr.ss_family,
									get_in_addr(
											(sockaddr*) &UDPClientInfo[i].RemoteAddr),
									remoteIP, INET6_ADDRSTRLEN);
							if (strcmp(UDPClientInfo[i].host, "")) {
								if ((nbytes
										= sendto(
												mySocketFD,
												buf,
												strlen(buf),
												0,
												(sockaddr *) &UDPClientInfo[i].RemoteAddr,
												sizeof UDPClientInfo[i].RemoteAddr))
										<= 0) {
									sprintf(
											Message,
											"Error on sending ClientList to %s with %s,%s\n",
											remoteIP, UDPClientInfo[i].host,
											UDPClientInfo[i].service);
									easyLog("error");
								} else {
									sprintf(
											Message,
											"ClientList successfully sent to %s with %s,%s\n",
											remoteIP, UDPClientInfo[i].host,
											UDPClientInfo[i].service);
									easyLog("success");
								}
							}
						}
						found = false;
					} else {
						sprintf(Message,
								"Login Failed: %s from %s with %s,%s\n",
								getClientUserName, remoteIP, host, service);
						easyLog("error");
						//************************************
						strcpy(buf, "%LOGIN_FAILED%");
						strcpy(buf, md5(buf).c_str());
						sendto(mySocketFD, buf, sizeof buf, 0,
								(sockaddr *) &RemoteAddr, sizeof RemoteAddr);
					}
				}// Not Found
			} //else receive
		}//for
	}
	ClientCheckUpdate("0000000000000000000000");
}// Server Manager

void Socket::DisconnectClients() {
	int i = 0;
	char users[10];
	strcpy(users, getClientCheck());
	while (i <= 9) {
		if (!strncmp(&users[i], "1", 1)) {
			int nbytes;
			char sendBuffer[256] = "";
			strcpy(sendBuffer, "%DISCONNECT%");
			strcpy(sendBuffer, md5(sendBuffer).c_str());
			if (Type == SOCK_STREAM) {
				if ((nbytes = send(i + mySocketFD + 1, sendBuffer,
						sizeof sendBuffer, 0)) == -1) {
					sprintf(
							Message,
							"Connection: Error on sending DISCONNECT command to %s on socket %d!\n",
							ListenAddress, i + mySocketFD + 1);
					easyLog("error");
				} else {
					sprintf(Message,
							"Connection: Connection on socket %d closed\n", i
									+ mySocketFD + 1);
					easyLog("success");
				}
			} else if (Type == SOCK_DGRAM) {
				char remoteIP[INET6_ADDRSTRLEN];
				inet_ntop(UDPClientInfo[i].RemoteAddr.ss_family, get_in_addr(
						(sockaddr*) &UDPClientInfo[i].RemoteAddr), remoteIP,
						INET6_ADDRSTRLEN);
				if ((nbytes = sendto(mySocketFD, sendBuffer, sizeof sendBuffer,
						0, (sockaddr *) &UDPClientInfo[i].RemoteAddr,
						sizeof UDPClientInfo[i].RemoteAddr)) <= 0) {
					sprintf(
							Message,
							"Connection: Error on sending DISCONNECT command to %s with %s,%s\n",
							remoteIP, UDPClientInfo[i].host,
							UDPClientInfo[i].service);
					easyLog("error");
				} else {
					sprintf(Message,
							"Connection: Connection %s with %s,%s closed\n",
							remoteIP, UDPClientInfo[i].host,
							UDPClientInfo[i].service);
					easyLog("success");
				}
			}
		}
		i++;
	}
}

int Socket::Login() {
	char sendBuffer[256];
	int nbytes;
	strcpy(sendBuffer, UserName);
	strcpy(&sendBuffer[20], Password);
	Specialize(sendBuffer);
	Encrypt(sendBuffer, sendBuffer, EncryptionType);
	if (Type == SOCK_STREAM) {
		if ((nbytes = send(ConnectionFD, sendBuffer, sizeof sendBuffer, 0))
				== -1) {
			sprintf(Message, "Login: Login failed due to sending data.\n");
			easyLog("error");
		} else {
			sprintf(Message, "Login: Login data successfully sent\n");
			easyLog("success");
		}
	} else if (Type == SOCK_DGRAM) {
		if ((nbytes = sendto(ConnectionFD, sendBuffer, sizeof sendBuffer, 0,
				(sockaddr *) &RemoteAddr, sizeof RemoteAddr)) <= 0) {
			perror("sendto");
			sprintf(Message, "Login: Login failed due to sending data.\n");
			easyLog("error");
		} else {
			sprintf(Message, "Login: Login data successfully sent\n");
			easyLog("success");
		}
	}
	return 0;
}

void Socket::ClientReceiverMode() {
	char recBuffer[256];
	int nbytes;
	char remoteIP[INET6_ADDRSTRLEN];
	char host[1024];
	char service[20];
	socklen_t addr_len = sizeof RemoteAddr;
	if (Type == SOCK_STREAM) {
		FD_ZERO(&Master_FDs);
		FD_SET(ConnectionFD,&Master_FDs);
		for (;;) {
			strcpy(recBuffer, "");
			Read_FDs = Master_FDs; // copy it
			if (NonBlockingMode) {
				TWait.tv_sec = 0;
				TWait.tv_usec = 750000;
				if (select(ConnectionFD + 1, &Read_FDs, NULL, NULL, &TWait)
						== -1) {
					sprintf(Message,
							"Internal Client Error!: Error on socket.\n");
					easyLog("error");
					if (AppStat)
						FlushAll();
					return;
				}
			} else {
				if (select(ConnectionFD + 1, &Read_FDs, NULL, NULL, NULL) == -1) {
					sprintf(Message,
							"Internal Client Error!: Error on socket.\n");
					easyLog("error");
					if (AppStat)
						FlushAll();
					return;
				}
			}
			if (FD_ISSET(ConnectionFD, &Read_FDs)) {
				if ((nbytes
						= recv(ConnectionFD, recBuffer, sizeof recBuffer, 0))
						<= 0) {
					if (nbytes == 0) {
						sprintf(Message, "Connection Lost: server hung up\n");
						easyLog("lost");
						shutdown(ConnectionFD, SHUT_RD);
						shutdown(ConnectionFD, SHUT_WR);
						close(ConnectionFD);
						FD_CLR(ConnectionFD, &Master_FDs);
						FD_ZERO(&Master_FDs);
						ClientCheckUpdate("0000000000000000000000");
						if (AppStat)
							FlushAll();
						return;
					} else {
						sprintf(Message,
								"Receive Error: cannot get data from server\n");
						easyLog("error");
					}
				} else {
					if (!strcmp(recBuffer, md5("%DISCONNECT%").c_str())) {
						sprintf(Message, "\t\t***Disconnected***\n");
						easyLog("bold");
						shutdown(ConnectionFD, SHUT_WR);
						shutdown(ConnectionFD, SHUT_RD);
						close(ConnectionFD);
						FD_CLR(ConnectionFD, &Master_FDs);
						FD_ZERO(&Master_FDs);
						ClientCheckUpdate("0000000000000000000000");
						if (AppStat)
							FlushAll();
						return;
					} else if (!strcmp(recBuffer, md5("%LOGIN_FAILED%").c_str())) {
						sprintf(Message, "\t\t***Login Failed***\n");
						easyLog("bold");
						shutdown(ConnectionFD, SHUT_WR);
						shutdown(ConnectionFD, SHUT_RD);
						close(ConnectionFD);
						FD_CLR(ConnectionFD, &Master_FDs);
						FD_ZERO(&Master_FDs);
						ClientCheckUpdate("0000000000000000000000");
						if (AppStat)
							FlushAll();
						return;
					}
					Decrypt(recBuffer, recBuffer, EncryptionType);
					unSpecialize(recBuffer);
					if (!strcmp(recBuffer, "%CONF%")) {
						sprintf(Message, "\t\t***Users updated***\n");
						easyLog("bold");
						ClientCheckUpdate(recBuffer);
						continue;
					} else {
						sprintf(Message, "%s: %s\n", recBuffer, &recBuffer[20]);
						easyLog("");
					}
				}
			}
		}
	} else if (Type == SOCK_DGRAM) {
		for (;;) {
			strcpy(recBuffer, "");
			if (NonBlockingMode) {
				TWait.tv_sec = 0;
				TWait.tv_usec = 750000;
				select(NULL, NULL, NULL, NULL, &TWait);
			}
			if ((nbytes = recvfrom(ConnectionFD, recBuffer, sizeof recBuffer,
					0, (sockaddr *) &RemoteAddr, &addr_len)) <= 0) {
				inet_ntop(RemoteAddr.ss_family, get_in_addr(
						(sockaddr*) &RemoteAddr), remoteIP, INET6_ADDRSTRLEN);
				getnameinfo((sockaddr *) &RemoteAddr, sizeof RemoteAddr, host,
						sizeof host, service, sizeof service, 0);
				if (NonBlockingMode) {
					//					printf(
					//							"Receive Error: Maybe error happened due to time out\n");
					//					sprintf(
					//							Message,
					//							"Receive Error: Maybe error happened due to time out\n");
					//					BufferInsert();
				} else {
					perror("recvfrom");
					sprintf(
							Message,
							"Receive Error: cannot get data from %s with %s,%s\n",
							remoteIP, host, service);
					easyLog("error");
				}
			} else {
				inet_ntop(RemoteAddr.ss_family, get_in_addr(
						(sockaddr*) &RemoteAddr), remoteIP, INET6_ADDRSTRLEN);
				getnameinfo((sockaddr *) &RemoteAddr, sizeof RemoteAddr, host,
						sizeof host, service, sizeof service, 0);
				if (!strcmp(recBuffer, md5("%DISCONNECT%").c_str())) {
					sprintf(Message, "\t\t***Disconnected***\n");
					easyLog("bold");
					char sendBuffer[256] = "";
					strcpy(sendBuffer, "%DISCONNECTED%");
					strcpy(sendBuffer, md5(sendBuffer).c_str());
					sendto(ConnectionFD, sendBuffer, sizeof sendBuffer, 0,
							(sockaddr *) &RemoteAddr, sizeof RemoteAddr);
					shutdown(ConnectionFD, SHUT_RDWR);
					close(ConnectionFD);
					ClientCheckUpdate("0000000000000000000000");
					if (AppStat)
						FlushAll();
					return;
				} else if (!strcmp(recBuffer, md5("%LOGIN_FAILED%").c_str())) {
					sprintf(Message, "\t\t***Login Failed***\n");
					easyLog("bold");
					shutdown(ConnectionFD, SHUT_RDWR);
					close(ConnectionFD);
					ClientCheckUpdate("0000000000000000000000");
					if (AppStat)
						FlushAll();
					return;
				}
				Decrypt(recBuffer, recBuffer, EncryptionType);
				unSpecialize(recBuffer);
				if (!strcmp(recBuffer, "%CONF%")) {
					sprintf(Message, "\t\t***Users updated***\n");
					easyLog("bold");
					ClientCheckUpdate(recBuffer);
					continue;
				} else {
					sprintf(Message, "%s: %s\n", recBuffer, &recBuffer[20]);
					easyLog("");
				}
			}
		}
	}
}
void Socket::ClientSendMode() {
	int nbytes;
	char sendBuffer[256] = "";
	strcpy(sendBuffer, getClientCheck());
	strcpy(&sendBuffer[10], UserName);
	strcpy(&sendBuffer[30], BufferGet());
	Specialize(sendBuffer);
	Encrypt(sendBuffer, sendBuffer, EncryptionType);
	if (Type == SOCK_STREAM) {
		if ((nbytes = send(ConnectionFD, sendBuffer, sizeof sendBuffer, 0))
				== -1) {
			sprintf(Message, "Send Error: %d bytes of data sent to %s!\n",
					nbytes, ListenAddress);
			easyLog("error");
		} else {
			Decrypt(sendBuffer, sendBuffer, EncryptionType);
			unSpecialize(sendBuffer);
			sprintf(Message, "Me: %s\n", &sendBuffer[30]);
			easyLog("");
		}
		gtk_widget_grab_focus(GW("ClientSendText"));
	} else if (Type == SOCK_DGRAM) {
		if ((nbytes = sendto(ConnectionFD, sendBuffer, strlen(sendBuffer), 0,
				(sockaddr *) &RemoteAddr, sizeof RemoteAddr)) <= 0) {
			perror("sendto");
			sprintf(Message, "Send Error: %d bytes of data sent to %s!\n",
					nbytes, ListenAddress);
			easyLog("error");
		} else {
			Decrypt(sendBuffer, sendBuffer, EncryptionType);
			unSpecialize(sendBuffer);
			sprintf(Message, "Me: %s\n", &sendBuffer[30]);
			easyLog("");
		}
	}
}

int sendall(int s, char *buf, int *len) {
	int total = 0; // how many bytes we've sent
	int bytesleft = *len; // how many we have left to send
	int n;
	while (total < *len) {
		n = send(s, buf + total, bytesleft, 0);
		if (n == -1) {
			break;
		}
		total += n;
		bytesleft -= n;
	}
	*len = total; // return number actually sent here
	return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

void *get_in_addr(sockaddr *sa) {
	return &(((sockaddr_in*) sa)->sin_addr);
}
