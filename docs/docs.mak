#==============================================================================
#
#    Documentation generation makefile
#    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this library; if not, write to the Free
#    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
#------------------------------------------------------------------------------
# docs.mak
#
#	A makefile for generating various output formats of the Crystal
#	Space documentation.
#
# WARNING:
#	This is a highly complex makefile.  Please be certain that you
#	understand it thoroughly before attempting any modifications.
#------------------------------------------------------------------------------

# Target descriptions
DESCRIPTION.devapi = developer API reference via Doxygen
DESCRIPTION.pubapi = public API reference via Doxygen
DESCRIPTION.htmldoc = user manual as HTML
DESCRIPTION.dvidoc = user manual as DVI
DESCRIPTION.psdoc = user manual as PostScript
DESCRIPTION.pdfdoc = user manual as PDF
DESCRIPTION.infodoc = user manual as Info
DESCRIPTION.repairdoc = Texinfo @node and @menu directives
DESCRIPTION.chmsupp = MS HTML Help support files (projects, TOCs, index)
# For 'cleandoc' target
DESCRIPTION.doc = generated documentation

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
DOCHELP += \
  $(NEWLINE)echo $"  make devapi       Make the $(DESCRIPTION.devapi)$" \
  $(NEWLINE)echo $"  make pubapi       Make the $(DESCRIPTION.pubapi)$" \
  $(NEWLINE)echo $"  make htmldoc      Make the $(DESCRIPTION.htmldoc)$" \
  $(NEWLINE)echo $"  make dvidoc       Make the $(DESCRIPTION.dvidoc)$" \
  $(NEWLINE)echo $"  make psdoc        Make the $(DESCRIPTION.psdoc)$" \
  $(NEWLINE)echo $"  make pdfdoc       Make the $(DESCRIPTION.pdfdoc)$" \
  $(NEWLINE)echo $"  make infodoc      Make the $(DESCRIPTION.infodoc)$" \
  $(NEWLINE)echo $"  make chmsupp      Make the $(DESCRIPTION.chmsupp)$" \
  $(NEWLINE)echo $"  make repairdoc    Repair $(DESCRIPTION.repairdoc)$" \
  $(NEWLINE)echo $"  make cleandoc     Clean all $(DESCRIPTION.doc)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: devapi pubapi htmldoc dvidoc psdoc pdfdoc infodoc repairdoc cleandoc chmsupp

devapi pubapi htmldoc dvidoc psdoc pdfdoc infodoc chmsupp:
	$(MAKE_TARGET) DO_DOC=yes

repairdoc:
	@echo $(SEPARATOR)
	@echo $"  Repairing $(DESCRIPTION.$@)$"
	@echo $(SEPARATOR)
	@$(MAKE) $(RECMAKEFLAGS) -f mk/cs.mak $@

cleandoc:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

NODEFIX = docs/support/nodefix.pl
TEXI2HTML = docs/support/texi2html
TEXI2DVI = texi2dvi
DVIPS = dvips
PS2PDF = ps2pdf
MAKEINFO = makeinfo
DOXYGEN = doxygen

# Root of the entire Crystal Space manual.
CSMANUAL_DIR  = docs/texinfo
CSMANUAL_FILE = cs-unix.txi

# texi2html configuration file.
TEXI2HTMLINIT = docs/support/texi2html.init

# Doxygen configuration files.
DOXYGEN_PUBAPI = docs/support/pubapi.dox
DOXYGEN_DEVAPI = docs/support/devapi.dox

