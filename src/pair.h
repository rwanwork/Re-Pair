/**************************************************************************
**  Re-Pair / Des-Pair
**  Compressor and decompressor based on recursive pairing.
**
**  Copyright (C) 2003-2022 by Raymond Wan, All rights reserved.
**  Contact:  rwan.work@gmail.com
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


#ifndef PAIR_H
#define PAIR_H

#define TENTPHRASE_SIZE 65521ull /*  Size of Tentative Phrase hash table  */
#define LEFT_HASHCODE 131071ull
#define RIGHT_HASHCODE 262139ull

R_UINT hashCode (R_UINT left, R_UINT right);
R_BOOLEAN isValidPair (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, SEQ_NODE *back, SEQ_NODE *front);
void scanPairs (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);

#endif

/*  End of pair.h  */

