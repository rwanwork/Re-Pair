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


#ifndef WMALLOC_H
#define WMALLOC_H

#define WM_SIZE 65536
#define TEMPSTRLEN 80

typedef struct wmstruct {
  void *ptr;
  size_t size;
  R_CHAR *file;
  R_UINT line;
  struct wmstruct *next;
} WMSTRUCT;

void *wmalloc (size_t y_arg);
void *wrealloc (void *x_arg, size_t y_arg);
void wfree (void *x_arg);

void initWMalloc (void);
void printWMalloc (void);
void printInUseWMalloc (void);
void countMalloc (void *ptr, size_t amount, const R_CHAR *file, R_UINT line);
void countFree (void *ptr);

#endif

