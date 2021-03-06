/* md2.h
 *
 * The MD2 hash function, described in RFC 1319.
 */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2003 Niels M?ller, Andreas Sigfridsson
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

/* This code originates from the Python Cryptography Toolkit, version 1.0.1.
   Further hacked by Andreas Sigfridsson and Niels M?ller. Original license:

   ===================================================================
   Distribute and use freely; there are no restrictions on further
   dissemination and usage except those imposed by the laws of your
   country of residence.  This software is provided "as is" without
   warranty of fitness for use or suitability for any purpose, express
   or implied. Use at your own risk or not at all.
   ===================================================================
   
   Incorporating the code into commercial products is permitted; you do
   not have to make source available or contribute your changes back
   (though that would be nice).
   
   --amk                                                    (www.amk.ca) */
   
#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <string.h>

#include "md2.h"

#include "macros.h"

static const uint8_t
S[256] = {
  41, 46, 67, 201, 162, 216, 124, 1, 61, 54, 84, 161, 236, 240, 6,
  19, 98, 167, 5, 243, 192, 199, 115, 140, 152, 147, 43, 217, 188,
  76, 130, 202, 30, 155, 87, 60, 253, 212, 224, 22, 103, 66, 111, 24,
  138, 23, 229, 18, 190, 78, 196, 214, 218, 158, 222, 73, 160, 251,
  245, 142, 187, 47, 238, 122, 169, 104, 121, 145, 21, 178, 7, 63,
  148, 194, 16, 137, 11, 34, 95, 33, 128, 127, 93, 154, 90, 144, 50,
  39, 53, 62, 204, 231, 191, 247, 151, 3, 255, 25, 48, 179, 72, 165,
  181, 209, 215, 94, 146, 42, 172, 86, 170, 198, 79, 184, 56, 210,
  150, 164, 125, 182, 118, 252, 107, 226, 156, 116, 4, 241, 69, 157,
  112, 89, 100, 113, 135, 32, 134, 91, 207, 101, 230, 45, 168, 2, 27,
  96, 37, 173, 174, 176, 185, 246, 28, 70, 97, 105, 52, 64, 126, 15,
  85, 71, 163, 35, 221, 81, 175, 58, 195, 92, 249, 206, 186, 197,
  234, 38, 44, 83, 13, 110, 133, 40, 132, 9, 211, 223, 205, 244, 65,
  129, 77, 82, 106, 220, 55, 200, 108, 193, 171, 250, 36, 225, 123,
  8, 12, 189, 177, 74, 120, 136, 149, 139, 227, 99, 232, 109, 233,
  203, 213, 254, 59, 0, 29, 57, 242, 239, 183, 14, 102, 88, 208, 228,
  166, 119, 114, 248, 235, 117, 75, 10, 49, 68, 80, 180, 143, 237,
  31, 26, 219, 153, 141, 51, 159, 17, 131, 20
};

static void
md2_transform(struct md2_ctx *ctx, const uint8_t *data)
{
  unsigned i;
  uint8_t t;
  
  memcpy(ctx->X + 16, data, MD2_DATA_SIZE);

  for (i = 0, t = ctx->C[15];
       i<MD2_DATA_SIZE; i++)
    {
      ctx->X[2 * MD2_DATA_SIZE + i]
	= ctx->X[i] ^ ctx->X[MD2_DATA_SIZE + i];
      t = (ctx->C[i] ^= S[data[i]^t]);
    }
  for (i = t = 0;
       i< MD2_DATA_SIZE + 2;
       t = (t + i) & 0xff, i++)
    {
      unsigned j;
      for (j = 0; j < 3 * MD2_DATA_SIZE; j++)
	t = (ctx->X[j] ^= S[t]);
    }
}

#if 0
static void
md2_final(struct md2_ctx *ctx)
{
  unsigned left = MD2_DATA_SIZE - ctx->index;
  memset(ctx->block + ctx->index, left, left);
  md2_transform(ctx, ctx->block);
}
#endif

void
md2_init(struct md2_ctx *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

void
md2_update(struct md2_ctx *ctx,
	   unsigned length,
	   const uint8_t *data)
{
  if (ctx->index)
    {
      /* Try to fill partial block */
      unsigned left = MD2_DATA_SIZE - ctx->index;
      if (length < left)
	{
	  memcpy(ctx->block + ctx->index, data, length);
	  ctx->index += length;
	  return; /* Finished */
	}
      else
	{
	  memcpy(ctx->block + ctx->index, data, left);
	  md2_transform(ctx, ctx->block);
	  data += left;
	  length -= left;
	}
    }
  while (length >= MD2_DATA_SIZE)
    {
      md2_transform(ctx, data);
      data += MD2_DATA_SIZE;
      length -= MD2_DATA_SIZE;
    }
  if ((ctx->index = length))     /* This assignment is intended */
    /* Buffer leftovers */
    memcpy(ctx->block, data, length);
}

void
md2_digest(struct md2_ctx *ctx,
	   unsigned length,
	   uint8_t *digest)
{
  unsigned left;
  
  assert(length <= MD2_DIGEST_SIZE);

  left = MD2_DATA_SIZE - ctx->index;
  memset(ctx->block + ctx->index, left, left);
  md2_transform(ctx, ctx->block);
  
  md2_transform(ctx, ctx->C);
  memcpy(digest, ctx->X, length);
  md2_init(ctx);
}
