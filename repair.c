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


#include <stdlib.h>
#include <stdio.h>
#include <limits.h>                                         /*  UINT_MAX  */
#include <string.h>
#include <getopt.h>                                  /*  getopt function  */
#include <errno.h>
#include <math.h>                                      /*  ceil function  */
#include <ctype.h>                                  /*  isalnum function  */
#include <sys/stat.h>

#include "common-def.h"
#include "wmalloc.h"
#include "repair-defn.h"
#include "seq.h"
#include "phrase.h"
#include "pair.h"
#include "phrasebuilder.h"
#include "writeout.h"
#include "bitout.h"
#include "repair.h"

/*  Static functions  */
static void usage (ARGS_INFO *args_struct);
static void initRepair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void uninitRepair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void executeRepair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void displayStats_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);

/*
**  Print out usage information
*/
static void usage (ARGS_INFO *args_struct) {
  fprintf (stderr, "Re-Pair (Recursive Pairing)\n");
  fprintf (stderr, "===========================\n\n");
  fprintf (stderr, "Usage:  %s [options]\n\n", args_struct -> progname);
  fprintf (stderr, "Options:\n");
  fprintf (stderr, "-a           :  Add primitives to generation 0.\n");
  fprintf (stderr, "-b <size>    :  Blocksize\t\t\t[default:  %u]\n", args_struct -> max_buffer_size);
  fprintf (stderr, "-f           :  Use punctuation flags for word-based parsing.\n");
  fprintf (stderr, "-i <file>    :  Input filename\t\t\t[Required]\n");
  fprintf (stderr, "-e <level>   :  Pairing heuristic.\t\t[default:  0]\n");
  fprintf (stderr, "           0  : No heuristic\n");
  fprintf (stderr, "           1  : Word-aligned Re-Pair\n");
  fprintf (stderr, "           2  : Obey which side symbol is on\n");
  fprintf (stderr, "           3  : No recursion\n");
  fprintf (stderr, "-l <length>  :  Length limit on phrases.\t[default:  %u]\n", args_struct -> max_length);
  fprintf (stderr, "-p <phrases> :  Maximum number of phrases\t[default:  %u]\n", args_struct -> max_phrases);
  fprintf (stderr, "-t <type>    :  Input data type \t\t[1 (default), 2, or 4]\n");
  fprintf (stderr, "-v           :  Verbose output\n");
  fprintf (stderr, "-w           :  Do word length counting to .wl file.\n");
  fprintf (stderr, "-x <count>   :  Minimum number of occurances before replacement\n\t\t\t\t\t[default:  %u]\n", args_struct -> max_keep_count);
  fprintf (stderr, "\nDefault sequence file is <filename.seq>.\n");
  fprintf (stderr, "Default phrase hierarchy file is <filename.prel>.\n\n");

  fprintf (stderr, "Re-Pair version:  %s (%s)\n\n", __DATE__, __TIME__);

  exit (EXIT_SUCCESS);
}


/*
**  Perform Re-Pair on one block
*/
static void executeRepair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
    R_UINT i;

    /*  Perform scanPairs  */
    scanPairs (prog_struct, block_struct);

    /*  Allocate queue and initialize to NULL  */
    /*  Make the priority queue bigger by one since position 0 is
    **  unused.  */
    block_struct -> pqueue_size = block_struct -> max_count + 1;
    block_struct -> pqueue = wmalloc ((block_struct -> pqueue_size) * (sizeof (TPHRASE*)));
    for (i = 0; i < block_struct -> pqueue_size; i++) {
        block_struct -> pqueue[i] = NULL;
    }

    /*  Populate queue with tentative phrases  */
    initQueue (prog_struct, block_struct);

    /*  Recursively pair phrases  */
    rePairPhrases (prog_struct, block_struct);

    /*  Sort primitives  */

    sortPrimitives (block_struct);

    block_struct -> sort_phrases = wmalloc (((block_struct -> num_prims) + (block_struct -> num_phrases)) * (sizeof (PHRASE)));

    /*  Sort phrases  */
    sortPhrases (prog_struct, block_struct);

    return;
}


