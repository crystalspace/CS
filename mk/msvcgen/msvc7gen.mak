#==============================================================================
#
#    Automatic MSVC7-compliant .SLN and .VCPROJ generation makefile
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
#	A makefile for synthesizing a complete set of MSVC7-compliant .SLN and
#	.VCPROJ project files based upon information gleaned from GNU makefiles
#	project-wide.
#
#	This process strives to enforce the fundamental invariant that if the
#	GNU makefile builds a working target (application, plug-in, library,
#	etc.), then the synthesized .SLN and .VCPROJ resources will also build a
#	working target.  Thus, the headache associated with manual maintenance
#	of the MSVC7 project files becomes a problem of the past.
#
# IMPORTS
#	In the discussion which follows, assume that "PROJECT" is the core name
#	of the module represented by a particular makefile.
#
#	The following general-purpose makefile variables are imported from
#	other makefiles, project-wide, in order to glean information needed to
#	generate MSVC7 project files.
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
#	Furthermore, the following variables specifically control .SLN and
#	.VCPROJ project file creation.  These variables should only appear in
#	makefiles for which a corresponding .VCPROJ file should be generated.
#	Note: the variable names contain 'DSP' so that the information for MSVC6
#	      DSW/DSP generation can be re-used.
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
#	o A .VCPROJ project file is generated for each project mentioned by the
#	  MSVC.DSP variable.
#
#	o A single .SLN file, named csall.sln, is generated.  It contains
#	  dependency information for all generated .VCPROJ projects.
#
#	The following makefile targets are exported:
#
#	o msvc7gen -- Generates the .SLN file csall.sln, as well as one .VCPROJ
#	  project files for each module mentioned by the MSVC.DSP variable.
#
#	o msvc7inst -- Copies the newly generated project files over top of the
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
DESCRIPTION.msvc7gen = MSVC 7 .SLN and .VCPROJ resources
DESCRIPTION.msvc7inst = $(DESCRIPTION.msvc7gen)

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PSEUDOHELP += \
  $(NEWLINE)echo $"  make msvc7gen     Rebuild the $(DESCRIPTION.msvc7gen)$" \
  $(NEWLINE)echo $"  make msvc7inst    Install the $(DESCRIPTION.msvc7inst)$"

# Set MSVC.PLUGINS.REQUIRED to a list of plug-ins for which DSP files must be
# generated even if the current makefile target (i.e. 'linux') would not
# normally build those plug-ins.  This list augments the normal PLUGINS list.
include mk/msvcgen/required.mak

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: msvc7gen msvc7inst msvc7genclean

msvc7gen:
	@echo $(SEPARATOR)
	@echo $"  Generating $(DESCRIPTION.$@)$"
	@echo $(SEPARATOR)
	@$(MAKE) $(RECMAKEFLAGS) -f mk/cs.mak $@ \
	DO_MSVCGEN=yes DO_ASM=no USE_MAKEFILE_CACHE=no \
	PLUGINS='$(PLUGINS) $(PLUGINS.DYNAMIC) $(MSVC.PLUGINS.REQUIRED)'

msvc7inst: msvc7gen
	@echo $(SEPARATOR)
	@echo $"  Installing $(DESCRIPTION.$@)$"
	@echo $(SEPARATOR)
	@$(MAKE) $(RECMAKEFLAGS) -f mk/cs.mak $@

msvc7genclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

MSVC7GEN = $(PERL) -Imk/msvcgen mk/msvcgen/msvc7gen.pl
MSVC7.TEMPLATE.DIR = mk/msvcgen/template7
ifneq (,$(MSVC7_QUIET))
MSVC7.SILENT = @
endif

MSVC7.CVS.BASE = mk
MSVC7.CVS.DIR = $(MSVC7.CVS.BASE)/visualc7
MSVC7.OUT.BASE = $(OUTBASE)mk
MSVC7.OUT.DIR = $(MSVC7.OUT.BASE)/visualc7
MSVC7.OUT.FRAGMENT = $(MSVC7.OUT.BASE)/fragment7
MSVC7.EXT.DSP = .vcproj
MSVC7.EXT.DSW = .sln
MSVC7.EXT.FRAGMENT = .frag
MSVC7.DSW = csall$(MSVC7.EXT.DSW)

# Prefixes for particular project types.  For instance, the name "csgeom"
# which is of type "library" is transformed into a project name "libcsgeom".
MSVC7.PREFIX.appgui  = app
MSVC7.PREFIX.appcon  = app
MSVC7.PREFIX.plugin  = plg
MSVC7.PREFIX.library = lib
MSVC7.PREFIX.group   = grp

# Special dependencies for particular project types.
MSVC7.DEPEND.appgui  =
MSVC7.DEPEND.appcon  =
MSVC7.DEPEND.plugin  =
MSVC7.DEPEND.library =
MSVC7.DEPEND.group   =

