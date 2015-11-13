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
#include <limits.h>                                         /*  UINT_MAX  */

#include "common-def.h"
#include "wmalloc.h"
#include "repair-defn.h"
#include "bitout.h"
#include "seq.h"
#include "phrase.h"
#include "utils.h"
#include "writeout.h"

static void intEncodeHierarchy (FILE *filedesc, R_UINT a, R_UINT b, R_ULL_INT lo, R_ULL_INT hi, struct phrase final_sorted_phrases[]);


/*
**  Writes out the sequence file an integer at a time from the array
**  of seq_entrys.  All integers are incremented by 1 since a 0
**  indicates the end of a block
*/
void encodeSequence_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT x;
  R_UCHAR buf[SIZE_OF_UINT];
  SEQ_NODE *seqentry = block_struct -> seq_buf;
  R_UINT seq_length = 0;

  do {
    if (seqentry -> value != SEQ_NODE_DELETED) {
      /*  Must increment all by 1 to allow 0 to be an end 
      **  of buffer marker  */
      x = seqentry -> value + 1;
      if (prog_struct -> word_flags == UW_YES) {
	if (seqentry -> punc_type == WT_PUNC) {
	  x = x | PUNC_FLAG;
	}
      }
      buf[0] = (R_UCHAR) (x >> 0) & 255;
      buf[1] = (R_UCHAR) (x >> 8) & 255;
      buf[2] = (R_UCHAR) (x >> 16) & 255;
      buf[3] = (R_UCHAR) (x >> 24);
      (void) fwrite (buf, SIZE_OF_UINT, 1, prog_struct -> seq_file);  
                                                   /*  Write out buffer  */
      seq_length++;
      if (seqentry == block_struct -> seq_buf_end) {
        break;
      }
      seqentry = NEXTSEQ;                       /*  Advance to next one  */
    }
  } while ((seqentry <= block_struct -> seq_buf_end)
    && (seqentry != block_struct -> seq_buf));
  /*
  **  Check if back to head of circular doubly linked list
  **  Need to be careful that the end is checked properly; especially
  **  the cases when the end was never replaced.  If so, (end + 1)
  **  and (end -> next_seq) lead to garbage values.
  */

  /*  Write a 0 out to indicate end of buffer  */
  buf[0] = (R_UCHAR) 0;               
  buf[1] = (R_UCHAR) 0;
  buf[2] = (R_UCHAR) 0;
  buf[3] = (R_UCHAR) 0;
  (void) fwrite (buf, SIZE_OF_UINT, 1, prog_struct -> seq_file);
  seq_length++;

  block_struct -> num_symbols = seq_length;

  return;
}

/*
**  Encodes a subset of an array of phrases where all of the
**  elements in the subset have the same generation
*/
static void intEncodeHierarchy (FILE *filedesc, R_UINT a, R_UINT b, R_ULL_INT lo, R_ULL_INT hi, PHRASE final_sorted_phrases[]) {
  R_ULL_INT mid = 0;
  R_UINT halfway = 0;
  R_UINT range = 0;

  range = b - a;

  switch (range) {
    case 0:  return;
    case 1:  binaryEncode (filedesc, final_sorted_phrases[a].unit, lo, hi);
             return;
  }
  halfway = range >> 1;
                       /*  Divide range by 2, getting the halfway point  */

  mid = final_sorted_phrases[a + halfway].unit;
                               /*  Obtain the unit of the halfway point  */

  binaryEncode (filedesc, mid, lo + (R_ULL_INT) halfway, hi - (R_ULL_INT) (range - halfway - 1));
                                        /*  Binary encode the mid-point  */
  intEncodeHierarchy (filedesc, a, a + halfway, lo, mid, final_sorted_phrases);
                                   /*  Recursively encode the left half  */
  intEncodeHierarchy (filedesc, a + halfway + 1, b, mid + 1ull, hi, final_sorted_phrases);
                                  /*  Recursively encode the right half  */

  return;
}


/*  Encode one block of the phrase hierarchy  */
void encodeHierarchy_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_ULL_INT kpp = 0;
  R_ULL_INT kppSqr = 0;
  R_ULL_INT kp = 0;
  R_ULL_INT kpSqr = 0;
  R_UINT max;
  R_UINT logRange;
  R_UINT topRange;
  R_UINT currentsize;
  SINGLE_NODE *sizes = block_struct -> sizelist;
  SINGLE_NODE *back_sizes = NULL;
  R_UINT currgen = 0;

  deltaEncode (prog_struct -> prel_file, block_struct -> num_prims + block_struct -> num_phrases, 0);
            /*  Delta encode the total number of primitives and phrases  */

  deltaEncode (prog_struct -> prel_file, block_struct -> num_prims, 1);
                        /*  Delta encode the total number of primitives  */

  max = block_struct -> sort_phrases[block_struct -> num_prims - 1].left;
                  /*  Get the maximum ASCII value of all the primitives  */
  logRange = (R_UINT) ceilLog (max + 1);
  topRange = 1u << logRange;
  gammaEncode (prog_struct -> prel_file, logRange, 0);
                    /*  Calculate and gamma encode the log of the range  */

  intEncodeHierarchy (prog_struct -> prel_file, 0, block_struct -> num_prims, 0, topRange, block_struct -> sort_phrases);
  currgen++;
                                              /*  Encode the primitives  */

  kp = block_struct -> num_prims;
  sizes = sizes -> next;  
                    /*  Get the node of the first generation of phrases  */
  back_sizes = sizes;
  while (kp < (block_struct -> num_prims + block_struct -> num_phrases)) {
    currentsize = sizes -> value;

#ifdef DEBUG
    fprintf (stderr, "Generation %d:  %d\n", currgen, currentsize);
#endif

    currgen++;
    gammaEncode (prog_struct -> prel_file, currentsize, 1);
                            /*  Gamma encode the size of the generation  */
    kpSqr = kp * kp;

    intEncodeHierarchy (prog_struct -> prel_file, (R_UINT) kp, (R_UINT) kp + currentsize, 0ull, kpSqr - kppSqr, block_struct -> sort_phrases);
 	                           /*  Encode the generation of phrases  */
    kpp = kp;
    kppSqr = kpSqr;
    kp += currentsize;
    back_sizes = sizes;
    sizes = sizes -> next;
    wfree (back_sizes);
  }

  /*  Remove the node with the size of primitives  */
  wfree (block_struct -> sizelist);
  block_struct -> sizelist = NULL;

  if (kp > (block_struct -> num_prims + block_struct -> num_phrases)) {
    fprintf (stderr, "Wrong generation sizes in %s, line %u.", __FILE__, __LINE__);
    exit (EXIT_FAILURE);
  }

  return;
}


