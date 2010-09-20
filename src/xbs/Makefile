# XBS Makefile
###########################################################################
##############################################################################
# Copyright 2003 Jonathan Cohen, Nat Duca, David Luebke, Brenden Schubert    #
#                Johns Hopkins University and University of Virginia         #
##############################################################################
# This file is distributed as part of the GLOD library, and as such, falls   #
# under the terms of the GLOD public license. GLOD is distributed without    #
# any warranty, implied or otherwise. See the GLOD license for more details. #
#                                                                            #
# You should have recieved a copy of the GLOD Open-Source License with this  #
# copy of GLOD; if not, please visit the GLOD web page,                      #
# http://www.cs.jhu.edu/~graphics/GLOD/license for more information          #
##############################################################################

default: all

all:
	make -C ..



##############################################################################

CC = g++
XBS_CFLAGS += -g3
#XBS_CFLAGS += -O2 -pg
XBS_CFLAGS += -O2 -fno-stack-protector
XBS_CFLAGS += -I. -I../mt -I../ply -I../../include -I../include -I../vds/
XBS_LFLAGS = -lGL $(wildcard ../mt/*.o) ../ply/plyfile.o $(wildcard ../vds/*.o) ../api/glod_glext.o -fno-stack-protector


XBS_STANDALONE_SRCS = \
			Discrete.C \
			Heap.C \
			Hierarchy.C \
			Metric.C \
			MLBPriorityQueue.C \
		 	Model.C \
			ModelShare.C \
			MTHierarchy.C \
			Operation.C \
			PermissionGrid.C \
			SimpQueue.C \
			View.C \
			xbs.C

XBS_STANDALONE_OBJS = $(addprefix ./build/, $(addsuffix .o, $(basename $(XBS_STANDALONE_SRCS))))

xbs: build $(XBS_STANDALONE_OBJS)
	$(CC) -o $@ $(XBS_CFLAGS) $(XBS_STANDALONE_OBJS) $(XBS_LFLAGS)

build:
	mkdir build

clean: clean_xbs
	make -C .. clean_xbs

clean_xbs:
	rm -f xbs
	rm -f xbs.o
	rm -f $(XBS_STANDALONE_OBJS)

build/%.o: %.C
	$(CC) -c $(XBS_CFLAGS) -o $@ $<