# Define extra Windows-specific targets which do not have associated makefiles.
include mk/msvcgen/win32.mak

# Macro to compose project name. (ex: "CSGEOM" becomes "libcsgeom")
MSVC7.PROJECT = $(MSVC7.PREFIX.$(DSP.$*.TYPE))$(DSP.$*.NAME)

# Macro to compose full project pathname.
# (ex: "CSGEOM" becomes "out/mk/visualc/libcsgeom.vcproj")
MSVC7.OUTPUT = $(MSVC7.OUT.DIR)/$(MSVC7.PROJECT)$(MSVC7.EXT.DSP)

# Macro to compose full fragment pathname.
# (ex: "CSGEOM" becomes "out/mk/fragment/libcsgeom.frag")
MSVC7.FRAGMENT = $(MSVC7.OUT.FRAGMENT)/$(MSVC7.PROJECT)$(MSVC7.EXT.FRAGMENT)

# Macro to compose entire list of resources which comprise a project.
MSVC7.CONTENTS = $(SRC.$*) $(INC.$*) $(CFG.$*) $(DSP.$*.RESOURCES)

# Macro to compose the entire dependency list for a particular project.
# Dependencies are gleaned from three variables: DSP.PROJECT.DEPEND,
# DEP.PROJECT, and MSVC7.DEPEND.TYPE where "PROJECT" represents the current
# project identifier and "TYPE" represents the current project's type (i.e.
# appgui, plugin, library, etc.).  Items in these lists are indirect project
# names (i.e. "CSGEOM") which are tranformed into actual project names (i.e.
# "libcsgeom") via the variables DSP.PROJECT.NAME and DSP.PROJECT.TYPE.  Also,
# a dependency on CSSYS is translated to WIN32SYS since the SRC.CSSYS and
# INC.CSSYS variables may not refer to the appropriate resources (if the
# makefiles are configured for Unix, for instance), whereas
# DSP.WIN32SYS.RESOURCES is guaranteed to refer to the correct resources for
# the Windows platform.
MSVC7.DEPEND.LIST = $(foreach d,$(sort $(subst CSSYS,WIN32SYS,\
  $(DEP.$*) $(DSP.$*.DEPEND) $(MSVC7.DEPEND.$(DSP.$*.TYPE)))),\
  $(MSVC7.PREFIX.$(DSP.$d.TYPE))$(DSP.$d.NAME))

# Macro to compose list of --depend directives from MSVC7.DEPEND.LIST.
MSVC7.DEPEND.DIRECTIVES = $(foreach d,$(MSVC7.DEPEND.LIST),--depend=$d)

# Macro to compose list of --library directives from DSP.PROJECT.LIBS.
MSVC7.LIBRARY.DIRECTIVES = $(foreach l,$(DSP.$*.LIBS),--library=$l)

# Macros to compose --lflags and --cflags directives from DSP.PROJECT.LFLAGS
# and DSP.PROJECT.CFLAGS.  These are slightly complicated because it is valid
# for LFLAGS and CFLAGS to contain embedded whitespace, and also by the fact
# that --lflags and --cflags should only appear on the command-line if the
# corresponding values of LFLAGS and CFLAGS, respectively, are non-empty.  The
# extra $(subst) step is used to eliminate the --lflags or --cflags directive
# in the event that LFLAGS or CFLAGS is empty.
MSVC7.LFLAGS.DIRECTIVE = $(subst --lflags='',,--lflags='$(DSP.$*.LFLAGS)')
MSVC7.CFLAGS.DIRECTIVE = $(subst --cflags='',,--cflags='$(DSP.$*.CFLAGS)')

