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
#include <ctype.h>                                    /*  alnum function  */
#include <errno.h>
#include <limits.h>                                         /*  UINT_MAX  */

#include "common-def.h"
#include "wmalloc.h"
#include "repair-defn.h"
#include "seq.h"
#include "phrase.h"
#include "repair.h"
#include "pair.h"

/*
**  Given 4 nodes representing two pairs, their equality are
**  checked.  Two pairs are equal if their left and right
**  components are equal
*/
static R_BOOLEAN pairEquals (SEQ_NODE *old_back, SEQ_NODE *old_front, SEQ_NODE *back, SEQ_NODE *front) {
  if ((old_back == NULL) || (old_front == NULL)) {
      return (R_FALSE);
  }
  if ((old_back -> value == back -> value) && (old_front -> value == front -> value)) {
      return (R_TRUE);
  }
  return (R_FALSE);
}


/*
**  Given a pair, a hash value is returned.  Code adapted from
**  Algorithms in C (Third edition) by Robert Sedgewick
*/
R_UINT hashCode (R_UINT left, R_UINT right) {
  R_UINT acc;
  acc = (R_UINT) ((((left * LEFT_HASHCODE) % TENTPHRASE_SIZE) + ((right * RIGHT_HASHCODE) % TENTPHRASE_SIZE)) % TENTPHRASE_SIZE);

  return (acc);
}


R_BOOLEAN isValidPair (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, SEQ_NODE *back, SEQ_NODE *front) {
  enum R_PHRASE_TYPE before_back_type = PT_NONE;
  enum R_PHRASE_TYPE back_type = block_struct -> temp_phrases[back -> value].mytype;
  enum R_PHRASE_TYPE front_type = block_struct -> temp_phrases[front -> value].mytype;
  enum R_PHRASE_TYPE after_front_type = PT_NONE;

  enum R_PHRASE_SIDE left_side = block_struct -> temp_phrases[back -> value].myside;
  enum R_PHRASE_SIDE right_side = block_struct -> temp_phrases[front -> value].myside;

  enum R_WORDPOS_TYPE before_back_ptype = WT_NONE;
  enum R_WORDPOS_TYPE back_ptype = back -> punc_type;
  enum R_WORDPOS_TYPE front_ptype = front -> punc_type;

  SEQ_NODE *seqentry = NULL;
  R_UINT temp_value;
  SEQ_NODE *temp_seq = NULL;

  if (block_struct -> temp_phrases[front -> value].length + block_struct -> temp_phrases[back -> value].length > prog_struct -> max_length) {
    return (R_FALSE);
  }

  /*  Punctuation-aligned Re-Pair  */
  if (prog_struct -> word_flags == UW_YES) {
    /*  (W + P) and (W + W) are fine */
    if (back_ptype == WT_WORD) {
      return (R_TRUE);
    }

    /*  (P + W) is not fine  */
    if (front_ptype == WT_WORD) {
      return (R_FALSE);
    }

    /*  Only remaining case is (P + P); need to check the preceeding
    **  one because P (P + P) is fine, but W (P + P) is not.  */

    /*  First two pairs -- no preceeding one to check, so fine.  */
    if (back == block_struct -> seq_buf) {
      return (R_TRUE);
    }
    else {
      seqentry = back;
      temp_seq = PREVSEQ;
      before_back_ptype = temp_seq -> punc_type;
      if (before_back_ptype == WT_PUNC) {
	return (R_TRUE);
      }
      else {
	return (R_FALSE);
      }
    }
  }

  /*  Word-aligned Re-Pair  */
  if (prog_struct -> apply_heuristics == HEUR_WA) {
    if (back_type == front_type) {
      return (R_TRUE);
    }

    if (back == block_struct -> seq_buf) {
      if ((back_type == PT_WORD) && (front_type == PT_NON_WORD)) {
        seqentry = front;
        temp_value = NEXTSEQVALUE;
        after_front_type = block_struct -> temp_phrases[temp_value].mytype;
        if (after_front_type != PT_NON_WORD) {
          return (R_TRUE);
        }
      }
      return (R_FALSE);
    }
    else if (front == block_struct -> seq_buf_end) {
      if ((back_type == PT_WORD) && (front_type == PT_NON_WORD)) {
        seqentry = back;
        temp_value = PREVSEQVALUE;
        before_back_type = block_struct -> temp_phrases[temp_value].mytype;
        if (before_back_type != PT_WORD) {
          return (R_TRUE);
        }
      }
      return (R_FALSE);
    }
    else if ((back_type == PT_WORD) && (front_type == PT_NON_WORD)) {
      seqentry = back;
      temp_value = PREVSEQVALUE;
      before_back_type = block_struct -> temp_phrases[temp_value].mytype;
      seqentry = front;
      temp_value = NEXTSEQVALUE;
      after_front_type = block_struct -> temp_phrases[temp_value].mytype;
      if ((before_back_type != PT_WORD) && (after_front_type != PT_NON_WORD)) {
        return (R_TRUE);
      }
    }
    return (R_FALSE);
  }
  else if (prog_struct -> apply_heuristics == HEUR_SIDE) {
    if ((left_side == SIDE_RIGHT ) || (right_side == SIDE_LEFT)) {
      return (R_FALSE);
    }
    return (R_TRUE);
  }
  else if (prog_struct -> apply_heuristics == HEUR_NORECUR) {
    if ((block_struct -> temp_phrases[back -> value].generation != 0) || (block_struct -> temp_phrases[front -> value].generation != 0)) {
      return (R_FALSE);
    }
    return (R_TRUE);
  }

  return (R_TRUE);
}


