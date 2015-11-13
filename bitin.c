/**************************************************************************
**  Re-Pair / Des-Pair
**  Compressor and decompressor based on recursive pairing.
**  
**  Version N/A (On Github) -- November 13, 2015
**  
**  Copyright (C) 2003, 2007, 2015 by Raymond Wan, All rights reserved.
**  Contact:  rwan.work@gmail.com
**  Organization:  Division of Life Science, Faculty of Science, Hong Kong
**                 University of Science and Technology, Hong Kong
**  
**  This file is part of Re-Pair / Des-Pair.
**  
**  Re-Pair / Des-Pair is free software; you can redistribute it and/or 
**  modify it under the terms of the GNU General Public License 
**  as published by the Free Software Foundation; either version 
**  3 of the License, or (at your option) any later version.
**  
**  Re-Pair / Des-Pair is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**  
**  You should have received a copy of the GNU General Public 
**  License along with Re-Pair / Des-Pair; if not, see 
**  <http://www.gnu.org/licenses/>.
**************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "common-def.h"
#include "wmalloc.h"
#include "utils.h"
#include "bitin.h"

#define UINT_SIZE_BITS 32

static R_UINT readBits (R_UINT bits, BITINREC *r);
static R_UINT unaryDecode (R_UINT lo, BITINREC *r);

BITINREC *newBitin (FILE *in) {
  BITINREC *bitrec;
  bitrec = wmalloc (sizeof (BITINREC));

  bitrec -> in = in;
  bitrec -> availableBits = 0;

  bitrec -> buffer = wmalloc (sizeof (R_UCHAR) * BITINREC_BUF_SIZE);
  bitrec -> bufferPos = bitrec -> buffer;
  bitrec -> bufferTop = bitrec -> buffer;
  bitrec -> bitBuffer = 0;

  return (bitrec);
}


static R_UINT readNext (R_UINT minBits, BITINREC *r) {
  R_UCHAR *b;
  R_UINT n;
  R_UINT newBits;

  if (r -> bufferPos == r -> bufferTop) {
    n = (R_UINT) fread (r -> buffer, 1, sizeof (r -> buffer), r -> in);
    if (ferror (r -> in) != R_FALSE) {
      perror (__FILE__);
      exit (EXIT_FAILURE);
    }
    if (n < 1) {
      fprintf (stderr, "ERROR:  Unexpected end of file in %s on line %d.\n", __FILE__, __LINE__);
      exit (EXIT_FAILURE);
    }
    r -> bufferPos = r -> buffer;
    r -> bufferTop = r -> bufferPos + n;
  }
  b = r -> bufferPos;
  r -> bufferPos = b + 4;

  r -> bitBuffer = (R_UINT) b[0]<<24|
    (R_UINT) b[1]<<16|
    (R_UINT) b[2]<<8|
    (R_UINT) b[3];

  if (r -> bufferPos > r -> bufferTop) {
    newBits = (R_UINT) (32 - ((r -> bufferPos - r -> bufferTop) * 8));
    if (newBits < minBits) {
      fprintf (stderr, "ERROR:  Unexpected end of file in %s on line %d.\n", __FILE__, __LINE__);
      exit (EXIT_FAILURE);
    }
    r -> bufferTop = r -> bufferPos;
    return (newBits);
  } 
  else {
    return (32u);
  }
}

static R_UINT readBits (R_UINT bits, BITINREC *r) {
  R_UINT x = 0;

  if (bits > 32) {
    fprintf (stderr, "Unexpected error -- too many bits (%s, %u).\n", __FILE__, __LINE__);
    exit (EXIT_FAILURE);
  }

  if (bits == 0) {
    return (0);
  }
  if (r -> availableBits == 0) {
    x = 0;
  }
  else {
    x = (r -> bitBuffer) >> (32u - bits);
  }
  if (bits <= r -> availableBits) {
    r -> bitBuffer <<= bits;
    r -> availableBits -= bits;
  }
  else {
    bits -= r -> availableBits;
    r -> availableBits = readNext (bits, r)-bits;
    x |= (r -> bitBuffer) >> (32u - bits);
    r -> bitBuffer <<= bits;
  }

  return (x);
}


static R_UINT unaryDecode (R_UINT lo, BITINREC *r) {
  R_UINT x;

  while (r -> bitBuffer == 0) {
    lo += r -> availableBits;
    r -> availableBits = readNext (1, r);
  }
  x = 0;
  while ((r -> bitBuffer & MASK_HIGHEST) != MASK_HIGHEST) {
    r -> bitBuffer <<= 1;
    x++;
  }
  r -> bitBuffer <<= 1;
  r -> availableBits -= x + 1;

  return (x + lo);
}


R_UINT boundedUnarydecode (R_UINT lo, R_UINT hi, BITINREC *r) {
  R_UINT x;

  hi -= lo + 1;
  while (r -> bitBuffer == 0) {
    if (r -> availableBits >= hi) {
      r -> availableBits -= hi;
      r -> bitBuffer <<= hi;
      return (lo + hi);
    }
    lo += r -> availableBits;
    hi -= r -> availableBits;
    r -> availableBits = readNext (1, r);
  }
  x = 0;
  while (x < hi) {
    if (r -> bitBuffer == UINT_MAX) {
      r -> bitBuffer <<= 1;
      --r -> availableBits;
      break;
    }
    r -> bitBuffer <<= 1;
    x++;
  }
  r -> availableBits -= x;

  return (x + lo);
}

R_ULL_INT binaryDecode(R_ULL_INT lo, R_ULL_INT hi, BITINREC *r) {
  R_ULL_INT x = 0ull;
  R_ULL_INT t = 0ull;
  R_UINT logint;
  R_UINT x0 = 0;
  R_UINT x1 = 0;

  if ((hi-=lo)==1ull) {
    return (lo);
  }
  logint = ceilLogULL (hi + 1ull);
  t = (1ull << (R_ULL_INT) logint) - hi;
   
  if (logint - 1 > UINT_SIZE_BITS) {
    x0 = readBits (logint - 1 - UINT_SIZE_BITS, r);
    x1 = readBits (UINT_SIZE_BITS, r);
    x = (R_ULL_INT) x0 << (R_ULL_INT) UINT_SIZE_BITS;
    x = x | x1;
  }
  else {
    x1 = readBits (logint - 1, r);
    x = (R_ULL_INT) x1 & (R_ULL_INT) MASK_LOWER;
  }

  if (x < t) {
    x += lo;
  }
  else {
    x = (x << 1ull) + (R_ULL_INT) readBits (1, r);
    x = x - t + lo;
  }

  return (x);
}

R_UINT gammaDecode (R_UINT lo, BITINREC *r) {
  R_UINT l = unaryDecode (0, r);
  R_UINT temp;
  R_UINT temp2;

  temp2 = readBits (l, r);
  temp = lo + (1 << l) - 1 + temp2;

  return (temp);
}

R_UINT deltaDecode (R_UINT lo, BITINREC *r) {
  R_UINT l = gammaDecode (0, r);

  return (lo + (1 << l) - 1 + readBits (l, r));
}

