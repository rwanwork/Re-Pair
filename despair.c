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
#include <string.h>
#include <stdlib.h>
#include <limits.h>                   /*  UINT_MAX, UCHAR_MAX, USHRT_MAX  */
#include <getopt.h>                                           /*  getopt  */

#include "common-def.h"
#include "wmalloc.h"
#include "despair-defn.h"
#include "outphrase.h"
#include "despair.h"
#include "bitin.h"
#include "phrase-slide-decode.h"

static void usage (ARGS_INFO *args_info);
static void intDecodeHierarchy (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT a, R_UINT b, R_ULL_INT lo, R_ULL_INT hi);
static void initDespair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void uninitDespair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void executeDespair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void decodeHierarchy_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void decodeSequence_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);


static void usage (ARGS_INFO *args_struct) {
  fprintf (stderr, "Des-Pair\n");
  fprintf (stderr, "========\n\n");
  fprintf (stderr, "Usage:  %s [options]\n\n", args_struct -> progname);
  fprintf (stderr, "Options:\n");
  fprintf (stderr, "-i <file> :  Input filename  [Required]\n");
  fprintf (stderr, "-t <type> :  Input data type [1 (default), 2, or 4]\n");
  fprintf (stderr, "-v        :  Verbose output\n");
  fprintf (stderr, "Des-Pair version:  %s (%s)\n\n", __DATE__, __TIME__);
  exit (EXIT_FAILURE);
}


void writeOutputFile (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT num) {
  R_UINT i = 0;
  R_UCHAR *temp_c = prog_struct -> out_buf_c;
  R_USHRT *temp_s = prog_struct -> out_buf_s;
  R_UINT *src = block_struct -> out_buf;

  if (prog_struct -> base_datatype == (R_UINT) sizeof (R_UCHAR)) {
    for (i = 0; i < num; i++) {
      if (src[i] > (R_UINT) UCHAR_MAX) {
        fprintf (stderr, "Symbol %u encountered.\n", src[i]);
        fprintf (stderr, "Symbol value exceeds limit of data type.");
        exit (EXIT_FAILURE);
      }
      temp_c[i] = (R_UCHAR) src[i];
    }
    (void) fwrite (temp_c, sizeof (R_UCHAR), (size_t) num, prog_struct -> out_file);
  }
  else if (prog_struct -> base_datatype == (R_UINT) sizeof (R_USHRT)) {
    for (i = 0; i < num; i++) {
      if (src[i] > USHRT_MAX) {
        fprintf (stderr, "Symbol %u encountered.\n", src[i]);
        fprintf (stderr, "Symbol value exceeds limit of data type.");
        exit (EXIT_FAILURE);
      }
      temp_s[i] = (R_USHRT) src[i];
    }
    (void) fwrite (temp_s, sizeof (R_USHRT), (size_t) num, prog_struct -> out_file);
  }
  else {
    (void) fwrite (src, sizeof (R_UINT), (size_t) num, prog_struct -> out_file);
  }

  return;
}


static void intDecodeHierarchy (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT a, R_UINT b, R_ULL_INT lo, R_ULL_INT hi) {
  R_ULL_INT mid;
  R_UINT halfway;
  R_UINT range;

  range = b - a;
  switch (range) {
    case 0:  return;
    case 1:  block_struct -> phrases_array[a].chiastic = binaryDecode (lo, hi, prog_struct -> bit_in_rec);
             return;
  }
  halfway = range >> 1;

  mid = binaryDecode(lo + (R_ULL_INT) halfway, hi - (R_ULL_INT) (range - halfway - 1), prog_struct -> bit_in_rec);
  block_struct -> phrases_array[a + halfway].chiastic = mid;
  intDecodeHierarchy (prog_struct, block_struct, a, a + halfway, lo, mid);
  intDecodeHierarchy (prog_struct, block_struct, a + halfway + 1, b, mid + 1ull, hi);

  return;
}


static void initDespair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  uninitDespair_OneBlock (prog_struct, block_struct);

  block_struct -> buffer_num = 1;
  prog_struct -> total_blocks++;

  return;
}


