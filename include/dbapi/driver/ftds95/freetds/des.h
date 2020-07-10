#ifndef DES_H
#define DES_H

/* $Id: des.h 487511 2015-12-17 20:11:40Z ucko $ */

#include <freetds/pushvis.h>

typedef unsigned char des_cblock[8];

typedef struct des_key
{
	unsigned char kn[16][8];
	TDS_UINT sp[8][64];
	unsigned char iperm[16][16][8];
	unsigned char fperm[16][16][8];
} DES_KEY;

void tds_des_set_odd_parity(des_cblock key);
int tds_des_ecb_encrypt(const void *plaintext, size_t len, DES_KEY * akey, des_cblock output);
int tds_des_set_key(DES_KEY * dkey, const des_cblock user_key, size_t len);
void tds_des_encrypt(DES_KEY * key, des_cblock block);
void _mcrypt_decrypt(DES_KEY * key, unsigned char *block);

#include <freetds/popvis.h>

#endif /* !DES_H */
