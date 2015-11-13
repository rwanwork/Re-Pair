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
#include <limits.h>

#include "common-def.h"
#include "wmalloc.h"
#include "repair-defn.h"
#include "seq.h"

/*  Creates a new SINGLE_NODE given an integer value to store  */
SINGLE_NODE *initSListNode (R_UINT value) {
  SINGLE_NODE *newnode = NULL;
  newnode = wmalloc (sizeof (SINGLE_NODE));
  newnode -> next = NULL;
  newnode -> value = value;
  return (newnode);
}

/*  Initializes the seq_node given to it by making it point to nothing  */
void initSeqNode (R_UINT value, SEQ_NODE *newnode) {
  R_UINT flag = 0;

  newnode -> prev_ptr = NULL;
  newnode -> next_ptr = NULL;

  flag = value & PUNC_FLAG;
  newnode -> value = value & NO_FLAGS;
  newnode -> punc_type = WT_NONE;
  if (flag == PUNC_FLAG) {
    newnode -> punc_type = WT_PUNC;
  }
  else {
    newnode -> punc_type = WT_WORD;
  }

  return;
}


/*  Deletes a node from a doubly linked list by unlinking it.  */
void deleteSeqNode (SEQ_NODE *deletenode, BLOCK_INFO *block_struct) {
  SEQ_NODE *previous;
  SEQ_NODE *next;
  SEQ_NODE *begin = block_struct -> seq_buf;
  SEQ_NODE *end = block_struct -> seq_buf_end;

  /*
  **  Example:  X1 X2 X3 X4 X5 where X3 is "deletenode" in cases 1 - 4
  **  Case 1:  X3 is the only deleted node
  **  Case 2:  X3 and X4 are both deleted nodes
  **  Case 2b:  X3 and X4 are both deleted nodes and X4 is end
  **  Case 3:  X2 and X3 are deleted nodes.
  **  Case 4:  X2, X3, and X4 are deleted nodes.
  **  Case 4b:  X2, X3, and X4 are deleted nodes and X4 is end;
  **            must not access (begin - 1)
  **  Case 5:  X1 is the first node and is "deletenode" -- not possible
  **  Case 6:  X5 is the last node and is "deletenode"
  */

  /*  Case 5  */
  if (deletenode == begin) {
    fprintf (stderr, "First node is being deleted.  Not possible!.\n");
    exit (EXIT_FAILURE);
  }

  if (deletenode != begin) {
    previous = deletenode - 1;
    if (deletenode == end) {
      next = begin;                                           /*  Case 6  */
    }
    else {
      next = deletenode + 1;                             /*  Cases 1 - 4  */
    }
    if ((next -> value != SEQ_NODE_DELETED) && (previous -> value == SEQ_NODE_DELETED)) {
                                                              /*  Case 3  */
      deletenode -> prev_ptr = previous -> prev_ptr;
      ((previous -> prev_ptr) + 1) -> next_ptr = next;
      deletenode -> next_ptr = NULL;
      previous -> prev_ptr = NULL;
    }
    if ((next -> value != SEQ_NODE_DELETED) && (previous-> value != SEQ_NODE_DELETED)) {
                                                              /*  Case 1  */
      deletenode -> prev_ptr = previous;
      deletenode -> next_ptr = next;
    }
    if ((next -> value == SEQ_NODE_DELETED) && (previous -> value != SEQ_NODE_DELETED)) {
                                                              /*  Case 2  */
      deletenode -> next_ptr = next -> next_ptr;
      if (next -> next_ptr == begin) {
  	                                                     /*  Case 2b  */
        next -> prev_ptr = previous;
      }
      else {
        ((next -> next_ptr) - 1) -> prev_ptr = previous;
      }
      next -> next_ptr = NULL;
      deletenode -> prev_ptr = NULL;
    }
    if ((next -> value == SEQ_NODE_DELETED) && (previous-> value == SEQ_NODE_DELETED)) {
	                                                     /*  Case 4  */
      if (next -> next_ptr == begin) {
  	                                                    /*  Case 4b  */
        next -> prev_ptr = previous -> prev_ptr;
      }
      else {
        ((next -> next_ptr) - 1) -> prev_ptr = previous -> prev_ptr;
      }
      ((previous -> prev_ptr) + 1) -> next_ptr = next -> next_ptr;
      previous-> prev_ptr = NULL;
      next -> next_ptr = NULL;
      deletenode -> prev_ptr = NULL;
      deletenode -> next_ptr = NULL;
    }
  }

  deletenode -> value = SEQ_NODE_DELETED;                  
                                                /*  Mark node as deleted  */
  return;
}


/*  Insert a seq_node into a doubly linked list between two nodes  */
void insertSeqPtr (SEQ_NODE *newnode, SEQ_NODE *prevnode, SEQ_NODE *nextnode) {
  newnode -> prev_ptr = prevnode;
  newnode -> next_ptr = nextnode;
  prevnode -> next_ptr = newnode;
  nextnode -> prev_ptr = newnode;
  return;
}



