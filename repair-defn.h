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


#ifndef REPAIR_DEFN_H
#define REPAIR_DEFN_H

/******************************
Forward declaration of important structures defined in other files
******************************/
struct single_node;                                            /*  seq.h  */
struct seq_node;                                               /*  seq.h  */
struct phrase;                                              /*  phrase.h  */
struct tphrase;                                             /*  phrase.h  */
struct memroot;                                            /*  smalloc.h  */
struct memindex;                                           /*  smalloc.h  */

/******************************
Redefine common primitive data types
******************************/
#define SIZE_OF_UINT 4

/******************************
Definitions
******************************/
#define MIN_PHRASE_SIZE (1 << 18)
                        /*  Minimum number of phrases is MIN_PHRASE_SIZE  */
#define UNINITIALIZED_GENERATION UINT_MAX
  /*  Generic definition used to mark something as "unused" or "invalid"  */

#define WORD_FLAG 0x00000000              /*  0000 0...  */
#define PUNC_FLAG 0x80000000              /*  1000 0...  */
#define NO_FLAGS 0x7FFFFFFF               /*  Everything but the top bit  */

enum R_PHRASE_TYPE { PT_NONE = 0, PT_WORD = 1, PT_NON_WORD = 2, PT_MIXED = 3 };
enum R_WORDPOS_TYPE { WT_NONE = 0, WT_WORD = 1, WT_PUNC = 2 };

/*  Use word flags?  If yes, remove them when Re-Pair ends, or keep them?  */
enum R_USE_WORDFLAGS { UW_NO = 0, UW_YES = 1 };

#define ISWORD(C)  ((isalnum ((R_INT) C)) || ((R_CHAR) C == '<') || ((R_CHAR) C == '>') || ((R_CHAR) C == '/'))

enum R_HEURISTICS { HEUR_NONE = 0, HEUR_WA = 1, HEUR_SIDE = 2, HEUR_NORECUR = 3 };

/*  Has the current phrase been used as a left phrase or 
**  a right phrase?  */
enum R_PHRASE_SIDE { SIDE_NONE = 0, SIDE_LEFT = 1, SIDE_RIGHT = 2 };

/******************************
Structure definitions
******************************/
/*
**  Structure to be filled in using command line arguments (or
**  directly through other means).  The meaning of each variable is
**  mentioned in the other structure definitions below.
*/
typedef struct args_info {
  R_CHAR *progname;
  FILE *prel_text_file;
  R_CHAR *base_filename;

  R_BOOLEAN verbose_level;
  R_UINT max_buffer_size;
  R_UINT max_length;
  R_UINT max_phrases;
  R_UINT max_keep_count;    
  enum R_HEURISTICS apply_heuristics;
  enum R_USE_WORDFLAGS word_flags;
  R_BOOLEAN add_prims;
  R_UINT max_prims;
  R_UINT base_datatype;
  R_BOOLEAN dowordlen;
} ARGS_INFO;


typedef struct prog_info {
  R_CHAR *progname;                                     /*  Program name  */

  FILE *in_file;                                          /*  Input file  */
  R_UINT in_file_size;                            /*  Size of input file  */
  FILE *seq_file;                               /*  Output sequence file  */
  FILE *prel_file;                               /*  Output prelude file  */
  FILE *prel_text_file;             /*  Output of prelude in text format  */
  FILE *shuff_file;                                       /*  Shuff file  */
  R_CHAR *base_filename;                               /*  Base filename  */

  FILE **seq_file_list;
  R_UINT **seq_buf_list;                             /*  Sequence buffer  */
  R_UINT *seq_buf_p_list;
              /*  Pointer to the current position in the sequence buffer  */

  /*
  **  Variables that the user can change at the command line  
  */
  R_BOOLEAN verbose_level;
  R_UINT max_buffer_size;                 /*  Maximum size of the buffer  */
  R_UINT max_length;                       /*  Maximum length of phrases  */
  R_UINT max_phrases;                      /*  Maximum number of phrases  */
  R_UINT max_keep_count;    
    /*  Perform replacement only if max_keep_count phrases or more exist  */
  enum R_HEURISTICS apply_heuristics;
  enum R_USE_WORDFLAGS word_flags;
  R_BOOLEAN add_prims;
  R_UINT max_prims;
  R_UINT base_datatype;
  R_BOOLEAN dowordlen;

  /*
  **  Statistics collected in the Re-Pairing process across all blocks
  */
  R_UINT maximum_total_num_phrases;
  R_UINT total_num_prims;
  R_UINT total_num_phrases;
  R_UINT total_num_symbols;
  R_UINT maximum_generations;
                                   /*  Need to add 1 since it is 0-based  */
  R_UINT maximum_primitives;
  R_UINT total_blocks;
  R_UINT total_sum_phrase_length;
  R_UINT max_longest_phrase_block;
  R_UINT max_longest_phrase_num;
  R_UINT max_longest_phrase_length;

  /*  Memory management data structures -- remved 2004/11/23 */
  /*  struct memroot *tphrase_root; */

  ARGS_INFO *args_struct;
                     /*  Structure of arguments passed from command line  */
                       /*  Should be NULL if no arguments were passed in  */

  /*  Used by deleteTPhraseNode in phrase.c  */
  R_UINT seq_nodelist_size;
  struct seq_node **seq_nodelist;
} PROG_INFO;


typedef struct block_info {
  R_UINT seq_buf_len;                      /*  Length of sequence buffer  */
  struct seq_node *seq_buf;                          /*  Sequence buffer  */
  struct seq_node *seq_buf_end;     
                               /*  Pointer to the end of sequence buffer  */

  R_UINT input_stack_size;
  struct seq_node *input_stack;
  /*  
  **  An array used to keep track of the end of the current block to be
  **  prepended to the beginning of the next block; required to prevent
  **  the word-parsing from cutting a word in two at the end of a block
  */

  /*
  **  Variables that are required in the Re-Pairing process
  */
  R_UINT *prims_array;   
                 /*  Array which keeps track of the number of primitives  */
  R_UINT prims_array_size;
          /*  Size of primitives array.  Note:  Not necessarily equal to  */
                  /*  the number of primitives due to gaps in the array.  */
  struct tphrase **tent_phrases;
                                              /*  Tentative phrase array  */
  R_UINT tent_phrases_size;
               /*  Size of tentative phrase array -- #define'd elsewhere  */
  struct tphrase **pqueue;
                                                      /*  Priority queue  */
  R_UINT pqueue_size;
                                              /*  Size of priority queue  */
  struct single_node *sizelist;
        /*  Singly linked list which keeps track of the generation sizes  */
  R_UINT tphrase_in_use;
        /*  The number of tentative phrases still under consideration 
	**  for replacement  */
  R_UINT max_count;
                     /*  Current maximum number of replacements required  */
                       /*  Value decreases during the Re-Pairing process  */

  /*
  **  Variables used in the sorting process
  */
  struct phrase *temp_phrases;
  R_UINT temp_phrases_size;
  struct phrase *sort_phrases;

  /*
  **  Statistics collected in the Re-Pairing process for the current block
  */
  R_UINT num_prims;                             /*  Number of primitives  */
  R_UINT num_phrases;   /*  Number of phrases (not including primitives)  */
  R_UINT num_generation;                             /*  Generation size  */
  R_UINT longest_phrase;
  R_UINT longest_phrase_length;
  R_UINT sum_phrase_length;
                          /*  Length of all phrases in the current block  */
  R_UINT num_symbols;
} BLOCK_INFO;
    

#endif

