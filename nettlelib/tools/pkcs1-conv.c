/* pkcs1-conv.c
 *
 * Converting pkcs#1 keys to sexp format. */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2005 Niels M?ller
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asn1.h"
#include "base64.h"
#include "buffer.h"
#include "rsa.h"

#include "getopt.h"
#include "misc.h"

enum object_type
  {
    RSA_PRIVATE_KEY = 0x200,
    RSA_PUBLIC_KEY,
    GENERAL_PUBLIC_KEY,
  };

static int
write_file(struct nettle_buffer *buffer, FILE *f)
{
  size_t res = fwrite(buffer->contents, 1, buffer->size, f);
  if (res < buffer->size)
    {
      werror("Write failed: %s.\n", strerror(errno));
      return 0;
    }
  else
    return 1;
}

/* Return 1 on success, 0 on error, -1 on eof */
static int
read_line(struct nettle_buffer *buffer, FILE *f)
{
  int c;
  
  while ((c = getc(f)) != EOF)
    {
      if (!NETTLE_BUFFER_PUTC(buffer, c))
	return 0;

      if (c == '\n')
	return 1;
    }
  if (ferror(f))
    {
      werror("Read failed: %s\n", strerror(errno));
      return 0;
    }
  
  else 
    return -1;
}

static int
read_file(struct nettle_buffer *buffer, FILE *f)
{
  int c;
  
  while ((c = getc(f)) != EOF)
    if (!NETTLE_BUFFER_PUTC(buffer, c))
      return 0;

  if (ferror(f))
    {
      werror("Read failed: %s\n", strerror(errno));
      return 0;
    }
  else
    return 1;
}

static const uint8_t
pem_start_pattern[11] = "-----BEGIN ";

static const uint8_t
pem_end_pattern[9] = "-----END ";

static const uint8_t
pem_trailer_pattern[5] = "-----";

static const char
pem_ws[33] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 0, 0, /* \t, \n, \v, \f, \r */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  1 /* SPC */
};

#define PEM_IS_SPACE(c) ((c) < sizeof(pem_ws) && pem_ws[(c)]) 

/* Returns 1 on match, otherwise 0. */ 
static int
match_pem_start(unsigned length, const uint8_t *line,
		unsigned *marker_start,
		unsigned *marker_length)
{
  while (length > 0 && PEM_IS_SPACE(line[length - 1]))
    length--;

  if (length > (sizeof(pem_start_pattern) + sizeof(pem_trailer_pattern))
      && memcmp(line, pem_start_pattern, sizeof(pem_start_pattern)) == 0
      && memcmp(line + length - sizeof(pem_trailer_pattern),
		pem_trailer_pattern, sizeof(pem_trailer_pattern)) == 0)
    {
      *marker_start = 11;
      *marker_length = length - (sizeof(pem_start_pattern) + sizeof(pem_trailer_pattern));

      return 1;
    }
  else
    return 0;
}

/* Returns 1 on match, -1 if the line is of the right form except for
   the marker, otherwise 0. */ 
static int
match_pem_end(unsigned length, const uint8_t *line,
	      unsigned marker_length,
	      const uint8_t *marker)
{
  while (length > 0 && PEM_IS_SPACE(line[length - 1]))
    length--;

  if (length > (sizeof(pem_end_pattern) + sizeof(pem_trailer_pattern))
      && memcmp(line, pem_end_pattern, sizeof(pem_end_pattern)) == 0
      && memcmp(line + length - sizeof(pem_trailer_pattern),
		pem_trailer_pattern, sizeof(pem_trailer_pattern)) == 0)
    {
      /* Right form. Check marker */
      if (length == marker_length + (sizeof(pem_end_pattern) + sizeof(pem_trailer_pattern))
	  && memcmp(line + sizeof(pem_end_pattern), marker, marker_length) == 0)
	return 1;
      else
	return -1;
    }
  else
    return 0;  
}

struct pem_info
{
  /* The FOO part in "-----BEGIN FOO-----" */
  unsigned marker_start;
  unsigned marker_length;
  unsigned data_start;
  unsigned data_length;
};

static int
read_pem(struct nettle_buffer *buffer, FILE *f,
	 struct pem_info *info)
{  
  /* Find start line */
  for (;;)
    {
      int res;

      nettle_buffer_reset(buffer);

      res = read_line(buffer, f);
      if (res != 1)
	return res;

      if (match_pem_start(buffer->size, buffer->contents,
			  &info->marker_start, &info->marker_length))
	break;
    }

  /* NUL-terminate the marker. Don't care to check for embedded NULs. */
  buffer->contents[info->marker_start + info->marker_length] = 0;

  info->data_start = buffer->size;

