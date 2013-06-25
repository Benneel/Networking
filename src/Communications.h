/*
 * Communications.h
 *
 *  Created on: Jul 5, 2011
 *      Author: ben
 */

#ifndef COMMUNICATIONS_H_
#define COMMUNICATIONS_H_

#include "TCPSocket.h"
#include "UDPSocket.h"

class Communications: public virtual TCPSocket, public virtual UDPSocket {
private:
	char CommunicationType[5];

	void ConfigureFlags(int, int, int, int);

public:
	Communications();
	virtual ~Communications();

	void init(const char*, const char*, const char*, const char*, const char*,
			const char*, const bool, const int);
	void Run();
	void GetMachineIPs(void);
};

#endif /* COMMUNICATIONS_H_ */
