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


#ifndef SEQ_H
#define SEQ_H

#define SEQ_NODE_DELETED UINT_MAX
#define NON_WORD_ARRAY_SIZE 65521
#define BOTH_ARRAY_SIZE 65521

/******************************
Structure definitions
******************************/
typedef struct single_node {
    R_UINT value;
    struct single_node *next;
} SINGLE_NODE;

typedef struct seq_node {
  struct seq_node *prev_ptr;
  struct seq_node *next_ptr;
  R_UINT value;
  enum R_WORDPOS_TYPE punc_type;
} SEQ_NODE;


 /*  Define functions used to get the previous and next sequence values  */
#define NEXTSEQVALUE ((seqentry + 1) -> value == UINT_MAX ? ((seqentry + 1) -> next_ptr) -> value : (seqentry + 1) -> value)
#define PREVSEQVALUE ((seqentry - 1) -> value == UINT_MAX ? ((seqentry - 1) -> prev_ptr) -> value : (seqentry - 1) -> value)

       /*  Define functions used to get the previous and next seq_nodes  */
#define NEXTSEQ ((seqentry + 1) -> value == UINT_MAX ? (seqentry + 1) -> next_ptr : (seqentry + 1))
#define PREVSEQ ((seqentry - 1) -> value == UINT_MAX ? (seqentry - 1) -> prev_ptr : (seqentry - 1))
#define CURRENTPOSNEXTSEQ (((current -> position) + 1) -> value == UINT_MAX ? ((current -> position) + 1) -> next_ptr : ((current -> position) + 1))

/*  Function for initializing a singly linked list node  */
SINGLE_NODE *initSListNode (R_UINT value);

/*  Functions for manipulating sequence nodes  */
void initSeqNode (R_UINT value, SEQ_NODE* newnode);
void deleteSeqNode (SEQ_NODE *deletenode, BLOCK_INFO *block_struct);

/*  Functions for manipulating the pointers of sequence nodes  */
void insertSeqPtr (SEQ_NODE *newnode, SEQ_NODE *prevnode, SEQ_NODE *nextnode);

#endif