static void initRepair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT i = 0;

  uninitRepair_OneBlock (prog_struct, block_struct);

  /*  Initialize sequence buffer  */
  block_struct -> seq_buf_len = prog_struct -> max_buffer_size;
  block_struct -> seq_buf = wmalloc ((block_struct -> seq_buf_len) * sizeof (SEQ_NODE));
  block_struct -> seq_buf_end = (block_struct -> seq_buf) + (block_struct -> seq_buf_len - 1);

  /*  Create and initialize array for primitives  */
  block_struct -> prims_array_size = prog_struct -> max_prims;
  block_struct -> prims_array = wmalloc (block_struct -> prims_array_size * sizeof (R_UINT));

  block_struct -> temp_phrases_size = prog_struct -> max_prims;
  block_struct -> temp_phrases = wmalloc ((block_struct -> temp_phrases_size) * (sizeof (PHRASE)));

  /*  Initialize all primitives in the temp_phrases array  */
  for (i = 0; i < block_struct -> prims_array_size; i++) {
    block_struct -> temp_phrases[i].left = i;  
                                               /*  i is the ASCII value  */
    block_struct -> temp_phrases[i].left_chiastic = 0;
    block_struct -> temp_phrases[i].right = i;
    block_struct -> temp_phrases[i].right_chiastic = 0;
    block_struct -> temp_phrases[i].unit = i;
 	                     /*  Unit = ASCII value for primitives only  */
    block_struct -> temp_phrases[i].generation = 0;
                                                      /*  Generation of 0  */
    block_struct -> temp_phrases[i].length = 1;
    block_struct -> temp_phrases[i].temp_index = i;
    if (prog_struct -> apply_heuristics == HEUR_WA) {
      if (ISWORD (i)) {
        block_struct -> temp_phrases[i].mytype = PT_WORD;
      }
      else {
        block_struct -> temp_phrases[i].mytype = PT_NON_WORD;
      }
    }
    block_struct -> temp_phrases[i].myside = SIDE_NONE;
  }

  if (prog_struct -> add_prims == R_TRUE) {
    for (i = 0; i < block_struct -> prims_array_size; i++) {
      block_struct -> num_prims += 1;
      block_struct -> prims_array[i] = 1;
    }
  }
  else {
    for (i = 0; i < block_struct -> prims_array_size; i++) {
      block_struct -> prims_array[i] = UNINITIALIZED_GENERATION;
    }
  }

  /*  Ensure that the zero-length word is always accounted for  */
  if (prog_struct -> base_datatype != (R_UINT) sizeof (R_UCHAR)) {
    block_struct -> num_prims += 1;
    block_struct -> prims_array[0] = 1;
  }

  /*
  **  Copy the end of the last buffer to this one
  */
  if (block_struct -> input_stack_size != 0) {
    for (i = 0; i < block_struct -> input_stack_size; i++) {
      initSeqNode ((R_UINT) block_struct -> input_stack[i].value, &block_struct -> seq_buf[i]);
      if (block_struct -> prims_array[(R_UINT) block_struct -> input_stack[i].value] == UNINITIALIZED_GENERATION) {
        block_struct -> prims_array[(R_UINT) block_struct -> input_stack[i].value] = 0;
	block_struct -> num_prims++;
      }
      block_struct -> prims_array[(R_UINT) block_struct -> input_stack[i].value]++;
    }
    wfree (block_struct -> input_stack);
  }
  block_struct -> input_stack = NULL;

  /*  Initialize tent_phrases hash table  */
  block_struct -> tent_phrases_size = (R_UINT) TENTPHRASE_SIZE;
  block_struct -> tent_phrases = wmalloc (block_struct -> tent_phrases_size * sizeof (TPHRASE*));
  for (i = 0; i < block_struct -> tent_phrases_size; i++) {
    block_struct -> tent_phrases[i] = NULL;
  }

  prog_struct -> total_blocks++;

  return;
}


