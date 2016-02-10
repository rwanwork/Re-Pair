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


#ifndef BITIN_H
#define BITIN_H

#define BITINREC_BUF_SIZE 32768 /* (1 << 15) */

typedef struct bitinrec {
  R_UINT bitBuffer;
  R_UINT availableBits;
  FILE *in;
  R_UCHAR *bufferPos;
  R_UCHAR *bufferTop;
  R_UCHAR *buffer;
} BITINREC;

BITINREC *newBitin (FILE *in);
R_UINT ceilLog (R_UINT x);
R_UINT ceilLogULL (R_ULL_INT x);
R_UINT boundedUnarydecode (R_UINT lo, R_UINT hi, BITINREC *r);
R_ULL_INT binaryDecode (R_ULL_INT lo, R_ULL_INT hi, BITINREC *r);
R_UINT gammaDecode (R_UINT lo, BITINREC *r);
R_UINT deltaDecode (R_UINT lo, BITINREC *r);

#endif
