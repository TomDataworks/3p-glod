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

# edit makefile.conf to add files!
# don't forget to add them to CVS!

HTML_GLOBALS = header.inc footer.inc

MAN_FILES+= glod \
            glodInit \
            glodShutdown \
            glodGetError \

MAN_FILES+=glodNewObject \
           glodInstanceObject \
           glodBuildObject \
           glodDeleteObject \
           glodObjectXform \
           glodBindObjectXform \
           glodReadbackObject \
           glodLoadObject \
           glodInsertArrays \
           glodInsertElements \
           glodFillArrays \
           glodFillElements \
           glodDrawPatch \
           glodObjectParameter \
           glodGetObjectParameter \

MAN_FILES+=glodNewGroup \
           glodDeleteGroup \
           glodAdaptGroup \
           glodGroupParameter \
           glodGetGroupParameter \

MAN_SEC=3
MAN_DST=../../doc/man/man3/

HTML_DST=../../doc/

NPP_DST=../../doc/                  # where the formatted html files go

TITLE=GLOD Documentation
RELEASE=GLOD 1.0

##################################
# Auto section
POD_FILES = $(addsuffix .pod,        $(basename $(MAN_FILES)))
POD_GARB  = $(addsuffix .garb,       $(basename $(MAN_FILES)))

POD_MANS  = $(addsuffix .$(MAN_SEC), $(basename $(MAN_FILES)))
POD_HTMLS = $(addsuffix .html,       $(basename $(MAN_FILES)))

MAN__DST  = $(dir $(MAN_DST))
HTML__DST = $(dir $(HTML_DST))
NPP__DST  = $(dir $(NPP_DST))

DATE=$(shell date +%D\ %r)

##############################################################################
##############################################################################
##############################################################################

all: man html

drop:
	@echo Removing all POD files becase you\'ve got them in CVS and because Windows is a horrible operating system and because your perl installation is broke.
	rm -f *.pod
	rm -f npp_html/*.inc

##############################################################################
# html target
##############################################################################

html: $(addprefix $(HTML__DST), $(POD_HTMLS)) other_html

other_html:
	@# call list
	perl -w ./mkCallList $(addprefix $(HTML__DST), $(POD_HTMLS)) > calls.npp
	perl -w ../../util/npp calls.npp $(NPP__DST)calls.html -D DATE "$(DATE)"
	@rm calls.npp

	@# linkify everything
	perl -w ./linkify $(addprefix $(NPP__DST), $(basename $(MAN_FILES))) $(addprefix $(NPP__DST), calls)
	
	@# more stuff
	cp $(NPP__DST)glod.html $(NPP__DST)index.html
	@rm -f pod2htm?.*

../../doc/%.html : %.pod $(HTML_GLOBALS)
	@# make the HTML file (.ht)
	pod2html --htmlroot=./ --infile=$(basename $(notdir $@)).pod --outfile=./$*.ht --podroot=./ --title="$(TITLE): $(basename $(notdir $@))" --noindex	
		
	@# strip the HTML and add markup
	@echo \#define TITLE GLOD Documentation: $* > ./$*.npp
	@echo \#include header.inc >> ./$*.npp
	perl -w ./htmlStrip ./$*.ht >> ./$*.npp
	@echo \#include footer.inc >> ./$*.npp

	@# process the NPP file into the final HTML file
	perl -w ../../util/npp ./$*.npp $@  -D DATE "$(DATE)"	

	@# cleanup
	@rm $*.npp
	@rm $*.ht
	@echo

##############################################################################
# other shtuff
##############################################################################
clean:
	/bin/rm -r -f $(MAN_COMPONENTS)
	/bin/rm -r -f $(HTML_DST)

check: $(POD_GARB)
	@echo All OK.

%.garb: %.pod
	@podchecker $<

# Man pages
##############################################################################

man: $(addprefix $(dir $(MAN_DST)), $(POD_MANS))
	@if ! test -e $(MAN__DST)glodGetGroupParameteriv.3; then ln -s glodGetGroupParameter.3 $(MAN__DST)glodGetGroupParameteriv.3; fi
	@if ! test -e $(MAN__DST)glodGetGroupParameterfv.3; then ln -s glodGetGroupParameter.3 $(MAN__DST)glodGetGroupParameterfv.3; fi
	@if ! test -e $(MAN__DST)glodGetObjectParameteriv.3; then ln -s glodGetObjectParameter.3 $(MAN__DST)glodGetObjectParameteriv.3; fi
	@if ! test -e $(MAN__DST)glodGetObjectParameterfv.3; then ln -s glodGetObjectParameter.3 $(MAN__DST)glodGetObjectParameterfv.3; fi

	@if ! test -e $(MAN__DST)glodObjectParameteri.3; then ln -s glodObjectParameter.3 $(MAN__DST)glodObjectParameteri.3; fi
	@if ! test -e $(MAN__DST)glodObjectParameterf.3; then ln -s glodObjectParameter.3 $(MAN__DST)glodObjectParameterf.3; fi
	@if ! test -e $(MAN__DST)glodGroupParameteri.3; then ln -s glodGroupParameter.3 $(MAN__DST)glodGroupParameteri.3; fi
	@if ! test -e $(MAN__DST)glodGroupParameterf.3; then ln -s glodGroupParameter.3 $(MAN__DST)glodGroupParameterf.3; fi

%.$(MAN_SEC): ../../../src/doc/%.pod
	pod2man --section=$(MAN_SEC) --release="$(RELEASE)" --center="$(TITLE)" -d "Today" $(basename $(notdir $@)).pod $@
