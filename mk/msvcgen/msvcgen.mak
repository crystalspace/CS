#==============================================================================
#
#    Automatic MSVC-compliant DSW and DSP generation makefile
#    Copyright (C) 2000,2001 by Eric Sunshine <sunshine@sunshineco.com>
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
# IMPORTS
#	In the discussion which follows, assume that "PROJECT" is the core name
#	of the module represented by a particular makefile.
#
#	The following general-purpose makefile variables are imported from
#	other makefiles, project-wide, in order to glean information needed to
#	generate MSVC project files.
#
#	o SRC.PROJECT -- List of source files which comprise this module.
#
#	o INC.PROJECT -- List of header files related to this module.
#
#	o DEP.PROJECT -- List of projects (typically library projects) upon
#	  which this module depends.  Each item in this list is the core name
#	  of some other module, such as "CSGEOM", "CSUTIL", or "CSSYS".
#
#	o CFG.PROJECT -- List of configuration files related to this module.
#
#	Furthermore, the following variables specifically control DSW and
#	DSP project file creation.  These variables should only appear in
#	makefiles for which a corresponding DSP file should be generated.
#
#	o MSVC.DSP -- Master list of modules for which project files should be
#	  generated.  Entries must be *appended* to this list with the "+="
#	  operator.  Each entry is the core name of a module as used within its
#	  makefile.  For example, soft3d.mak, modifies this variable with the
#	  expression "MSVC.DSP += SOFT3D".
#
#	o DSP.PROJECT.NAME -- Base name (such as "soft3d") for the generated
#	  project and target.  This name is used to compose the DSP file name,
#	  the end target name (such as "soft3d.dll"), and the displayed project
#	  name in the Visual-C++ IDE.  In general, it should be identical to
#	  the base name of the target which is generated for non-Windows
#	  platforms.
#
#	o DSP.PROJECT.TYPE -- Module's type.  It should be one of "appgui",
#	  "appcon", "library", "plugin", or "group", which stand for GUI
#	  application, console application, static library, plug-in module, and
#	  pseudo-dependency group, respectively.
#
#	o DSP.PROJECT.RESOURCES -- List of extra human-readable resources
#	  related to this module which are not covered by CFG.PROJECT.  These
#	  resources are made available for browsing in the Visual-C++ IDE as a
#	  convenience to the user.  Some good candidates, among others, for
#	  this variable are files having the suffixes .inc, .y (yacc), .l
#	  (lex), and .txt.
#
#	o DSP.PROJECT.DEPEND -- List of extra dependencies for this module.
#	  Entries in this list have the same format as those in the DEP.PROJECT
#	  list.  This variable is generally only used for pseudo-dependency
#	  group projects (see CS/mk/msvcgen/win32.mak).
#
#	o DSP.PROJECT.LIBS -- List of extra Windows-specific libraries with
#	  which this module should be linked in addition to those already
#	  mentioned in the template file for this project type.  A .lib suffix
#	  is automatically added to each item in this list if absent.
#	  Typically, libraries are only specified for executable and plug-in
#	  projects.  This variable differs from DSP.PROJECT.DEPEND in that it
#	  refers to libraries which exist outside of the project graph (such as
#	  wsock32.lib or opengl32.lib), whereas DSP.PROJECT.DEPEND always
#	  refers to modules which are members of the project graph.
#
#	o DSP.PROJECT.LFLAGS -- Specifies extra Windows-specific linker options
#	  which should be used in addition to those already mentioned in the
#	  template file.  Typically, linker options are only specified for
#	  executable and plug-in projects.  Keep in mind that these flags are
#	  passed through the (Bourne) shell during the project file generation
#	  process, thus it may be necessary to specially protect any embedded
#	  quote characters.
#
#	o DSP.PROJECT.CFLAGS -- Specifies extra Windows-specific compiler
#	  options which should be used in addition to those already mentioned
#	  in the template file.  Keep in mind that these flags are passed
#	  through the (Bourne) shell during the project file generation
#	  process, thus it may be necessary to specially protect embedded quote
#	  characters.
#
#	The automatic MSVC project file generation mechanism also employs
#	additional makefile components from the CS/mk/msvcgen directory.
#
#	o win32.mak -- Extends MSVC.DSP with extra project targets which are
#	  specific to Windows or which are not otherwise represented by
#	  stand-alone makefiles within the project hierarchy.
#
#	o required.mak -- Sets the value of the MSVC.PLUGINS.REQUIRED variable.
#	  This variable supplements the list of plug-in modules defined by the
#	  PLUGINS variable (see CS/mk/user.mak) and ensures that the correct
#	  set of DSP files are generated even when invoking the project file
#	  generation process from a non-Windows platform such as Unix.
#
# EXPORTS
#	The following files are exported by this makefile:
#
#	o A DSP project file is generated for each project mentioned by the
#	  MSVC.DSP variable.
#
#	o A single DSW file, named csall.dsw, is generated.  It contains
#	  dependency information for all generated DSP projects.
#
#	The following makefile targets are exported:
#
#	o msvcgen -- Generates the DSW file csall.dsw, as well as one DSP
#	  project files for each module mentioned by the MSVC.DSP variable.
#
#	o msvcinst -- Copies the newly generated project files over top of the
#	  existing files from the CVS repository and informs the user as to
#	  exactly which CVS commands must be invoked in order to permanently
#	  commit the new files to the repository.
#
#	The following makefile variables are exported by this makefile:
#
#	o DO_MSVCGEN has the value `yes' while the `msvcgen' makefile target is
#	  running.  In general, this variable can be ignored, but in special
#	  cases a makefile may check this variable to alter its behavior.  For
#	  instance, in rare circumstances, a makefile may need to use a
#	  different $(wildcard) expression during the `msvcgen' process.
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
	DO_MSVCGEN=yes DO_ASM=no USE_MAKEFILE_CACHE=no \
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
MSVC.DEPEND.appgui  =
MSVC.DEPEND.appcon  =
MSVC.DEPEND.plugin  =
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
MSVC.CONTENTS = $(SRC.$*) $(INC.$*) $(CFG.$*) $(DSP.$*.RESOURCES)

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

