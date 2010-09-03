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

TOP=./
include samples.conf

all: files
release: files
debug: files

files: $(BIN_DST)simple $(BIN_DST)readback $(BIN_DST)scene

$(BIN_DST)simple:
	make -C simple


$(BIN_DST)readback:
	make -C readback

$(BIN_DST)scene:
	make -C scene

$(BIN_DST)mesh:
	make -C mesh

clean:
	make -C simple clean
	make -C readback clean
	make -C scene clean
	make -C mesh clean
	rm -f Samples.ncb Samples.opt

source_release:
	make -C ./ clean
