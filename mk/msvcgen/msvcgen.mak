#==============================================================================
#
#    Automatic MSVC-compliant DSW and DSP generation makefile
#    Copyright (C) 2000-2003 by Eric Sunshine <sunshine@sunshineco.com>
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
#	A makefile for synthesizing a complete set of MSVC-compliant workspace
# 	and project files based upon information gleaned from GNU makefiles
#	project-wide.
#
#	This process strives to enforce the fundamental invariant that if the
#	GNU makefile builds a working target (application, plug-in, library,
#	etc.), then the synthesized workspace (DSW/SLN) and project files
#	(DSP/VCPROJ) will also build a working target.  Thus, the headache
#	associated with manual maintenance of the MSVC project files becomes a
#	problem of the past.
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
#	  of some other module, such as "CSGEOM", "CSUTIL", or "CSTOOL".
#
#	o INF.PROJECT -- The meta-data file for this project.  This optional
#	  file contains information related to this project which can be
#	  accessed at run-time.  For example, for plugin modules, the meta-data
#	  file (in XML format) contains information about the SCF classes
#	  published by the plugin, in addition to other information the plugin
#	  might want to publish.  Typically, the meta-data file has the same
#	  basename as the generated target (see DSP.PROJECT.NAME), but with a
#	  different file extension.  For instance, .csplugin is the extension
#	  used for meta-data files for plugin modules.
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
#	o $(PROJECT.EXE).WINRSRC -- List of .rc files related to this project.
#	  These files are combined and fed to MSVC's resource compiler.  Note
#	  that this is expanded with $($(PROJECT.EXE).WINRSRC).
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
#	o DSP.PROJECT.DELAYLIBS -- List of DLLs (sans suffix) which should be
#	  delay-loaded (that is, they are not loaded when the
#	  application/plugin itself is loaded, but rather the first time a
#	  contained function is invoked.)
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
#	o required.mak -- Sets the value of the MSVC.PLUGINS.REQUIRED and
#	  MSVC.MAKE.FLAGS variables.  MSVC.PLUGINS.REQUIRED supplements the
#	  list of plug-in modules defined by the PLUGINS variable (see
#	  CS/mk/user.mak) and ensures that the correct set of DSP files are
#	  generated even when invoking the project file generation process from
#	  a non-Windows platform such as Unix.  MSVC.MAKE.FLAGS allows
#	  additional flags to be sent to the child "make" invocation.  This is
#	  useful when one needs to define additional make variables which
#	  affect the synthesis process.
#
# EXPORTS
#	The following files are exported by this makefile:
#
#	o A DSP/VCPROJ project file is generated for each project mentioned by
#	  the MSVC.DSP variable.
#
#	o A single DSW/SLN workspace file, named csall.dsw/csall.sln, is
#	  generated.  It contains dependency information for all generated
#	  DSP/VCPROJ projects.
#
#	The following makefile targets are exported:
#
#	o msvc6gen -- Generates the MSVC6 DSW workspace csall.dsw, as well as
#	  one DSP project files for each module mentioned by the MSVC.DSP
#	  variable.
#
#	o msvc7gen -- Generates the MSVC7 SLN workspace csall.sln, as well as
#	  one VCPROJ project files for each module mentioned by the MSVC.DSP
#	  variable.
#
#	o msvc6inst -- Copies the newly generated DSW and DSP files over top of
#	  the existing files from the CVS repository and informs the user as to
#	  exactly which CVS commands must be invoked in order to permanently
#	  commit the new files to the repository.
#
#	o msvc7inst -- Copies the newly generated SLN and VCPROJ files over top
#	  of the existing files from the CVS repository and informs the user as
#	  to exactly which CVS commands must be invoked in order to permanently
#	  commit the new files to the repository.
#
#	The following makefile variables are exported by this makefile:
#
#	o DO_MSVCGEN has the value `yes' while the `msvc6gen' or `msvc7gen'
#	  makefile target is running.  In general, this variable can be
#	  ignored, but in special cases a makefile may check this variable to
#	  alter its behavior.  For instance, in rare circumstances, a makefile
#	  may need to use a different $(wildcard) expression during the
#	  generation process.
#
#	o MSVCGEN_VERSION is either '6' or '7' and is used internally to
#	  determine the right behaviour for either MSVC 6 or 7 projects.
#
#------------------------------------------------------------------------------