  for (;;)
    {
      unsigned line_start = buffer->size;

      if (read_line(buffer, f) != 1)
	return 0;

      switch (match_pem_end(buffer->size - line_start,
			    buffer->contents + line_start,
			    info->marker_length,
			    buffer->contents + info->marker_start))
	{
	case 0:
	  break;
	case -1:
	  werror("PEM END line doesn't match BEGIN.\n");
	  return 0;
	case 1:
	  /* Return base 64 data; let caller do the decoding */ 
	  info->data_length = line_start - info->data_start;
	  return 1;
	}
    }
}

static int
decode_base64(struct nettle_buffer *buffer,
	      unsigned start, unsigned *length)
{
  struct base64_decode_ctx ctx;
  
  base64_decode_init(&ctx);

  /* Decode in place */
  if (base64_decode_update(&ctx,
			   length, buffer->contents + start,
			   *length, buffer->contents + start)
      && base64_decode_final(&ctx))
    return 1;
  
  else
    {
      werror("Invalid base64 date.\n");
      return 0;
    }
}

static int
convert_rsa_public_key(struct nettle_buffer *buffer, unsigned length, const uint8_t *data)
{
  struct rsa_public_key pub;
  int res;
  
  rsa_public_key_init(&pub);

  if (rsa_keypair_from_der(&pub, NULL, 0,
			   length, data))
    {
      /* Reuses the buffer */
      nettle_buffer_reset(buffer);
      res = rsa_keypair_to_sexp(buffer, NULL, &pub, NULL);
    }
  else
    {
      werror("Invalid PKCS#1 public key.\n");
      res = 0;
    }
  rsa_public_key_clear(&pub);
  return res;
}

static int
convert_rsa_private_key(struct nettle_buffer *buffer, unsigned length, const uint8_t *data)
{
  struct rsa_public_key pub;
  struct rsa_private_key priv;
  int res;
  
  rsa_public_key_init(&pub);
  rsa_private_key_init(&priv);

  if (rsa_keypair_from_der(&pub, &priv, 0,
			   length, data))
    {
      /* Reuses the buffer */
      nettle_buffer_reset(buffer);
      res = rsa_keypair_to_sexp(buffer, NULL, &pub, &priv);
    }
  else
    {
      werror("Invalid PKCS#1 private key.\n");
      res = 0;
    }
  rsa_public_key_clear(&pub);
  rsa_private_key_clear(&priv);

  return res;
}

/* Returns 1 on success, 0 on error, and -1 for unsupported algorithms. */
static int
convert_public_key(struct nettle_buffer *buffer, unsigned length, const uint8_t *data)
{
  /* SubjectPublicKeyInfo ::= SEQUENCE {
         algorithm		AlgorithmIdentifier,
	 subjectPublicKey 	BIT STRING
     }

     AlgorithmIdentifier ::= SEQUENCE {
         algorithm  	OBJECT IDENTIFIER,
	 parameters 	OPTIONAL
     }
  */
  struct asn1_der_iterator i;
  struct asn1_der_iterator j;
  int res = 0;

  if (asn1_der_iterator_first(&i, length, data) == ASN1_ITERATOR_CONSTRUCTED
      && i.type == ASN1_SEQUENCE
      && asn1_der_decode_constructed_last(&i) == ASN1_ITERATOR_CONSTRUCTED
      && i.type == ASN1_SEQUENCE

      /* Use the j iterator to parse the algorithm identifier */
      && asn1_der_decode_constructed(&i, &j) == ASN1_ITERATOR_PRIMITIVE
      && j.type == ASN1_IDENTIFIER
      && asn1_der_iterator_next(&i) == ASN1_ITERATOR_PRIMITIVE
      && i.type == ASN1_BITSTRING

      /* Use i to parse the object wrapped in the bit string. For all
	 currently supported key types, it is a sequence. */
      && asn1_der_decode_bitstring_last(&i) == ASN1_ITERATOR_CONSTRUCTED)
    {
      /* pkcs-1 {
	     iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs-1(1)
	     modules(0) pkcs-1(1)
	 }

	 --
	 -- When rsaEncryption is used in an AlgorithmIdentifier the
	 -- parameters MUST be present and MUST be NULL.
	 --
	 rsaEncryption    OBJECT IDENTIFIER ::= { pkcs-1 1 }
      */
      static const uint8_t id_rsaEncryption[9] =
	{ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01 };
      
      switch (j.length)
	{
	unknown:
	default:
	  werror("SubjectPublicKeyInfo: Unsupported algorithm.\n");
	  res = -1;
	  break;
	  
	case 9:
	  if (memcmp(j.data, id_rsaEncryption, 9) == 0)
	    {
	      if (asn1_der_iterator_next(&j) == ASN1_ITERATOR_PRIMITIVE
		  && j.type == ASN1_NULL
		  && j.length == 0
		  && asn1_der_iterator_next(&j) == ASN1_ITERATOR_END)
		{
		  struct rsa_public_key pub;

		  rsa_public_key_init(&pub);

		  if (rsa_public_key_from_der_iterator(&pub, 0, &i))
		    {
		      nettle_buffer_reset(buffer);
		      res = rsa_keypair_to_sexp(buffer, NULL, &pub, NULL) > 0;
		    }
		}
	      if (!res)
		werror("SubjectPublicKeyInfo: Invalid RSA key.\n");
	      break;
	    }
	  else goto unknown;
	}
    }
  else
    werror("SubjectPublicKeyInfo: Invalid object.\n");
  
  return res;
}

