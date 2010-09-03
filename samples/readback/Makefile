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
TOP=../
include ../samples.conf

PROG1 = $(BIN_DST)read_model
FILES1 = read_model

PROG2 = $(BIN_DST)read_cut
FILES2 = read_cut

CFLAGS+=-g -Wall
ifneq ($(strip $(OSTYPE)),Darwin)
LFLAGS+=-lGL -lglut -lGLU -lXi -lXmu
else
LFLAGS+= -lobjc.A -framework OpenGL -framework GLUT
endif
LFLAGS+=-lGLOD -lply

# Stuff
files :  $(PROG1) $(PROG2)

$(PROG1): $(addsuffix .o, $(FILES1))
	g++ -o $(PROG1) $(addsuffix .o, $(FILES1)) $(LFLAGS)

$(PROG2): $(addsuffix .o, $(FILES2))
	g++ -o $(PROG2) $(addsuffix .o, $(FILES2)) $(LFLAGS)


clean : 
	rm -f $(PROG1) $(PROG2) $(addsuffix .o, $(FILES1)) $(addsuffix .o, $(FILES2))
	rm -f *.sbr *.pch *.pdb *.plg *.ilk *.dll *.exe *.idb *.obj

%.o: %.c 
	gcc -c -o $@ $(CFLAGS) $<
%.o: %.cpp
	g++ -c -o $@ $(CFLAGS) $<

depend:
	makedepend -I. -Y $(addsuffix .c, $(FILES))

# DO NOT DELETE
