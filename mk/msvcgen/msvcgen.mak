#==============================================================================
#
#    Automatic MSVC-compliant DSW and DSP generation makefile
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
# msvcgen.mak
#
#	A makefile for synthesizing a complete set of MSVC-compliant DSW and
#	DSP project files based upon information gleaned from GNU makefiles
#	project-wide.
#
#	This process strives to enforce the fundamental invariant that if the
#	GNU makefile builds a working target (application, plug-in, library,
#	etc.), then the synthesized DSW and DSP resources will also build a
#	working target.  Thus, the headache associated with manual maintenance
#	of the MSVC project files becomes a problem of the past.
#
#------------------------------------------------------------------------------

# Target description
DESCRIPTION.msvcgen = MSVC DSW and DSP resources
DESCRIPTION.msvcinst = $(DESCRIPTION.msvcgen)

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PSEUDOHELP += \
  $(NEWLINE)echo $"  make msvcgen      Rebuild the $(DESCRIPTION.msvcgen)$" \
  $(NEWLINE)echo $"  make msvcinst     Install the $(DESCRIPTION.msvcinst)$"

# Set MSVC.PLUGINS.REQUIRED to a list of plug-ins for which DSP files must be
# generated even if the current makefile target (i.e. 'linux') would not
# normally build those plug-ins.  This list augments the normal PLUGINS list.
include mk/msvcgen/required.mak

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: msvcgen msvcinst msvcgenclean

msvcgen:
	@echo $(SEPARATOR)
	@echo $"  Generating $(DESCRIPTION.$@)$"
	@echo $(SEPARATOR)
	@$(MAKE) $(RECMAKEFLAGS) -f mk/cs.mak $@ \
	PLUGINS='$(PLUGINS) $(PLUGINS.DYNAMIC) $(MSVC.PLUGINS.REQUIRED)'

msvcinst: msvcgen
	@echo $(SEPARATOR)
	@echo $"  Installing $(DESCRIPTION.$@)$"
	@echo $(SEPARATOR)
	@$(MAKE) $(RECMAKEFLAGS) -f mk/cs.mak $@

msvcgenclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

MSVCGEN = $(PERL) mk/msvcgen/msvcgen.pl
MSVC.TEMPLATE.DIR = mk/msvcgen/template
ifneq (,$(MSVC_QUIET))
MSVC.SILENT = @
endif

MSVC.CVS.BASE = mk
MSVC.CVS.DIR = $(MSVC.CVS.BASE)/visualc
MSVC.OUT.BASE = $(OUTBASE)mk
MSVC.OUT.DIR = $(MSVC.OUT.BASE)/visualc
MSVC.OUT.FRAGMENT = $(MSVC.OUT.BASE)/fragment
MSVC.EXT.DSP = .dsp
MSVC.EXT.DSW = .dsw
MSVC.EXT.FRAGMENT = .dwi
MSVC.DSW = csall$(MSVC.EXT.DSW)

# Prefixes for particular project types.  For instance, the name "csgeom"
# which is of type "library" is transformed into a project name "libcsgeom".
MSVC.PREFIX.appgui  = app
MSVC.PREFIX.appcon  = app
MSVC.PREFIX.plugin  = plg
MSVC.PREFIX.library = lib
MSVC.PREFIX.group   = grp

# Special dependencies for particular project types.
MSVC.DEPEND.appgui  = WIN32EXE
MSVC.DEPEND.appcon  = WIN32EXE
MSVC.DEPEND.plugin  = WIN32DLL
MSVC.DEPEND.library =
MSVC.DEPEND.group   =

# Define extra Windows-specific targets which do not have associated makefiles.
include mk/msvcgen/win32.mak

# Macro to compose project name. (ex: "CSGEOM" becomes "libcsgeom")
MSVC.PROJECT = $(MSVC.PREFIX.$(DSP.$*.TYPE))$(DSP.$*.NAME)

# Macro to compose full project pathname.
# (ex: "CSGEOM" becomes "out/mk/visualc/libcsgeom.dsp")
MSVC.OUTPUT = $(MSVC.OUT.DIR)/$(MSVC.PROJECT)$(MSVC.EXT.DSP)

# Macro to compose full fragment pathname.
# (ex: "CSGEOM" becomes "out/mk/fragment/libcsgeom.dwi")
MSVC.FRAGMENT = $(MSVC.OUT.FRAGMENT)/$(MSVC.PROJECT)$(MSVC.EXT.FRAGMENT)

# Macro to compose entire list of resources which comprise a project.
# The file static.cpp is filtered out since it is not needed by MSVC.
MSVC.CONTENTS = $(filter-out apps/support/static.cpp,$(SRC.$*)) \
  $(INC.$*) $(CFG.$*) $(DSP.$*.RESOURCES)

# Macro to compose the entire dependency list for a particular project.
# Dependencies are gleaned from three variables: DSP.PROJECT.DEPEND,
# DEP.PROJECT, and MSVC.DEPEND.TYPE where "PROJECT" represents the current
# project identifier and "TYPE" represents the current project's type (i.e.
# appgui, plugin, library, etc.).  Items in these lists are indirect project
# names (i.e. "CSGEOM") which are tranformed into actual project names (i.e.
# "libcsgeom") via the variables DSP.PROJECT.NAME and DSP.PROJECT.TYPE.  Also,
# a dependency on CSSYS is translated to WIN32SYS since the SRC.CSSYS and
# INC.CSSYS variables may not refer to the appropriate resources (if the
# makefiles are configured for Unix, for instance), whereas
# DSP.WIN32SYS.RESOURCES is guaranteed to refer to the correct resources for
# the Windows platform.
MSVC.DEPEND.LIST = $(foreach d,$(sort $(subst CSSYS,WIN32SYS,\
  $(DEP.$*) $(DSP.$*.DEPEND) $(MSVC.DEPEND.$(DSP.$*.TYPE)))),\
  $(MSVC.PREFIX.$(DSP.$d.TYPE))$(DSP.$d.NAME))

