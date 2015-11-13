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


#ifndef DESPAIR_H
#define DESPAIR_H

/******************************
Definitions
******************************/
#define OUT_BUF_SIZE (0x40000)  /*  262144  */
#define SEQ_BUF_SIZE (0x40000)  /*  262144  */

/******************************
Structure definitions
******************************/
typedef struct pair {
  R_UINT left;                                            /*  Left child  */
  R_UINT right;                                          /*  Right child  */
  R_UINT len;                                       /*  Length of phrase  */
  R_UINT *pos;                    /*  Last position in the output buffer  */
                                              /*  Must be signed integer  */
  R_ULL_INT buffer_num;             /*  Last buffer number phrase was in  */
  R_ULL_INT chiastic;                                 /*  Chiastic slide  */
} PAIR;

ARGS_INFO *parseArguments (R_INT argc, R_CHAR *argv[], ARGS_INFO *args_struct);
void executeDespair_File (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void initDespair (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void uninitDespair (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void writeOutputFile (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT num);

#endif
