# GLOD Top Level Makefile
###########################################################################
# $Id: Makefile,v 1.38 2004/10/20 19:33:23 gfx_friends Exp $
# $Revision: 1.38 $
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

# setup default targets &c
include ../Makefile.conf
VDS_DIR=./vds

# more stuff
CFLAGS += -I../include/ -I./include -I/uf6/gfx/include -fno-stack-protector
LFLAGS += -L../lib/ -fno-stack-protector

# rules

all: $(DEFAULT_TARGET)

debug: glod_all
release: glod_all

debug: TARGET_COMMAND = debug
release: TARGET_COMMAND = release


debug: CFLAGS += $(DEBUG_FLAG) -DDEBUG -D_DEBUG

release: CFLAGS += -DRELEASE
release: CFLAGS += -DRELEASE -O3

# GLOD API Files
API_SRC += 	glod_core.cpp \
		glod_glext.cpp \
		glod_group.cpp \
		glod_noop_funcs.cpp \
		glod_objects.cpp \
		GroupParams.cpp \
		ObjectParams.cpp \
		RawConvert.cpp \
		Raw.cpp \
		hash.c
API_FILES = $(addprefix ./api/, $(API_SRC))

# GLOD Hierarchy Files
ifeq ($(strip $(HWOS)), Linux)
LFLAGS += -lGL
endif 
ifeq ($(strip $(HWOS)), Darwin)
LFLAGS += -framework OpenGL -framework GLUT
endif
HIERARCHY_SRC = 
HIERARCHY_FILES = $(addprefix ./hierarchy/, $(HIERARCHY_SRC))

# XBS Files
CFLAGS += -I./xbs/
XBS_SRC = 	Continuous.C \
		Discrete.C \
		DiscretePatch.C \
		Heap.C \
		Hierarchy.C \
		Metric.C \
		MLBPriorityQueue.C \
		Model.C \
		ModelShare.C \
		Operation.C \
		SimpQueue.C \
		View.C \
		PermissionGrid.C \
		vds_callbacks.cpp
XBS_FILES = $(addprefix ./xbs/, $(XBS_SRC))

# PLY Files
PLY_SRC = plyfile.c PlyModel.c
PLY_FILES = $(addprefix ./ply/, $(PLY_SRC))

# MT Files
CFLAGS += -I./mt/
MT_SRC = 	drawMT.C \
		mt.C \
		plycollapses.C \
		sphMT.C \
		tmesh.C \
		triMT.C \
		plyMT.C
MT_FILES = $(addprefix ./mt/, $(MT_SRC))

# VDS Flags
CFLAGS += -I$(VDS_DIR)

###########################################################################
# Dynamically create all target names
GLOD_SOURCES =	$(PLY_FILES) $(MT_FILES) \
		$(XBS_FILES) $(HIERARCHY_FILES) $(API_FILES)

