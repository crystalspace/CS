# Target description
DESCRIPTION.csconfig = cs-config build options script

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PSEUDOHELP += \
  $(NEWLINE)echo $"  make csconfig     Build the $(DESCRIPTION.csconfig)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csconfig csconfigclean

all: csconfig
csconfig:
	$(MAKE_TARGET) DO_CREATE_CSCONFIG=yes
csconfigclean:
	$(MAKE_CLEAN)

endif

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSCONFIG.EXE = cs-config
CSCONFIG.DEP = config.mak mk/user.mak $(TARGET_MAKEFILE)
CSCONFIG.TMP = $(OUT)/csconfig.tmp

TO_INSTALL.EXE	+= $(CSCONFIG.EXE)

# This section is specially protected by DO_INSTALL in order to prevent
# execution of sed commands for _all_ other build targets.  DO_CREATE_CSCONFIG
# is only defined when the top-level 'csconfig' target is invoked.

ifeq ($(DO_CREATE_CSCONFIG),yes)

CSCONFIG.VERFILE := include/csver.h
CSCONFIG.VMAJOR  := $(shell sed -e '/\#define[ 	][ 	]*CS_VERSION_MAJOR/!d' -e '/\#define[ 	][ 	]*CS_VERSION_MAJOR/s/\(\#define[ 	][ 	]*CS_VERSION_MAJOR[ 	][ 	]*"\)\([^\\"]*\)"\(.*\)/\2/' < $(CSCONFIG.VERFILE))
CSCONFIG.VMINOR  := $(shell sed -e '/\#define[ 	][ 	]*CS_VERSION_MINOR/!d' -e '/\#define[ 	][ 	]*CS_VERSION_MINOR/s/\(\#define[ 	][ 	]*CS_VERSION_MINOR[ 	][ 	]*"\)\([^\\"]*\)"\(.*\)/\2/' < $(CSCONFIG.VERFILE))
CSCONFIG.RDATE   := $(shell sed -e '/\#define[ 	][ 	]*CS_RELEASE_DATE/!d'  -e '/\#define[ 	][ 	]*CS_RELEASE_DATE/s/\(\#define[ 	][ 	]*CS_RELEASE_DATE[ 	][ 	]*"\)\([^\\"]*\)"\(.*\)/\2/' < $(CSCONFIG.VERFILE))

# Some makefile variables reference $@, and we want that reference to appear
# verbatim in the synthesized makefile fragment.  Unfortunately, when these
# variables are referenced inside the $(CSCONFIG.EXE) target, $(CSCONFIG.EXE)
# is interpolated in place of $@, therefore we must protect $@ against such
# interpolation.
# NOTE: This does not solve the more general problem where we want to reproduce
# the entire variable's value verbatim.  For example, if a variable's value
# references "$(basename $(notdir $@))", we would like to reproduce that entire
# function calling sequence in the synthesized makefile fragment.
# Unfortunately, such cases are not currently handled.
CSCONFIG.LFLAGS.DLL  = $(subst $@,$$@,$(LFLAGS.DLL))
CSCONFIG.LFLAGS.EXE  = $(subst $@,$$@,$(LFLAGS.EXE))
CSCONFIG.LINK.PLUGIN = $(subst $@,$$@,$(LINK.PLUGIN))
CSCONFIG.DO.SHARED.PLUGIN.PREAMBLE  = \
  $(subst $@,$$@,$(DO.SHARED.PLUGIN.PREAMBLE))
CSCONFIG.DO.SHARED.PLUGIN.POSTAMBLE = \
  $(subst $@,$$@,$(DO.SHARED.PLUGIN.POSTAMBLE))
CSCONFIG.PLUGIN.POSTFLAGS = \
  $(subst $@,$$@,$(PLUGIN.POSTFLAGS))

endif # ifeq ($(DO_CREATE_CSCONFIG),yes)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csconfig csconfigclean

all: $(CSCONFIG.EXE)
clean: csconfigclean