static void uninitRepair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT num_prims_and_phrases;

  if (block_struct -> seq_buf != NULL) {
    wfree (block_struct -> seq_buf);
  }
  block_struct -> seq_buf = NULL;
  block_struct -> seq_buf_end = NULL;
  block_struct -> seq_buf_len = 0;

  if (block_struct -> prims_array != NULL) {
    wfree (block_struct -> prims_array);
  }
  block_struct -> prims_array = NULL;
  block_struct -> prims_array_size = 0;

  if (block_struct -> tent_phrases != NULL) {
    wfree (block_struct -> tent_phrases);
  }
  block_struct -> tent_phrases = NULL;
  block_struct -> tent_phrases_size = (R_UINT) TENTPHRASE_SIZE;

  if (block_struct -> pqueue != NULL) {
    wfree (block_struct -> pqueue);
  }
  block_struct -> pqueue = NULL;
  block_struct -> pqueue_size = 0;

  block_struct -> sizelist = NULL;

  block_struct -> tphrase_in_use = 0;
  block_struct -> max_count = 0;

  if (block_struct -> temp_phrases != NULL) {
    wfree (block_struct -> temp_phrases);
  }
  block_struct -> temp_phrases = NULL;

  if (block_struct -> sort_phrases != NULL) {
    wfree (block_struct -> sort_phrases);
  }
  block_struct -> sort_phrases = NULL;


  num_prims_and_phrases = block_struct -> num_prims + block_struct -> num_phrases;
  if (block_struct -> num_prims > prog_struct -> maximum_primitives) {
    prog_struct -> maximum_primitives = block_struct -> num_prims;
  }

  if (num_prims_and_phrases > prog_struct -> maximum_total_num_phrases) {
    prog_struct -> maximum_total_num_phrases = num_prims_and_phrases;
  }
  prog_struct -> total_num_prims += block_struct -> num_prims;
  prog_struct -> total_num_phrases += block_struct -> num_phrases;

  if (block_struct -> num_generation > prog_struct -> maximum_generations) {
    prog_struct -> maximum_generations = block_struct -> num_generation;
  }

  prog_struct -> total_sum_phrase_length += block_struct -> sum_phrase_length;
  if (block_struct -> longest_phrase_length > prog_struct -> max_longest_phrase_length) {
    prog_struct -> max_longest_phrase_block = prog_struct -> total_blocks;
    prog_struct -> max_longest_phrase_num = block_struct -> longest_phrase;
    prog_struct -> max_longest_phrase_length = block_struct -> longest_phrase_length;
  }

  prog_struct -> total_num_symbols += block_struct -> num_symbols;

  block_struct -> num_prims = 0;
  block_struct -> num_phrases = 0;
  block_struct -> num_generation = 0;
  block_struct -> longest_phrase = 0;
  block_struct -> longest_phrase_length = 0;
  block_struct -> sum_phrase_length = 0;
  block_struct -> num_symbols = 0;

  return;
}


static void displayStats_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {

#ifdef DEBUG
  fprintf (stderr, "Statistics for current block:\n");
  fprintf (stderr, "\tBlock number:  %d\n", prog_struct -> total_blocks);
  fprintf (stderr, "\tNumber of primitives:  %d\n", block_struct -> num_prims);
  fprintf (stderr, "\tNumber of phrases:  %d\n", block_struct -> num_phrases);
  fprintf (stderr, "\tLength of sequence:  %d\n", block_struct -> num_symbols);
#endif

  /*  Must add 1 to num_generations since it is 0-based  */
  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stderr, "%5u\t%5u\t%7u\t  %15u\t%11u\t%7u\n", prog_struct -> total_blocks, block_struct -> num_prims, block_struct -> num_phrases, block_struct -> num_prims + block_struct -> num_phrases, block_struct -> num_generation + 1, block_struct -> num_symbols);
  }

  return;
}

