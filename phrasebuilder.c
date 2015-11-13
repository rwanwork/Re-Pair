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
#include <stdlib.h>                                  /*  Used for qsort  */
#include <string.h>
#include <limits.h>

#include "common-def.h"
#include "wmalloc.h"
#include "repair-defn.h"
#include "seq.h"
#include "pair.h"
#include "phrase.h"
#include "phrase-slide-encode.h"
#include "phrasebuilder.h"

static void removeTentativePhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, TPHRASE *deletenode, TPHRASE **queue, TPHRASE **arr);
static void decrCount (SEQ_NODE *seqentry, PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void incrCount (SEQ_NODE *seqentry, PROG_INFO *prog_struct, BLOCK_INFO *block_struct, PAIRED **alreadypaired);

/*
**  Delete a tentative phrase from the priority queue and the
**  hash table.  Performs checks to make sure the tentative
**  phrase is not the first on the list.
*/
static void removeTentativePhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, TPHRASE *deletenode, TPHRASE **queue, TPHRASE **arr) {
  block_struct -> tphrase_in_use -= 1;
  if ((arr != NULL) && ((*arr) != NULL)) {
    if ((*arr) == deletenode) {
      if (((*arr) == (*arr) -> prev) && ((*arr) == (*arr) -> next)) {
	 (*arr) = NULL;  
                          /*  tphrase is last in slot; make slot NULL  */
      }
      else {
        (*arr) = (*arr) -> next;
		   /*  tphrase is first in the slot; move to next one  */
      }
    }
  }

  if ((queue != NULL) && ((*queue) != NULL)) {
    if ((*queue) == deletenode) {
      if (((*queue) == (*queue) -> prev_queue) && ((*queue) == (*queue) -> next_queue)) {
        (*queue) = NULL;
                          /*  tphrase is last in slot; make slot NULL  */
      }
      else {
        (*queue) = (*queue) -> next_queue;
		   /*  tphrase is first in the slot; move to next one  */
      }
    }
  }

  deleteTPhraseNode (prog_struct, deletenode);

  return;
}


/*
**  Initialize the priority queue by going through the hash table
**  looking for non-NULL entries.  If a non-NULL entry is found, then
**  the entire list is processed by first counting the length of the
**  list and then processing everything.
*/
void initQueue (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT i, k;
  TPHRASE *currentphrase;
  TPHRASE *headphrase;
  TPHRASE *deletephrase = NULL;
  R_UINT phrase_count = 0;

  for (i = 0; i < block_struct -> tent_phrases_size; i++) {
    headphrase = block_struct -> tent_phrases[i];
    currentphrase = headphrase;
    if (headphrase != NULL) {
      phrase_count = 0;
      deletephrase = NULL;
      /*  Count the number of phrases  */
      do {
	phrase_count++;
	currentphrase = currentphrase -> next;
      } while (headphrase != currentphrase);

      /*  Process each phrase  */
      currentphrase = headphrase;
      for (k = 0; k < phrase_count; k++) {
        if (currentphrase -> count >= (prog_struct -> max_keep_count)) {
          insertTPhraseLastQueue (currentphrase, &(block_struct -> pqueue[currentphrase -> count]));
	  currentphrase = currentphrase -> next;
        }
	else {
	  deletephrase = currentphrase;
	  currentphrase = currentphrase -> next;
	  removeTentativePhrase (prog_struct, block_struct, deletephrase, NULL, &(block_struct -> tent_phrases[i]));
	}
      }
    }
  }

  return;
}


