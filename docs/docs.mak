#==============================================================================
#
#    Documentation generation makefile
#    Copyright (C) 2000-2004 by Eric Sunshine <sunshine@sunshineco.com>
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
DESCRIPTION.apihtml    = public API reference at HTML
DESCRIPTION.apichm     = public API reference as MS compressed HTML
DESCRIPTION.apidevhtml = developer API reference as HTML
DESCRIPTION.manualhtml = user manual as HTML
DESCRIPTION.manualdvi  = user manual as DVI
DESCRIPTION.manualps   = user manual as PostScript
DESCRIPTION.manualpdf  = user manual as PDF
DESCRIPTION.manualinfo = user manual as Info
DESCRIPTION.manualchm  = user manual as MS compressed HTML
DESCRIPTION.repairdoc  = Texinfo @node and @menu directives
# For 'cleandoc' target
DESCRIPTION.doc = generated documentation

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
DOCHELP += \
  $(NEWLINE)echo $"  make apihtml      Make the $(DESCRIPTION.apihtml)$" \
  $(NEWLINE)echo $"  make apichm       Make the $(DESCRIPTION.apichm)$" \
  $(NEWLINE)echo $"  make apidevhtml   Make the $(DESCRIPTION.apidevhtml)$" \
  $(NEWLINE)echo $"  make manualhtml   Make the $(DESCRIPTION.manualhtml)$" \
  $(NEWLINE)echo $"  make manualdvi    Make the $(DESCRIPTION.manualdvi)$" \
  $(NEWLINE)echo $"  make manualps     Make the $(DESCRIPTION.manualps)$" \
  $(NEWLINE)echo $"  make manualpdf    Make the $(DESCRIPTION.manualpdf)$" \
  $(NEWLINE)echo $"  make manualinfo   Make the $(DESCRIPTION.manualinfo)$" \
  $(NEWLINE)echo $"  make manualchm    Make the $(DESCRIPTION.manualchm)$" \
  $(NEWLINE)echo $"  make repairdoc    Repair $(DESCRIPTION.repairdoc)$" \
  $(NEWLINE)echo $"  make cleandoc     Clean all $(DESCRIPTION.doc)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: apihtml apichm apidevhtml manualhtml manualdvi manualps manualpdf \
  manualinfo manualchm repairdoc cleandoc manualchm

apihtml apichm apidevhtml manualhtml manualdvi manualps manualpdf manualinfo \
manualchm:
	$(MAKE_TARGET) DO_DOC=yes

repairdoc:
	@echo $(SEPARATOR)
	@echo $"  Repairing $(DESCRIPTION.$@)$"
	@echo $(SEPARATOR)
	@$(MAKE) $(RECMAKEFLAGS) -f $(SRCDIR)/mk/cs.mak $@

cleandoc:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

NODEFIX = $(SRCDIR)/docs/support/nodefix.pl
TEXI2HTML = docs/support/texi2html
PS2PDF = ps2pdf
DOXYGEN_RUN = TOP="$(SRCDIR)" ; export TOP ; $(CMD.DOXYGEN)

# Root of the entire Crystal Space manual source tree.
CSMANUAL_DIR  = docs/texinfo
CSMANUAL_FILE = cs-unix.txi

# texi2html configuration file.
TEXI2HTMLINIT = docs/support/texi2html.init

# Doxygen configuration files.
DOXYGEN_PUBAPI = $(SRCDIR)/docs/doxygen/pubapi.dox
DOXYGEN_DEVAPI = $(SRCDIR)/docs/doxygen/devapi.dox