/*
**  Parse arguments and use them to set numerous variables in the
**  structures.
*/
ARGS_INFO *parseArguments (R_INT argc, R_CHAR *argv[], ARGS_INFO *args_struct) {
  /*  Declarations required for getopt  */
  R_INT c;
  R_UINT i = 0;
  R_UINT *buf = NULL;
  FILE *fp = NULL;
  R_UINT items = 0;
  R_UINT maxsym = 0;

  args_struct -> base_filename = NULL;

  if (args_struct == NULL) {
    fprintf (stderr, "Structure ARGS_INFO needs to be malloc'ed in [%s] on line %d.\n", __FILE__, __LINE__);
    exit (EXIT_FAILURE);
  }

  /*  Initialize variables  */
  args_struct -> progname = NULL;
  args_struct -> max_buffer_size = MAX_BUFFER_SIZE;
  args_struct -> prel_text_file = NULL;
  args_struct -> base_filename = NULL;
  args_struct -> apply_heuristics = HEUR_NONE;
  args_struct -> word_flags = UW_NO;
  args_struct -> add_prims = R_FALSE;
  args_struct -> max_length = UINT_MAX;
  args_struct -> max_phrases = UINT_MAX;
  args_struct -> verbose_level = R_FALSE;
  args_struct -> max_keep_count = MIN_KEEP_COUNT;
  args_struct -> base_datatype = (R_UINT) sizeof (R_UCHAR);
  args_struct -> max_prims = MIN_PRIMS_ARRAY;
  args_struct -> dowordlen = R_FALSE;

  /*
  **  Initialize to the name of the program
  */
  args_struct -> progname = argv[0];

  /*  Print usage information if no arguments  */
  if (argc == 1) {
    usage (args_struct);
  }

  while (R_TRUE) {
    c = getopt (argc, argv, "ab:fe:i:l:p:t:vwx:?");
    if (c == EOF) {
      break;
    }

    switch (c) {
    case 'a':
      args_struct -> add_prims = R_TRUE;
      break;
    case 'b':
      args_struct -> max_buffer_size = (R_UINT) atoi (optarg);
      if (args_struct -> max_buffer_size > MAX_BUFFER_SIZE) {
        fprintf (stderr, "Option with -b must be less than or equal to MAX_BUFFER_SIZE\n");
        exit (EXIT_FAILURE);
      }
      break;
    case 'f':
      args_struct -> word_flags = UW_YES;
      break;
    case 'e':
      args_struct -> apply_heuristics = atoi (optarg);
      break;
    case 'i':
      args_struct -> base_filename = optarg;
      break;
    case 'l':
      args_struct -> max_length = (R_UINT) atoi (optarg);
      break;
    case 'p':
      args_struct -> max_phrases = (R_UINT) atoi (optarg);
      break;
    case 't':
      args_struct -> base_datatype = (R_UINT) atoi (optarg);
      if ((args_struct -> base_datatype != (R_UINT) sizeof (R_UCHAR)) && 
          (args_struct -> base_datatype != (R_UINT) sizeof (R_USHRT)) &&
          (args_struct -> base_datatype != (R_UINT) sizeof (R_UINT))) {
        fprintf (stderr, "Input data type (-t) not valid.\n");
        exit (EXIT_FAILURE);
      }
      break;
    case 'v':
      args_struct -> verbose_level = R_TRUE;
      break;
    case 'w':
      args_struct -> dowordlen = R_TRUE;
      break;
    case 'x':
      args_struct -> max_keep_count = (R_UINT) atoi (optarg);
      if (args_struct -> max_keep_count < MIN_KEEP_COUNT) {
        fprintf (stderr, "The value for -x can not be less than MIN_KEEP_COUNT.\n");
        exit (EXIT_FAILURE);
      }
      break;
    case '?':
      usage (args_struct);
      break;
    default:
      fprintf (stderr, "getopt returned erroneous character code.\n");
      exit (EXIT_FAILURE);
    }
  }

  if (optind < argc) {
    fprintf (stderr, "The following arguments were not valid:  ");
    while (optind < argc) {
      fprintf (stderr, "%s ", argv[optind++]);
    }
    fprintf (stderr, "Run program with the -? option for help.\n");
    exit (EXIT_FAILURE);
  }

  if (args_struct -> base_filename == NULL) {
    fprintf (stderr, "Input filename required with the -i option.");
    exit (EXIT_FAILURE);
  }

  if ((args_struct -> apply_heuristics == HEUR_WA) && (args_struct -> base_datatype != (R_UINT) sizeof (R_UCHAR))) {
    fprintf (stderr, "Word-aligned parsing with the specified data type is not possible.");
    exit (EXIT_FAILURE);
  }

  if ((args_struct -> word_flags == UW_YES) && (args_struct -> base_datatype != (R_UINT) sizeof (R_UINT))) {
    fprintf (stderr, "Word-based parsing with punctuation flags not possible with the specified data type.");
    exit (EXIT_FAILURE);
  }

  /*  Determine the largest symbol  */
  if (args_struct -> base_datatype == (R_UINT) sizeof (R_UINT)) {
    FOPEN (args_struct -> base_filename, fp, "r");
    buf = wmalloc (sizeof (R_UINT) * INPUT_BUFFER_SIZE);
    maxsym = 0;
    do {
      items = (R_UINT) fread (buf, sizeof (R_UINT), (size_t) INPUT_BUFFER_SIZE, fp);
      for (i = 0; i < items; i++) {
	buf[i] = buf[i] & NO_FLAGS;
        if (buf[i] > maxsym) {
	  maxsym = buf[i];
	}
      }
    } while (feof (fp) == R_FALSE);
    wfree (buf);
    FCLOSE (fp);
    args_struct -> max_prims = maxsym + 1;
  }

  return (args_struct);
}



