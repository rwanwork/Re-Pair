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


#include <stdio.h>
#include <stdlib.h>

#include "common-def.h"
#include "repair-defn.h"
#include "utils.h"
#include "bitout.h"

static void unaryEncode (FILE *filedesc, R_UINT x, R_UINT lo);
static void unaryEncodeUpperLimit (FILE *filedesc, R_UINT x, R_UINT lo, R_UINT hi);

void writeBits (FILE *filedesc, R_UINT x, R_UINT bits, R_BOOLEAN isflush) {
  R_UINT n_written;
  static R_UINT bitBuffer = 0;
  static R_UINT unusedBits = UINT_SIZE_BITS;
  static R_UINT bufferPos = 0;
  static R_UCHAR buffer[BUFFERTOP];

  if (bits > UINT_SIZE_BITS) {
    fprintf (stderr, "Error:  bits larger than UINT_SIZE_BITS in %s, line %u.", __FILE__, __LINE__);
    exit (EXIT_FAILURE);
  }

  if (isflush == R_FALSE) {
    if (bits <= unusedBits) {          /*  If buffer still has space  */
      if (bits != UINT_SIZE_BITS) {
        bitBuffer = (bitBuffer << bits) | (x & ((1 << bits) - 1));
      }
      else {
        bitBuffer = x;
      }
      unusedBits -= bits;
    }
    else {                              /*  If buffer will overflow  */
      bitBuffer = (bitBuffer << unusedBits) | ((x >> (bits - unusedBits)) & ((1 << (unusedBits)) - 1));
      buffer[bufferPos++] = (R_UCHAR) (bitBuffer >> (UINT_SIZE_BITS - 8));
      buffer[bufferPos++] = (R_UCHAR) (bitBuffer >> (UINT_SIZE_BITS - 16));
      buffer[bufferPos++] = (R_UCHAR) (bitBuffer >> (UINT_SIZE_BITS - 24));
      buffer[bufferPos++] = (R_UCHAR) (bitBuffer);

      if (bufferPos >= BUFFERTOP) {            /*  Write bits out  */
	n_written = (R_UINT) fwrite (buffer, sizeof (buffer[0]), (size_t) bufferPos, filedesc);
        bufferPos = 0;
      }
      if (bits - unusedBits != UINT_SIZE_BITS) {
        bitBuffer = x & ((1 << (bits - unusedBits)) - 1);
      }
      else {
        bitBuffer = x;
      }
      unusedBits += UINT_SIZE_BITS - bits;
    }
  }
  else {                                            /*  Flush buffers  */
    bitBuffer <<= unusedBits;      /*  Pad remaining space with 0's  */
    while (unusedBits < UINT_SIZE_BITS) {
      buffer[bufferPos++] = (R_UCHAR) (bitBuffer >> (UINT_SIZE_BITS - 8));
      bitBuffer <<= 8;
      unusedBits += 8;
    }

    if (bufferPos > 0) {
      n_written = (R_UINT) fwrite (buffer, sizeof (buffer[0]), (size_t) bufferPos, filedesc);
      (void) fflush (filedesc);
    }
    bitBuffer = 0;
    unusedBits = UINT_SIZE_BITS;
    bufferPos = 0;
  }
}


/*
**  Encodes x which is at least 'lo' in unary.
*/
static void unaryEncode (FILE *filedesc, R_UINT x, R_UINT lo) {
  x -= lo;
  while (x >= UINT_SIZE_BITS) {
    writeBits (filedesc, 0, UINT_SIZE_BITS, R_FALSE);
    x -= UINT_SIZE_BITS;
  }
  writeBits (filedesc, 1, x + 1, R_FALSE);
}


/*
**  Encodes x which is at least 'lo' in unary with an
**  upper bound.
*/
static void unaryEncodeUpperLimit (FILE *filedesc, R_UINT x, R_UINT lo, R_UINT hi) {
  R_UINT b = x - lo;

  while (b >= UINT_SIZE_BITS) {
    writeBits (filedesc, 0, UINT_SIZE_BITS, R_FALSE);
    b -= UINT_SIZE_BITS;
  }
  if (x < hi - 1) {
    writeBits (filedesc, 1, b + 1, R_FALSE);
  }
  else {
    writeBits (filedesc, 0, b, R_FALSE);
  }
}


/*
**  Writes the number 'x' as a binary number in the minimum number
**  of bits, where 'x' is guaranteed to be in the range 'lo' ..
**  'hi - 1'.  If 'hi - lo' is not a power of two, shorter codes
**  are used for the lower part of the range rather than the higher
**  one.
*/
void binaryEncode (FILE *filedesc, R_ULL_INT x, R_ULL_INT lo, R_ULL_INT hi) {
  R_UINT logint;
  R_ULL_INT t;
  R_UINT x0 = 0;
  R_UINT x1 = 0;

  hi -= lo;
  x -= lo;
  logint = ceilLogULL (hi + 1ull);
  t = (1ull << (R_ULL_INT) logint) - hi;

  if (x < t) {
    if (logint - 1 > UINT_SIZE_BITS) {
      x0 = (R_UINT) (x >> (R_ULL_INT) UINT_SIZE_BITS);
      x1 = (R_UINT) (x & MASK_LOWER);
      writeBits (filedesc, x0, logint - 1 - UINT_SIZE_BITS, R_FALSE);
      writeBits (filedesc, x1, UINT_SIZE_BITS, R_FALSE);
    }
    else {
      x1 = (R_UINT) (x & MASK_LOWER);
      writeBits (filedesc, x1, logint - 1, R_FALSE);
    }
  }
  else {
    if (logint > UINT_SIZE_BITS) {
      x0 = (R_UINT) ((x + t) >> (R_ULL_INT) UINT_SIZE_BITS);
      x1 = (R_UINT) ((x + t) & MASK_LOWER);
      writeBits (filedesc, x0, logint - UINT_SIZE_BITS, R_FALSE);
      writeBits (filedesc, x1, UINT_SIZE_BITS, R_FALSE);
    }
    else {
      x1 = (R_UINT) ((x + t) & MASK_LOWER);
      writeBits (filedesc, x1, logint, R_FALSE);
    }
  }
}


/*
**  Gamma encode resulting in an exponent in unary and
**  mantissa in binary.
*/
void gammaEncode (FILE *filedesc, R_UINT x, R_UINT lo) {
  R_UINT logx;

  x -= lo - 1;
  logx = floorLog (x);

  unaryEncode (filedesc, logx, 0);
  writeBits (filedesc, x - (1 << logx), logx, R_FALSE);
}


/*
**  Gamma encode with upper bound.
*/
void gammaEncodeUpperLimit (FILE *filedesc, R_UINT x, R_UINT lo, R_UINT hi) {
  R_UINT logx;
  R_UINT loghi;

  x -= lo - 1;
  hi -= lo;
  logx = floorLog (x);
  loghi = floorLog (hi);
  unaryEncodeUpperLimit (filedesc, logx, 0, loghi + 1);
  if (logx < loghi) {
    writeBits (filedesc, x - (1 << logx), logx, R_FALSE);
  }
  else {
    binaryEncode (filedesc, x, (1ull << logx), hi + 1);
  }
}


/*
**  Delta encode with exponent in gamma and mantissa in binary.
*/
void deltaEncode (FILE *filedesc, R_UINT x, R_UINT lo) {
  R_UINT logx;

  x -= lo - 1;
  logx = floorLog (x);
  gammaEncode (filedesc, logx, 0);
  writeBits (filedesc, x - (1 << logx), logx, R_FALSE);
}