# Macros to compose lists of existing and newly created DSW and DSP files.
MSVC.CVS.FILES = $(sort $(subst $(MSVC.CVS.DIR)/,,$(wildcard \
  $(addprefix $(MSVC.CVS.DIR)/*,$(MSVC.EXT.DSP) $(MSVC.EXT.DSW)))))
MSVC.OUT.FILES = $(sort $(subst $(MSVC.OUT.DIR)/,,$(wildcard \
  $(addprefix $(MSVC.OUT.DIR)/*,$(MSVC.EXT.DSP) $(MSVC.EXT.DSW)))))

# Quick'n'dirty macro to compare two file lists and report the appropriate
# CVS "add" and "remove" commands which the user will need to invoke in order
# to update the CVS repository with the newly generated files.
MSVC.DIFF = $(PERL) -e '\
@d1=($(foreach f,$(MSVC.CVS.FILES),"$f"$(COMMA))); $$f1 = shift @d1; \
@d2=($(foreach f,$(MSVC.OUT.FILES),"$f"$(COMMA))); $$f2 = shift @d2; \
while (defined($$f1) or defined($$f2)) { \
  if (!defined($$f2) or (defined($$f1) and $$f1 lt $$f2)) { \
    print "cvs remove $$f1\n"; $$f1 = shift @d1; } \
  elsif (!defined($$f1) or $$f1 gt $$f2) { \
    print "cvs add -kb $$f2\n"; $$f2 = shift @d2; } \
  else { $$f1 = shift @d1; $$f2 = shift @d2; } \
}'

# Messages informing the user that the reported CVS commands must be invoked
# in order to update the CVS repository with the newly generated project files.
MSVC.CVS.WARNING.1 = \
*ATTENTION* You must invoke the following commands in order to
MSVC.CVS.WARNING.2 = \
*ATTENTION* You must invoke the commands listed above in order to
MSVC.CVS.WARNING.3 = \
permanently commit the generated project files to the CVS repository.

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: msvcgen msvcinst msvcgenclean dswgen

# Directory creation targets.
$(MSVC.OUT.BASE): $(OUTBASE)
	-$(MSVC.SILENT)$(MKDIR)

$(MSVC.OUT.DIR) $(MSVC.OUT.FRAGMENT): $(MSVC.OUT.BASE)
	$(MSVC.SILENT)$(MKDIR)

# Build a DSP project file and an associated DSW fragment file.
%.MAKEDSP:
	$(MSVC.SILENT)$(MSVCGEN) --quiet --dsp \
	--name $(DSP.$*.NAME) \
	--template $(DSP.$*.TYPE) \
	--project $(MSVC.PROJECT) \
	--output $(MSVC.OUTPUT) \
	--fragment $(MSVC.FRAGMENT) \
	$(foreach d,$(MSVC.DEPEND.LIST),--depend $d) \
	--template-dir $(MSVC.TEMPLATE.DIR) \
	$(MSVC.CONTENTS)

# Build the project-wide DSW file (csall.dsw).
dswgen:
	$(MSVC.SILENT)$(MSVCGEN) --quiet --dsw \
	--output $(MSVC.OUT.DIR)/$(MSVC.DSW) \
	--template-dir $(MSVC.TEMPLATE.DIR) \
	$(wildcard $(MSVC.OUT.FRAGMENT)/*$(MSVC.EXT.FRAGMENT))

# Build all Visual-C++ DSW and DSP project files.  The DSW file is built last
# since it is composed of the fragment files generated as part of the DSP file
# synthesis process.
msvcgen: \
  msvcgenclean \
  $(MSVC.OUT.DIR) \
  $(MSVC.OUT.FRAGMENT) \
  $(addsuffix .MAKEDSP,$(MSVC.DSP)) \
  dswgen

# Install the generated project files in place of the files from the CVS
# repository and inform the user as to which CVS commands must be manually
# invoked in order to permanently commit the generated files to the repository.
msvcinst:
ifneq (,$(strip $(MSVC.CVS.FILES) $(MSVC.OUT.FILES)))
	@echo $"Installing project files.$"
endif
ifneq (,$(strip $(MSVC.CVS.FILES)))
	@$(RM) $(addprefix $(MSVC.CVS.DIR)/,$(MSVC.CVS.FILES))
endif
ifneq (,$(strip $(MSVC.OUT.FILES)))
	@$(CP) $(addprefix $(MSVC.OUT.DIR)/,$(MSVC.OUT.FILES)) $(MSVC.CVS.DIR)
endif
	@echo $(SEPARATOR)
	@echo $"  $(MSVC.CVS.WARNING.1)$"
	@echo $"  $(MSVC.CVS.WARNING.3)$"
	@echo $(SEPARATOR)
	@echo $"$(CD) $(MSVC.CVS.DIR)$"
	@$(MSVC.DIFF)
	@echo $"cvs commit$"
	@echo $(SEPARATOR)
	@echo $"  $(MSVC.CVS.WARNING.2)$"
	@echo $"  $(MSVC.CVS.WARNING.3)$"
	@echo $(SEPARATOR)

# Scrub the sink; mop the floor; wash the dishes; paint the door.
clean: msvcgenclean
msvcgenclean:
	$(MSVC.SILENT)$(RMDIR) $(MSVC.OUT.BASE)

endif # ifeq ($(MAKESECTION),targets)