/*
**  Perform Re-Pair on a file
*/
void executeRepair_File (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT curr_seq_buf_len = 0;
  R_UINT items_read = 0;
  R_UINT k = 0;
  R_UINT m = 0;
  R_UINT i = 0;

  /*  Declare various buffers, depending on which data type is used as input  */
  R_UINT input_buffer[INPUT_BUFFER_SIZE];
  /*  input_buffer_end points just off array  */
  R_UINT *input_buffer_end = input_buffer + INPUT_BUFFER_SIZE;
  R_UINT *input_buffer_p = input_buffer_end;
  R_UCHAR *input_buffer_c = NULL;
  R_USHRT *input_buffer_s = NULL;

  if (prog_struct -> base_datatype == (R_UINT) sizeof (R_UCHAR)) {
    input_buffer_c = wmalloc (sizeof (R_UCHAR) * INPUT_BUFFER_SIZE);
  }
  else if (prog_struct -> base_datatype == (R_UINT) sizeof (R_USHRT)) {
    input_buffer_s = wmalloc (sizeof (R_USHRT) * INPUT_BUFFER_SIZE);
  }

  /*  Fill one block  */
  while ((block_struct -> input_stack_size != 0) || (input_buffer_p < input_buffer_end) || (ftell (prog_struct -> in_file) < (R_L_INT) prog_struct -> in_file_size)) {
    initRepair_OneBlock (prog_struct, block_struct);
    curr_seq_buf_len = 0;
    curr_seq_buf_len += block_struct -> input_stack_size;
    block_struct -> input_stack_size = 0;

    /*  Fill one sequence  */
    while (curr_seq_buf_len < block_struct -> seq_buf_len && ((ftell (prog_struct -> in_file) < (R_L_INT) prog_struct -> in_file_size) || (input_buffer_p < input_buffer_end))) {
      if (input_buffer_p == input_buffer_end) {
        switch (prog_struct -> base_datatype) {
          case 1:
            items_read = (R_UINT) fread (input_buffer_c, sizeof (R_UCHAR), (size_t) INPUT_BUFFER_SIZE, prog_struct -> in_file);
            for (i = 0; i < items_read; i++) {
              input_buffer[i] = (R_UINT) input_buffer_c[i];
            }
            break;
          case 2:
            items_read = (R_UINT) fread (input_buffer_s, sizeof (R_USHRT), (size_t) INPUT_BUFFER_SIZE, prog_struct -> in_file);
            for (i = 0; i < items_read; i++) {
              input_buffer[i] = (R_UINT) input_buffer_s[i];
            }
            break;
          case 4:
            items_read = (R_UINT) fread (input_buffer, sizeof (R_UINT), (size_t) INPUT_BUFFER_SIZE, prog_struct -> in_file);
            break;
        }
        input_buffer_p = input_buffer;
        input_buffer_end = input_buffer + items_read;
      }
      if (ferror (prog_struct -> in_file) != R_FALSE) {
        fprintf (stderr, "Fatal error in reading from input file!\n");
        exit (EXIT_FAILURE);
      }

      if ((*input_buffer_p & NO_FLAGS) >= block_struct -> prims_array_size) {
        fprintf (stderr, "Symbol %u encountered.\n", *input_buffer_p);
        fprintf (stderr, "Symbol out of range in input buffer in %s, line %u.\n", __FILE__, __LINE__);
        exit (EXIT_FAILURE);
      }

      /*  New primitive found  */
      if (block_struct -> prims_array[(*input_buffer_p & NO_FLAGS)] == UNINITIALIZED_GENERATION) {
        block_struct -> num_prims += 1;
        block_struct -> prims_array[(*input_buffer_p & NO_FLAGS)] = 0;
      }

      /*  Do not increment if maximum number of primitives is reached;
      **  basically prevents counter from overflowing back to 0.  */
      if (block_struct -> prims_array[(*input_buffer_p & NO_FLAGS)] != UNINITIALIZED_GENERATION - 1) {
        block_struct -> prims_array[(*input_buffer_p & NO_FLAGS)] += 1;
      }

      initSeqNode ((R_UINT) *input_buffer_p, &(block_struct -> seq_buf[curr_seq_buf_len]));
      curr_seq_buf_len++;
      input_buffer_p++;
    }

    if (curr_seq_buf_len < block_struct -> seq_buf_len) {
      block_struct -> seq_buf_len = curr_seq_buf_len;
      block_struct -> seq_buf_end = block_struct -> seq_buf + (block_struct -> seq_buf_len - 1);
    }

    /*  Rollback sequence  */
    if ((prog_struct -> apply_heuristics == HEUR_WA) && ((ftell (prog_struct -> in_file) < (R_L_INT) prog_struct -> in_file_size) || (input_buffer_p < input_buffer_end))) {
      k = block_struct -> seq_buf_len - 1;
      while ((k > 0) && (!ISWORD (block_struct -> seq_buf[k].value))) {
	k--;
      }
      while ((k > 0) && (ISWORD (block_struct -> seq_buf[k].value))) {
	k--;
      }
      /*
      **  At this point, k will point to the last SEQ_NODE of the shortened
      **  block_struct -> seq_buf.
      */
      if (k != 1) {
	/*
	**  m is used to iterate through the end of the array to copy
	**  the values to an "input_stack".
	*/
	m = k + 1;
	block_struct -> input_stack = wmalloc (((block_struct -> seq_buf_len - m) * sizeof (SEQ_NODE)));
	for (k = 0; k < block_struct -> seq_buf_len - m; k++) {
	  initSeqNode ((R_UINT) block_struct -> seq_buf[m + k].value, &block_struct -> input_stack[k]);
	  block_struct -> prims_array[block_struct -> seq_buf[m + k].value]--;
	  if (block_struct -> prims_array[block_struct -> seq_buf[m + k].value] == 0) {
	    block_struct -> num_prims--;
            block_struct -> prims_array[block_struct -> seq_buf[m + k].value] = UNINITIALIZED_GENERATION;
	  }
	}

        /*  Decrease sequence from block_struct -> seq_buf_len by the number
        **  of characters copied  */
	block_struct -> seq_buf_len -= k;
	block_struct -> seq_buf_end = block_struct -> seq_buf + (block_struct -> seq_buf_len - 1);
	block_struct -> input_stack_size = k;
      }
      else {
        /*  Roll back sequence to the beginning  */
      }
    }

    (block_struct -> sizelist) = initSListNode (block_struct -> num_prims);

    executeRepair_OneBlock (prog_struct, block_struct);
    encodeHierarchy_OneBlock (prog_struct, block_struct);
    encodeSequence_OneBlock (prog_struct, block_struct);
    displayStats_OneBlock (prog_struct, block_struct);

    uninitRepair_OneBlock (prog_struct, block_struct);
  }

  if (input_buffer_c != NULL) {
    wfree (input_buffer_c);
  }
  else if (input_buffer_s != NULL) {
    wfree (input_buffer_s);
  }

  return;
}

