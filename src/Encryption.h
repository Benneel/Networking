/*
 * Encryption.h
 *
 *  Created on: Aug 19, 2011
 *      Author: ben
 */

#ifndef ENCRYPTION_H_
#define ENCRYPTION_H_

#include <iostream>
using namespace std;

#include <stdio.h> //Keyboard & Monitor
#include <stdlib.h> //exit(1) & atoi()
#include <string.h>

#include "openssl/evp.h"

class Encryption {
private:
	EVP_CIPHER_CTX ectx, dctx;

	string digest_to_hex_string(const unsigned char *digest, int len);

protected:
	int EncryptionType;
	char Password[15];

	void EncryptionInit();
	void Encrypt(const char plaintext[256], char ciphertext[256], int Type);
	void Decrypt(const char ciphertext[256], char plaintext[256], int Type);

public:
	string md5(string text);

	Encryption();
	virtual ~Encryption();
};

#endif /* ENCRYPTION_H_ */