/* NOTE: Destroys contents of buffer */
/* Returns 1 on success, 0 on error, and -1 for unsupported algorithms. */
static int
convert_type(struct nettle_buffer *buffer,
	     enum object_type type,
	     unsigned length, const uint8_t *data)
{
  int res;
  
  switch(type)
    {
    default:
      abort();

    case GENERAL_PUBLIC_KEY:
      res = convert_public_key(buffer, length, data);
      break;

    case RSA_PUBLIC_KEY:
      res = convert_rsa_public_key(buffer, length, data);
      break;

    case RSA_PRIVATE_KEY:
      res = convert_rsa_private_key(buffer, length, data);
      break;
    }

  if (res > 0)
    res = write_file(buffer, stdout);

  return res;
}

static int
convert_file(struct nettle_buffer *buffer,
	     FILE *f,
	     enum object_type type,
	     int base64)
{
  if (type)
    {
      read_file(buffer, f);
      if (base64 && !decode_base64(buffer, 0, &buffer->size))
	return 0;
      
      if (convert_type(buffer, type,
		       buffer->size, buffer->contents) != 1)
	return 0;

      return 1;
    }
  else
    {
      /* PEM processing */
      for (;;)
	{
	  struct pem_info info;
	  const uint8_t *marker;
	  
	  nettle_buffer_reset(buffer);
	  switch (read_pem(buffer, f, &info))
	    {
	    default:
	      return 0;
	    case 1:
	      break;
	    case -1:
	      /* EOF */
	      return 1;
	    }

	  if (!decode_base64(buffer, info.data_start, &info.data_length))
	    return 0;

	  marker = buffer->contents + info.marker_start;

	  type = 0;
	  switch (info.marker_length)
	    {
	    case 10:
	      if (memcmp(marker, "PUBLIC KEY", 10) == 0)
		{
		  type = GENERAL_PUBLIC_KEY;
		  break;
		}
	    case 14:
	      if (memcmp(marker, "RSA PUBLIC KEY", 14) == 0)
		{
		  type = RSA_PUBLIC_KEY;
		  break;
		}

	    case 15:
	      if (memcmp(marker, "RSA PRIVATE KEY", 15) == 0)
		{
		  type = RSA_PRIVATE_KEY;
		  break;
		}
	    }
	  
	  if (!type)
	    werror("Ignoring unsupported object type `%s'.\n", marker);

	  else if (convert_type(buffer, type,
				info.data_length,
				buffer->contents + info.data_start) != 1)
	    return 0;
	}
    }
}


int
main(int argc, char **argv)
{
  struct nettle_buffer buffer;
  enum object_type type = 0;
  int base64 = 0;
  int c;
  
  static const struct option options[] =
    {
      /* Name, args, flag, val */
      { "help", no_argument, NULL, '?' },
      { "version", no_argument, NULL, 'V' },
      { "private-rsa-key", no_argument, NULL, RSA_PRIVATE_KEY },
      { "public-rsa-key", no_argument, NULL, RSA_PUBLIC_KEY },
      { "public-key-info", no_argument, NULL, GENERAL_PUBLIC_KEY },
      { "base-64", no_argument, NULL, 'b' },
      { NULL, 0, NULL, 0 }
    };

  while ( (c = getopt_long(argc, argv, "V?b", options, NULL)) != -1)
    {
      switch (c)
	{
	default:
	  abort();

	case 'b':
	  base64 = 1;
	  break;

	case RSA_PRIVATE_KEY:
	case RSA_PUBLIC_KEY:
	case GENERAL_PUBLIC_KEY:
	  type = c;
	  break;

	case '?':
	  printf("FIXME: Usage information.\n");
	  return EXIT_SUCCESS;

	case 'V':
	  printf("pkcs1-conv (" PACKAGE_STRING ")\n");
	  exit (EXIT_SUCCESS);
	}
    }

  nettle_buffer_init_realloc(&buffer, NULL, nettle_xrealloc);  

  if (optind == argc)
    {
      if (!convert_file(&buffer, stdin, type, base64))
	return EXIT_FAILURE;
    }
  else
    {
      int i;
      const char *mode = (type || base64) ? "r" : "rb";
      
      for (i = optind; i < argc; i++)
	{
	  FILE *f = fopen(argv[i], mode);
	  if (!f)
	    die("Failed to open `%s': %s.\n", argv[i], strerror(errno));

	  if (!convert_file(&buffer, f, type, base64))
	    return EXIT_FAILURE;

	  fclose(f);
	}
    }
  return EXIT_SUCCESS;
}