/*
**  Initialize the two main data structures
*/
void initRepair (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_CHAR *temp_filename = NULL;
  ARGS_INFO *args_struct = prog_struct -> args_struct;

  /*  Statistics on file  */
  struct stat statbuffer;

  if ((prog_struct == NULL) || (block_struct == NULL)) {
    fprintf (stderr, "Structures prog_struct and block_struct need to be malloc'ed.\n");
    exit (EXIT_FAILURE);
  }

  /*  Initialize variables in structures  */
  prog_struct -> progname = NULL;

  prog_struct -> in_file = NULL;
  prog_struct -> in_file_size = 0;
  prog_struct -> seq_file = NULL;
  prog_struct -> prel_file = NULL;
  prog_struct -> prel_text_file = NULL;
  prog_struct -> shuff_file = NULL;
  prog_struct -> base_filename = NULL;

  prog_struct -> verbose_level = R_FALSE;
  prog_struct -> max_buffer_size = MAX_BUFFER_SIZE;
  prog_struct -> max_length = UINT_MAX;
  prog_struct -> max_phrases = UINT_MAX;
  prog_struct -> max_keep_count = MIN_KEEP_COUNT;
  prog_struct -> apply_heuristics = HEUR_NONE;
  prog_struct -> word_flags = UW_NO;
  prog_struct -> add_prims = R_FALSE;
  prog_struct -> base_datatype = (R_UINT) sizeof (R_UCHAR);
  prog_struct -> max_prims = MIN_PRIMS_ARRAY;
  prog_struct -> dowordlen = R_FALSE;

  prog_struct -> maximum_total_num_phrases = 0;
  prog_struct -> total_num_phrases = 0;
  prog_struct -> total_num_prims = 0;
  prog_struct -> total_num_symbols = 0;
  prog_struct -> maximum_generations = 0;
  prog_struct -> maximum_primitives = 0;
  prog_struct -> total_blocks = 0;
  prog_struct -> total_sum_phrase_length = 0;
  prog_struct -> max_longest_phrase_block = 0;
  prog_struct -> max_longest_phrase_num = 0;
  prog_struct -> max_longest_phrase_length = 0;

  if (prog_struct -> args_struct != NULL) {
    /*  Set variables from args_struct  */
    prog_struct -> progname = wmalloc (sizeof (R_CHAR) * (strlen (args_struct -> progname) + 1));
    strcpy (prog_struct -> progname, args_struct -> progname);
    prog_struct -> max_buffer_size = args_struct -> max_buffer_size;
    prog_struct -> prel_text_file = args_struct -> prel_text_file;
    prog_struct -> base_filename = wmalloc (sizeof (R_CHAR) * (strlen (args_struct -> base_filename) + 1));
    strcpy (prog_struct -> base_filename, args_struct -> base_filename);
    prog_struct -> apply_heuristics = args_struct -> apply_heuristics;
    prog_struct -> word_flags = args_struct -> word_flags;
    prog_struct -> add_prims = args_struct -> add_prims;
    prog_struct -> max_length = args_struct -> max_length;
    prog_struct -> max_phrases = args_struct -> max_phrases;
    prog_struct -> verbose_level = args_struct -> verbose_level;
    prog_struct -> max_keep_count = args_struct -> max_keep_count;
    prog_struct -> base_datatype = args_struct -> base_datatype;
    prog_struct -> max_prims = args_struct -> max_prims;
    prog_struct -> dowordlen = args_struct -> dowordlen;
  }

  prog_struct -> seq_nodelist_size = INIT_NODELIST_SIZE;
  prog_struct -> seq_nodelist = wmalloc (prog_struct -> seq_nodelist_size * sizeof (SEQ_NODE*));

  if (args_struct -> base_filename != NULL) {
    /*  Open source file  */
    prog_struct -> in_file = fopen (prog_struct -> base_filename, "r");
    if (prog_struct -> in_file == NULL) {
      fprintf (stderr, "Input file not found.");
      exit (EXIT_FAILURE);
    }

    /*  Get statistics on file  */
    if (stat (prog_struct -> base_filename, &statbuffer) != 0) {
      fprintf (stderr, "Error in obtaining file info for %s.\n", prog_struct -> base_filename);
      exit (EXIT_FAILURE);
    }
    prog_struct -> in_file_size = (R_UINT) statbuffer.st_size;

    if (prog_struct -> in_file_size < prog_struct -> max_buffer_size) {
      prog_struct -> max_buffer_size = prog_struct -> in_file_size;
    }

    if (prog_struct -> in_file_size == 0) {
      fprintf (stderr, "Empty input file.");
      exit (EXIT_FAILURE);
    }

    temp_filename = wmalloc ((sizeof(R_CHAR)*(strlen (prog_struct -> base_filename)+7)));

    temp_filename = strcpy (temp_filename, prog_struct -> base_filename);
    temp_filename = strcat (temp_filename, ".seq");

    prog_struct -> seq_file = fopen (temp_filename, "w");
    if (prog_struct -> seq_file == NULL) {
      fprintf (stderr, "Error creating seq file in %s on line %u.\n", __FILE__, __LINE__);
      exit (EXIT_FAILURE);
    }

    /*  Create prel file  */
    temp_filename = strcpy (temp_filename, prog_struct -> base_filename);
    temp_filename = strcat (temp_filename, ".prel");
    prog_struct -> prel_file = fopen (temp_filename, "w");
    if (prog_struct -> prel_file == NULL) {
      fprintf (stderr, "Error creating prel file.\n");
    }

    wfree (temp_filename);
  }
  else {
  }

  /*  Initialize values that are reset at the beginning of each block.  */
  /*  Assumes uninitRepair_OneBlock will be run soon.  */
  block_struct -> seq_buf = NULL; 
  block_struct -> input_stack = NULL;
  block_struct -> input_stack_size = 0;
  block_struct -> prims_array = NULL;  
  block_struct -> tent_phrases = NULL;
  block_struct -> pqueue = NULL;

  block_struct -> temp_phrases = NULL;
  block_struct -> sort_phrases = NULL;

  /*  Initialize values to 0 before calling uninitRepair_OneBlock  */
  block_struct -> num_prims = 0;
  block_struct -> num_phrases = 0;
  block_struct -> num_generation = 0;
  block_struct -> sum_phrase_length = 0;
  block_struct -> longest_phrase_length = 0;
  block_struct -> num_symbols = 0;

  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stderr, "Block\tPrims\tPhrases\t  Prims + Phrases\tGenerations\tSymbols\n\n");
  }

  return;
}