# Target description
DESCRIPTION.msvc6gen = MSVC DSW and DSP resources
DESCRIPTION.msvc6inst = $(DESCRIPTION.msvc6gen)
DESCRIPTION.msvc7gen = MSVC7 SLN and VCPROJ resources
DESCRIPTION.msvc7inst = $(DESCRIPTION.msvc7gen)

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PSEUDOHELP += \
  $(NEWLINE)echo $"  make msvc6gen     Rebuild the $(DESCRIPTION.msvc6gen)$" \
  $(NEWLINE)echo $"  make msvc6inst    Install the $(DESCRIPTION.msvc6inst)$" \
  $(NEWLINE)echo $"  make msvc7gen     Rebuild the $(DESCRIPTION.msvc7gen)$" \
  $(NEWLINE)echo $"  make msvc7inst    Install the $(DESCRIPTION.msvc7inst)$"

# Set MSVC.PLUGINS.REQUIRED to a list of plug-ins for which DSP/VCPROJ files
# must be generated even if the currently configured platform (i.e. Unix)
# would not normally build those plug-ins.  This list augments the normal
# PLUGINS list.  Also set MSVC.MAKE.FLAGS for additional flags for the child
# "make" invocation.
include $(SRCDIR)/mk/msvcgen/required.mak

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: msvc6gen msvc6inst msvcgenclean msvc7gen msvc7inst msvc7genclean

define MSVCGEN_BUILD
  @echo $(SEPARATOR)
  @echo $"  Generating $(DESCRIPTION.$@)$"
  @echo $(SEPARATOR)
  @$(MAKE) $(RECMAKEFLAGS) -f $(SRCDIR)/mk/cs.mak msvcgen \
  DO_MSVCGEN=yes DO_ASM=no USE_MAKEFILE_CACHE=no $(MSVC.MAKE.FLAGS) \
  PLUGINS='$(PLUGINS) $(PLUGINS.DYNAMIC) $(MSVC.PLUGINS.REQUIRED)'
endef

define MSVCGEN_INSTALL
  @echo $(SEPARATOR)
  @echo $"  Installing $(DESCRIPTION.$@)$"
  @echo $(SEPARATOR)
  @$(MAKE) $(RECMAKEFLAGS) -f $(SRCDIR)/mk/cs.mak msvcinst
endef

define MSVCGEN_CLEAN
  @echo $(SEPARATOR)
  @echo $"  Cleaning up the $(DESCRIPTION.$(subst clean,,$@))$"
  @echo $(SEPARATOR)
  +@$(MAKE) $(RECMAKEFLAGS) -f $(SRCDIR)/mk/cs.mak msvcgenclean
endef

msvc6gen:
	$(MSVCGEN_BUILD) MSVCGEN_VERSION=6

msvc6inst: msvc6gen
	$(MSVCGEN_INSTALL) MSVCGEN_VERSION=6

msvc6genclean:
	$(MSVCGEN_CLEAN) MSVCGEN_VERSION=6

msvc7gen:
	$(MSVCGEN_BUILD) MSVCGEN_VERSION=7

msvc7inst: msvc7gen
	$(MSVCGEN_INSTALL) MSVCGEN_VERSION=7

msvc7genclean:
	$(MSVCGEN_CLEAN) MSVCGEN_VERSION=7

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

MSVCGEN = $(PERL) $(SRCDIR)/mk/msvcgen/msvcgen.pl
ifeq ($(MSVCGEN_VERSION),6)
MSVCGEN.EXTRA =
MSVC.TEMPLATE.DIR = $(SRCDIR)/mk/msvcgen/template6
MSVC.CVS.FLAGS = -kb
else
MSVCGEN.EXTRA = --xml-protect
MSVC.TEMPLATE.DIR = $(SRCDIR)/mk/msvcgen/template7
MSVC.CVS.FLAGS =
endif
ifneq (,$(MSVC_QUIET))
MSVC.SILENT = @
endif

MSVC.CVS.BASE = $(SRCDIR)/mk
MSVC.OUT.BASE.0 = $(OUTBASE)/mk
MSVC.OUT.BASE = $(MSVC.OUT.BASE.0)/msvcgen
ifeq ($(MSVCGEN_VERSION),6)
MSVC.CVS.DIR = $(MSVC.CVS.BASE)/visualc6
MSVC.OUT.DIR = $(MSVC.OUT.BASE)/visualc6
MSVC.OUT.FRAGMENT = $(MSVC.OUT.BASE)/fragment6
MSVC.EXT.PROJECT = dsp
MSVC.EXT.WORKSPACE = dsw
else
MSVC.CVS.DIR = $(MSVC.CVS.BASE)/visualc7
MSVC.OUT.DIR = $(MSVC.OUT.BASE)/visualc7
MSVC.OUT.FRAGMENT = $(MSVC.OUT.BASE)/fragment7
MSVC.EXT.PROJECT = vcproj
MSVC.EXT.WORKSPACE = sln
endif
MSVC.EXT.FRAGMENT = frag
MSVC.EXT.RESOURCES = rc
MSVC.WORKSPACE = csall.$(MSVC.EXT.WORKSPACE)

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