/*
**  Remove current active pair as a candidate for replacement.
**  Remove its associated tentative phrase as well if the
**  phrase count becomes 0.
*/
static void decrCount (SEQ_NODE *seqentry, PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT hashcode;
  TPHRASE *currentphrase;
  TPHRASE *headphrase;

  if ((seqentry -> prev_ptr != NULL) && (seqentry -> next_ptr != NULL)) {
    hashcode = hashCode (seqentry -> value, NEXTSEQVALUE);
    headphrase = block_struct -> tent_phrases[hashcode];
    currentphrase = headphrase;

    if (headphrase != NULL) {
      do {
        if ((currentphrase -> left != seqentry -> value) || (currentphrase -> right != NEXTSEQVALUE)) {
          currentphrase = currentphrase -> next;
        }
        else {
          /*  Check if this seq_node is the first on the position list  */
          if ((currentphrase -> position) == seqentry) {
            currentphrase -> position = (currentphrase -> position) -> next_ptr;
          }
          (void) unlinkTPhraseQueue (currentphrase, &(block_struct -> pqueue[currentphrase -> count]));
          unlinkSeqPtr (seqentry, currentphrase);

		  /*  Decide if tphrase should be deleted or reinserted  */
          if (currentphrase -> count == 0) {
            removeTentativePhrase (prog_struct, block_struct, currentphrase, NULL, &(block_struct -> tent_phrases[hashcode]));
          }
          else {
            insertTPhraseLastQueue (currentphrase, &(block_struct -> pqueue[currentphrase -> count]));
          }
          return;
        }
      } while (currentphrase != headphrase);
    }
  }

  return;
}


/*
**  Make the current pair into an active pair.  Create an associated
**  tentative phrase and add it to the hash table and priority queue
**  if necessary.
*/
static void incrCount (SEQ_NODE *seqentry, PROG_INFO *prog_struct, BLOCK_INFO *block_struct, PAIRED **alreadypaired) {
  R_UINT hashcode;
  TPHRASE *currentphrase;
  TPHRASE *headphrase;
  PAIRED *headpaired;
  PAIRED *currentpaired;

  if (!isValidPair (prog_struct, block_struct, seqentry, NEXTSEQ)) {
    return;
  }

  hashcode = hashCode (seqentry -> value, NEXTSEQVALUE);

  headpaired = alreadypaired[hashcode];
  currentpaired = headpaired;
  if (currentpaired != NULL) {
    while (currentpaired != NULL) {
      if ((currentpaired -> left == seqentry -> value) && (currentpaired -> right == NEXTSEQVALUE)) {
#ifdef DEBUG
        fprintf (stderr, "Previous pair (%d, %d) found.\n", currentpaired -> left, currentpaired -> right);
#endif
        return;
      }
      currentpaired = currentpaired -> next;
    }
  }

  headphrase = block_struct -> tent_phrases[hashcode];
  currentphrase = headphrase;

  /*  Hash table slot is NULL  */
  if (currentphrase == NULL) {
    currentphrase = initTPhrase (prog_struct, seqentry -> value, NEXTSEQVALUE, seqentry);
    block_struct -> tent_phrases[hashcode] = currentphrase;
    block_struct -> tphrase_in_use += 1;
    currentphrase = block_struct -> tent_phrases[hashcode];
    insertTPhraseLastQueue (currentphrase, &(block_struct -> pqueue[currentphrase -> count]));
    return;
  }

  do {
    if ((currentphrase -> left != seqentry -> value) || (currentphrase -> right != NEXTSEQVALUE)) {
      currentphrase = currentphrase -> next;
    }
    else {
      /*  Check neighbouring seq_nodes to see if they will be replaced */
      if (((currentphrase -> position) -> prev_ptr == NEXTSEQ) || ((currentphrase -> position) -> prev_ptr == PREVSEQ)) {
        return;
      }
      insertSeqPtrLast (seqentry, currentphrase);
	               /*  Insert seqentry into tphrase's position list  */
      currentphrase = unlinkTPhraseQueue (currentphrase, &(block_struct -> pqueue[(currentphrase -> count) - 1]));
      insertTPhraseLastQueue (currentphrase, &(block_struct -> pqueue[currentphrase -> count]));
      return;
      }  /*  else  */
  } while (currentphrase != headphrase);

     /*  Hash table slot is not NULL, but this pair was not found.  So  */
                                             /*  create a new tphrase.  */
  currentphrase = initTPhrase (prog_struct, seqentry -> value, NEXTSEQVALUE, seqentry);
  (void) insertTPhraseLast (currentphrase, &(block_struct -> tent_phrases[hashcode]));
  block_struct -> tphrase_in_use += 1;
  insertTPhraseLastQueue (currentphrase, &(block_struct -> pqueue[currentphrase -> count]));

  return;
}


