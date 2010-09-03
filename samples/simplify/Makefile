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

PROG = $(BIN_DST)simplify
FILES = simplify

ifneq ($(strip $(OSTYPE)),Darwin)
LFLAGS+=-lGL -lglut -lGLU -lXi -lXmu -lGLOD -lply
else
LFLAGS+= -framework Foundation -lGLOD -lply -framework OpenGL -framework GLUT
endif
CFLAGS+=$(DEBUG_FLAG) -Wall

# App Rules
files :  $(PROG)

$(PROG): $(addsuffix .o, $(FILES))
	g++ -o $@ $? $(LFLAGS)

clean : 
	rm -f $(PROG) $(addsuffix .o, $(FILES))
	rm -f *.sbr *.pch *.pdb *.plg *.ilk *.dll *.exe *.idb *.obj

%.o: %.c
	gcc -c -o $@ $(CFLAGS) $<

%.o: %.cpp
	g++ -c -o $@ $(CFLAGS) $<

depend:
	makedepend -I. -Y $(addsuffix .c, $(FILES))

# DO NOT DELETE
