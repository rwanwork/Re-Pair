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
#include <limits.h>

#include "common-def.h"
#include "despair-defn.h"
#include "despair.h"
#include "phrase-slide-decode.h"

void setUnitPrimitives (BLOCK_INFO *block_struct) {
  R_UINT i;

  for (i = 0; i < block_struct -> num_prims; i++) {
    block_struct -> prims_buf[i] = (R_UINT) block_struct -> phrases_array[i].chiastic;
    block_struct -> phrases_array[i].buffer_num = UINT_MAX;
    block_struct -> phrases_array[i].pos = block_struct -> prims_buf + i;
    block_struct -> phrases_array[i].len = 1;
  }

  return;
}

void setUnitPhrasesHorizontal (R_ULL_INT kp, R_ULL_INT kpp, R_ULL_INT kpsqr, R_ULL_INT kppsqr, R_UINT s, PAIR *phrases) {
  R_ULL_INT centre = kpp * (kp - kpp);
  R_ULL_INT lineLen = kp - kpp;
  R_ULL_INT x = 0;
  R_ULL_INT m;
  R_ULL_INT i, j;
  R_UINT l, r;

  for (i = kp, j = 0, m = lineLen; (i < kp + s ) && ((x = phrases[i].chiastic) < centre); ++i) {
    while (x >= (R_ULL_INT) m) {
      m += lineLen;
      j++;
    }
    l = (R_UINT) j;
    r = (R_UINT) (x - (m - lineLen) + kpp);
    phrases[i].left = l;
    phrases[i].right = r;
    phrases[i].buffer_num = 0;
    phrases[i].pos = NULL;
    phrases[i].len = 0;
  }
  lineLen = kp;
  for (j = kpp, m = centre + lineLen; i < s + kp; ++i) {
    x = phrases[i].chiastic;
    while (x >= (R_ULL_INT) m) {
      m += lineLen;
      j++;
    }
    l = (R_UINT) j;
    r = (R_UINT) (x - (m - lineLen));
    phrases[i].left = l;
    phrases[i].right = r;
    phrases[i].buffer_num = 0;
    phrases[i].pos = NULL;
    phrases[i].len = 0;
  }

  return;
}


void setUnitPhrasesChiastic (R_ULL_INT k1, R_ULL_INT k2, R_ULL_INT k1sqr, R_ULL_INT k2sqr, R_UINT s, PAIR *phrases) {
  R_ULL_INT centre = 2 * k2 * (k1 - k2);
  R_ULL_INT lineLen = 2 * (k1 - k2);
  R_ULL_INT m, i, j, h;
  R_ULL_INT x = 0;
  R_UINT l, r;

  for (i = k1, j = 0, m = lineLen; i < k1 + s && (x = phrases[i].chiastic) < centre; ++i) {
    while (x >= m) {
      m += lineLen;
      j++;
    }
    if ((h = x - (m - lineLen)) < (k1 - k2)) {
      l = (R_UINT) j;
      r = (R_UINT) (k1 - h - 1ull);
    } 
    else {
      r = (R_UINT) j;
      l = (R_UINT) (h + k2 + k2 - k1);
    }
    phrases[i].left = l;
    phrases[i].right = r;
    phrases[i].buffer_num = 0;
    phrases[i].pos = NULL;
    phrases[i].len = 0;
  }
  for (j = (k1 - k2 - 1ull), m = k1sqr - k2sqr - j * j; i < s + k1; ++i) {
    x = phrases[i].chiastic;
    while (x >= m) {
      m += j;
      m += --j;
    }
    if ((h = x - (m - j - j - 1ull)) <= j) {
      l = (R_UINT) (k1 - j - 1ull);
      r = (R_UINT) (k1 - h - 1ull);
    } 
    else {
      l = (R_UINT) (k1 + h - j - j - 1ull);
      r = (R_UINT) (k1 - j - 1ull);
    }
    phrases[i].left = l;
    phrases[i].right = r;
    phrases[i].buffer_num = 0;
    phrases[i].pos = NULL;
    phrases[i].len = 0;
  }

  return;
}