GLOD_OBJECTS =  $(addsuffix .$(OBJ_EXT), $(basename $(GLOD_SOURCES)))
OTHER_OBJECTS += $(wildcard $(VDS_DIR)/*.$(OBJ_EXT))	
OTHER_OBJECTS += $(VDS_DIR)/libvds.a

###########################################################################
# GLOD Build Tree

glod_all: ../lib/$(PLY_LIBRARY_NAME)
glod_all: vds_dir  # these have their own makefiles

# Build shared library
glod_all: ../lib/$(GLOD_LIBRARY_NAME)
../lib/$(GLOD_LIBRARY_NAME): $(GLOD_OBJECTS) $(OTHER_OBJECTS)
	$(CC) $(GCC_SHAREDFLAG) $(LFLAGS) -o ../lib/$(GLOD_LIBRARY_NAME) $(GLOD_OBJECTS) $(OTHER_OBJECTS)

# Build plylib which is needed by samples
../lib/$(PLY_LIBRARY_NAME): ./ply/plyfile.o ./ply/PlyModel.o
	ar ruv $@ $+
ifeq ($(strip $(HWOS)), Darwin)
	ranlib ../lib/$(PLY_LIBRARY_NAME)
endif

# Build glodlib dependencies
depend: Makefile.depend
Makefile.depend:
	if ! test -e $@; then touch $@; fi
	-makedepend -f Makefile.depend -Y $(CFLAGS) $(GLOD_SOURCES) 2&>/dev/null


# Other building code
vds_dir:
	make -C ./vds $(TARGET_COMMAND)

doc:
	make -C doc $(TARGET_COMMAND)

# Other routines for cleanup
clean:
	rm -f Makefile.depend*
	rm -f ../lib/$(GLOD_LIBRARY_NAME)
	rm -rf $(GLOD_OBJECTS)
	make -C xbs clean_xbs                # hack to allow XBS to build itself as well
	make -C $(VDS_DIR) clean
	make -C doc clean

clean_api:
	rm -f $(addsuffix .$(OBJ_EXT), $(basename $(API_FILES)))

clean_hierarchy:
	rm -f $(addsuffix .$(OBJ_EXT), $(basename $(HIERARCHY_FILES)))

clean_xbs:
	rm -f $(addsuffix .$(OBJ_EXT), $(basename $(XBS_FILES)))

clean_mt:
	rm -f $(addsuffix .$(OBJ_EXT), $(basename $(MT_FILES)))

source_release:
	make -C ./ clean
	make -C samples/ source_release
	-find ../ -name CVS -exec rm -r -f \{\} \;

include Makefile.depend

###########################################################################
# $Log: Makefile,v $
# Revision 1.38  2004/10/20 19:33:23  gfx_friends
# Rewrote MTHierarchy driver, made the VDS hack in Operation.C vds-specific
#
# Revision 1.37  2004/08/04 22:26:27  gfx_friends
# Miscellaneous patches, including more stuff added to the release notes.
#
# Revision 1.36  2004/07/21 15:31:08  gfx_friends
# Bugfixes. -n
#
# Revision 1.35  2004/07/19 19:18:41  gfx_friends
# Fixes to MacOSX command line build and also removed ancient references to GeomLOD, which was our original In-Chromium implementation. -n
#
# Revision 1.34  2004/07/14 19:47:57  gfx_friends
# Near and far plane estimation on simple.
#
# Revision 1.33  2004/06/29 12:57:14  gfx_friends
# JC: Made things compile under Linux with Chris Niski's error metric stuff.
#
# Revision 1.32  2004/06/11 21:15:05  gfx_friends
# fixed a makefile to work with osx
#
# Revision 1.31  2004/06/11 19:56:37  gfx_friends
# Added release targets to Linux makefiles. -nat
#
# Revision 1.30  2004/06/11 06:45:19  gfx_friends
# Wrote makefiles for the samples directory and top level directory.
#
# Revision 1.29  2004/06/10 22:53:12  gfx_friends
# Fixed makefile to compile glod/src
#
# Revision 1.28  2004/06/10 16:13:37  gfx_friends
# Added a Multi-level Bucket (MLB) Priority Queue to replace the
# standard binary heap as the main xbs priority queue. It makes use of
# the fact that the xbs priority queue is mostly monotonic, with newly
# inserted items almost always having larger keys than the key of the
# item at the top of the heap. It seems to be about 75% faster than the
# binary heap, so the priority queue is no longer a major bottleneck,
# even for the current fast error metric computation.
#
# Revision 1.27  2004/06/01 19:40:42  gfx_friends
# Changed the main makefile a bit, links in vdslib directly, because the samples do not compile otherwise
#
# Revision 1.26  2004/02/06 17:25:08  gfx_friends
# Discrete_manual mkIII plus one or two small changes to the makefiles
#
# Revision 1.25  2004/02/05 20:34:07  gfx_friends
# Modified Discrete and Continuous and Hierarchy to use glod_glext.h instead of their own custom VBO code.
#
# Revision 1.24  2004/02/05 20:04:12  gfx_friends
# Added glod_glext.h which provides top-level _glBindBufferARB (etc) functions. Note the underscore at the front.... this is to prevent collissions with some systems who'se gl.h defines ARB functions. These are initialized at bootup. A glodHasVBO() function is useful for dynamic use of them. --nat
#
# Revision 1.23  2004/02/05 18:31:41  gfx_friends
# Changed the makefile system for Linux & MacOz. There is a single toplevel makefile that controls most of the builds... but xbs & vds still are untouched. --nat
#
# Revision 1.22  2004/02/04 18:55:41  gfx_friends
# fixed makefiles to work proper using macs... also small change to the sample programs, which would prevent them from running in osx
#
# Revision 1.21  2003/08/28 19:01:46  gfx_friends
# Created a source_release target for releasing GLOD source code.
#
# Revision 1.20  2003/08/27 20:29:10  gfx_friends
# Fixed a bug in screen-space-error computation and improved the functionality of GLOD_App.
#
# Revision 1.19  2003/07/26 01:17:12  gfx_friends
# Fixed copyright notice. Added wireframe to sample apps. Minor
# revisions to documentation.
#
# Revision 1.18  2003/07/23 19:55:25  gfx_friends
# Added copyright notices to GLOD. I'm making a release.
#
# Revision 1.17  2003/07/23 00:24:45  gfx_friends
# A bunch of usability patches.
#
# Revision 1.16  2003/07/21 23:10:50  gfx_friends
# Added cut readback support. I'm still debugging, but need to move computers to my home. --n
#
# Revision 1.15  2003/07/21 03:07:56  gfx_friends
# Trying to break the dependencies between the samples directory and the top level directory so that I can distribute /samples without the whole source tree. -n
#
# Revision 1.14  2003/07/18 23:43:25  gfx_friends
# Detritus.
#
# Revision 1.13  2003/07/18 23:28:02  gfx_friends
# New release codes.
#
# Revision 1.12  2003/07/15 20:18:52  gfx_friends
# Major documentation effort and basic distribution readiness. We now have pod-based documentation for each GLOD function. It will build HTML or Man pages on Linux. To use the man pages, append glod/doc/man to your manpath after running make in doc or doing a top-level make. Also new is a release target... a top level make release builds with -O2 and any flags you also set based on the release target (See glod.conf). Also, #define DEBUG is active when building for debug.
#
# Revision 1.11  2003/07/09 22:50:02  gfx_friends
# Major documentation effort and minor API changes. On the API change side,
# GLODBuildObject now recieves the format flag for an object being built, while LoadObject now requires NewObject to
# have been called before it can be called. NewObject simply creates an object and group.
#
# On the documentation side, the sources in ./api now contain a ton of inline comments which document
# the API routines using Doxygen tagging syntax. A top-level makefile target, docs, allows you to build HTML documentation out of these files. When I've finished the documentation, we can also make the same comments go to UNIX Man pages and Windows RTF/HTML help files. I'm still documenting the API. However, if you run make docs on a linux box or some PC with Doxygen installed on it, you'll get the docs and can check them out.
#
# Cheers,
#
# -- Nat
#
# Revision 1.10  2003/07/01 20:49:14  gfx_friends
# Readback for Discrete LOD now works. See samples/readback/read_model for demo and test. This, parenthetically, is also the most fully functional test program that we have. I will bring "simple" up to speed eventually, but in the meantime, you can use read_model similarly (run --help) in much the same way as previous ones to do testing.
#
# Revision 1.9  2003/06/09 18:55:55  gfx_friends
# Moved mt/lib to mt/, in an effort to further clean up. Also, turned mt/domake into a makefile, which makes compilation a little prettier although no different. -- Nat
#
# Revision 1.8  2003/06/09 18:24:13  gfx_friends
# Major change: moved around directory names and removed stale directories left over from our previous attempts at GLOD. The 'core' directory, which binds the GLOD API to our sublibraries of VDS,XBS, and MT, is now called 'API'. We also now have a 'samples' directory which contains 'simple', an insulting spinner program to verify basic operation, as well as 'scene', which is a weak scene graph program that ultimately should verify some more complex functionality of GLOD. A few other commits are going to be necessary to finalize these changes; however, this brings GLOD to a point that is more in line with a distributable piece of software; -- Nat
#
# Revision 1.7  2003/06/06 18:25:34  gfx_friends
# Fixed the unix build; there is a wierd hack with GL.h to make us work with GLh.h
#
# Revision 1.6  2003/06/05 20:46:17  gfx_friends
# Now, everything compiles into libGLOD.so --- no other libraries are needed
#
# Revision 1.5  2003/06/04 16:59:02  gfx_friends
# Chromium removal.
#
# Revision 1.4  2003/01/15 00:03:34  gfx_friends
# Patch to f_headers.
#
# Revision 1.3  2003/01/14 23:58:08  gfx_friends
# File.
#
# Revision 1.2  2003/01/14 22:50:24  gfx_friends
# Major restructure of source organization (nat)
#
# Revision 1.1.1.1  2003/01/02 21:21:14  duca
# First checkin: (1) Chromium SPU, (2) standalone library, (3) build system
#   -- Nat
#
###########################################################################
