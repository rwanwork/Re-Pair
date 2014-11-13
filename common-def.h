/*    Re-Pair / Des-Pair
**    Compressor and decompressor based on recursive pairing.
**    Copyright (C) 2003, 2007 by Raymond Wan (rwan@kuicr.kyoto-u.ac.jp)
**
**    Version 1.0.1 -- 2007/04/02
**
**    This file is part of the Re-Pair / Des-Pair software.
**
**    Re-Pair / Des-Pair is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
**    Re-Pair / Des-Pair is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License along
**    with Re-Pair / Des-Pair; if not, write to the Free Software Foundation, Inc.,
**    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef COMMON_DEF_H
#define COMMON_DEF_H

/***************************************************************************
Redefine common primitive data types
***************************************************************************/
typedef int R_INT;
typedef unsigned short int R_USHRT;
typedef unsigned int R_UINT;
typedef unsigned long int R_UL_INT;
typedef signed long int R_L_INT;
typedef unsigned long long int R_ULL_INT;
typedef double R_DOUBLE;
typedef float R_FLOAT;

typedef unsigned char R_UCHAR;
typedef char R_CHAR;

/***************************************************************************
Define booleans
***************************************************************************/
#ifndef R_TRUE
#define R_TRUE 1
#endif

#ifndef R_FALSE
#define R_FALSE 0
#endif

#ifndef R_BOOLEAN
#define R_BOOLEAN unsigned int
#endif

/***************************************************************************
Bit masking
***************************************************************************/
#define MASK_EIGHT (0xFFu)  /*  255  */
#define MASK_LOWER (0xFFFFFFFFu)  /*  4294967295  */
#define MASK_HIGHEST (1u << ((sizeof (unsigned int) * 8) - 1))  /*  2147483648  */

#define FOPEN(FILENAME,FP,MODE) \
  FP = fopen ((R_CHAR*) FILENAME, MODE); \
  if (FP == NULL) { \
    if (strcmp (MODE, "w") == 0) { \
      fprintf (stderr, "Error creating %s.\n", FILENAME); \
    } \
    else { \
      fprintf (stderr, "Error opening %s.\n", FILENAME); \
    } \
    fprintf (stderr, "Error %s %s.\n", MODE == "w" ? "creating" : "opening", FILENAME); \
    exit (EXIT_FAILURE); \
  }

#define FCLOSE(FP) \
  (void) fclose (FP);

#endif

