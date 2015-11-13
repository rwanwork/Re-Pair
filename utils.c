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
#include "utils.h"

/* return the binary log of x, 0 if there's an error */
R_UINT floorLog (R_UINT x) {
  R_UINT y = 0;

  while (x != 0) {
    x >>= 1;
    y++;
  }

  if (y == 0) {
    fprintf (stderr, "Unexpected error in %s, line %u.\n", __FILE__, __LINE__);
    exit (EXIT_FAILURE);
  }

  return (y - 1);
}


/*  Return the ceiling binary log of x, 0 if there is an error  */
R_UINT ceilLog (R_UINT x) {
  R_UINT y = 0;
  x--;
  while (x != 0) {
    x >>= 1;
    y++;
  }

  return (y);
}

R_UINT ceilLogULL (R_ULL_INT x) {
  R_UINT y = 0;
  x--;
  while (x != 0) {
    x >>= 1ull;
    y++;
  }

  return (y);
}