# Copy additional images to doxygen output dir
OUT.DOXYGEN.IMAGES = $(CP) docs/support/*.jpg 

# Root of the target directory hierarchy.
OUT.DOC = $(OUTBASE)/docs

# Relative path which refers to main CS directory from within one of the
# specific output directories, such as $(OUT.DOC.HTML).  The value of this
# variable must reflect the value of $(OUT.DOC) and $(OUTBASE)/.
OUT.DOC.UNDO = ../../..

# This section is specially protected by DO_DOC in order to prevent the lengthy
# $(wildcard) operations from impacting _all_ other build targets.  DO_DOC is
# only defined when a top-level documentaiton target is invoked.

ifeq ($(DO_DOC),yes)

# Target directory for each output format.
OUT.DOC.API.DEV = $(OUT.DOC)/devapi
OUT.DOC.API.PUB = $(OUT.DOC)/pubapi
OUT.DOC.HTML    = $(OUT.DOC)/html
OUT.DOC.DVI     = $(OUT.DOC)/dvi
OUT.DOC.PS      = $(OUT.DOC)/ps
OUT.DOC.PDF     = $(OUT.DOC)/pdf
OUT.DOC.INFO    = $(OUT.DOC)/info

# DVI log file
DOC.DVI.LOG = $(OUT.DOC.DVI)/$(addsuffix .log,$(basename $(CSMANUAL_FILE)))

# List of potential image types understood by Texinfo's @image{} directive.
DOC.IMAGE.EXTS = png jpg gif eps txt

# Source and target image lists.
DOC.IMAGE.LIST = $(strip \
  $(wildcard $(addprefix $(CSMANUAL_DIR),$(foreach ext,$(DOC.IMAGE.EXTS),\
  $(addsuffix $(ext),* */* */*/* */*/*/* */*/*/*/* */*/*/*/*/*)))))
OUT.DOC.IMAGE.LIST = $(subst $(CSMANUAL_DIR)/,,$(DOC.IMAGE.LIST))

# Manual decomposition of image directories achieved by progressively
# stripping off one layer of subdirectories at a time.
OUT.DOC.IMAGE.DIRS.0 := $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.LIST)))
OUT.DOC.IMAGE.DIRS.1 := $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.DIRS.0)))
OUT.DOC.IMAGE.DIRS.2 := $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.DIRS.1)))
OUT.DOC.IMAGE.DIRS.3 := $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.DIRS.2)))
OUT.DOC.IMAGE.DIRS.4 := $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.DIRS.3)))
OUT.DOC.IMAGE.DIRS.5 := $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.DIRS.4)))
OUT.DOC.IMAGE.DIRS.6 := $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.DIRS.5)))
OUT.DOC.IMAGE.DIRS.7 := $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.DIRS.6)))
OUT.DOC.IMAGE.DIRS.8 := $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.DIRS.7)))
OUT.DOC.IMAGE.DIRS.9 := $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.DIRS.8)))

# Recomposition of image directory components in an ordered list which can be
# fed to $(MKDIR) in order to recreate entire source image directory hiearchy
# within target directory.
OUT.DOC.IMAGE.DIRS.ALL := $(sort $(filter-out .,\
  $(OUT.DOC.IMAGE.DIRS.0) \
  $(OUT.DOC.IMAGE.DIRS.1) \
  $(OUT.DOC.IMAGE.DIRS.2) \
  $(OUT.DOC.IMAGE.DIRS.3) \
  $(OUT.DOC.IMAGE.DIRS.4) \
  $(OUT.DOC.IMAGE.DIRS.5) \
  $(OUT.DOC.IMAGE.DIRS.6) \
  $(OUT.DOC.IMAGE.DIRS.7) \
  $(OUT.DOC.IMAGE.DIRS.8) \
  $(OUT.DOC.IMAGE.DIRS.9)))

# Recomposed image directory components for each potential output format.
# Used to create target directory hiearchy when generating a particular format.
OUT.DOC.IMAGE.DIRS.HTML = \
  $(addprefix $(OUT.DOC.HTML)/,$(OUT.DOC.IMAGE.DIRS.ALL))
OUT.DOC.IMAGE.DIRS.DVI  = \
  $(addprefix $(OUT.DOC.DVI)/,$(OUT.DOC.IMAGE.DIRS.ALL))
OUT.DOC.IMAGE.DIRS.PS   = \
  $(addprefix $(OUT.DOC.PS)/,$(OUT.DOC.IMAGE.DIRS.ALL))
OUT.DOC.IMAGE.DIRS.PDF  = \
  $(addprefix $(OUT.DOC.PDF)/,$(OUT.DOC.IMAGE.DIRS.ALL))
OUT.DOC.IMAGE.DIRS.INFO = \
  $(addprefix $(OUT.DOC.INFO)/,$(OUT.DOC.IMAGE.DIRS.ALL))

# List of top-level image directories only.  List is composed by filtering out
# directory names containing a slash, thus leaving only top-level names.
# Used to remove target image directories for output formats which do not
# require presence of images after conversion is complete.
OUT.DOC.IMAGE.DIRS.TOP := $(filter-out $(foreach dir,\
  $(dir $(OUT.DOC.IMAGE.DIRS.ALL)),$(dir)%),$(OUT.DOC.IMAGE.DIRS.ALL))

endif # ifeq ($(DO_DOC),yes)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

# This section is specially protected by DO_DOC in order to prevent the lengthy
# $(wildcard) operations from impacting _all_ other build targets.  DO_DOC is
# only defined when a top-level documentaiton target is invoked.

ifeq ($(DO_DOC),yes)

.PHONY: devapi pubapi htmldoc dvidoc psdoc pdfdoc infodoc
.PHONY: do-devapi do-pubapi do-htmldoc do-dvidoc do-infodoc

# Rules for making output and image directories.
$(OUT.DOC): $(OUTBASE)
	-$(MKDIR)

$(OUT.DOC.API.DEV) $(OUT.DOC.API.PUB) $(OUT.DOC.HTML) $(OUT.DOC.DVI) \
$(OUT.DOC.PS) $(OUT.DOC.PDF) $(OUT.DOC.INFO): $(OUT.DOC)
	-$(MKDIR)

$(OUT.DOC.IMAGE.DIRS.HTML) $(OUT.DOC.IMAGE.DIRS.DVI) $(OUT.DOC.IMAGE.DIRS.PS) \
$(OUT.DOC.IMAGE.DIRS.PDF)  $(OUT.DOC.IMAGE.DIRS.INFO):
	$(MKDIR)

# Rule for removing a particular directory.  To remove directory "foo",
# specify "foo.CLEAN" as dependency of some target.
%.CLEAN:
	$(RMDIR) $*

# Rule for copying $(CSMANUAL_FILE) to a target directory.  To copy file to
# directory "foo", specify "foo.SOURCE" as dependency of some target.
%.SOURCE:
	$(CP) $(CSMANUAL_DIR)/$(CSMANUAL_FILE) $*

# Rule for removing $(CSMANUAL_FILE) from a target directory.  To remove file
# from directory "foo", specify "foo.ZAPSOURCE" as dependency of some target.
%.ZAPSOURCE:
	$(RM) $*/$(CSMANUAL_FILE)