# Macros to compose lists of existing and newly created .SLN and .VCPROJ files.
MSVC7.CVS.FILES = $(sort $(subst $(MSVC7.CVS.DIR)/,,$(wildcard \
  $(addprefix $(MSVC7.CVS.DIR)/*,$(MSVC7.EXT.DSP) $(MSVC7.EXT.DSW)))))
MSVC7.OUT.FILES = $(sort $(subst $(MSVC7.OUT.DIR)/,,$(wildcard \
  $(addprefix $(MSVC7.OUT.DIR)/*,$(MSVC7.EXT.DSP) $(MSVC7.EXT.DSW)))))

# Quick'n'dirty macro to compare two file lists and report the appropriate
# CVS "add" and "remove" commands which the user will need to invoke in order
# to update the CVS repository with the newly generated files.
MSVC7.DIFF = $(PERL) -e '\
@d1=($(foreach f,$(MSVC7.CVS.FILES),"$f"$(COMMA))); $$f1 = shift @d1; \
@d2=($(foreach f,$(MSVC7.OUT.FILES),"$f"$(COMMA))); $$f2 = shift @d2; \
while (defined($$f1) or defined($$f2)) { \
  if (!defined($$f2) or (defined($$f1) and $$f1 lt $$f2)) { \
    print "cvs remove $$f1\n"; $$f1 = shift @d1; } \
  elsif (!defined($$f1) or $$f1 gt $$f2) { \
    print "cvs add $$f2\n"; $$f2 = shift @d2; } \
  else { $$f1 = shift @d1; $$f2 = shift @d2; } \
}'

# Messages informing the user that the reported CVS commands must be invoked
# in order to update the CVS repository with the newly generated project files.
MSVC7.CVS.WARNING.1 = \
*ATTENTION* You must invoke the following commands in order to
MSVC7.CVS.WARNING.2 = \
*ATTENTION* You must invoke the commands listed above in order to
MSVC7.CVS.WARNING.3 = \
permanently commit the generated project files to the CVS repository.

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: msvc7gen msvc7inst msvc7genclean slngen

# Directory creation targets.
# [res: they appear already in msvcgen.mak]
#$(MSVC7.OUT.BASE): $(OUTBASE)
#	-$(MSVC7.SILENT)$(MKDIR)

$(MSVC7.OUT.DIR) $(MSVC7.OUT.FRAGMENT): $(MSVC7.OUT.BASE)
	$(MSVC7.SILENT)$(MKDIR)

# Build a .VCPROJ project file and associated .SLN fragment files.
%.MAKEVCPROJ:
	$(MSVC7.SILENT)$(MSVC7GEN) --quiet --vcproj \
	--name=$(DSP.$*.NAME) \
	--template=$(DSP.$*.TYPE) \
	--template-dir=$(MSVC7.TEMPLATE.DIR) \
	--project=$(MSVC7.PROJECT) \
	--output=$(MSVC7.OUTPUT) \
	--fragment=$(MSVC7.FRAGMENT) \
	$(MSVC7.DEPEND.DIRECTIVES) \
	$(MSVC7.LIBRARY.DIRECTIVES) \
	$(MSVC7.LFLAGS.DIRECTIVE) \
	$(MSVC7.CFLAGS.DIRECTIVE) \
	$(MSVC7.CONTENTS)

# Build the project-wide .SLN file (csall.sln).
slngen:
	$(MSVC7.SILENT)$(MSVC7GEN) --quiet --sln \
	--output=$(MSVC7.OUT.DIR)/$(MSVC7.DSW) \
	--template-dir=$(MSVC7.TEMPLATE.DIR) \
	$(wildcard $(MSVC7.OUT.FRAGMENT)/*$(MSVC7.EXT.FRAGMENT))

# Build all Visual-C++ .SLN and .VCPROJ project files.  The .SLN file is built last
# since it is composed of the fragment files generated as part of the .VCPROJ file
# synthesis process.
msvc7gen: \
  msvc7genclean \
  $(MSVC7.OUT.DIR) \
  $(MSVC7.OUT.FRAGMENT) \
  $(addsuffix .MAKEVCPROJ,$(MSVC.DSP)) \
  slngen

# Install the generated project files in place of the files from the CVS
# repository and inform the user as to which CVS commands must be manually
# invoked in order to permanently commit the generated files to the repository.
msvc7inst:
ifneq (,$(strip $(MSVC7.CVS.FILES) $(MSVC7.OUT.FILES)))
	@echo $"Installing project files.$"
endif
ifneq (,$(strip $(MSVC7.CVS.FILES)))
	@$(RM) $(addprefix $(MSVC7.CVS.DIR)/,$(MSVC7.CVS.FILES))
endif
ifneq (,$(strip $(MSVC7.OUT.FILES)))
	@$(CP) $(addprefix $(MSVC7.OUT.DIR)/,$(MSVC7.OUT.FILES)) $(MSVC7.CVS.DIR)
endif
	@echo $(SEPARATOR)
	@echo $"  $(MSVC7.CVS.WARNING.1)$"
	@echo $"  $(MSVC7.CVS.WARNING.3)$"
	@echo $(SEPARATOR)
	@echo $"$(CD) $(MSVC7.CVS.DIR)$"
	@$(MSVC7.DIFF)
	@echo $"cvs commit$"
	@echo $(SEPARATOR)
	@echo $"  $(MSVC7.CVS.WARNING.2)$"
	@echo $"  $(MSVC7.CVS.WARNING.3)$"
	@echo $(SEPARATOR)

# Scrub the sink; mop the floor; wash the dishes; paint the door.
clean: msvc7genclean
msvc7genclean:
	$(MSVC7.SILENT)$(RMDIR) $(MSVC7.OUT.BASE)

endif # ifeq ($(MAKESECTION),targets)
