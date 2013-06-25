/*
 * Communications.cpp
 *
 *  Created on: Jul 5, 2011
 *      Author: ben
 */

#include "Communications.h"

Communications::Communications() {
	// TODO Auto-generated constructor stub
	cout << "Constructing Communication...\n";
}

void Communications::init(const char* prgMode, const char* comType,
		const char* IPver, const char* comIP, const char* comPort,
		const char* comPassword, const bool comNonBlock,
		const int comEncryption) {
	if (AppStat)
		FlushAll();
	//Program Mode***********************************
	strcpy(ProgramMode, prgMode);
	//Socket Type************************************
	if (!strcmp(IPver, "IPv4")) {
		Domain = AF_INET;
		ConfigureFlags(AF_INET, -1, 0, AI_PASSIVE);
	} else if (!strcmp(IPver, "IPv6")) {
		Domain = AF_INET6;
		ConfigureFlags(AF_INET6, -1, 0, AI_PASSIVE);
	} else {
		Domain = AF_UNSPEC;
		ConfigureFlags(AF_UNSPEC, -1, 0, AI_PASSIVE);
	}
	strcpy(IPversion, IPver);
	//Connection Type********************************
	strcpy(CommunicationType, comType);
	if (!strcmp(comType, "TCP")) {
		ConfigureFlags(-1, SOCK_STREAM, 0, AI_PASSIVE);
	} else if (!strcmp(comType, "UDP")) {
		ConfigureFlags(-1, SOCK_DGRAM, 0, AI_PASSIVE);
	}
	strcpy(ListenAddress, comIP);
	strcpy(ListenPort, comPort);
	NonBlockingMode = comNonBlock;
	//Security***************************************
	strcpy(Password, comPassword);
	EncryptionType = comEncryption;
}

Communications::~Communications() {
	// TODO Auto-generated destructor stub
	cout << "Destructing Communication...\n";
}

void Communications::Run() {
	if (!strcmp(ProgramMode, "Server")) {
		if (!strcmp(CommunicationType, "TCP")) {
			CreateTCPServer();
		} else if (!strcmp(CommunicationType, "UDP")) {
			CreateUDPServer();
		}
	} else if (!strcmp(ProgramMode, "Client")) {
		if (!strcmp(CommunicationType, "TCP")) {
			CreateTCPClientReceiverMode();
		} else if (!strcmp(CommunicationType, "UDP")) {
			CreateUDPClientReceiverMode();
		}
	}
	AppStat = false;
}

void Communications::GetMachineIPs(void) {
	struct addrinfo *p;
	struct addrinfo hints;
	char remoteIP[INET6_ADDRSTRLEN];
	int cnt = 0, rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(HostName, NULL, &hints, &p)) != 0) {
		fprintf(stderr, "Error on getting network device information!! (%s)\n",
				gai_strerror(rv));
		fprintf(LogFD,
				"%sError on getting network device information!! (%s)\n",
				GetCurrentTime().c_str(), gai_strerror(rv));
		return;
	}

	printf("IP addresses for '%s':\n", HostName);
	fprintf(LogFD, "%sIP addresses for '%s':\n", GetCurrentTime().c_str(),
			HostName);
	for (; p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver = new char();

		cnt++;
		// get the pointer to the address itself,
		// different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET) { // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *) p->ai_addr;
			addr = &(ipv4->sin_addr);
			strcpy(ipver, "IPv4");
		} else { // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) p->ai_addr;
			addr = &(ipv6->sin6_addr);
			strcpy(ipver, "IPv6");
		}

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, remoteIP, sizeof remoteIP);
		printf("  --device %i: %s, %s\n", cnt, ipver, remoteIP);
		fprintf(LogFD, "%s  --device %i: %s, %s\n", GetCurrentTime().c_str(),
				cnt, ipver, remoteIP);
	}
	freeaddrinfo(p);
	cout << endl;
}

void Communications::ConfigureFlags(int D, int T, int P, int F) {
	if (D != -1)
		Domain = D;
	if (T != -1)
		Type = T;
	if (P != -1)
		Protocol = P;
	if (F != -1)
		Flag = F;
}
