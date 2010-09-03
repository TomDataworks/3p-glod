# GLOD Top Level Makefile
##############################################################################
# $Id: Makefile,v 1.5 2004/07/19 19:18:40 gfx_friends Exp $
# $Revision: 1.5 $
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

# setup default Cflags, Lflags, HWOS
include Makefile.conf

all: $(DEFAULT_TARGET)


debug: TARGET_COMMAND=debug
release: TARGET_COMMAND=release

debug: all_files
release: all_files

##############################################################################
all_files: d_lib d_samples

d_samples:
	make -C ./samples/ $(TARGET_COMMAND)

d_lib:
	make -C ./src/ $(TARGET_COMMAND)

clean:
	make -C ./src clean
	make -C ./samples clean
	

##############################################################################
# $Log: Makefile,v $
# Revision 1.5  2004/07/19 19:18:40  gfx_friends
# Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
#
# Revision 1.4  2004/06/11 19:56:36  gfx_friends
# Added release targets to Linux makefiles. -nat
#
# Revision 1.3  2004/06/11 19:29:30  gfx_friends
# First, sorry about all the commit messages. I'm sure you've got better things to do than listen to me type when I'd really prefer to be chasing bugs. But at any rate, this contains a fix to the makefile at the top level.
#
# Revision 1.2  2004/06/11 06:45:15  gfx_friends
# Wrote makefiles for the samples directory and top level directory.
#
# Revision 1.1  2004/06/10 20:49:00  gfx_friends
# Added top level makefile.
#
##############################################################################
