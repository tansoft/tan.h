/* rsa-md5-sign.c
 *
 * Signatures using RSA and MD5.
 */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2001, 2003 Niels M�ller
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

#include "rsa.h"

#include "bignum.h"
#include "pkcs1.h"

void
rsa_md5_sign(const struct rsa_private_key *key,
             struct md5_ctx *hash,
             mpz_t s)
{
  assert(key->size >= RSA_MINIMUM_N_OCTETS);

  pkcs1_rsa_md5_encode(s, key->size - 1, hash);

  rsa_compute_root(key, s, s);
}

void
rsa_md5_sign_digest(const struct rsa_private_key *key,
		    const uint8_t *digest,
		    mpz_t s)
{
  assert(key->size >= RSA_MINIMUM_N_OCTETS);

  pkcs1_rsa_md5_encode_digest(s, key->size - 1, digest);

  rsa_compute_root(key, s, s);
}