/*
**  Recursively pair active phrases according to decreasing frequency
**  of tentative phrases.  Continue until no tentative phrase occurs
**  MAX_KEEP_COUNT or more.
*/
void rePairPhrases (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
#ifdef TPHRASE_IN_USE
  R_UINT was_tphrase_in_use = block_struct -> tphrase_in_use;
#endif
  R_UINT replacements = 0;
  TPHRASE *current;
  SEQ_NODE *dlist;
  SEQ_NODE *seqentry;
  R_UINT i;
  TPHRASE *temp_remove;
  R_UINT replacecount;
  R_UINT y = 0;                                /*  replacement  */

  SEQ_NODE *alpha;
  SEQ_NODE *beta;
  SEQ_NODE *gamma;
  R_UINT oldvalue;

  SEQ_NODE **seqentrylist;
  R_UINT seqentrylist_count = (prog_struct -> max_buffer_size >> 1);

  R_UINT tphrasecount;
  R_UINT leftunit, rightunit, generation;

  PAIRED **alreadypaired;
  PAIRED *prevpaired;
  PAIRED *currpaired;
  R_UINT hashvalue;

  enum R_HEURISTICS apply_heuristics = prog_struct -> apply_heuristics;
  enum R_PHRASE_SIDE leftside = SIDE_NONE;
  enum R_PHRASE_SIDE rightside = SIDE_NONE;

#ifdef TPHRASE_IN_USE
  R_INT max_pairs_diff = 0;
  R_INT pairs_diff = 0;
  FILE *numpairs_fp = NULL;
  R_UINT samplerate = 1;

  FOPEN ("numpairs.data", numpairs_fp, "w");
#endif

  alreadypaired = wmalloc (((R_UINT) TENTPHRASE_SIZE) * (sizeof (PAIRED*)));
  for (i = 0; i < (R_UINT) TENTPHRASE_SIZE; i++) {
    alreadypaired[i] = NULL;
  }
  seqentrylist = wmalloc (seqentrylist_count * (sizeof (SEQ_NODE*)));

  block_struct -> num_phrases = 0;

  while ((block_struct -> max_count) >= (prog_struct -> max_keep_count)) {
    /*  Start at the tentative phrases with highest priority  */
    current = block_struct -> pqueue[block_struct -> max_count];
    if (current != NULL) {
      do {
        leftside = block_struct -> temp_phrases[(current -> position) -> value].myside;
        rightside = block_struct -> temp_phrases[CURRENTPOSNEXTSEQ -> value].myside;

        /*  Obtain generation of the two nodes  */
        leftunit = block_struct -> temp_phrases[(current -> position) -> value].generation;
        rightunit = block_struct -> temp_phrases[CURRENTPOSNEXTSEQ -> value].generation;

	if (((apply_heuristics == HEUR_SIDE) && ((leftside == SIDE_RIGHT) || (rightside == SIDE_LEFT))) || ((apply_heuristics == HEUR_NORECUR) && ((leftside == SIDE_RIGHT) || (rightside == SIDE_LEFT)))) {
	}
	else {
        replacements++;
        block_struct -> num_phrases++;

        if ((block_struct -> num_phrases + block_struct -> prims_array_size) > block_struct -> temp_phrases_size) {
          if (block_struct -> temp_phrases_size > (UINT_MAX >> 1)) {
            fprintf (stderr, "ERROR.  Maximum number of phrases of %u exceeded!\n", block_struct -> num_phrases + block_struct -> prims_array_size);
            exit (EXIT_FAILURE);
	  }
          block_struct -> temp_phrases_size = block_struct -> temp_phrases_size << 1;
          block_struct -> temp_phrases = wrealloc (block_struct -> temp_phrases, (block_struct -> temp_phrases_size) * (sizeof (PHRASE)));
        }

        /*  Calculate generation of phrase  */
        if (leftunit > rightunit) {
          generation = leftunit + 1;
        }
        else {
          generation = rightunit + 1;
        }

        /*  Add phrase to array  */
        y = transferTPhraseNode (prog_struct, block_struct, (block_struct -> prims_array_size - 1) + replacements, current -> position, CURRENTPOSNEXTSEQ, generation);

        if (seqentrylist_count < current -> count) {
          while (seqentrylist_count < current -> count) {
            /*  Increase number of SEQ_NODEs  */
            seqentrylist_count = seqentrylist_count << 1;
	  }
          seqentrylist = wrealloc (seqentrylist, seqentrylist_count * (sizeof (SEQ_NODE*)));
        }

        dlist = current -> position;
        for (replacecount = 0; replacecount < current -> count; replacecount++) {
          seqentrylist[replacecount] = dlist;
          dlist = dlist -> next_ptr;
        }

		/*  Get count since recursive pairing will change it  */
        tphrasecount = current -> count;
        for (replacecount = 0; replacecount < tphrasecount; replacecount++) {

 		                   /*  Get the seq_entry to be replaced  */
          seqentry = seqentrylist[replacecount];

	                      /*  Decrement count of neighbouring nodes  */
	  if (seqentry != block_struct -> seq_buf) {
            decrCount (PREVSEQ, prog_struct, block_struct);
  	  }
	  if ((seqentry != block_struct -> seq_buf_end) && (NEXTSEQ != block_struct -> seq_buf)) {
            decrCount (NEXTSEQ, prog_struct, block_struct);
	  }

	  if (seqentry -> punc_type != NEXTSEQ -> punc_type) {
            seqentry -> punc_type = WT_PUNC;
	  }
	  else {
	    /*  No change to punctuation type  */
	  }

		        /*  Delete the second of the two to be replaced  */
          deleteSeqNode (NEXTSEQ, block_struct);
          unlinkSeqPtr (seqentry, current);
          oldvalue = seqentry -> value;
          seqentry -> value = y;                    /*  Replace character  */

		              /*  Increment count of neighbouring nodes  */
          if (seqentry != block_struct -> seq_buf) {
            incrCount (PREVSEQ, prog_struct, block_struct, alreadypaired);
	  }
          if ((seqentry != block_struct -> seq_buf_end) && (NEXTSEQ != block_struct -> seq_buf)) {
            incrCount (seqentry, prog_struct, block_struct, alreadypaired);
	  }

	/*  
	**  Prefer check to handle problem with overlapping active pairs.
	**  Case:
	**  ... [seqentry] [alpha] [beta] [gamma] ...
	**
	**  If these are active pairs:  
        **    ([seqentry], [alpha]); ([beta], [gamma])
	**  and ([alpha], [beta]) should be an active pair but
	**  [seqentry] == [alpha] == [beta] (i.e., overlapping pairs)
	**  but seqentry was just replaced.
	*/
  	  if (seqentry != block_struct -> seq_buf_end) {
            alpha = NEXTSEQ;
            if (alpha != block_struct -> seq_buf_end) {
              beta = ((alpha + 1) -> value == SEQ_NODE_DELETED ? (alpha + 1) -> next_ptr : (alpha + 1));
              if (beta != block_struct -> seq_buf_end) {
  	      /*  If seqentry, alpha, and beta have the same values 
	      **  and alpha is not the left side of a pair  */
                if ((oldvalue == alpha -> value) && (alpha -> value == beta -> value) && (alpha -> next_ptr == NULL)) {
                  gamma = ((beta + 1) -> value == SEQ_NODE_DELETED ? (beta + 1) -> next_ptr : (beta + 1));
                  /*  If gamma is a pair and beta and gamma have different values  */
                  if ((gamma -> next_ptr != NULL) && (beta -> value != gamma -> value)) {
	          /*  Make alpha into a pair  */
	            incrCount (alpha, prog_struct, block_struct, alreadypaired);
		  }
	        }
	      }
	    }
	  }
        }

	/*  Record pair  */
	currpaired = wmalloc (sizeof (PAIRED));
	currpaired -> left = current -> left;
	currpaired -> right = current -> right;
        hashvalue = hashCode (current -> left, current -> right);
        currpaired -> next = alreadypaired[hashvalue];
	alreadypaired[hashvalue] = currpaired;
	}
        current = current -> next_queue;

 		                /*  Remove the current tentative phrase  */
        removeTentativePhrase (prog_struct, block_struct, current -> prev_queue, &(block_struct -> pqueue[block_struct -> max_count]), &(block_struct -> tent_phrases[hashCode ((current -> prev_queue) -> left, (current -> prev_queue) -> right)]));
        if ((block_struct -> pqueue[block_struct -> max_count]) == NULL) {
          current = NULL;
        } 

        block_struct -> seq_buf_len -= block_struct -> max_count;

		/*  Remove all tentative phrases with counts less than */
                /*  maxKeepCount  */
        for (i = 0; i < prog_struct -> max_keep_count; i++) {
	  if (block_struct -> pqueue[i] != NULL) {
	    for (temp_remove = (block_struct -> pqueue[i]) -> next_queue; block_struct -> pqueue[i] != NULL;) {
              temp_remove = temp_remove -> next_queue;
              removeTentativePhrase (prog_struct, block_struct, temp_remove -> prev_queue, &(block_struct -> pqueue[i]), &(block_struct -> tent_phrases[hashCode ((temp_remove -> prev_queue) -> left, (temp_remove -> prev_queue) -> right)]));
	    }
          }
        }
#ifdef TPHRASE_IN_USE
        if ((replacements == 1) || ((replacements % samplerate) == 0)) {
          fprintf (numpairs_fp, "%d %d\n", replacements, block_struct -> tphrase_in_use);
        }
        pairs_diff = (R_INT) (block_struct -> tphrase_in_use) - (R_INT) (was_tphrase_in_use);
        if (pairs_diff > max_pairs_diff) {
          max_pairs_diff = pairs_diff;
	}
	was_tphrase_in_use = block_struct -> tphrase_in_use;
#endif
      } while ((current != NULL) && (block_struct -> num_phrases < (prog_struct -> max_phrases)));
    }
    block_struct -> pqueue[block_struct -> max_count] = NULL;
    block_struct -> max_count -= 1;
  }

#ifdef DEBUG
  fprintf (stderr, "::: total number of replacements:  %d\n", replacements);
#endif

  /*  Clear the alreadypaired hash table  */
  for (i = 0; i < (R_UINT) TENTPHRASE_SIZE; i++) {
    if (alreadypaired[i] != NULL) {
      currpaired = alreadypaired[i];
      prevpaired = currpaired;
      do {
        currpaired = currpaired -> next;
        wfree (prevpaired);
        prevpaired = currpaired;
      } while (currpaired != NULL);
    }
  }
  wfree (alreadypaired);
  wfree (seqentrylist);

#ifdef TPHRASE_IN_USE
  fprintf (stderr, "Maximum difference in pairs under consideration between two replacements:  %d\n", max_pairs_diff);
  FCLOSE (numpairs_fp);
#endif

  return;
}