ifeq ($(DO_CREATE_CSCONFIG),yes)

csconfig: $(OUTDIRS) $(CSCONFIG.EXE)

# Create csconfig.tmp for the make variables that need to be transferred.
$(CSCONFIG.EXE): $(CSCONFIG.DEP)
	@echo Generating cs-config script...
	@$(RM) cs-config

	@cat scripts/cs-config/cs-config.temppre > cs-config
	@echo $"#	WARNING: This script is generated automatically!$" >> cs-config
	@echo			>> cs-config
	@echo $"CRYSTAL="$${CRYSTAL-$(INSTALL_DIR)}"$"	>> cs-config
	@echo $"prefix="$${CRYSTAL}"$"			>> cs-config
	@echo $"makeout="$(OUT)"$"			>> cs-config
	@echo $"exec_prefix="$${prefix}"$"		>> cs-config
	@echo $"version="$(CSCONFIG.VMAJOR)"$"		>> cs-config
	@echo $"longversion="$(CSCONFIG.VMAJOR).$(CSCONFIG.VMINOR) ($(CSCONFIG.RDATE))"$" >> cs-config
	@echo $"includedir="$${prefix}/include"$"	>> cs-config
	@echo $"if test -r "$${prefix}/lib"; then$"	>> cs-config
	@echo $"  libdir="$${prefix}/lib"$"		>> cs-config
	@echo $"fi$"					>> cs-config
	@echo $"syslibs="$(LIBS.EXE)"$"			>> cs-config
	@echo $"common_cflags="$(CFLAGS)"$"		>> cs-config
	@echo $"common_cxxflags="$(CFLAGS)"$"		>> cs-config
	@echo $"$"					>> cs-config
	@echo $"makevars()$"				>> cs-config
	@echo $"{$"					>> cs-config
	@echo $"#The EOF is specially quoted to prevent the shell from$" >> cs-config
	@echo $"#interpolating values into makefile variables which it$" >> cs-config
	@echo $"#is dumping into the makefile fragment.  For example,$" >> cs-config
	@echo $"#if $@ appears in a makefile variable, we do not want$" >> cs-config
	@echo $"#the shell trying to interpolate that value.$" >> cs-config
	@echo $"cat <<"EOF"$"				>> cs-config
	@echo $"# Crystal Space config make variables for $(DESCRIPTION.$(TARGET)).$" >> cs-config
	@echo $"# Automatically generated. $" 		>> cs-config
	@echo $"EXE=$(EXE)$" 				>> cs-config
	@echo $"DLL=$(DLL)$" 				>> cs-config
	@echo $"LFLAGS.EXE=$(CSCONFIG.LFLAGS.EXE)$" 	>> cs-config
	@echo $"DO.SHARED.PLUGIN.PREAMBLE=$(CSCONFIG.DO.SHARED.PLUGIN.PREAMBLE)$" >> cs-config
	@echo $"DO.SHARED.PLUGIN.POSTAMBLE=$(CSCONFIG.DO.SHARED.PLUGIN.POSTAMBLE)$" >> cs-config
	@echo $"LIBS.EXE.PLATFORM=$(LIBS.EXE.PLATFORM)$">> cs-config
	@echo $"LINK.PLUGIN=$(CSCONFIG.LINK.PLUGIN)$" 	>> cs-config
	@echo $"LFLAGS.DLL=$(CSCONFIG.LFLAGS.DLL)$" 	>> cs-config
	@echo $"PLUGIN.POSTFLAGS=$(CSCONFIG.PLUGIN.POSTFLAGS)$"	>> cs-config
	@echo $"EOF$"					>> cs-config
	@echo $"}$"					>> cs-config
	@echo						>> cs-config
	@cat scripts/cs-config/cs-config.temppost	>> cs-config

	@chmod +x cs-config
	
endif # ifeq ($(DO_CREATE_CSCONFIG),yes)

csconfigclean:
	-$(RM) $(CSCONFIG.EXE) 

endif
