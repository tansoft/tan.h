/* memxor.c
 *
 * $Id: memxor.c,v 1.1 2007/04/05 14:20:35 nisse Exp $
 */

/* XOR LEN bytes starting at SRCADDR onto DESTADDR.  Result undefined
   if the source overlaps with the destination.
   Return DESTADDR. */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "memxor.h"

uint8_t *
memxor(uint8_t *dst, const uint8_t *src, size_t n)
{
  size_t i;
  for (i = 0; i<n; i++)
    dst[i] ^= src[i];

  return dst;
}

uint8_t *
memxor3(uint8_t *dst, const uint8_t *a, const uint8_t *b, size_t n)
{
  size_t i;
  for (i = 0; i<n; i++)
    dst[i] = a[i] ^ b[i];

  return dst;
}