static void uninitDespair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT num_prims_and_phrases;

  num_prims_and_phrases = block_struct -> num_prims + block_struct -> num_phrases;
  if (num_prims_and_phrases > prog_struct -> maximum_total_num_phrases) {
    prog_struct -> maximum_total_num_phrases = num_prims_and_phrases;
  }
  prog_struct -> total_num_prims += block_struct -> num_prims;
  prog_struct -> total_num_phrases += block_struct -> num_phrases;
  prog_struct -> total_num_symbols += block_struct -> num_symbols;

  if (block_struct -> num_generation > prog_struct -> maximum_generations) {
    prog_struct -> maximum_generations = block_struct -> num_generation;
  }

  if (block_struct -> num_prims > prog_struct -> maximum_primitives) {
    prog_struct -> maximum_primitives = block_struct -> num_prims;
  }

  /*  Flush output buffer  */
  if ((block_struct -> prims_buf != NULL) && (block_struct -> out_buf != block_struct -> out_buf_p)) {
    writeOutputFile (prog_struct, block_struct, (R_UINT) (block_struct -> out_buf_p - block_struct -> out_buf));
  }

  if (block_struct -> prims_buf != NULL) {
    wfree (block_struct -> prims_buf);
  }
  block_struct -> prims_buf = NULL;
  block_struct -> out_buf = NULL;
  block_struct -> out_buf_end = NULL;
  block_struct -> out_buf_p = NULL;

  block_struct -> num_prims = 0;
  block_struct -> num_phrases = 0;
  block_struct -> num_symbols = 0;
  block_struct -> num_generation = 0;
  block_struct -> num_seq_blocks = 0;
  block_struct -> total_phrase_length = 0;
  block_struct -> max_longest_phrase_length = 0;

  if (block_struct -> generation_array != NULL) {
    wfree (block_struct -> generation_array);
  }
  block_struct -> generation_array = NULL;

  if (block_struct -> phrases_array != NULL) {
    wfree (block_struct -> phrases_array);
  }
  block_struct -> phrases_array = NULL;

  return;
}


static void executeDespair_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {

  decodeHierarchy_OneBlock (prog_struct, block_struct);
  if (block_struct -> num_phrases + block_struct -> num_prims == 0) {
    return;
  }

#ifdef DEBUG
    for (i = 0; i < (block_struct -> num_phrases + block_struct -> num_prims); i++) {
      if (block_struct -> phrases_array[i].len == 0) {
        block_struct -> phrases_array[i].len = block_struct -> phrases_array[block_struct -> phrases_array[i].left].len + block_struct -> phrases_array[block_struct -> phrases_array[i].right].len;
      }
      if (block_struct -> phrases_array[i].len > block_struct -> max_longest_phrase_length) {
        block_struct -> max_longest_phrase_length = block_struct -> phrases_array[i].len;
      }
      block_struct -> total_phrase_length += block_struct -> phrases_array[i].len;
    }
#endif

  decodeSequence_OneBlock (prog_struct, block_struct);

  return;
}