# Project types for which version information should be generated.
MSVC.MAKEVERRC.appgui  = $(MSVC.MAKEVERRC.COMMAND)
MSVC.MAKEVERRC.appcon  = $(MSVC.MAKEVERRC.COMMAND)
MSVC.MAKEVERRC.plugin  = $(MSVC.MAKEVERRC.COMMAND)
MSVC.MAKEVERRC.library =
MSVC.MAKEVERRC.group   =

MSVC.MAKEMETARC.appgui  = 
MSVC.MAKEMETARC.appcon  = 
MSVC.MAKEMETARC.plugin  = $(MSVC.MAKEMETARC.COMMAND)
MSVC.MAKEMETARC.library =
MSVC.MAKEMETARC.group   =

MSVC.MERGERC.appgui  = $(MSVC.MERGERC.COMMAND)
MSVC.MERGERC.appcon  = $(MSVC.MERGERC.COMMAND)
MSVC.MERGERC.plugin  = $(MSVC.MERGERCMETA.COMMAND)
MSVC.MERGERC.library =
MSVC.MERGERC.group   =

# Name of project.rc file for types which require version information.
MSVC.VERSIONRC.CVS.appgui  = $(MSVC.VERSIONRC.CVS.NAME)
MSVC.VERSIONRC.OUT.appgui  = $(MSVC.VERSIONRC.OUT.NAME)
MSVC.VERSIONRC.CVS.appcon  = $(MSVC.VERSIONRC.CVS.NAME)
MSVC.VERSIONRC.OUT.appcon  = $(MSVC.VERSIONRC.OUT.NAME)
MSVC.VERSIONRC.CVS.plugin  = $(MSVC.VERSIONRC.CVS.NAME)
MSVC.VERSIONRC.OUT.plugin  = $(MSVC.VERSIONRC.OUT.NAME)
MSVC.VERSIONRC.CVS.library =
MSVC.VERSIONRC.OUT.library =
MSVC.VERSIONRC.CVS.group   =
MSVC.VERSIONRC.OUT.group   =

# Define extra Windows-specific targets which do not have associated makefiles.
include $(SRCDIR)/mk/msvcgen/win32.mak

# Macro to compose project name. (ex: "CSGEOM" becomes "libcsgeom")
MSVC.PROJECT = $(MSVC.PREFIX.$(DSP.$*.TYPE))$(DSP.$*.NAME)

# Macro to compose full project pathname.
# (ex: "CSGEOM" becomes "out/mk/visualc/libcsgeom.dsp")
MSVC.OUTPUT = $(MSVC.OUT.DIR)/$(MSVC.PROJECT).$(MSVC.EXT.PROJECT)

# Macro to compose full fragment pathname.
# (ex: "CSGEOM" becomes "out/mk/fragment/libcsgeom.frag")
MSVC.FRAGMENT = $(MSVC.OUT.FRAGMENT)/$(MSVC.PROJECT).$(MSVC.EXT.FRAGMENT)

# Macros to compose project.rc filename for standard and build locations.
MSVC.VERSIONRC.CVS.NAME = $(MSVC.CVS.DIR)/$(MSVC.PROJECT).$(MSVC.EXT.RESOURCES)
MSVC.VERSIONRC.OUT.NAME = $(MSVC.OUT.DIR)/$(MSVC.PROJECT).$(MSVC.EXT.RESOURCES)

# Macros to compose project.rc filename for a given project.
MSVC.VERSIONRC.CVS = $(MSVC.VERSIONRC.CVS.$(DSP.$*.TYPE))
MSVC.VERSIONRC.OUT = $(MSVC.VERSIONRC.OUT.$(DSP.$*.TYPE))
MSVC.VERSIONRC.TEMP = $(MSVC.OUT.FRAGMENT)/version.tmp
MSVC.METARC.TEMP = $(MSVC.OUT.FRAGMENT)/meta.tmp

# Module name/description for project.rc.
MSVC.VERSIONDESC = $(DESCRIPTION.$(DSP.$*.NAME))