void uninitRepair (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  /*
  **  If output was sent to a file, then put an end of file marker
  **  on the prelude file and close both the seq and prel files.
  */
  if (prog_struct -> prel_file != NULL) {
    writeBits (prog_struct -> prel_file, 1, 1, R_FALSE);
    writeBits (prog_struct -> prel_file, 0, 0, R_FALSE);
    writeBits (prog_struct -> prel_file, 0, 0, R_FALSE);
    writeBits (prog_struct -> prel_file, 0, 0, R_TRUE);

    FCLOSE (prog_struct -> seq_file);
    FCLOSE (prog_struct -> prel_file);
    if (prog_struct -> prel_text_file != NULL) {
      FCLOSE (prog_struct -> prel_text_file);
    }

  }
  
#ifdef DEBUG
    fprintf (stderr, "\nOverall Statistics:\n\n");
    fprintf (stderr, "Input filename:  %s\n", prog_struct -> base_filename != NULL ? prog_struct -> base_filename : "N/A");
    fprintf (stderr, "Input file size:  %d\n", prog_struct -> base_filename != NULL ? prog_struct -> in_file_size : 0);
    fprintf (stderr, "Total number of phrases:  %d\n", prog_struct -> total_num_phrases);
    fprintf (stderr, "Total number of blocks:  %d\n", prog_struct -> total_blocks);
    fprintf (stderr, "Total sequence length:  %d\n", prog_struct -> total_num_symbols);
    if (prog_struct -> prel_text_file != NULL) {
      fprintf (stderr, "Average length of phrases:  %f per phrase\n", (R_DOUBLE) prog_struct -> total_sum_phrase_length / (R_DOUBLE) prog_struct -> total_num_phrases);
      fprintf (stderr, "The longest phrases was:\n");
      fprintf (stderr, "\tphrase number:  %d\n", prog_struct -> max_longest_phrase_num);
      fprintf (stderr, "\tblock number:  %d\n", prog_struct -> max_longest_phrase_block);
      fprintf (stderr, "\tphrase length:  %d\n", prog_struct -> max_longest_phrase_length);
    }

    fprintf (stderr, "\nMaximum for one phrase hierarchy:\n");
    fprintf (stderr, "\tNumber of primitives and phrases:  %d\n", prog_struct -> maximum_total_num_phrases);
    fprintf (stderr, "\tGeneration:  %d\n", prog_struct -> maximum_generations + 1);
                                   /*  Add 1 since generation is 0-based  */
    fprintf (stderr, "\tNumber of primitives:  %d\n", prog_struct -> maximum_primitives);
#endif

  /*  Must add 1 to maximum_generations since it is 0-based  */
  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stderr, "-------------------------------------------------------------------------\n");
    fprintf (stderr, "%5u\t%5u\t%7u\t  %15u\t%11u\t%7u\n", prog_struct -> total_blocks, prog_struct -> total_num_prims, prog_struct -> total_num_phrases, prog_struct -> total_num_prims + prog_struct -> total_num_phrases, prog_struct -> maximum_generations + 1, prog_struct -> total_num_symbols);
  }

  wfree (prog_struct -> seq_nodelist);
  wfree (prog_struct -> progname);
  wfree (prog_struct -> base_filename);

  return;
}


