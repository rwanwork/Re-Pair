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


#ifndef PHRASE_SLIDE_DECODE_H
#define PHRASE_SLIDE_DECODE_H

void setUnitPrimitives(BLOCK_INFO *block_struct);
void setUnitPhrasesHorizontal (R_ULL_INT kp, R_ULL_INT kpp, R_ULL_INT kpsqr, R_ULL_INT kppsqr, R_UINT s, PAIR *units);
void setUnitPhrasesChiastic (R_ULL_INT k1, R_ULL_INT k2, R_ULL_INT k1sqr, R_ULL_INT k2sqr, R_UINT s, PAIR *units);

#endif