# Command to generate the project.rc file.
MSVC.MAKEVERRC.COMMAND = $(RUN_SCRIPT) $(SRCDIR)/libs/cssys/win32/mkverres.sh \
  '$(MSVC.VERSIONRC.TEMP)' '$(MSVC.VERSIONDESC)' '$(SRCDIR)/include/csver.h'
MSVC.MAKEMETARC.COMMAND = $(RUN_SCRIPT) $(SRCDIR)/libs/cssys/win32/mkmetadatares.sh \
  '$(MSVC.METARC.TEMP)' '$(INF.$*)'
MSVC.MERGERC.COMMAND = $(RUN_SCRIPT) $(SRCDIR)/libs/cssys/win32/mergeres.sh \
  '$(MSVC.VERSIONRC.OUT)' '$(SRCDIR)' '../..' '$(MSVC.VERSIONRC.TEMP)' \
  '$($($*.EXE).WINRSRC)'
MSVC.MERGERCMETA.COMMAND = $(RUN_SCRIPT) $(SRCDIR)/libs/cssys/win32/mergeres.sh \
  '$(MSVC.VERSIONRC.OUT)' '$(SRCDIR)' '../..' '$(MSVC.VERSIONRC.TEMP)' \
  '$($($*.EXE).WINRSRC)' '$(MSVC.METARC.TEMP)'

# Command to generate the project.rc file for a given project.
MSVC.MAKEVERRC = $(MSVC.MAKEVERRC.$(DSP.$*.TYPE))
MSVC.MAKEMETARC = $(MSVC.MAKEMETARC.$(DSP.$*.TYPE))
MSVC.MERGERC = $(MSVC.MERGERC.$(DSP.$*.TYPE))

# Macro to compose entire list of resources which comprise a project.
MSVC.CONTENTS = $(subst $(SRCDIR)/,, \
  $(SRC.$*) $(INC.$*) $(CFG.$*) $(DSP.$*.RESOURCES) $(MSVC.VERSIONRC.CVS))

# Macro to compose the optional --meta-file option from INF.PROJECT.
MSVC.METAFILE.DIRECTIVE = \
  $(subst $(SRCDIR)/,,$(foreach m,$(INF.$*),--meta-file=$m))

# Macro to compose the entire dependency list for a particular project.
# Dependencies are gleaned from three variables: DSP.PROJECT.DEPEND,
# DEP.PROJECT, and MSVC.DEPEND.TYPE where "PROJECT" represents the current
# project identifier and "TYPE" represents the current project's type (i.e.
# appgui, appcon, plugin, library, etc.).  Items in these lists are indirect
# project names (i.e. "CSGEOM") which are tranformed into actual project names
# (i.e. "libcsgeom") via the variables DSP.PROJECT.NAME and DSP.PROJECT.TYPE.
MSVC.DEPEND.LIST = $(foreach d,$(sort \
  $(DEP.$*) $(DSP.$*.DEPEND) $(MSVC.DEPEND.$(DSP.$*.TYPE))),\
  $(MSVC.PREFIX.$(DSP.$d.TYPE))$(DSP.$d.NAME))

# Macro to compose list of --depend directives from MSVC.DEPEND.LIST.
MSVC.DEPEND.DIRECTIVES = $(foreach d,$(MSVC.DEPEND.LIST),--depend=$d)

# Macro to compose list of --library and --delaylibs directives from
# DSP.PROJECT.LIBS.
MSVC.LIBRARY.DIRECTIVES = \
  $(foreach l,$(DSP.$*.LIBS),--library=$l) \
  $(foreach l,$(DSP.$*.DELAYLIBS),--delaylib=$l)

# Macros to compose --lflags and --cflags directives from DSP.PROJECT.LFLAGS
# and DSP.PROJECT.CFLAGS.  These are slightly complicated because it is valid
# for LFLAGS and CFLAGS to contain embedded whitespace, and also by the fact
# that --lflags and --cflags should only appear on the command-line if the
# corresponding values of LFLAGS and CFLAGS, respectively, are non-empty.  The
# extra $(subst) step is used to eliminate the --lflags or --cflags directive
# in the event that LFLAGS or CFLAGS is empty.
MSVC.LFLAGS.DIRECTIVE = $(subst --lflags='',,--lflags='$(DSP.$*.LFLAGS)')
MSVC.CFLAGS.DIRECTIVE = $(subst --cflags='',,--cflags='$(DSP.$*.CFLAGS)')