# Rule for copying a selected subset of the source image list to a target
# directory.  To copy images of type "png", "jpg", and "gif" to directory
# "foo", specify "foo/png.jpg.gif.IMAGES" as dependency of some target.
%.IMAGES:
ifeq ($(DOC.IMAGE.LIST),)
	@echo "No images to copy."
else
	@echo "Copying images." $(foreach file,$(filter $(foreach \
	ext,$(subst ., ,$(notdir $*)),%.$(ext)),$(DOC.IMAGE.LIST)),\
	$(NEWLINE)$(CP) $(file) $(subst $(CSMANUAL_DIR)/,$(dir $*),$(file)))
endif

# Rule for removing a selected subset of the source image list from a target
# directory.  To remove images of type "png", "jpg", and "gif" from directory
# "foo", specify "foo/png.jpg.gif.ZAPIMAGES" as dependency of some target.
%.ZAPIMAGES:
	@echo "Purging images."
ifneq ($(OUT.DOC.IMAGE.LIST),)
	@$(RM) $(addprefix $(dir $*),$(filter $(foreach \
	ext,$(subst ., ,$(notdir $*)),%.$(ext)),$(OUT.DOC.IMAGE.LIST)))
ifneq ($(OUT.DOC.IMAGE.DIRS.TOP),)
	@$(RMDIR) $(addprefix $(dir $*),$(OUT.DOC.IMAGE.DIRS.TOP))
endif
endif

# Rules to generate developer API documentation from all header files.
do-devapi: $(OUT.DOC.API.DEV)
	$(OUT.DOXYGEN.IMAGES) $(OUT.DOC.API.DEV)
	$(DOXYGEN) $(DOXYGEN_DEVAPI)

devapi: $(OUT.DOC.API.DEV).CLEAN do-devapi

# Rules to generate public API documentation from public header files.
do-pubapi: $(OUT.DOC.API.PUB)
	$(OUT.DOXYGEN.IMAGES) $(OUT.DOC.API.PUB)
	$(DOXYGEN) $(DOXYGEN_PUBAPI)

pubapi: $(OUT.DOC.API.PUB).CLEAN do-pubapi

# Rule to perform actual HTML conversion of $(CSMANUAL_FILE).
do-htmldoc:
	$(CD) $(OUT.DOC.HTML); $(PERL) $(OUT.DOC.UNDO)/$(TEXI2HTML) \
	-init_file $(OUT.DOC.UNDO)/$(TEXI2HTMLINIT) -prefix cs \
	-I $(OUT.DOC.UNDO)/$(CSMANUAL_DIR) $(CSMANUAL_FILE)

# Rule to generate HTML format output.  Target images are retained since
# generated HTML files reference them.
htmldoc: \
  $(OUT.DOC.HTML).CLEAN \
  $(OUT.DOC.HTML) \
  $(OUT.DOC.IMAGE.DIRS.HTML) \
  $(OUT.DOC.HTML).SOURCE \
  $(OUT.DOC.HTML)/png.jpg.gif.IMAGES \
  do-htmldoc \
  $(OUT.DOC.HTML).ZAPSOURCE

