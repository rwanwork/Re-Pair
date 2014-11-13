##    Re-Pair / Des-Pair
##    Compressor and decompressor based on recursive pairing.
##    Copyright (C) 2003, 2007 by Raymond Wan (rwan@kuicr.kyoto-u.ac.jp)
##
##    Version 1.0.1 -- 2007/04/02
##
##    This file is part of the Re-Pair / Des-Pair software.
##
##    Re-Pair / Des-Pair is free software; you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation; either version 2 of the License, or
##    (at your option) any later version.
##
##    Re-Pair / Des-Pair is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License along
##    with Re-Pair / Des-Pair; if not, write to the Free Software Foundation, Inc.,
##    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
##

CC	= gcc
LIBS	= -lm 
CFLAGS  = -Wno-long-long -pedantic -Wall -Wwrite-strings -Wcast-align \
          -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs   \
          -Wshadow -Winline -O3 $(EXTRA_CFLAGS) $(DESPAIR_EXPAND_MODE)

COMMON_DIR	= ../common

#######################################################################
##  Extra cflags.  Add to EXTRA_CFLAGS, but do not add more than one
##  due to the amount of output that would be produced.
EXTRA_CFLAGS =
##  Print the number of tentative phrases under consideration for
##    replacement (i.e., tphrase_in_use):  -DTPHRASE_IN_USE
##  Print malloc information:  -DCOUNT_MALLOC
##  Print out more information than -v:  -DDEBUG

#######################################################################
##  Select an expansion mode for Des-Pair which balances space with
##  time:
##    -DNORMAL_EXPAND -- Employ a fixed-sized buffer of recently
##                       expanded phrases.  Default and used by the
##                       Proc. IEEE 2000 paper.
##    -DFAVOUR_TIME_EXPAND -- Favour time by maintaining a large buffer
##                            of all phrases.
##    -DFAVOUR_MEMORY_EXPAND -- Favour memory by using a smaller buffer
##                              and expanding phrases as necessary.
DESPAIR_EXPAND_MODE = -DNORMAL_EXPAND 

PROGRAMS		=	repair despair
REPAIR_SRC		=	repair.c main-repair.c seq.c phrase.c \
				phrasebuilder.c phrase-slide-encode.c \
				pair.c writeout.c bitout.c utils.c \
                                wmalloc.c 
REPAIR_SRC_OBJS		=	${REPAIR_SRC:.c=.o}

DESPAIR_SRC		=	despair.c main-despair.c bitin.c \
				phrase-slide-decode.c utils.c outphrase.c \
				wmalloc.c
DESPAIR_SRC_OBJS	=	${DESPAIR_SRC:.c=.o}

#######################################################################
# Rules
#######################################################################

all:  $(PROGRAMS)

##  Not used.
link:
	rm -f utils.h utils.c common-def.h wmalloc.h wmalloc.c
	ln -s $(COMMON_DIR)/common-def.h common-def.h
	ln -s $(COMMON_DIR)/utils.h utils.h
	ln -s $(COMMON_DIR)/utils.c utils.c
	ln -s $(COMMON_DIR)/wmalloc.h wmalloc.h
	ln -s $(COMMON_DIR)/wmalloc.c wmalloc.c

repair:  $(REPAIR_SRC_OBJS) 
	$(CC) $(CFLAGS) -o repair $(REPAIR_SRC_OBJS) $(LIBS) 

despair:  $(DESPAIR_SRC_OBJS) 
	$(CC) $(CFLAGS) -o despair $(DESPAIR_SRC_OBJS) $(LIBS) 


#######################################################################
# Cleaning up
#######################################################################

clean:
	rm -f *~ *.o

clobber:  clean
	rm -f $(PROGRAMS)