# Macro to compose list of --depend directives from MSVC.DEPEND.LIST.
MSVC.DEPEND.DIRECTIVES = $(foreach d,$(MSVC.DEPEND.LIST),--depend=$d)

# Macro to compose list of --library directives from DSP.PROJECT.LIBS.
MSVC.LIBRARY.DIRECTIVES = $(foreach l,$(DSP.$*.LIBS),--library=$l)

# Macros to compose --lflags and --cflags directives from DSP.PROJECT.LFLAGS
# and DSP.PROJECT.CFLAGS.  These are slightly complicated because it is valid
# for LFLAGS and CFLAGS to contain embedded whitespace, and also by the fact
# that --lflags and --cflags should only appear on the command-line if the
# corresponding values of LFLAGS and CFLAGS, respectively, are non-empty.  The
# extra $(subst) step is used to eliminate the --lflags or --cflags directive
# in the event that LFLAGS or CFLAGS is empty.
MSVC.LFLAGS.DIRECTIVE = $(subst --lflags='',,--lflags='$(DSP.$*.LFLAGS)')
MSVC.CFLAGS.DIRECTIVE = $(subst --cflags='',,--cflags='$(DSP.$*.CFLAGS)')

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
	--name=$(DSP.$*.NAME) \
	--template=$(DSP.$*.TYPE) \
	--template-dir=$(MSVC.TEMPLATE.DIR) \
	--project=$(MSVC.PROJECT) \
	--output=$(MSVC.OUTPUT) \
	--fragment=$(MSVC.FRAGMENT) \
	$(MSVC.DEPEND.DIRECTIVES) \
	$(MSVC.LIBRARY.DIRECTIVES) \
	$(MSVC.LFLAGS.DIRECTIVE) \
	$(MSVC.CFLAGS.DIRECTIVE) \
	$(MSVC.CONTENTS)

# Build the project-wide DSW file (csall.dsw).
dswgen:
	$(MSVC.SILENT)$(MSVCGEN) --quiet --dsw \
	--output=$(MSVC.OUT.DIR)/$(MSVC.DSW) \
	--template-dir=$(MSVC.TEMPLATE.DIR) \
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