# Rule to perform actual DVI conversion of $(CSMANUAL_FILE).
do-dvidoc:
	$(CP) $(CSMANUAL_DIR)/texinfo.tex $(OUT.DOC.DVI)
	$(CD) $(OUT.DOC.DVI); $(TEXI2DVI) --batch --quiet -I '.' -I `pwd` \
	-I `$(CD) $(OUT.DOC.UNDO)/$(CSMANUAL_DIR); $(PWD)` $(CSMANUAL_FILE)
	$(MV) $(OUT.DOC.DVI)/$(addsuffix .dvi,$(basename $(CSMANUAL_FILE))) \
	$(OUT.DOC.DVI)/cs.dvi
	$(RM) $(OUT.DOC.DVI)/texinfo.tex

# Rule to generate DVI format output.  Target images are retained since
# generated DVI file references them.
dvidoc: \
  $(OUT.DOC.DVI).CLEAN \
  $(OUT.DOC.DVI) \
  $(OUT.DOC.IMAGE.DIRS.DVI) \
  $(OUT.DOC.DVI).SOURCE \
  $(OUT.DOC.DVI)/eps.IMAGES \
  do-dvidoc \
  $(OUT.DOC.DVI).ZAPSOURCE
	@echo $"$"
	@echo $">>> Documentation conversion completed without errors, however$"
	@echo $">>> check $(DOC.DVI.LOG) for warnings about formatting problems.$"
	@echo $"$"

# Rule to perform actual PS conversion from DVI file.
do-psdoc:
	$(CD) $(OUT.DOC.DVI); \
	$(DVIPS) -q -o $(OUT.DOC.UNDO)/$(OUT.DOC.PS)/cs.ps cs.dvi

# Rule to generate PS format output.  Target images are incorporated directly
# into PostScript file from within DVI target directory.
psdoc: \
  $(OUT.DOC.PS).CLEAN \
  $(OUT.DOC.PS) \
  dvidoc \
  do-psdoc
	@echo $"$"
	@echo $">>> Documentation conversion completed without errors, however$"
	@echo $">>> check $(DOC.DVI.LOG) for warnings about formatting problems.$"
	@echo $"$"

# Rule to perform actual PDF conversion from PS file.
do-pdfdoc:
	$(PS2PDF) $(OUT.DOC.PS)/cs.ps $(OUT.DOC.PDF)/cs.pdf

# Rule to generate PDF format output.
pdfdoc: \
  $(OUT.DOC.PDF).CLEAN \
  $(OUT.DOC.PDF) \
  psdoc \
  do-pdfdoc
	@echo $"$"
	@echo $">>> Documentation conversion completed without errors, however$"
	@echo $">>> check $(DOC.DVI.LOG) for warnings about formatting problems.$"
	@echo $"$"

# Rule to perform actual Info conversion of $(CSMANUAL_FILE).
do-infodoc:
	$(CD) $(OUT.DOC.INFO); $(MAKEINFO) -I $(OUT.DOC.UNDO)/$(CSMANUAL_DIR) \
	--output=cs $(CSMANUAL_FILE)

# Rule to generate Info format output.  Target images are removed after
# conversion since images are incorporated directly into generated Info files.
infodoc: \
  $(OUT.DOC.INFO).CLEAN \
  $(OUT.DOC.INFO) \
  $(OUT.DOC.IMAGE.DIRS.INFO) \
  $(OUT.DOC.INFO).SOURCE \
  $(OUT.DOC.INFO)/txt.IMAGES \
  do-infodoc \
  $(OUT.DOC.INFO).ZAPSOURCE \
  $(OUT.DOC.INFO)/txt.ZAPIMAGES

chmsupp: 
	$(CD) $(OUT.DOC); $(PERL) -I../../docs/support/winhelp ../../docs/support/winhelp/gendoctoc.pl
	$(CP) docs/support/winhelp/csapi.hhp $(OUT.DOC)
	$(CP) docs/support/winhelp/csdocs.hhp $(OUT.DOC)

endif # ifeq ($(DO_DOC),yes)

# Repair out-of-date and broken @node and @menu directives in Texinfo source.
.PHONY: repairdoc
repairdoc:
	$(PERL) $(NODEFIX) --include-dir=$(CSMANUAL_DIR) $(CSMANUAL_FILE)

# Remove all target documentation directories.
.PHONY: cleandoc
clean: cleandoc
cleandoc:
	$(RMDIR) $(OUT.DOC)

endif # ifeq ($(MAKESECTION),targets)
