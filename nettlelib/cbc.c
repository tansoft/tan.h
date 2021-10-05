/* cbc.c
 *
 * Cipher block chaining mode.
 */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2001 Niels M�ller
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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cbc.h"

#include "memxor.h"
#include "nettle-internal.h"

void
cbc_encrypt(void *ctx, nettle_crypt_func f,
	    unsigned block_size, uint8_t *iv,
	    unsigned length, uint8_t *dst,
	    const uint8_t *src)
{
  assert(!(length % block_size));

  for ( ; length; length -= block_size, src += block_size, dst += block_size)
    {
      memxor(iv, src, block_size);
      f(ctx, block_size, dst, iv);
      memcpy(iv, dst, block_size);
    }
}

/* Reqires that dst != src */
static void
cbc_decrypt_internal(void *ctx, nettle_crypt_func f,
		     unsigned block_size, uint8_t *iv,
		     unsigned length, uint8_t *dst,
		     const uint8_t *src)
{
  assert(length);
  assert( !(length % block_size) );
  assert(src != dst);
  
  /* Decrypt in ECB mode */
  f(ctx, length, dst, src);

  /* XOR the cryptotext, shifted one block */
  memxor(dst, iv, block_size);
  memxor(dst + block_size, src, length - block_size);
  memcpy(iv, src + length - block_size, block_size);
}

/* Don't allocate any more space than this on the stack */
#define CBC_BUFFER_LIMIT 4096

void
cbc_decrypt(void *ctx, nettle_crypt_func f,
	    unsigned block_size, uint8_t *iv,
	    unsigned length, uint8_t *dst,
	    const uint8_t *src)
{
  assert(!(length % block_size));

  if (!length)
    return;

  if (src != dst)
    cbc_decrypt_internal(ctx, f, block_size, iv,
			 length, dst, src);
  else
    {
      /* We need a copy of the ciphertext, so we can't ECB decrypt in
       * place.
       *
       * If length is small, we allocate a complete copy of src on the
       * stack. Otherwise, we allocate a block of size at most
       * CBC_BUFFER_LIMIT, and process that amount of data at a
       * time.
       *
       * NOTE: We assume that block_size <= CBC_BUFFER_LIMIT. */

      unsigned buffer_size;

      if (length <= CBC_BUFFER_LIMIT)
	buffer_size = length;
      else
	buffer_size
	  = CBC_BUFFER_LIMIT - (CBC_BUFFER_LIMIT % block_size);

      {
	TMP_DECL(buffer, uint8_t, CBC_BUFFER_LIMIT);
	TMP_ALLOC(buffer, buffer_size);

	for ( ; length > buffer_size;
	      length -= buffer_size, dst += buffer_size, src += buffer_size)
	  {
	    memcpy(buffer, src, buffer_size);
	    cbc_decrypt_internal(ctx, f, block_size, iv,
				 buffer_size, dst, buffer);
	  }
	/* Now, we have at most CBC_BUFFER_LIMIT octets left */
	memcpy(buffer, src, length);
	
	cbc_decrypt_internal(ctx, f, block_size, iv,
			     length, dst, buffer);
      }
    }
}

#if 0
#include "twofish.h"
#include "aes.h"

static void foo(void)
{
  struct CBC_CTX(struct twofish_ctx, TWOFISH_BLOCK_SIZE) ctx;
  uint8_t src[TWOFISH_BLOCK_SIZE];
  uint8_t dst[TWOFISH_BLOCK_SIZE];
  
  CBC_ENCRYPT(&ctx, twofish_encrypt, TWOFISH_BLOCK_SIZE, dst, src);

  /* Should result in a warning */
  CBC_ENCRYPT(&ctx, aes_encrypt, TWOFISH_BLOCK_SIZE, dst, src);
  
}

static void foo2(void)
{
  struct twofish_ctx ctx;
  uint8_t iv[TWOFISH_BLOCK_SIZE];
  uint8_t src[TWOFISH_BLOCK_SIZE];
  uint8_t dst[TWOFISH_BLOCK_SIZE];
  
  CBC_ENCRYPT2(&ctx, twofish_encrypt, TWOFISH_BLOCK_SIZE, iv, TWOFISH_BLOCK_SIZE, dst, src);
  /* Should result in a warning */
  CBC_ENCRYPT2(&ctx, aes_encrypt, TWOFISH_BLOCK_SIZE, iv, TWOFISH_BLOCK_SIZE, dst, src);
}

#endif
