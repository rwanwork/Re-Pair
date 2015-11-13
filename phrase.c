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

#include "common-def.h"
#include "wmalloc.h"
#include "repair-defn.h"
#include "seq.h"
#include "pair.h"
#include "phrase.h"

static TPHRASE *insertTPhraseNode (TPHRASE *node, TPHRASE *prevnode, TPHRASE *nextnode);
static TPHRASE *insertTPhraseNodeQueue (TPHRASE *oldnode, TPHRASE *prevnode, TPHRASE *nextnode);

/*
**  Unlinks a seq_node from a doubly linked list.  Also checks if
**  it is the last node for that tphrase.  If so, makes the tphrase's
**  pointer NULL.
*/
void unlinkSeqPtr (SEQ_NODE *deletenode, TPHRASE *tph) {
  if ((tph -> position == deletenode) && ((tph -> position) -> next_ptr == deletenode)) {
    tph -> position = NULL;
  }
  tph -> count -= 1;
  (deletenode -> prev_ptr) -> next_ptr = deletenode -> next_ptr;
  (deletenode -> next_ptr) -> prev_ptr = deletenode -> prev_ptr;
  deletenode -> prev_ptr = NULL;
  deletenode -> next_ptr = NULL;
  return;
}


/*  Inserts a seq_node after the node pointed to by list  */
void insertSeqPtrFirst (SEQ_NODE *newnode, TPHRASE *tph) {
  if ((tph -> position) != NULL) {
    insertSeqPtr (newnode, (tph -> position), (tph -> position) -> next_ptr);
  }
  else {
    insertSeqPtr (newnode, newnode, newnode);
    (tph -> position) = newnode;
  }
  (tph -> count)++;

  return;
}

/*  Inserts a seq_node before the node pointed to by list  */
void insertSeqPtrLast (SEQ_NODE *newnode, TPHRASE *tph) {
  if ((tph -> position) != NULL) {
    insertSeqPtr (newnode, (tph -> position) -> prev_ptr, (tph -> position));
  }
  else {
    insertSeqPtr (newnode, newnode, newnode);
    (tph -> position) = newnode;
  }
  (tph -> count)++;

  return;
}

/*  Given two integers and a seq_node, a tentativephrase is returned  */
TPHRASE *initTPhrase (PROG_INFO *prog_struct, R_UINT left, R_UINT right, SEQ_NODE *ptrnode) {
  TPHRASE *tph = NULL;

  tph = wmalloc (sizeof (TPHRASE));

  tph -> prev = tph;
  tph -> next = tph;
  tph -> prev_queue = tph;
  tph -> next_queue = tph;
  tph -> left = left;
  tph -> right = right;
  tph -> count = 0;
  tph -> position = NULL;
  /*  tph -> position must be NULL for insertSeqPtrLast to work  */

  insertSeqPtrLast (ptrnode, tph);

  return (tph);
}


/*
**  Given a node, inserted it into a list between prevnode and
**  nextnode
*/
static TPHRASE *insertTPhraseNode (TPHRASE *node, TPHRASE *prevnode, TPHRASE *nextnode) {
  node -> prev = prevnode;
  node -> next = nextnode;
  prevnode -> next = node;
  nextnode -> prev = node;
  return (node);
}

/*
**  Deletes a tphrase node from a doubly linked list and deallocat
**  memory
*/
void deleteTPhraseNode (PROG_INFO *prog_struct, TPHRASE *tph) {
  R_UINT i;
  SEQ_NODE *current = tph -> position;
  R_UINT tphrasecount = tph -> count;

  if (tph -> count > prog_struct -> seq_nodelist_size) {
    while (tph -> count > prog_struct -> seq_nodelist_size) {
      prog_struct -> seq_nodelist_size = prog_struct -> seq_nodelist_size << 1;
    }
#ifdef DEBUG
    fprintf (stderr, "Enlarging to %u\n", prog_struct -> seq_nodelist_size);
#endif
    prog_struct -> seq_nodelist = wrealloc (prog_struct -> seq_nodelist, prog_struct -> seq_nodelist_size * sizeof (SEQ_NODE*));
  }

  /*  Unlink node from hash table  */
  (tph -> prev) -> next = tph -> next;
  (tph -> next) -> prev = tph -> prev;
  tph -> prev = NULL;
  tph -> next = NULL;

  /*  Unlink node from queue  */
  (tph -> prev_queue) -> next_queue = tph -> next_queue;
  (tph -> next_queue) -> prev_queue = tph -> prev_queue;
  tph -> prev_queue = NULL;
  tph -> next_queue = NULL;

  /*  Place all of the seq_nodes into an array  */
  for (i = 0; i < tph -> count; i++) {
    prog_struct -> seq_nodelist[i] = current;
    current = current -> next_ptr;
  }

  /*  Integrity check  */
  if (current != tph -> position) {
    fprintf (stderr, "Unexpected error:  current != tph -> position in deleteTPhraseNode in %s, line %u.", __FILE__, __LINE__);
    exit (EXIT_FAILURE);
  }

  /*  Unlink seq_nodes  */
  for (i = 0; i < tphrasecount; i++) {
    unlinkSeqPtr (prog_struct -> seq_nodelist[i], tph);
  }

  /*  Deallocate memory  */
  wfree (tph);

  return;
}