# Macro to compose a list of generated fragment file basenames (without the
# .pjf, .cff, .dpf suffixes.
MSVC.FRAG.FILES = $(sort \
  $(patsubst %.pjf,%, \
  $(patsubst %.cff,%, \
  $(patsubst %.dpf,%, \
  $(wildcard $(MSVC.OUT.FRAGMENT)/*.$(MSVC.EXT.FRAGMENT).*)))))

# Macros to compose lists of existing and newly created DSW/SLN and DSP/VCPROJ
# files.
MSVC.CVS.FILES = $(sort $(subst $(MSVC.CVS.DIR)/,,\
  $(wildcard $(addprefix $(MSVC.CVS.DIR)/*,\
  .$(MSVC.EXT.PROJECT) .$(MSVC.EXT.WORKSPACE) \
  .$(MSVC.EXT.RESOURCES)))))
MSVC.OUT.FILES = $(sort $(subst $(MSVC.OUT.DIR)/,,\
  $(wildcard $(addprefix $(MSVC.OUT.DIR)/*,\
  .$(MSVC.EXT.PROJECT) .$(MSVC.EXT.WORKSPACE) \
  .$(MSVC.EXT.RESOURCES)))))

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
    print "cvs add $(MSVC.CVS.FLAGS) $$f2\n"; $$f2 = shift @d2; } \
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
$(MSVC.OUT.BASE.0): $(OUTBASE)
	-$(MSVC.SILENT)$(MKDIR)

$(MSVC.OUT.BASE): $(MSVC.OUT.BASE.0)
	-$(MSVC.SILENT)$(MKDIR)

$(MSVC.OUT.DIR) $(MSVC.OUT.FRAGMENT): $(MSVC.OUT.BASE)
	$(MSVC.SILENT)$(MKDIR)

# Build a project project file and an associated DSW/SLN fragment file.
%.MAKEPROJECT:
	$(MSVC.SILENT)$(MSVC.MAKEVERRC)
	$(MSVC.SILENT)$(MSVC.MAKEMETARC)
	$(MSVC.SILENT)$(MSVC.MERGERC)
	$(MSVC.SILENT)$(MSVCGEN) \
	--quiet \
	--project \
	--project-extension=$(MSVC.EXT.PROJECT) \
	--name=$(DSP.$*.NAME) \
	--template=$(DSP.$*.TYPE) \
	--template-dir=$(MSVC.TEMPLATE.DIR) \
	--project-name=$(MSVC.PROJECT) \
	--output=$(MSVC.OUTPUT) \
	--fragment=$(MSVC.FRAGMENT) \
	$(MSVC.METAFILE.DIRECTIVE) \
	$(MSVC.DEPEND.DIRECTIVES) \
	$(MSVC.LIBRARY.DIRECTIVES) \
	$(MSVC.LFLAGS.DIRECTIVE) \
	$(MSVC.CFLAGS.DIRECTIVE) \
	$(MSVCGEN.EXTRA) \
	$(MSVC.CONTENTS)

# Build the project-wide workspace file (csall.dsw/.sln).
workspacegen:
	$(MSVC.SILENT)$(MSVCGEN) \
	--quiet \
	--workspace \
	--workspace-extension=$(MSVC.EXT.WORKSPACE) \
	--output=$(MSVC.OUT.DIR)/$(MSVC.WORKSPACE) \
	--template-dir=$(MSVC.TEMPLATE.DIR) \
	$(MSVCGEN.EXTRA) \
	$(MSVC.FRAG.FILES)

# Build all Visual C++ DSW/SLN workspace and DSP/VCPROJ project files.  The
# DSW/SLN workspace is built last since it is composed of the fragment files
# generated as part of the DSP/VCPROJ file synthesis process.
msvcgen: \
  msvcgenclean \
  $(MSVC.OUT.DIR) \
  $(MSVC.OUT.FRAGMENT) \
  $(addsuffix .MAKEPROJECT,$(MSVC.DSP)) \
  workspacegen

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
clean: msvcgencleanall

# Clean all files, regardless of MSVCGEN_VERSION.
msvcgencleanall:
	$(MSVC.SILENT)$(RMDIR) $(MSVC.OUT.BASE)

# Clean the files the invocation specified by MSVCGEN_VERSION.
msvcgenclean:
	$(MSVC.SILENT)$(RMDIR) $(MSVC.OUT.DIR) $(MSVC.OUT.FRAGMENT)

endif # ifeq ($(MAKESECTION),targets)