static void decodeHierarchy_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct)
{
  R_ULL_INT kpsqr;
  R_ULL_INT kppsqr = 0;
  R_ULL_INT kp = 0;
  R_ULL_INT kpp = 0;
  R_UINT generation_size = 0;
  R_UINT curr_gen = 0;
  R_UINT i = 0;
   
  block_struct -> generation_array = wmalloc (sizeof (GENNODE) * MAX_GEN);

  /*  Total number of phrases + primitives */
  block_struct -> num_phrases = deltaDecode (0, prog_struct -> bit_in_rec);
  if (block_struct -> num_phrases == 0) {
    return;
  }

  /*  Total number of primitives  */
  block_struct -> num_prims = deltaDecode (1, prog_struct -> bit_in_rec);

  /*  Remove the number of primitives from the count of phrases  */
  block_struct -> num_phrases -= block_struct -> num_prims;

#ifdef DEBUG
  fprintf (stderr, "Prims / phrases:  %u / %u\n", block_struct -> num_prims, block_struct -> num_phrases);
#endif

  block_struct -> prims_buf = wmalloc (sizeof (R_UINT) * (block_struct -> num_prims + OUT_BUF_SIZE));
  block_struct -> out_buf = block_struct -> prims_buf + block_struct -> num_prims;

  /*  Need OUT_BUF_SIZE or else we would access out of the array  */
  block_struct -> out_buf_end = block_struct -> out_buf + OUT_BUF_SIZE;
  block_struct -> out_buf_p = block_struct -> out_buf;

  generation_size = block_struct -> num_prims;
  block_struct -> generation_array[curr_gen].size = generation_size;

#ifdef DEBUG
  fprintf (stderr, "Generation %u size:  %u\n", curr_gen, generation_size);
#endif
  curr_gen++;

  block_struct -> phrases_array = wmalloc ((block_struct -> num_phrases + block_struct -> num_prims) * sizeof (PAIR));

  /*  Decode the primitives  */
  intDecodeHierarchy (prog_struct, block_struct, 0, block_struct -> num_prims, 0, 1ull << gammaDecode (0, prog_struct -> bit_in_rec));
  setUnitPrimitives (block_struct);

  /*  Initialize generation to 1 to include the primitives, decoded
  **  earlier.  */
  block_struct -> num_generation = 1;
  
  /*  decode the phrases  */
  for (kp = generation_size; kp < block_struct -> num_phrases + block_struct -> num_prims; kp += generation_size, ++block_struct -> num_generation) {
    /*  size of generation  */
    generation_size = gammaDecode (1, prog_struct -> bit_in_rec);

    block_struct -> generation_array[curr_gen].size = generation_size;
#ifdef DEBUG
    fprintf (stderr, "Generation %u size:  %u\n", curr_gen, generation_size);
#endif
    curr_gen++;

    /*  decode pairs  */
    kpsqr =  (R_ULL_INT) kp * (R_ULL_INT) kp;

    intDecodeHierarchy (prog_struct, block_struct, (R_UINT) kp, (R_UINT) (kp + generation_size), 0ull, kpsqr - kppsqr);
    setUnitPhrasesChiastic (kp, kpp, kpsqr, kppsqr, generation_size, block_struct -> phrases_array);

    kpp = kp;
    kppsqr = kpsqr;
  }

  block_struct -> generation_array[0].cumm_size = 0;
  for (i = 1; i < curr_gen; i++) {
    block_struct -> generation_array[i].cumm_size = block_struct -> generation_array[i - 1].size + block_struct -> generation_array[i - 1].cumm_size;
  }

  block_struct -> generation_array = wrealloc (block_struct -> generation_array, sizeof (GENNODE) * (curr_gen));

  return;
}


static void decodeSequence_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT x = 0;
  R_UINT bytes_read;
  R_UINT symbol_count = 0;

#ifdef FAVOUR_TIME_EXPAND
  R_UINT i = 0;
  R_UINT j = 0;
  R_UINT k = 0;
#endif

#ifdef FAVOUR_TIME_EXPAND
  for (; i < (block_struct -> num_prims + block_struct -> num_phrases); i++) {
    block_struct -> phrases_array[i].len = block_struct -> phrases_array[block_struct -> phrases_array[i].left].len + block_struct -> phrases_array[block_struct -> phrases_array[i].right].len;
    block_struct -> phrases_array[i].pos = wmalloc (block_struct -> phrases_array[i].len * sizeof (R_UINT));
    for (j = 0; j < block_struct -> phrases_array[block_struct -> phrases_array[i].left].len; j++) {
      block_struct -> phrases_array[i].pos[j] = block_struct -> phrases_array[block_struct -> phrases_array[i].left].pos[j];
    }
    for (k = 0; k < block_struct -> phrases_array[block_struct -> phrases_array[i].right].len; k++) {
      block_struct -> phrases_array[i].pos[j] = block_struct -> phrases_array[block_struct -> phrases_array[i].right].pos[k];
      j++;
    }
  }
