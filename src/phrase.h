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


#ifndef PHRASE_H
#define PHRASE_H


/******************************
Structure definitions
******************************/
typedef struct tphrase {
  struct tphrase *prev;                   /*  Pointers for hash table  */
  struct tphrase *next;
  struct tphrase *prev_queue;         /*  Pointers for priority queue  */
  struct tphrase *next_queue;
  struct seq_node *position;               /*  Pointers for seq_nodes  */
  R_UINT left;
  R_UINT right;
  R_UINT count;                               /*  Number of seq_nodes  */
  /*  R_UINT myname;*/
} TPHRASE;

typedef struct phrase {
                /*  Non-normalized value of the left child (non-final)  */
  R_UINT left;  
  R_UINT right;

  /*  Initially, generation of the left child.  Then, in the sorting
  **  phase, used as the final value of the left child.  */
  R_UINT left_chiastic;
  R_UINT right_chiastic;

  R_ULL_INT unit;                                      /*  Unit number  */
  R_UINT generation;                           /*  Phrase's generation  */
  R_UINT length;                                   /*  Phrase's length  */
  R_UINT temp_index;                         /*  Temporary index value  */
  R_UINT final_index;                            /*  Final index value  */
  enum R_PHRASE_TYPE mytype;
  enum R_PHRASE_SIDE myside;
} PHRASE;


/*  Functions for manipulating tphrases with seq nodes  */
void unlinkSeqPtr (SEQ_NODE *deletenode, TPHRASE *tph);
void insertSeqPtrFirst (SEQ_NODE *newnode, TPHRASE *tph);
void insertSeqPtrLast (SEQ_NODE *newnode, TPHRASE *tph);

TPHRASE *initTPhrase (PROG_INFO *prog_struct, R_UINT left, R_UINT right, SEQ_NODE *ptrnode);

/*  Functions for manipulating tphrases with hash table linked list  */
void deleteTPhraseNode (PROG_INFO *prog_struct, TPHRASE *deletenode);
TPHRASE *insertTPhraseLast (TPHRASE *node, TPHRASE **list);
TPHRASE *unlinkTPhraseList (TPHRASE *unlinknode, TPHRASE **arr);

/*  Functions for manipulating tphrases with queues  */
void insertTPhraseLastQueue (TPHRASE *node, TPHRASE **list);
TPHRASE *unlinkTPhraseQueue (TPHRASE *unlinknode, TPHRASE **queue);
R_UINT transferTPhraseNode (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT pos, SEQ_NODE *firstnode, SEQ_NODE *secondnode, R_UINT generation);

#endif

/*  End of phrase.h  */
