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


#ifndef BITOUT_H
#define BITOUT_H

#define BUFFERTOP 131072
#define UINT_SIZE_BITS 32

void writeBits (FILE *filedesc, R_UINT x, R_UINT bits, R_BOOLEAN isflush);
void binaryEncode (FILE *filedesc, R_ULL_INT x, R_ULL_INT lo, R_ULL_INT hi);
void gammaEncode (FILE *filedesc, R_UINT x, R_UINT lo);
void gammaEncodeUpperLimit (FILE *filedesc, R_UINT x, R_UINT lo, R_UINT hi);
void deltaEncode (FILE *filedesc, R_UINT x, R_UINT lo);

#endif