/*  Insert given node before the node pointed to by list  */
TPHRASE *insertTPhraseLast (TPHRASE *node, TPHRASE **list) {
  TPHRASE *newnode;

  if ((*list) != NULL) {
    newnode = insertTPhraseNode (node, (*list) -> prev, (*list));
  }
  else {
    newnode = node;
    (*list) = newnode;
  }

  return (newnode);
}


/*  Detach node from hash table's doubly-linked list  */
TPHRASE *unlinkTPhraseList (TPHRASE *unlinknode, TPHRASE **arr) {
  if ((arr != NULL) && ((*arr) != NULL)) {
    if ((*arr) == unlinknode) {
      if (((*arr) == (*arr) -> prev) && ((*arr) == (*arr) -> next)) {
        (*arr) = NULL;
      }
      else {
        (*arr) = (*arr) -> prev;
      }
    }
  }

  (unlinknode -> prev) -> next = unlinknode -> next;
  (unlinknode -> next) -> prev = unlinknode -> prev;
  unlinknode -> next = unlinknode;
  unlinknode -> prev = unlinknode;
  return (unlinknode);
}


/*  Attach a node to the priority queue between prevnode and nextnode  */
static TPHRASE *insertTPhraseNodeQueue (TPHRASE *oldnode, TPHRASE *prevnode, TPHRASE *nextnode) {

  oldnode -> prev_queue = prevnode;
  oldnode -> next_queue = nextnode;
  prevnode -> next_queue = oldnode;
  nextnode -> prev_queue = oldnode;
  return (oldnode);
}


/*  Insert a node before the node pointed to by list  */
void insertTPhraseLastQueue (TPHRASE *node, TPHRASE **list) {
  TPHRASE *newnode;
  if ((*list) != NULL) {
    newnode = insertTPhraseNodeQueue (node, (*list) -> prev_queue, (*list));
  }
  else {
    newnode = node;
    (*list) = newnode;
  }

  return;
}


/*  Detach a node from the priority queue's linked list  */
TPHRASE *unlinkTPhraseQueue (TPHRASE *unlinknode, TPHRASE **queue) {

  if ((queue != NULL) && ((*queue) != NULL)) {
    if ((*queue) == unlinknode) {
      if (((*queue) == (*queue) -> prev_queue) && ((*queue) == (*queue) -> next_queue)) {
        (*queue) = NULL;
      }
      else {
        (*queue) = (*queue) -> prev_queue;
      }
    }
  }

  (unlinknode -> prev_queue) -> next_queue = unlinknode -> next_queue;
  (unlinknode -> next_queue) -> prev_queue = unlinknode -> prev_queue;
  unlinknode -> next_queue = unlinknode;
  unlinknode -> prev_queue = unlinknode;
  return (unlinknode);
}


/*  Transfer a tphrase's information into a phrase  */
R_UINT transferTPhraseNode (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT pos, SEQ_NODE *firstnode, SEQ_NODE *secondnode, R_UINT generation) {
  PHRASE *ph = &block_struct -> temp_phrases[pos];

  enum R_PHRASE_TYPE first_type;
  enum R_PHRASE_TYPE second_type;

  ph -> left = firstnode -> value;
  ph -> left_chiastic = 0;                           /*  Used later  */
  ph -> right = secondnode -> value;
  ph -> right_chiastic = 0;                          /*  Used later  */
  ph -> unit = 0;                                    /*  Used later  */
  ph -> generation = generation;
  ph -> length = block_struct -> temp_phrases[(firstnode) -> value].length + block_struct -> temp_phrases[(secondnode) -> value].length;
  ph -> temp_index = pos;
  ph -> final_index = 0;                             /*  Used later  */

  if (prog_struct -> apply_heuristics == HEUR_WA) {
    first_type = block_struct -> temp_phrases[ph -> left].mytype;
    second_type = block_struct -> temp_phrases[ph -> right].mytype;

    ph -> mytype = PT_WORD;
    if ((first_type == PT_NON_WORD) && (second_type == PT_NON_WORD)) {
      ph -> mytype = PT_NON_WORD;
    }

    if ((first_type == PT_MIXED) || (second_type == PT_MIXED) || 
        first_type != second_type) {
      ph -> mytype = PT_MIXED;
    }
  }

  if (block_struct -> temp_phrases[ph -> left].myside != SIDE_NONE) {
  } 
  else if (block_struct -> temp_phrases[ph -> right].myside != SIDE_NONE) {
  }
  else {
    block_struct -> temp_phrases[ph -> left].myside = SIDE_LEFT;
    block_struct -> temp_phrases[ph -> right].myside = SIDE_RIGHT;
  }
  block_struct -> temp_phrases[pos].myside = SIDE_NONE;

  return (pos);
}

