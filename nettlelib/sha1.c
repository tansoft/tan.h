/* sha1.c
 *
 * The sha1 hash function.
 * Defined by http://www.itl.nist.gov/fipspubs/fip180-1.htm.
 */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2001 Peter Gutmann, Andrew Kuchling, Niels M�ller
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

/* Here's the first paragraph of Peter Gutmann's posting,
 * <30ajo5$oe8@ccu2.auckland.ac.nz>: 
 *
 * The following is my SHA (FIPS 180) code updated to allow use of the "fixed"
 * SHA, thanks to Jim Gillogly and an anonymous contributor for the information on
 * what's changed in the new version.  The fix is a simple change which involves
 * adding a single rotate in the initial expansion function.  It is unknown
 * whether this is an optimal solution to the problem which was discovered in the
 * SHA or whether it's simply a bandaid which fixes the problem with a minimum of
 * effort (for example the reengineering of a great many Capstone chips).
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "sha.h"

#include "macros.h"

/* A block, treated as a sequence of 32-bit words. */
#define SHA1_DATA_LENGTH 16

/* SHA initial values */

#define h0init  0x67452301L
#define h1init  0xEFCDAB89L
#define h2init  0x98BADCFEL
#define h3init  0x10325476L
#define h4init  0xC3D2E1F0L

/* Initialize the SHA values */

void
sha1_init(struct sha1_ctx *ctx)
{
  /* Set the h-vars to their initial values */
  ctx->digest[ 0 ] = h0init;
  ctx->digest[ 1 ] = h1init;
  ctx->digest[ 2 ] = h2init;
  ctx->digest[ 3 ] = h3init;
  ctx->digest[ 4 ] = h4init;

  /* Initialize bit count */
  ctx->count_low = ctx->count_high = 0;
  
  /* Initialize buffer */
  ctx->index = 0;
}

#define SHA1_INCR(ctx) ((ctx)->count_high += !++(ctx)->count_low)

void
sha1_update(struct sha1_ctx *ctx,
	    unsigned length, const uint8_t *buffer)
{
  if (ctx->index)
    { /* Try to fill partial block */
      unsigned left = SHA1_DATA_SIZE - ctx->index;
      if (length < left)
	{
	  memcpy(ctx->block + ctx->index, buffer, length);
	  ctx->index += length;
	  return; /* Finished */
	}
      else
	{
	  memcpy(ctx->block + ctx->index, buffer, left);

	  _nettle_sha1_compress(ctx->digest, ctx->block);
	  SHA1_INCR(ctx);

	  buffer += left;
	  length -= left;
	}
    }
  while (length >= SHA1_DATA_SIZE)
    {
      _nettle_sha1_compress(ctx->digest, buffer);
      SHA1_INCR(ctx);

      buffer += SHA1_DATA_SIZE;
      length -= SHA1_DATA_SIZE;
    }
  if ((ctx->index = length))     /* This assignment is intended */
    /* Buffer leftovers */
    memcpy(ctx->block, buffer, length);
}
	  
/* Final wrapup - pad to SHA1_DATA_SIZE-byte boundary with the bit pattern
   1 0* (64-bit count of bits processed, MSB-first) */

static void
sha1_final(struct sha1_ctx *ctx)
{
  uint32_t bitcount_high;
  uint32_t bitcount_low;
  unsigned i;
  
  i = ctx->index;
  
  /* Set the first char of padding to 0x80.  This is safe since there is
     always at least one byte free */

  assert(i < SHA1_DATA_SIZE);
  ctx->block[i++] = 0x80;

  if (i > (SHA1_DATA_SIZE - 8))
    { /* No room for length in this block. Process it and
	 pad with another one */
      memset(ctx->block + i, 0, SHA1_DATA_SIZE - i);
      
      _nettle_sha1_compress(ctx->digest, ctx->block);
      i = 0;
    }
  if (i < (SHA1_DATA_SIZE - 8))
    memset(ctx->block + i, 0, (SHA1_DATA_SIZE - 8) - i);

  /* There are 512 = 2^9 bits in one block */  
  bitcount_high = (ctx->count_high << 9) | (ctx->count_low >> 23);
  bitcount_low = (ctx->count_low << 9) | (ctx->index << 3);

  /* This is slightly inefficient, as the numbers are converted to
     big-endian format, and will be converted back by the compression
     function. It's probably not worth the effort to fix this. */
  WRITE_UINT32(ctx->block + (SHA1_DATA_SIZE - 8), bitcount_high);
  WRITE_UINT32(ctx->block + (SHA1_DATA_SIZE - 4), bitcount_low);

  _nettle_sha1_compress(ctx->digest, ctx->block);
}

void
sha1_digest(struct sha1_ctx *ctx,
	    unsigned length,
	    uint8_t *digest)
{
  unsigned i;
  unsigned words;
  unsigned leftover;
  
  assert(length <= SHA1_DIGEST_SIZE);

  sha1_final(ctx);
  
  words = length / 4;
  leftover = length % 4;

  for (i = 0; i < words; i++, digest += 4)
    WRITE_UINT32(digest, ctx->digest[i]);

  if (leftover)
    {
      uint32_t word;
      unsigned j = leftover;
      
      assert(i < _SHA1_DIGEST_LENGTH);
      
      word = ctx->digest[i];
      
      switch (leftover)
	{
	default:
	  abort();
	case 3:
	  digest[--j] = (word >> 8) & 0xff;
	  /* Fall through */
	case 2:
	  digest[--j] = (word >> 16) & 0xff;
	  /* Fall through */
	case 1:
	  digest[--j] = (word >> 24) & 0xff;
	}
    }
  sha1_init(ctx);
}