/*
**  scanPairs looks at the seq array and checks two characters a time.  Two
**  characters form a pair.  First, if the current pair matches the previous
**  pair, then an overlapping pair has been found, so nothing is done.
**
**  Otherwise, the pair is hashed and looked up in the tentPhrases hash
**  table.  If this is a new pair, then a new tphrase is created with a
**  pointer to the seq_node of the first character.  If this is not a new
**  pair, then the seq_node of the first character is added to the
**  tphrase's position list.
**
**  The insertion function will increment the tphrase's count.  If this count
**  exceeds maxCount, then maxCount takes on this new value.
**
**  Then, the next iteration of the loop is done when a new pair is made
**  by advancing one character
*/
void scanPairs (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT hashcode;
  SEQ_NODE *front;
  SEQ_NODE *back = block_struct -> seq_buf;
  SEQ_NODE *old_front;
  SEQ_NODE *old_back;
  TPHRASE *currentphrase;
  TPHRASE *headphrase = NULL;
  R_BOOLEAN found = R_FALSE;
  SEQ_NODE *dummy;

  /*  seq is of length one, so just return  */
  if (block_struct -> seq_buf == block_struct -> seq_buf_end) {
    return;
  }

                           /*  Create a "dummy" seq_node with a -1 value  */
  dummy = wmalloc (sizeof (SEQ_NODE));
  initSeqNode (SEQ_NODE_DELETED, dummy);
  old_front = dummy;
  old_back = dummy;

  block_struct -> max_count = 0;
  do {
    /*  Front will not go off the end of the array because we 
    **  check if back = end at the end of the do...while loop  */
    front = back + 1;
    if (pairEquals (old_back, old_front, back, front) == R_TRUE) {
      old_back = dummy;
      old_front = dummy;   
                                 /*  Prevent counting "aaa" as two pairs  */
    }
    else {
      /*a
      **  Do not enter the loop if word-Repair is used and it is not a valid
      **  pair.  The following line is derived from the following truth
      **  table.  A = parsing performed; B = valid pair found
      **
      **    A    B    B'    (A && B')    (A && B')'
      **    0    0    1        0            1
      **    0    1    0        0            1
      **    1    0    1        1            0
      **    1    1    0        0            1
      */
      if (isValidPair (prog_struct, block_struct, back, front)) {
        hashcode = hashCode (back -> value, front -> value);
                                       /*  Calculate hashcode for pair  */
        headphrase = block_struct -> tent_phrases[hashcode];
        currentphrase = headphrase;
        if (headphrase != NULL) {
          found = R_FALSE;
          do {
            if ((currentphrase -> left != back -> value) || (currentphrase -> right != front -> value)) {
              currentphrase = currentphrase -> next;
  	    }
            else {
              found = R_TRUE;
              insertSeqPtrLast (back, currentphrase);
    			              /*  Insert back into currentphrase  */
              break;
            }
          } while (currentphrase != headphrase);
        }
        else {
          found = R_FALSE;
        }

        if (found == R_FALSE) {
 	                               /*  New tphrase has to be created  */
          currentphrase = initTPhrase (prog_struct, back -> value, front -> value, back);
          headphrase = insertTPhraseLast (currentphrase, &(block_struct -> tent_phrases[hashcode]));
          currentphrase = headphrase;
          block_struct -> tphrase_in_use += 1;
        }

        if (currentphrase -> count > block_struct -> max_count) {
          (block_struct -> max_count) = currentphrase -> count;
        }                         
                                 /*  Check if count larger than maxCount  */
      }
      old_back = back;
      old_front = front;                     /*  Move to the next pair  */
    }
    back = front;                    /*  Move along seq data structure  */
  } while (back != block_struct -> seq_buf_end);
                                 /*  Continue until end of array reached  */
  wfree (dummy);

  return;
}

