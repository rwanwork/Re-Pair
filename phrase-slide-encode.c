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

#include "common-def.h"
#include "repair-defn.h"
#include "seq.h"
#include "phrase.h"
#include "phrase-slide-encode.h"

/*  Calculate the horizontal slide for the given left and right values  */
R_ULL_INT horizontalSlide (R_ULL_INT left, R_ULL_INT right, R_ULL_INT kp, R_ULL_INT kpp, R_ULL_INT kppsqr) {
  R_ULL_INT result;

  if (left < kpp) {
    result = (left * (kp - kpp) + right - kpp);
  }
  else {
    result = (left * kp + right - kppsqr);
  }

  return (result);
}


/*  Calculate the chiastic slide for the given left and right values  */
R_ULL_INT chiasticSlide (R_ULL_INT left, R_ULL_INT right, R_ULL_INT kp, R_ULL_INT kpp, R_ULL_INT kppsqr) {
  R_ULL_INT result;

  if (left < kpp) {
    result = ((left + left) * (kp - kpp) + kp - 1ull - right);
  }
  else {
    if (right < kpp) {
      result = ((1ull + right + right) * (kp - kpp) + left - kpp);
    }
    else {
      if (left <= right) {
        result = (left * (kp + kp - left) + kp - 1ull - right - kppsqr);
      }
      else {
        result = (right * (kp + kp - right - 2ull) + kp - 1ull + left - kppsqr);
      }
    }
 }

 return (result);
}


/*
**  Comparison function used by C library qsort function to compare
**  two phrases using their generations
*/
R_INT genComparison (const PHRASE *first, const PHRASE *second) {
    return ((R_INT) (first -> generation) - (R_INT) (second -> generation));
}


/*
**  Comparison function used by C library qsort function to compare
**  two phrases using their units
*/
R_INT unitComparison (const PHRASE *first, const PHRASE *second) {
  if (first -> unit > second -> unit) {
    return (1);
  }
  else {
    if (first -> unit < second -> unit) {
      return (-1);
    }
    else {
      return (0);
    }
  }
}

