/* aes.h
 *
 * The aes/rijndael block cipher.
 */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2001 Niels M?ller
 *  
 * The nettle library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 * 
 * The nettle library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with the nettle library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */
 
#ifndef NETTLE_AES_H_INCLUDED
#define NETTLE_AES_H_INCLUDED

#include "nettle-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define aes_set_encrypt_key nettle_aes_set_encrypt_key
#define aes_set_decrypt_key nettle_aes_set_decrypt_key
#define aes_encrypt nettle_aes_encrypt
#define aes_decrypt nettle_aes_decrypt

#define AES_BLOCK_SIZE 16

/* Variable key size between 128 and 256 bits. But the only valid
 * values are 16 (128 bits), 24 (192 bits) and 32 (256 bits). */
#define AES_MIN_KEY_SIZE 16
#define AES_MAX_KEY_SIZE 32

#define AES_KEY_SIZE 32

struct aes_ctx
{
  uint32_t keys[60];  /* maximum size of key schedule */
  unsigned  nrounds;  /* number of rounds to use for our key size */
};

void
aes_set_encrypt_key(struct aes_ctx *ctx,
		    unsigned length, const uint8_t *key);
void
aes_set_decrypt_key(struct aes_ctx *ctx,
		   unsigned length, const uint8_t *key);

void
aes_encrypt(struct aes_ctx *ctx,
	    unsigned length, uint8_t *dst,
	    const uint8_t *src);
void
aes_decrypt(struct aes_ctx *ctx,
	    unsigned length, uint8_t *dst,
	    const uint8_t *src);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_AES_H_INCLUDED */
