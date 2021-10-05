/* des3.h
 *
 * Triple DES cipher. Three key encrypt-decrypt-encrypt.
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

#include "des.h"

/* It's possible to make some more general pipe construction, like the
 * lsh/src/cascade.c, but as in practice it's never used for anything
 * like triple DES, it's not worth the effort. */

/* On success, returns 1 and sets ctx->status to DES_OK (zero). On
 * error, returns 0 and sets ctx->status accordingly. */
int
des3_set_key(struct des3_ctx *ctx, const uint8_t *key)
{
  unsigned i;
  for (i = 0; i<3; i++, key += DES_KEY_SIZE)
    if (!des_set_key(&ctx->des[i], key))
      {
	ctx->status = ctx->des[i].status;
	return 0;
      }
  ctx->status = DES_OK;
  return 1;
}

void
des3_encrypt(const struct des3_ctx *ctx,
	     unsigned length, uint8_t *dst,
	     const uint8_t *src)
{
  des_encrypt(&ctx->des[0],
	      length, dst, src);
  des_decrypt(&ctx->des[1],
	      length, dst, dst);
  des_encrypt(&ctx->des[2],
	      length, dst, dst);
}

void
des3_decrypt(const struct des3_ctx *ctx,
	     unsigned length, uint8_t *dst,
	     const uint8_t *src)
{
  des_decrypt(&ctx->des[2],
	      length, dst, src);
  des_encrypt(&ctx->des[1],
	      length, dst, dst);
  des_decrypt(&ctx->des[0],
	      length, dst, dst);
} 
