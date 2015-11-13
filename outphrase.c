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
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "common-def.h"
#include "despair-defn.h"
#include "despair.h"
#include "outphrase.h"

#ifdef FAVOUR_TIME_EXPAND
void outPhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT i) {
  R_UINT *pos;
  R_UINT n;
  R_UINT bytes_to_copy = 0;

  /*  Expand from buffer  */
  pos = block_struct -> phrases_array[i].pos;
  n = block_struct -> phrases_array[i].len;
  bytes_to_copy = block_struct -> out_buf_end - block_struct -> out_buf_p;
  while (n > bytes_to_copy) {
    memcpy (block_struct -> out_buf_p, pos, sizeof (R_UINT) * bytes_to_copy);
    writeOutputFile (prog_struct, block_struct, OUT_BUF_SIZE);
    n = n - bytes_to_copy;
    pos = pos + bytes_to_copy;
    block_struct -> out_buf_p = block_struct -> out_buf;
    block_struct -> buffer_num++;
    bytes_to_copy = block_struct -> out_buf_end - block_struct -> out_buf_p;
  }
  memcpy (block_struct -> out_buf_p, pos, (sizeof (R_UINT) * n));
  block_struct -> out_buf_p += n;

  return;
}
#endif


#ifdef FAVOUR_MEMORY_EXPAND
void outPhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT i) {
  R_UINT *pos;
  R_UINT n;
  R_UINT bytes_to_copy = 0;

  /*  Expand from buffer  */
  if (block_struct -> phrases_array[i].buffer_num == UINT_MAX) {
    pos = block_struct -> phrases_array[i].pos;
    n = block_struct -> phrases_array[i].len;
    bytes_to_copy = block_struct -> out_buf_end - block_struct -> out_buf_p;
    while (n > bytes_to_copy) {
      memcpy (block_struct -> out_buf_p, pos, sizeof (R_UINT) * bytes_to_copy);
      writeOutputFile (prog_struct, block_struct, OUT_BUF_SIZE);
      n = n - bytes_to_copy;
      pos = pos + bytes_to_copy;
      block_struct -> out_buf_p = block_struct -> out_buf;
      block_struct -> buffer_num++;
      bytes_to_copy = block_struct -> out_buf_end - block_struct -> out_buf_p;
    }
    memcpy (block_struct -> out_buf_p, pos, (sizeof (R_UINT) * n));
    block_struct -> out_buf_p += n;
  }
  /*  Not found in buffer; recursively decode  */
  else {
    block_struct -> phrases_array[i].pos = block_struct -> out_buf_p;
    outPhrase (prog_struct, block_struct, block_struct -> phrases_array[i].left);
    outPhrase (prog_struct, block_struct, block_struct -> phrases_array[i].right);
    /*  Calculate length of phrase  */
    if (block_struct -> phrases_array[i].len == 0) {
      block_struct -> phrases_array[i].len = block_struct -> phrases_array[block_struct -> phrases_array[i].left].len + block_struct -> phrases_array[block_struct -> phrases_array[i].right].len;
    }
    if ((block_struct -> out_buf_end - block_struct -> phrases_array[i].pos) >= block_struct -> phrases_array[i].len) {
      block_struct -> phrases_array[i].buffer_num = block_struct -> buffer_num;
    }
  }

  return;
}
#endif



#ifdef NORMAL_EXPAND
void outPhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT i) {
  R_UINT *pos;
  R_UINT n;
  R_UINT bytes_to_copy = 0;

  /*  Expand from buffer  */
  if (block_struct -> phrases_array[i].buffer_num >= block_struct -> buffer_num) {
    pos = block_struct -> phrases_array[i].pos;
    n = block_struct -> phrases_array[i].len;
    bytes_to_copy = block_struct -> out_buf_end - block_struct -> out_buf_p;
    while (n > bytes_to_copy) {
      memcpy (block_struct -> out_buf_p, pos, sizeof (R_UINT) * bytes_to_copy);
      writeOutputFile (prog_struct, block_struct, OUT_BUF_SIZE);
      n = n - bytes_to_copy;
      pos = pos + bytes_to_copy;
      block_struct -> out_buf_p = block_struct -> out_buf;
      block_struct -> buffer_num++;
      bytes_to_copy = block_struct -> out_buf_end - block_struct -> out_buf_p;
    }
    memcpy (block_struct -> out_buf_p, pos, (sizeof (R_UINT) * n));
    block_struct -> out_buf_p += n;
  }
  /*  Not found in buffer; recursively decode  */
  else {
    block_struct -> phrases_array[i].pos = block_struct -> out_buf_p;
    outPhrase (prog_struct, block_struct, block_struct -> phrases_array[i].left);
    outPhrase (prog_struct, block_struct, block_struct -> phrases_array[i].right);
    /*  Calculate length of phrase  */
    if (block_struct -> phrases_array[i].len == 0) {
      block_struct -> phrases_array[i].len = block_struct -> phrases_array[block_struct -> phrases_array[i].left].len + block_struct -> phrases_array[block_struct -> phrases_array[i].right].len;
    }
    if ((block_struct -> out_buf_end - block_struct -> phrases_array[i].pos) >= block_struct -> phrases_array[i].len) {
      block_struct -> phrases_array[i].buffer_num = block_struct -> buffer_num;
    }
  }

  return;
}
#endif