#endif

  while (R_TRUE) {
    if (prog_struct -> seq_buf_p  == prog_struct -> seq_buf_end) {
      if (feof (prog_struct -> seq_file) != R_FALSE) {
        fprintf(stderr, "ERROR:  Unexpected EOF. %s: %u.\n", __FILE__, __LINE__);
        exit (EXIT_FAILURE);
      }

      bytes_read = (R_UINT) fread (prog_struct -> seq_buf, sizeof (*(prog_struct -> seq_buf)), SEQ_BUF_SIZE, prog_struct -> seq_file);
      prog_struct -> seq_buf_end = prog_struct -> seq_buf + bytes_read;
      if (ferror (prog_struct -> seq_file) != R_FALSE) {
	fprintf (stderr, "ERROR:  Reading input sequence file.\n");
	exit (EXIT_FAILURE);
      }
      prog_struct -> seq_buf_p = prog_struct -> seq_buf;
    }
    x = (*(prog_struct -> seq_buf_p)) - 1;
    prog_struct -> seq_buf_p++;
    symbol_count++;

    /*
    **  Loop exits when x >= num_phrases; since x is an unsigned int,
    **  exits when x == -1
    */
    if (x == UINT_MAX) {
      break;
    }

    outPhrase (prog_struct, block_struct, x);

  }

  block_struct -> num_symbols = symbol_count;

  return;
}


static void displayStats_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
#ifdef DEBUG
  fprintf (stderr, "Statistics for current block:\n");
  fprintf (stderr, "\tBlock number:  %u\n", prog_struct -> total_blocks);
  fprintf (stderr, "\tNumber of primitives:  %u\n", block_struct -> num_prims);
  fprintf (stderr, "\tNumber of phrases:  %u\n", block_struct -> num_phrases);
  fprintf (stderr, "\tNumber of generations:  %u\n", block_struct -> num_generation);
  fprintf (stderr, "\tSequence length (symbols):  %u\n", block_struct -> num_symbols);

  fprintf (stderr, "\tLongest phrase length (primitives):  %u\n", block_struct -> max_longest_phrase_length);
  fprintf (stderr, "Average phrase length over %u primitives and phrases:  %f\n", block_struct -> num_prims + block_struct -> num_phrases, (R_FLOAT) block_struct -> total_phrase_length / (R_FLOAT) (block_struct -> num_prims + block_struct -> num_phrases));
#endif

  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stdout, "%u\t%u\t%u\t\t%u\t\t%u\t\t%u\n", prog_struct -> total_blocks, block_struct -> num_prims, block_struct -> num_phrases, block_struct -> num_prims + block_struct -> num_phrases, block_struct -> num_generation, block_struct -> num_symbols);
  }

  return;
}

void executeDespair_File (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  while (R_TRUE) {
    initDespair_OneBlock (prog_struct, block_struct);
    executeDespair_OneBlock (prog_struct, block_struct);
    if (block_struct -> num_phrases + block_struct -> num_prims == 0) {
      break;
    }
    displayStats_OneBlock (prog_struct, block_struct);
  }

  uninitDespair_OneBlock (prog_struct, block_struct);

  return;
}   


ARGS_INFO *parseArguments (int argc, char *argv[], ARGS_INFO *args_struct) {
  /*  Declarations required for getopt  */
  R_INT c;

  args_struct -> progname = argv[0];
  args_struct -> base_filename = NULL;
  args_struct -> base_datatype = (R_UINT) sizeof (R_UCHAR);
  args_struct -> verbose_level = R_FALSE;

  /*  Print usage information if no arguments  */
  if (argc == 1) {
    usage (args_struct);
  }

  /*  Check arguments  */
  while (R_TRUE) {
    c = getopt (argc, argv, "i:t:v?");
    if (c == EOF) {
      break;
    }

    switch (c) {
    case 0:
      break;
    case 'i':
      args_struct -> base_filename = optarg;
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
    case '?':
      usage (args_struct);
      break;
    default:
      fprintf (stderr, "Unexpected error:  getopt returned character code 0%d.\n", c);
      exit (EXIT_FAILURE);
    }

  }  /*  while  */

  if (optind < argc) {
    fprintf (stderr, "The following arguments were not valid:  ");
    while (optind < argc) {
      fprintf (stderr, "%s ", argv[optind++]);
    }
    fprintf (stderr, "\nRun %s with the -? option for help.\n", args_struct -> progname);
    exit (EXIT_FAILURE);
  }

  /*  Check for input filename  */
  if (args_struct -> base_filename == NULL) {
    fprintf (stderr, "Error.  Input filename required with the -i option.\n");
    fprintf (stderr, "\nRun %s with the -? option for help.\n", args_struct -> progname);
    exit (EXIT_FAILURE);
  }

  return (args_struct);
}