/*
**  Copies the primitives into phrase structures.  No "sorting" is
**  done since they are in the prims array already sorted.
*/
void sortPrimitives (BLOCK_INFO *block_struct) {
  R_UINT i = 0;
  R_UINT j = 0;

  for (i = 0; i < block_struct -> prims_array_size; i++) {
    if (block_struct -> prims_array[i] == UNINITIALIZED_GENERATION) {
      block_struct -> temp_phrases[i].generation = UNINITIALIZED_GENERATION;
    }
    else {
      block_struct -> temp_phrases[i].final_index = j;   /*  Future index  */
      block_struct -> prims_array[i] = j;
      j++;
    }
  }

  return;
}


/*
**  Copies the phrases from a linked list to an array.  Then, it
**  sorts the array by generation.  Afterwards, for each phrase in
**  each generation, a unit is assigned to it.  A sort on unit is
**  done on that generation and then finally, a final_index is
**  assigned to each phrase.  This final_index is reassigned to all
**  of the seq_nodes in the seq array.
*/
void sortPhrases (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT i, j;
  SINGLE_NODE *endsizeList = block_struct -> sizelist;
  R_ULL_INT kp;
  R_ULL_INT kpp = 0;
  R_ULL_INT kppSqr = 0;
  R_UINT number_of_prims = (block_struct -> sizelist) -> value;
  R_UINT tempnPhrases = block_struct -> num_phrases;
  R_UINT currentsize;
  R_UINT currentgen;
  R_UINT leftarray;
  R_UINT rightarray;
  R_UINT generations = 0;
                               /*  Start at 0, to include the primitives  */
  R_UINT *new_index;

  SEQ_NODE *seqentry;
  FILE *wordlen_fp = NULL;
  R_CHAR *wordlen_fname = NULL;
  R_UINT *wordlen = NULL;
  R_UINT maxwordlen = 0;

  new_index = wmalloc ((block_struct -> prims_array_size + block_struct -> num_phrases) * sizeof (R_UINT));

  /*  Keep track of the final_index for the primitives  */
  for (i = 0; i < block_struct -> prims_array_size; i++) {
    new_index[i] = block_struct -> prims_array[i];
  }

  kp = number_of_prims;
    
  j = number_of_prims;
  for (i = block_struct -> prims_array_size; i < (block_struct -> prims_array_size + block_struct -> num_phrases); i++) {

               /*  If either the left or right value of the phrase is a  */
	                      /*  primitive, then assign the unit value  */
    if (block_struct -> temp_phrases[i].left < block_struct -> prims_array_size) {
      block_struct -> temp_phrases[i].left_chiastic = block_struct -> temp_phrases[block_struct -> temp_phrases[i].left].final_index;
    }
    if (block_struct -> temp_phrases[i].right < block_struct -> prims_array_size) {
      block_struct -> temp_phrases[i].right_chiastic = block_struct -> temp_phrases[block_struct -> temp_phrases[i].right].final_index;
    }

    block_struct -> temp_phrases[i].temp_index = i;
	          /*  Store the phrase's old position for faster lookup  */
  }

    /*  Quicksort all of the phrases using their generations  */
  qsort (&block_struct -> temp_phrases[block_struct -> prims_array_size], (size_t) block_struct -> num_phrases, sizeof (PHRASE), (R_INT (*)(const void *,const void *))genComparison);

    /*  Start from the first phrase  */
  leftarray = rightarray = block_struct -> prims_array_size;

  while (tempnPhrases > 0) {
    currentsize = 0;
    generations++;
    currentgen = block_struct -> temp_phrases[leftarray].generation;
 	                                    /*  Get current generation  */

      /*  Obtain all phrases from current generation  */
    while (block_struct -> temp_phrases[rightarray].generation == currentgen) {
      rightarray++;
      currentsize++;

	/*  Prevent rightarray from going pass the array size  */
      if (rightarray - leftarray == tempnPhrases) {
        break;
      }
    }
        /*  Add size to sizeList for later retrieval  */
    endsizeList -> next = initSListNode (currentsize);
    endsizeList = endsizeList -> next;

        /*  Assign left and right unit to phrases of the current generation  */
                         /*  if the left and right value are not primitives  */
    for (i = 0; i < currentsize; i++) {
      if (block_struct -> temp_phrases[leftarray + i].left >= block_struct -> prims_array_size) {
        block_struct -> temp_phrases[leftarray + i].left_chiastic = new_index[block_struct -> temp_phrases[leftarray + i].left];
      }
      if (block_struct -> temp_phrases[leftarray + i].right >= block_struct -> prims_array_size) {
        block_struct -> temp_phrases[leftarray + i].right_chiastic = new_index[block_struct -> temp_phrases[leftarray + i].right];
      }
    }

    /*  Calculate Horizontal or Chiastic slide on phrases  */
    for (i = 0; i < currentsize; i++) {
      block_struct -> temp_phrases[leftarray + i].unit = chiasticSlide (block_struct -> temp_phrases[leftarray + i].left_chiastic, block_struct -> temp_phrases[leftarray + i].right_chiastic, kp, kpp, kppSqr);
    }

        /*  Quicksort phrases in current generation on unit  */
    qsort (&block_struct -> temp_phrases[leftarray], (size_t) currentsize, sizeof (PHRASE), (R_INT (*)(const void *,const void *))unitComparison);

        /*  Assign final index  */
    for (i = 0; i < currentsize; i++) {
      new_index[block_struct -> temp_phrases[leftarray + i].temp_index] = j;
      block_struct -> temp_phrases[leftarray + i].final_index = j;
      j++;
    }

        /*  Move the left array to the right array  */
    leftarray = rightarray;
    kpp = kp;
    kppSqr = kpp * kpp;
    kp += currentsize;
    tempnPhrases -= currentsize;
  }

  /*  Update seq_nodes with new index values  */
  seqentry = block_struct -> seq_buf;
  do {
    seqentry -> value = new_index[seqentry -> value];
    if (seqentry == block_struct -> seq_buf_end) {
      break;
    }
    seqentry = NEXTSEQ;
  } while ((seqentry <= block_struct -> seq_buf_end) && (seqentry != block_struct -> seq_buf));


  /*
  **  Copy block_struct -> temp_phrases to
  **  block_struct -> sort_phrases, omitting primitives that have
  **  never been seen.
  */
  j = 0;
  for (i = 0; i < (block_struct -> prims_array_size + block_struct -> num_phrases); i++) {
    if (block_struct -> temp_phrases[i].generation != UNINITIALIZED_GENERATION) {
      block_struct -> sort_phrases[j] = block_struct -> temp_phrases[i];
      j++;
    }
  }

  block_struct -> num_generation = generations;

  if (prog_struct -> dowordlen == R_TRUE) {
    wordlen_fname = wmalloc (sizeof (R_CHAR) * (strlen (prog_struct -> base_filename) + 1 + 3));
    strcpy (wordlen_fname, prog_struct -> base_filename);
    strcat (wordlen_fname, ".wl");
    FOPEN (wordlen_fname, wordlen_fp, "w");
    maxwordlen = block_struct -> num_prims + block_struct -> num_phrases;
    (void) fwrite (&maxwordlen, sizeof (R_UINT), 1, wordlen_fp);
    wordlen = wmalloc (sizeof (R_UINT) * maxwordlen);
    for (i = 0; i < block_struct -> num_prims; i++) {
      wordlen[i] = 1;
      (void) fwrite (&wordlen[i], sizeof (R_UINT), 1, wordlen_fp);
    }
    for (; i < maxwordlen; i++) {
      wordlen[i] = wordlen[block_struct -> sort_phrases[i].left_chiastic] + wordlen[block_struct -> sort_phrases[i].right_chiastic];
      (void) fwrite (&wordlen[i], sizeof (R_UINT), 1, wordlen_fp);
    }
    FCLOSE (wordlen_fp);
  }

  wfree (new_index);

  return;
}

