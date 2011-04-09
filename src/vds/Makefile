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
LIBNAME=libvds.a

CFLAGS=-m32 -g -DLinux -I/uf6/gfx/include -fno-stack-protector
LFLAGS=-lGL -lpthread -L./   -lzt -fno-stack-protector
#LFLAGS+=-L/usr/X11R6/lib/ -lglx -lX11

OBJ_SUFFIX=.o
CODE_SUFFIX=.cpp

FILES=  cut forestbuilder forest manager \
	node nodequeue primtypes \
	renderer simplifier tri vif \
	freelist

OBJECTS=$(addsuffix $(OBJ_SUFFIX), $(FILES))
CODE=$(addsuffix $(CODE_SUFFIX), $(FILES))

all : debug


debug: CFLAGS += -g -DDEBUG -D_DEBUG
release: CFLAGS += -O3 -DRELEASE

debug: vdslib
release: vdslib
vdslib: $(LIBNAME)

$(LIBNAME) : $(OBJECTS)
	ar ruv $(LIBNAME) $(OBJECTS)
ifeq ($(strip $(shell uname -s)), Darwin)
	ranlib $(LIBNAME)
endif

.cpp.o:
	g++ -I/usr/X11R6/include -c -o $@ $(CFLAGS) $<

clean:
	rm -f $(OBJECTS) $(LIBNAME)

depend:
	makedepend -I. -I../../include -I../include -I../xbs -Y $(CODE)

source_release:
	make -C ./ clean
	rm -r -f stderror stdrender ply2vds vds2vif vdsview vif2vds vdsimple ply2vif
	rm -r -f ./*.a
	rm -r -f ./*.lib
	-find -name CVS -exec rm -r -f \{\} \;

# DO NOT DELETE

cut.o: cut.h vds.h zthreads.h primtypes.h renderer.h node.h simplifier.h
cut.o: nodequeue.h vdsaux.h forest.h vif.h tri.h
forestbuilder.o: vds.h zthreads.h primtypes.h forestbuilder.h forest.h
forestbuilder.o: renderer.h cut.h simplifier.h nodequeue.h vdsaux.h tri.h
forestbuilder.o: node.h vif.h
forest.o: vds.h zthreads.h primtypes.h forest.h renderer.h cut.h simplifier.h
forest.o: nodequeue.h vdsaux.h tri.h node.h vif.h forest_debug_functions.cpp
manager.o: manager.h vds.h zthreads.h primtypes.h renderer.h cut.h
manager.o: simplifier.h nodequeue.h vdsaux.h forest.h vif.h tri.h node.h
node.o: forest.h vds.h zthreads.h primtypes.h renderer.h cut.h simplifier.h
node.o: nodequeue.h vdsaux.h tri.h node.h vif.h
nodequeue.o: nodequeue.h vds.h zthreads.h primtypes.h vdsaux.h forest.h
nodequeue.o: renderer.h cut.h simplifier.h tri.h node.h vif.h
primtypes.o: primtypes.h
renderer.o: renderer.h vds.h zthreads.h primtypes.h cut.h simplifier.h
renderer.o: nodequeue.h vdsaux.h forest.h vif.h tri.h node.h manager.h
simplifier.o: simplifier.h vds.h zthreads.h primtypes.h nodequeue.h vdsaux.h
simplifier.o: forest.h renderer.h cut.h node.h vif.h tri.h
tri.o: tri.h vds.h zthreads.h primtypes.h forest.h renderer.h cut.h
tri.o: simplifier.h nodequeue.h vdsaux.h node.h vif.h
vif.o: vif.h primtypes.h vds.h zthreads.h