void initDespair (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_CHAR *prelName = NULL;
  R_CHAR *seqName = NULL;
  R_CHAR *outName = NULL;

  /*  Initialize values in PROG_INFO  */
  prog_struct -> progname = NULL;
  prog_struct -> out_file = NULL;
  prog_struct -> seq_file = NULL;
  prog_struct -> prel_file = NULL;
  prog_struct -> base_filename = NULL;
  prog_struct -> base_datatype = (R_UINT) sizeof (R_UCHAR);

  prog_struct -> seq_buf = NULL;
  prog_struct -> seq_buf_end = NULL;
  prog_struct -> seq_buf_p = NULL;
  prog_struct -> verbose_level = R_FALSE;
  prog_struct -> maximum_total_num_phrases = 0;
  prog_struct -> total_num_prims = 0;
  prog_struct -> total_num_phrases = 0;
  prog_struct -> total_num_symbols = 0;
  prog_struct -> maximum_generations = 0;
  prog_struct -> maximum_primitives = 0;
  prog_struct -> bit_in_rec = NULL;
  prog_struct -> total_blocks = 0;

  /*  Initialize values in BLOCK_INFO  */
  /*  Assumes that initDespair_OneBlock will be run soon  */
  block_struct -> phrases_array = NULL;
  block_struct -> prims_buf = NULL;
  block_struct -> num_prims = 0;
  block_struct -> num_phrases = 0;
  block_struct -> num_symbols = 0;
  block_struct -> num_generation = 0;
  block_struct -> num_seq_blocks = 0;
  block_struct -> total_phrase_length = 0;
  block_struct -> max_longest_phrase_length = 0;

  if (prog_struct -> args_struct != NULL) {
    prog_struct -> progname = (prog_struct -> args_struct) -> progname;
    prog_struct -> base_filename = (prog_struct -> args_struct) -> base_filename;
    prog_struct -> verbose_level = (prog_struct -> args_struct) -> verbose_level;
    prog_struct -> base_datatype = (prog_struct -> args_struct) -> base_datatype;
  }

  if (prog_struct -> base_filename != NULL) {
    prelName = wmalloc ((strlen (prog_struct -> base_filename) + 6) * sizeof (R_CHAR));
    strcpy (prelName, prog_struct -> base_filename);
    strcat (prelName, ".prel");
    prog_struct -> prel_file = fopen (prelName, "r");
    if (! prog_struct -> prel_file) {
      perror (prelName);
      exit (EXIT_FAILURE);
    }
    wfree (prelName);

    seqName = wmalloc ((strlen (prog_struct -> base_filename) + 5) * sizeof (R_CHAR));
    strcpy (seqName, prog_struct -> base_filename);
    strcat (seqName, ".seq");
    prog_struct -> seq_file = fopen (seqName, "r");
    if (! prog_struct -> seq_file) {
      perror (seqName);
      exit (EXIT_FAILURE);
    }
    wfree (seqName);

    outName = wmalloc ((strlen (prog_struct -> base_filename) + 3) * sizeof (R_CHAR));
    strcpy (outName, prog_struct -> base_filename);
    strcat (outName, ".u");
    prog_struct -> out_file = fopen (outName, "w");
    if (! prog_struct -> out_file) {
      perror (outName);
      exit (EXIT_FAILURE);
    }
    wfree (outName);

  }
  else {
  }

  prog_struct -> bit_in_rec = newBitin (prog_struct -> prel_file);
  prog_struct -> seq_buf = wmalloc (SEQ_BUF_SIZE * sizeof (R_UINT));

  prog_struct -> out_buf_c = NULL;
  prog_struct -> out_buf_s = NULL;
  if (prog_struct -> base_datatype == (R_UINT) sizeof (R_UCHAR)) {
    prog_struct -> out_buf_c = wmalloc (sizeof (R_UCHAR) * OUT_BUF_SIZE);
  }
  else if (prog_struct -> base_datatype == (R_UINT) sizeof (R_USHRT)) {
    prog_struct -> out_buf_s = wmalloc (sizeof (R_USHRT) * OUT_BUF_SIZE);
  }

  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stdout, "Block\tPrims\tPhrases\t\tPrims + Phrases\tGenerations\tSymbols\n");
  }

  return;
}


