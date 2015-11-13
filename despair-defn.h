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


#ifndef DESPAIR_DEFN_H
#define DESPAIR_DEFN_H

/******************************
Forward declaration of important structures defined in other files
******************************/
struct gennode;
struct bitinrec;                                          /*  bitinput.h  */
struct pair;                                               /*  despair.h  */

/******************************
Definitions
******************************/
#define UNINITIALIZED_GENERATION UINT_MAX
  /*  Generic definition used to mark something as "unused" or "invalid"  */
#define MAX_GEN 256
  /*  Maximum generations  */

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
  R_CHAR *base_filename;
  R_UINT base_datatype;

  R_BOOLEAN apply_split;
  R_BOOLEAN verbose_level;
} ARGS_INFO;


typedef struct gennode {
  R_UINT size;
  R_UINT cumm_size;
  R_UINT **bitmaps;
} GENNODE;


typedef struct prog_info {
  R_CHAR *progname;                                     /*  Program name  */

  FILE *out_file;                                        /*  Output file  */
  FILE *seq_file;                                /*  Input sequence file  */
  FILE **seq_file_list;
  FILE *prel_file;                                /*  Input prelude file  */
  R_CHAR *base_filename;                               /*  Base filename  */
  R_UINT base_datatype;

  R_UINT *seq_buf;                                   /*  Sequence buffer  */
  R_UINT *seq_buf_end;
                               /*  Pointer to the end of sequence buffer  */
  R_UINT *seq_buf_p;
              /*  Pointer to the current position in the sequence buffer  */

  R_UINT **seq_buf_list;                             /*  Sequence buffer  */
  R_UINT *seq_buf_p_list;
              /*  Pointer to the current position in the sequence buffer  */
  R_UINT *seq_buf_end_list;
                           /*  Pointer to the end of the sequence buffer  */

  R_UCHAR *out_buf_c;
  R_USHRT *out_buf_s;

  /*
  **  Variables that the user can change at the command line  
  */
  R_BOOLEAN apply_split;
  R_BOOLEAN verbose_level;

  /*
  **  Statistics collected in the Despair process across all blocks
  */
  R_UINT maximum_total_num_phrases;
  R_UINT total_num_prims;
  R_UINT total_num_phrases;
  R_UINT total_num_symbols;
  R_UINT maximum_generations;
  R_UINT maximum_primitives;

  struct bitinrec *bit_in_rec;
  R_UINT total_blocks;

  ARGS_INFO *args_struct;
                     /*  Structure of arguments passed from command line  */
                       /*  Should be NULL if no arguments were passed in  */
} PROG_INFO;


typedef struct block_info {
  struct pair *phrases_array;
  struct gennode *generation_array;
  R_ULL_INT buffer_num;

  R_UINT *prims_buf;
  R_UINT *out_buf;
  R_UINT *out_buf_end;
  R_UINT *out_buf_p;

  /*
  **  Statistics collected in the Despair process for the current block
  */
  R_UINT num_prims;                             /*  Number of primitives  */
  R_UINT num_phrases;   /*  Number of phrases (not including primitives)  */
  R_UINT num_symbols;
  R_UINT num_generation;                          
            /*  Number of generations; includes the primitive generation  */
  R_UINT num_seq_blocks;
                     /*  Number of sequence blocks; should be at least 1  */

  R_UINT total_phrase_length;
  R_UINT max_longest_phrase_length;

} BLOCK_INFO;
    

#endif

