/**************************************************************************
**  Re-Pair / Des-Pair
**  Compressor and decompressor based on recursive pairing.
**
**  Copyright (C) 2003-2022 by Raymond Wan, All rights reserved.
**  Contact:  rwan.work@gmail.com
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

#include "common-def.h"
#include "wmalloc.h"
#include "despair-defn.h"
#include "despair.h"
#include "main-despair.h"

R_INT main (R_INT argc, R_CHAR *argv[]) {
  PROG_INFO *prog_struct = NULL;
  BLOCK_INFO *block_struct = NULL;
  ARGS_INFO *args_struct = NULL;

#ifdef COUNT_MALLOC
  initWMalloc ();
#endif
#ifdef FAVOUR_TIME_EXPAND
  fprintf (stderr, "Des-Pair mode:  Symbol expansion favouring time.\n\n");
#endif

#ifdef FAVOUR_MEMORY_EXPAND
  fprintf (stderr, "Des-Pair mode:  Symbol expansion favouring memory.\n\n");
#endif

#ifdef NORMAL_EXPAND
  fprintf (stderr, "Des-Pair mode:  Normal symbol expansion.\n\n");
#endif

  prog_struct = wmalloc (sizeof (PROG_INFO));
  block_struct = wmalloc (sizeof (BLOCK_INFO));
  args_struct = wmalloc (sizeof (ARGS_INFO));

  prog_struct -> args_struct = parseArguments (argc, argv, args_struct);

  initDespair (prog_struct, block_struct);

  executeDespair_File (prog_struct, block_struct);

  uninitDespair (prog_struct, block_struct);

  wfree (prog_struct);
  wfree (block_struct);
  wfree (args_struct);
#ifdef COUNT_MALLOC
  printWMalloc ();
#endif

  return (EXIT_SUCCESS);
}