void uninitDespair (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {

#ifdef DEBUG
    fprintf (stderr, "Overall Statistics:\n\n");
    fprintf (stderr, "Input filename:  %s\n", prog_struct -> base_filename != NULL ? prog_struct -> base_filename : "N/A");
    fprintf (stderr, "Total number of primitives:  %u\n", prog_struct -> total_num_prims);
    fprintf (stderr, "Total number of phrases:  %u\n", prog_struct -> total_num_phrases);
    fprintf (stderr, "Total sequence length (symbols):  %u\n", prog_struct -> total_num_symbols);
    fprintf (stderr, "\nMaximum for one phrase hierarchy:\n");
    fprintf (stderr, "\tNumber of primitives and phrases:  %u\n", prog_struct -> maximum_total_num_phrases);
    fprintf (stderr, "\tGeneration:  %u\n", prog_struct -> maximum_generations);
    fprintf (stderr, "\tNumber of primitives:  %u\n", prog_struct -> maximum_primitives);
#endif

  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stdout, "---------------------------------------------------------------------------\n");
    fprintf (stdout, "%u\t%u\t%u\t\t%u\t\t%u\t\t%u\n", prog_struct -> total_blocks - 1, prog_struct -> total_num_prims, prog_struct -> total_num_phrases, prog_struct -> total_num_prims + prog_struct -> total_num_phrases, prog_struct -> maximum_generations, prog_struct -> total_num_symbols);
  }

  if (prog_struct -> prel_file != NULL) {
    FCLOSE (prog_struct -> prel_file);
  }
  prog_struct -> prel_file = NULL;

  if (prog_struct -> seq_file != NULL) {
    FCLOSE (prog_struct -> seq_file);
  }
  prog_struct -> seq_file = NULL;

  if (prog_struct -> out_file != NULL) {
    FCLOSE (prog_struct -> out_file);
  }
  prog_struct -> out_file = NULL;

  if (prog_struct -> seq_buf != NULL) {
    wfree (prog_struct -> seq_buf);
  }
  prog_struct -> seq_buf = NULL;
  prog_struct -> seq_buf_end = NULL;
  prog_struct -> seq_buf_p = NULL;

  if (prog_struct -> bit_in_rec != NULL) {
    wfree ((prog_struct -> bit_in_rec) -> buffer);
    wfree (prog_struct -> bit_in_rec);
  }
  prog_struct -> bit_in_rec = NULL;

  if (prog_struct -> out_buf_c != NULL) {
    wfree (prog_struct -> out_buf_c);
  }
  else if (prog_struct -> out_buf_s != NULL) {
    wfree (prog_struct -> out_buf_s);
  }

  if (block_struct -> phrases_array != NULL) {
    wfree (block_struct -> phrases_array);
  }

  if (block_struct -> prims_buf != NULL) {
    wfree (block_struct -> prims_buf);
  }
  block_struct -> prims_buf = NULL;
  block_struct -> out_buf = NULL;
  block_struct -> out_buf_end = NULL;
  block_struct -> out_buf_p = NULL;

  return;
}