# Copy additional images to doxygen output dir
OUT.DOXYGEN.IMAGES = $(CP) $(SRCDIR)/docs/doxygen/*.png 

# Root of the target directory hierarchy.
OUT.DOC = $(OUTBASE)/docs

# This section is specially protected by DO_DOC in order to prevent the lengthy
# $(wildcard) operations from impacting _all_ other build targets.  DO_DOC is
# only defined when a top-level documentaiton target is invoked.

ifeq ($(DO_DOC),yes)

# Absolute path of SRCDIR.
SRCDIRABS := $(shell $(CD) $(SRCDIR); $(PWD))

# Target directory for each output format.
OUT.DOC.API.PUB = $(OUT.DOC)/html/api
OUT.DOC.API.DEV = $(OUT.DOC)/html/apidev
OUT.DOC.HTML    = $(OUT.DOC)/html/manual
OUT.DOC.DVI     = $(OUT.DOC)/dvi/manual
OUT.DOC.PS      = $(OUT.DOC)/ps/manual
OUT.DOC.PDF     = $(OUT.DOC)/pdf/manual
OUT.DOC.INFO    = $(OUT.DOC)/info/manual

# DVI log file
DOC.DVI.LOG = $(OUT.DOC.DVI)/$(addsuffix .log,$(basename $(CSMANUAL_FILE)))

# List of potential image types understood by Texinfo's @image{} directive.
DOC.IMAGE.EXTS = png jpg gif eps txt

# Source and target image lists.
DOC.IMAGE.LIST = $(strip $(wildcard $(addprefix $(SRCDIR)/$(CSMANUAL_DIR),\
  $(foreach ext,$(DOC.IMAGE.EXTS),\
  $(addsuffix $(ext),* */* */*/* */*/*/* */*/*/*/* */*/*/*/*/*)))))
OUT.DOC.IMAGE.LIST = $(subst $(SRCDIR)/$(CSMANUAL_DIR)/,,$(DOC.IMAGE.LIST))

# List of image directory names which must be created in the ouptut location.
OUT.DOC.IMAGE.DIRS := $(sort $(patsubst %/,%,$(dir $(OUT.DOC.IMAGE.LIST))))

# Composed image directory components for each potential output format.  Used
# for creating target directory hiearchy when generating a particular format.
OUT.DOC.IMAGE.DIRS.HTML = \
  $(addprefix $(OUT.DOC.HTML)/,$(OUT.DOC.IMAGE.DIRS))
OUT.DOC.IMAGE.DIRS.DVI  = \
  $(addprefix $(OUT.DOC.DVI)/,$(OUT.DOC.IMAGE.DIRS))
OUT.DOC.IMAGE.DIRS.PS   = \
  $(addprefix $(OUT.DOC.PS)/,$(OUT.DOC.IMAGE.DIRS))
OUT.DOC.IMAGE.DIRS.PDF  = \
  $(addprefix $(OUT.DOC.PDF)/,$(OUT.DOC.IMAGE.DIRS))
OUT.DOC.IMAGE.DIRS.INFO = \
  $(addprefix $(OUT.DOC.INFO)/,$(OUT.DOC.IMAGE.DIRS))

# List of top-level image directories only.  List is composed by filtering out
# directory names containing a slash, thus leaving only top-level names.
# Used to remove target image directories for output formats which do not
# require presence of images after conversion is complete.
OUT.DOC.IMAGE.DIRS.TOP := \
  $(sort $(foreach d,$(OUT.DOC.IMAGE.DIRS),$(word 1,$(subst /, ,$(d)))))

endif # ifeq ($(DO_DOC),yes)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

# This section is specially protected by DO_DOC in order to prevent the lengthy
# $(wildcard) operations from impacting _all_ other build targets.  DO_DOC is
# only defined when a top-level documentaiton target is invoked.

ifeq ($(DO_DOC),yes)

.PHONY: apidevhtml apihtml manualhtml manualdvi manualps manualpdf manualinfo \
  do-apidevhtml do-apihtml do-manualhtml do-manualdvi do-manualinfo

$(OUT.DOC.API.PUB) \
  $(OUT.DOC.API.DEV) \
  $(OUT.DOC.HTML) \
  $(OUT.DOC.DVI) \
  $(OUT.DOC.PS) \
  $(OUT.DOC.PDF) \
  $(OUT.DOC.INFO) \
  $(OUT.DOC.IMAGE.DIRS.HTML) \
  $(OUT.DOC.IMAGE.DIRS.DVI) \
  $(OUT.DOC.IMAGE.DIRS.PS) \
  $(OUT.DOC.IMAGE.DIRS.PDF) \
  $(OUT.DOC.IMAGE.DIRS.INFO):
	@$(MKDIRS)

# Rule for removing a particular directory.  To remove directory "foo",
# specify "foo.CLEAN" as dependency of some target.
%.CLEAN:
	$(RMDIR) $*

# Rule for copying $(CSMANUAL_FILE) to a target directory.  To copy file to
# directory "foo", specify "foo.SOURCE" as dependency of some target.
%.SOURCE:
	$(CP) $(SRCDIR)/$(CSMANUAL_DIR)/$(CSMANUAL_FILE) $*

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
	$(NEWLINE)$(CP) $(file) $(subst $(SRCDIR)/$(CSMANUAL_DIR)/,\
	$(dir $*),$(file)))
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
do-apidevhtml: $(OUT.DOC.API.DEV)
	$(OUT.DOXYGEN.IMAGES) $(OUT.DOC.API.DEV)
	$(DOXYGEN_RUN) $(DOXYGEN_DEVAPI)

apidevhtml: $(OUT.DOC.API.DEV).CLEAN do-apidevhtml

# Rules to generate public API documentation from public header files.
do-apihtml: $(OUT.DOC.API.PUB)
	$(OUT.DOXYGEN.IMAGES) $(OUT.DOC.API.PUB)
	$(DOXYGEN_RUN) $(DOXYGEN_PUBAPI)

apihtml: $(OUT.DOC.API.PUB).CLEAN do-apihtml

# Rule to perform actual HTML conversion of $(CSMANUAL_FILE).
do-manualhtml:
	$(CD) $(OUT.DOC.HTML); $(PERL) $(SRCDIRABS)/$(TEXI2HTML) \
	-init_file $(SRCDIRABS)/$(TEXI2HTMLINIT) -prefix cs \
	-I $(SRCDIRABS)/$(CSMANUAL_DIR) $(CSMANUAL_FILE)

# Rule to generate HTML format output.  Target images are retained since
# generated HTML files reference them.
manualhtml: \
  $(OUT.DOC.HTML).CLEAN \
  $(OUT.DOC.HTML) \
  $(OUT.DOC.IMAGE.DIRS.HTML) \
  $(OUT.DOC.HTML).SOURCE \
  $(OUT.DOC.HTML)/png.jpg.gif.IMAGES \
  do-manualhtml \
  $(OUT.DOC.HTML).ZAPSOURCE

# Rule to perform actual DVI conversion of $(CSMANUAL_FILE).
do-manualdvi:
	$(CP) $(SRCDIR)/$(CSMANUAL_DIR)/texinfo.tex $(OUT.DOC.DVI)
	$(CD) $(OUT.DOC.DVI); $(CMD.TEXI2DVI) --batch --quiet -I '.' -I `pwd` \
	-I $(SRCDIRABS)/$(CSMANUAL_DIR) $(CSMANUAL_FILE)
	$(MV) $(OUT.DOC.DVI)/$(addsuffix .dvi,$(basename $(CSMANUAL_FILE))) \
	$(OUT.DOC.DVI)/cs.dvi
	$(RM) $(OUT.DOC.DVI)/texinfo.tex

# Rule to generate DVI format output.  Target images are retained since
# generated DVI file references them.
manualdvi: \
  $(OUT.DOC.DVI).CLEAN \
  $(OUT.DOC.DVI) \
  $(OUT.DOC.IMAGE.DIRS.DVI) \
  $(OUT.DOC.DVI).SOURCE \
  $(OUT.DOC.DVI)/eps.IMAGES \
  do-manualdvi \
  $(OUT.DOC.DVI).ZAPSOURCE
	@echo $"$"
	@echo $">>> Documentation conversion completed without errors,$"
	@echo $">>> however check $(DOC.DVI.LOG) for warnings about$"
	@echo $">>> formatting problems.$"
	@echo $"$"

# Rule to perform actual PS conversion from DVI file.
do-manualps:
	$(CD) $(OUT.DOC.DVI); $(CMD.DVIPS) -q -o cs.ps cs.dvi
	$(MV) $(OUT.DOC.DVI)/cs.ps $(OUT.DOC.PS)/cs.ps

# Rule to generate PS format output.  Target images are incorporated directly
# into PostScript file from within DVI target directory.
manualps: \
  $(OUT.DOC.PS).CLEAN \
  $(OUT.DOC.PS) \
  manualdvi \
  do-manualps
	@echo $"$"
	@echo $">>> Documentation conversion completed without errors,$"
	@echo $">>> however check $(DOC.DVI.LOG) for warnings about.$"
	@echo $">>> formatting problems.$"
	@echo $"$"

# Rule to perform actual PDF conversion from PS file.
do-manualpdf:
	$(PS2PDF) $(OUT.DOC.PS)/cs.ps $(OUT.DOC.PDF)/cs.pdf

# Rule to generate PDF format output.
manualpdf: \
  $(OUT.DOC.PDF).CLEAN \
  $(OUT.DOC.PDF) \
  manualps \
  do-manualpdf
	@echo $"$"
	@echo $">>> Documentation conversion completed without errors,$"
	@echo $">>> however, check $(DOC.DVI.LOG) for warnings about$"
	@echo $">>> formatting problems.$"
	@echo $"$"

# Rule to perform actual Info conversion of $(CSMANUAL_FILE).
do-manualinfo:
	$(CD) $(OUT.DOC.INFO); $(CMD.MAKEINFO) \
	-I $(SRCDIRABS)/$(CSMANUAL_DIR) --output=cs $(CSMANUAL_FILE)

# Rule to generate Info format output.  Target images are removed after
# conversion since images are incorporated directly into generated Info files.
manualinfo: \
  $(OUT.DOC.INFO).CLEAN \
  $(OUT.DOC.INFO) \
  $(OUT.DOC.IMAGE.DIRS.INFO) \
  $(OUT.DOC.INFO).SOURCE \
  $(OUT.DOC.INFO)/txt.IMAGES \
  do-manualinfo \
  $(OUT.DOC.INFO).ZAPSOURCE \
  $(OUT.DOC.INFO)/txt.ZAPIMAGES

# Rule to convert manual from HTML to MS compressed HTML.
manualchm: 
	$(CD) $(OUT.DOC); $(PERL) -I$(SRCDIRABS)/docs/support/winhelp \
	$(SRCDIRABS)/docs/support/winhelp/gendoctoc.pl manual html/manual
	$(CP) $(SRCDIR)/docs/support/winhelp/csmanual.hhp $(OUT.DOC)

# Rule to convert public API reference from HTML to MS compressed HTML.
apichm: 
	$(CD) $(OUT.DOC); $(PERL) -I$(SRCDIRABS)/docs/support/winhelp \
	$(SRCDIRABS)/docs/support/winhelp/gendoctoc.pl api html/api
	$(CP) $(SRCDIR)/docs/support/winhelp/csapi.hhp $(OUT.DOC)

endif # ifeq ($(DO_DOC),yes)

# Repair out-of-date and broken @node and @menu directives in Texinfo source.
.PHONY: repairdoc
repairdoc:
	$(PERL) $(NODEFIX) --include-dir=$(SRCDIR)/$(CSMANUAL_DIR) \
	$(SRCDIR)/$(CSMANUAL_DIR)/$(CSMANUAL_FILE)

# Remove all target documentation directories.
.PHONY: cleandoc
clean: cleandoc
cleandoc:
	$(RMDIR) $(OUT.DOC)

endif # ifeq ($(MAKESECTION),targets)
