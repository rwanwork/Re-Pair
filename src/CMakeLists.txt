###########################################################################
##  Re-Pair / Des-Pair
##  Compressor and decompressor based on recursive pairing.
##
##  Copyright (C) 2003-2022 by Raymond Wan, All rights reserved.
##  Contact:  rwan.work@gmail.com
##
##  This file is part of Re-Pair / Des-Pair.
##  
##  Re-Pair / Des-Pair is free software; you can redistribute it and/or 
##  modify it under the terms of the GNU General Public License 
##  as published by the Free Software Foundation; either version 
##  3 of the License, or (at your option) any later version.
##  
##  Re-Pair / Des-Pair is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##  
##  You should have received a copy of the GNU General Public 
##  License along with Re-Pair / Des-Pair; if not, see 
##  <http://www.gnu.org/licenses/>.
###########################################################################

##  Set the minimum required CMake version
cmake_minimum_required (VERSION 3.5 FATAL_ERROR)


########################################
##  Variables specific to this project
set (TARGET_NAME_REPAIR "repair")
set (TARGET_NAME_DESPAIR "despair")
set (CURR_PROJECT_NAME "Re-Pair")


########################################
##  Source files

##  Source files common to both Re-Pair and Des-Pair
set (COMMON_SRC_FILES
  utils.c
  wmalloc.c 
)

##  Source files for Re-Pair
set (REPAIR_SRC_FILES
  repair.c 
  main-repair.c 
  seq.c 
  phrase.c 
  phrasebuilder.c 
  phrase-slide-encode.c 
  pair.c 
  writeout.c 
  bitout.c 
)

##  Source files for Des-Pair
set (DESPAIR_SRC_FILES
  despair.c 
  main-despair.c 
  bitin.c
  phrase-slide-decode.c 
  outphrase.c
)


########################################
##  Set up the software

##  Project name
project (${CURR_PROJECT_NAME} VERSION 1.0 LANGUAGES C)
message (STATUS "Setting up ${CURR_PROJECT_NAME}, version ${PROJECT_VERSION}...")


########################################
##  Create the executables

##  Compressor
if (NOT TARGET ${TARGET_NAME_REPAIR})
  add_executable (${TARGET_NAME_REPAIR} ${COMMON_SRC_FILES} ${REPAIR_SRC_FILES})
  install (TARGETS ${TARGET_NAME_REPAIR} DESTINATION bin)
endif (NOT TARGET ${TARGET_NAME_REPAIR})

##  Decompressor
if (NOT TARGET ${TARGET_NAME_DESPAIR})
  add_executable (${TARGET_NAME_DESPAIR} ${COMMON_SRC_FILES} ${DESPAIR_SRC_FILES})
  install (TARGETS ${TARGET_NAME_DESPAIR} DESTINATION bin)
endif (NOT TARGET ${TARGET_NAME_DESPAIR})


########################################
##  Extra cflags.  Add to EXTRA_CFLAGS, but do not add more than one
##  due to the amount of output that would be produced.
##  * Print the number of tentative phrases under consideration for
##    replacement (i.e., tphrase_in_use):  -DTPHRASE_IN_USE
##  * Print malloc information:  -DCOUNT_MALLOC
##  * Print out more information than -v:  -DDEBUG

set (EXTRA_CFLAGS "")


########################################
##  Select an expansion mode for Des-Pair which balances space with
##  time:
##    -DNORMAL_EXPAND -- Employ a fixed-sized buffer of recently
##                       expanded phrases.  Default and used by the
##                       Proc. IEEE 2000 paper.
##    -DFAVOUR_TIME_EXPAND -- Favour time by maintaining a large buffer
##                            of all phrases.
##    -DFAVOUR_MEMORY_EXPAND -- Favour memory by using a smaller buffer
##                              and expanding phrases as necessary.
##
##  Must choose one and CANNOT be left blank.  (Compiler errors will result.)
set (DESPAIR_EXPAND_MODE "-DNORMAL_EXPAND")

##  Turn on lots of warnings; set optimization flag to -O3
set (MY_C_FLAGS "-Wno-long-long -pedantic -Wall -Wwrite-strings -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wshadow -Winline -O3")

##  Create the final compiler flags for the C compiler
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MY_C_FLAGS} ${DESPAIR_EXPAND_MODE} ${EXTRA_CFLAGS}")


############################################################
##  CTest
############################################################

# ENABLE_TESTING ()

