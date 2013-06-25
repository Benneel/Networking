/*
 * Encryption.cpp
 *
 *  Created on: Aug 19, 2011
 *      Author: ben
 */

#include "Encryption.h"

Encryption::Encryption() {
	// TODO Auto-generated constructor stub
	cout << "Constructing Encryption...\n";

}

Encryption::~Encryption() {
	// TODO Auto-generated destructor stub
	cout << "Destructing Encryption...\n";
}
/******************************************************************************
 public utils
 ******************************************************************************/
string Encryption::digest_to_hex_string(const unsigned char *digest, int len) {
	char* hexstring = (char*) malloc(len * 2 + 1);
	int i;
	len = len > 510 ? 510 : len;
	for (i = 0; i < len; i++) {
		sprintf(&hexstring[2 * i], "%02x", digest[i]);
	}
	string hex(hexstring);
	free(hexstring);
	return hex;
}
/******************************************************************************
 md5
 ******************************************************************************/
string Encryption::md5(string text) {
	EVP_MD_CTX mdctx;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_len;
	EVP_DigestInit(&mdctx, EVP_md5());
	EVP_DigestUpdate(&mdctx, text.c_str(), text.size());
	EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
	EVP_MD_CTX_cleanup(&mdctx);
	return digest_to_hex_string(md_value, md_len);
}
/******************************************************************************
 Two way hashing: AES, DES, BlowFish, RC4
 ******************************************************************************/
void Encryption::EncryptionInit() {
	unsigned char key[EVP_MAX_KEY_LENGTH];
	unsigned char iv[EVP_MAX_IV_LENGTH] = "Ben";
	EVP_CIPHER_CTX_init(&ectx);
	EVP_CIPHER_CTX_init(&dctx);
	/* 8 bytes to salt the key_data during key generation. This is an example of
	 compiled in salt. We just read the bit pattern created by these two 4 byte
	 integers on the stack as 64 bits of contigous salt material -
	 ofcourse this only works if sizeof(int) >= 4 */
	unsigned int salt[] = { 12345, 54321 };
	int i, nrounds = 5;
	switch (EncryptionType) {
	case 0:
		//No Encryption Mode
		break;
	case 1:
		i = EVP_BytesToKey(EVP_aes_128_cfb(), EVP_sha1(),
				(unsigned char *) &salt, (unsigned char *) Password, strlen(
						Password), nrounds, key, iv);
		EVP_EncryptInit_ex(&ectx, EVP_aes_128_cfb(), NULL,
				(unsigned char*) key, (unsigned char*) &iv);
		EVP_DecryptInit_ex(&dctx, EVP_aes_128_cfb(), NULL,
				(unsigned char*) key, (unsigned char*) &iv);
		break;
	case 2:
		i = EVP_BytesToKey(EVP_des_ede_cfb(), EVP_sha1(),
				(unsigned char *) &salt, (unsigned char *) Password, strlen(
						Password), nrounds, key, iv);
		EVP_EncryptInit_ex(&ectx, EVP_des_ede_cfb(), NULL,
				(unsigned char*) key, (unsigned char*) &iv);
		EVP_DecryptInit_ex(&dctx, EVP_des_ede_cfb(), NULL,
				(unsigned char*) key, (unsigned char*) &iv);
		break;
	case 3:
		i = EVP_BytesToKey(EVP_rc4(), EVP_sha1(), (unsigned char *) &salt,
				(unsigned char *) Password, strlen(Password), nrounds, key, iv);
		EVP_EncryptInit_ex(&ectx, EVP_rc4(), NULL, (unsigned char*) key,
				(unsigned char*) &iv);
		EVP_DecryptInit_ex(&dctx, EVP_rc4(), NULL, (unsigned char*) key,
				(unsigned char*) &iv);
		break;
	case 4:
		i = EVP_BytesToKey(EVP_bf_cfb(), EVP_sha1(), (unsigned char *) &salt,
				(unsigned char *) Password, strlen(Password), nrounds, key, iv);
		EVP_EncryptInit_ex(&ectx, EVP_bf_cfb(), NULL, (unsigned char*) key,
				(unsigned char*) &iv);
		EVP_DecryptInit_ex(&dctx, EVP_bf_cfb(), NULL, (unsigned char*) key,
				(unsigned char*) &iv);
		break;
	}
	return;
}
void Encryption::Encrypt(const char plaintext[256], char ciphertext[256],
		int Type) {
	if (Type) {
		EncryptionInit();
		int in_len = 0, out_len = 0;
		in_len = strlen(plaintext);
		EVP_EncryptUpdate(&ectx, (unsigned char*) ciphertext, &out_len,
				(unsigned char*) plaintext, in_len);
		EVP_EncryptFinal_ex(&ectx, (unsigned char*) ciphertext[out_len],
				&out_len);
		EVP_CIPHER_CTX_cleanup(&ectx);
		EVP_CIPHER_CTX_cleanup(&dctx);
	}
}
void Encryption::Decrypt(const char ciphertext[256], char plaintext[256],
		int Type) {
	if (Type) {
		EncryptionInit();
		int in_len = 0, out_len = 0;
		out_len = strlen(ciphertext);
		EVP_DecryptUpdate(&dctx, (unsigned char*) plaintext, &in_len,
				(unsigned char*) ciphertext, out_len);
		EVP_DecryptFinal_ex(&dctx, (unsigned char*) plaintext[in_len], &in_len);
		EVP_CIPHER_CTX_cleanup(&ectx);
		EVP_CIPHER_CTX_cleanup(&dctx);
	}
}
