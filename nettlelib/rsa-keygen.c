/* rsa-keygen.c
 *
 * Generation of RSA keypairs
 */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2002 Niels M?ller
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

#include "rsa.h"
#include "bignum.h"

#ifndef DEBUG
# define DEBUG 0
#endif

#if DEBUG
# include <stdio.h>
#endif


/* Returns a random prime of size BITS */
static void
bignum_random_prime(mpz_t x, unsigned bits,
		    void *random_ctx, nettle_random_func random,
		    void *progress_ctx, nettle_progress_func progress)
{
  assert(bits);
  
  for (;;)
    {
      nettle_mpz_random_size(x, random_ctx, random, bits);
      mpz_setbit(x, bits - 1);

      /* Miller-rabin count of 25 is probably much overkill. */
      nettle_next_prime(x, x, 25, 10000, progress_ctx, progress);

      if (mpz_sizeinbase(x, 2) == bits)
	break;
    }
}

int
rsa_generate_keypair(struct rsa_public_key *pub,
		     struct rsa_private_key *key,
		     void *random_ctx, nettle_random_func random,
		     void *progress_ctx, nettle_progress_func progress,
		     unsigned n_size,
		     unsigned e_size)
{
  mpz_t p1;
  mpz_t q1;
  mpz_t phi;
  mpz_t tmp;

  if (e_size)
    {
      /* We should choose e randomly. Is the size reasonable? */
      if ((e_size < 16) || (e_size > n_size) )
	return 0;
    }
  else
    {
      /* We have a fixed e. Check that it makes sense */

      /* It must be odd */
      if (!mpz_tstbit(pub->e, 0))
	return 0;

      /* And 3 or larger */
      if (mpz_cmp_ui(pub->e, 3) < 0)
	return 0;
    }
  
  if (n_size < RSA_MINIMUM_N_BITS)
    return 0;
  
  mpz_init(p1); mpz_init(q1); mpz_init(phi); mpz_init(tmp);

  /* Generate primes */
  for (;;)
    {
      /* Generate p, such that gcd(p-1, e) = 1 */
      for (;;)
	{
	  bignum_random_prime(key->p, (n_size+1)/2,
			      random_ctx, random,
			      progress_ctx, progress);
	  mpz_sub_ui(p1, key->p, 1);
      
	  /* If e was given, we must chose p such that p-1 has no factors in
	   * common with e. */
	  if (e_size)
	    break;
	  
	  mpz_gcd(tmp, pub->e, p1);

	  if (mpz_cmp_ui(tmp, 1) == 0)
	    break;
	  else if (progress!=0) progress(progress_ctx, 'c');
	} 

      if (progress!=0)
	progress(progress_ctx, '\n');
      
      /* Generate q, such that gcd(q-1, e) = 1 */
      for (;;)
	{
	  bignum_random_prime(key->q, n_size/2,
			      random_ctx, random,
			      progress_ctx, progress);
	  mpz_sub_ui(q1, key->q, 1);
      
	  /* If e was given, we must chose q such that q-1 has no factors in
	   * common with e. */
	  if (e_size)
	    break;
	  
	  mpz_gcd(tmp, pub->e, q1);

	  if (mpz_cmp_ui(tmp, 1) == 0)
	    break;
	  else if (progress!=0) progress(progress_ctx, 'c');
	}

      /* Now we have the primes. Is the product of the right size? */
      mpz_mul(pub->n, key->p, key->q);
      
      if (mpz_sizeinbase(pub->n, 2) != n_size)
	/* We might get an n of size n_size-1. Then just try again. */
	{
#if DEBUG
	  fprintf(stderr,
		  "\nWanted size: %d, p-size: %d, q-size: %d, n-size: %d\n",
		  n_size,
		  mpz_sizeinbase(key->p,2),
		  mpz_sizeinbase(key->q,2),
		  mpz_sizeinbase(pub->n,2));
#endif
	  if (progress!=0)
	    {
	      progress(progress_ctx, 'b');
	      progress(progress_ctx, '\n');
	    }
	  continue;
	}
      
      if (progress!=0)
	progress(progress_ctx, '\n');

      /* c = q^{-1} (mod p) */
      if (mpz_invert(key->c, key->q, key->p))
	/* This should succeed everytime. But if it doesn't,
	 * we try again. */
	break;
      else if (progress!=0) progress(progress_ctx, '?');
    }

  mpz_mul(phi, p1, q1);
  
  /* If we didn't have a given e, generate one now. */
  if (e_size)
    {
      int retried = 0;
      for (;;)
	{
	  nettle_mpz_random_size(pub->e,
				 random_ctx, random,
				 e_size);
	
	  /* Make sure it's odd and that the most significant bit is
	   * set */
	  mpz_setbit(pub->e, 0);
	  mpz_setbit(pub->e, e_size - 1);

	  /* Needs gmp-3, or inverse might be negative. */
	  if (mpz_invert(key->d, pub->e, phi))
	    break;

	  if (progress!=0) progress(progress_ctx, 'e');
	  retried = 1;
	}
      if (retried && progress)
	progress(progress_ctx, '\n');	
    }
  else
    {
      /* Must always succeed, as we already that e
       * doesn't have any common factor with p-1 or q-1. */
      int res = mpz_invert(key->d, pub->e, phi);
      assert(res);
    }

  /* Done! Almost, we must compute the auxillary private values. */
  /* a = d % (p-1) */
  mpz_fdiv_r(key->a, key->d, p1);

  /* b = d % (q-1) */
  mpz_fdiv_r(key->b, key->d, q1);

  /* c was computed earlier */

  pub->size = key->size = (unsigned)(mpz_sizeinbase(pub->n, 2) + 7) / 8;
  assert(pub->size >= RSA_MINIMUM_N_OCTETS);
  
  mpz_clear(p1); mpz_clear(q1); mpz_clear(phi); mpz_clear(tmp);

  return 1;
}
