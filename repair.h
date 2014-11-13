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

#ifndef REPAIR_H
#define REPAIR_H


/**************************************************************************
Definitions
**************************************************************************/
#define INPUT_BUFFER_SIZE 65536u           /*  Input buffer size 65,536  */
#define MIN_PRIMS_ARRAY 256u       /*  Minimum size of primitives array  */
#define MAX_PRIMS_ARRAY UINT_MAX        
             /*  Maximum size of primitives array; matches the datatype  */
                                         /*  of the array of primitives  */
#define MAX_BUFFER_SIZE UINT_MAX
                                  /*  Maximum length of sequence buffer  */
#define MIN_KEEP_COUNT 2u
                               /*  Minimum occurrences required to pair  */

#define INIT_NODELIST_SIZE 65536u                           /* (1 << 16) */

/**************************************************************************
Function prototypes
**************************************************************************/
ARGS_INFO *parseArguments (R_INT argc, R_CHAR *argv[], ARGS_INFO *args_struct);
void initRepair (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void uninitRepair (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void executeRepair_File (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);

#endif

