# Target description
DESCRIPTION.cs-config = cs-config build options script

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PSEUDOHELP += \
  $(NEWLINE)echo $"  make cs-config    Build the $(DESCRIPTION.cs-config)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cs-config cs-configclean

all: cs-config
cs-config:
	$(MAKE_APP) DO_CREATE_CSCONFIG=yes
cs-configclean:
	$(MAKE_CLEAN)

endif

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSCONFIG.EXE = cs-config
CSCONFIG.DEP = \
  config.mak \
  $(SRCDIR)/mk/user.mak \
  $(wildcard $(SRCDIR)/mk/local.mak) \
  $(wildcard local.mak) \
  $(SRCDIR)/$(TARGET_MAKEFILE) \
  $(SRCDIR)/scripts/cs-config/cs-config.temppre \
  $(SRCDIR)/scripts/cs-config/cs-config.temppost
CSCONFIG.TMP = $(OUT)/csconfig.tmp

TO_INSTALL.EXE	+= $(CSCONFIG.EXE)
# @@@ Enable this when TO_INSTALL.ETC functionality is added to install.mak.
#TO_INSTALL.ETC += $(wildcard $(SRCDIR)/mk/autoconf/*.m4)

# This section is specially protected by DO_CREATE_CSCONFIG in order to prevent
# execution of sed commands for _all_ other build targets.  DO_CREATE_CSCONFIG
# is only defined when the top-level `csconfig' target is invoked.

ifeq ($(DO_CREATE_CSCONFIG),yes)

CSCONFIG.VERFILE := $(SRCDIR)/include/csver.h
CSCONFIG.VMAJOR  := $(shell sed -e '/\#define[ 	][ 	]*CS_VERSION_MAJOR/!d' -e 's/\#define[ 	][ 	]*CS_VERSION_MAJOR[ 	][ 	]*CS_VER_QUOTE(\(..*\)).*/\1/' < $(CSCONFIG.VERFILE))
CSCONFIG.VMINOR  := $(shell sed -e '/\#define[ 	][ 	]*CS_VERSION_MINOR/!d' -e 's/\#define[ 	][ 	]*CS_VERSION_MINOR[ 	][ 	]*CS_VER_QUOTE(\(..*\)).*/\1/' < $(CSCONFIG.VERFILE))
CSCONFIG.RDATE   := $(shell sed -e '/\#define[ 	][ 	]*CS_RELEASE_DATE/!d'  -e 's/\#define[ 	][ 	]*CS_RELEASE_DATE[ 	][ 	]*CS_VER_QUOTE(\(..*\)).*/\1/' < $(CSCONFIG.VERFILE))

# Extract needed makefile fragments directly from target makefile.
CSCONFIG.MAKEFRAG = \
  cat $(wildcard $(SRCDIR)/mk/*.mak) $(SRCDIR)/$(TARGET_MAKEFILE) | \
  sed '/<cs-config>/,/<\/cs-config>/!d;/<cs-config>/d;/<\/cs-config>/d'

endif # ifeq ($(DO_CREATE_CSCONFIG),yes)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.cs-config cs-configclean

all: $(CSCONFIG.EXE)
clean: cs-configclean

ifeq ($(DO_CREATE_CSCONFIG),yes)

build.cs-config: $(OUTDIRS) $(CSCONFIG.EXE)

# Create csconfig.tmp for the make variables that need to be transferred.
$(CSCONFIG.EXE): $(CSCONFIG.DEP)
	@echo Generating cs-config script...
	@$(RM) cs-config

	@cat $(SRCDIR)/scripts/cs-config/cs-config.temppre > cs-config
	@echo $"# WARNING: This script is generated automatically!$" >> cs-config
	@echo >> cs-config
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
	@echo $"syslibs="$(LIBS.EXE) $(LFLAGS.GENERAL)"$" >> cs-config
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
	@echo $"CSTHREAD.CFLAGS=$(CSTHREAD.CFLAGS)$"	>> cs-config
	@echo $"CSTHREAD.LFLAGS=$(CSTHREAD.LFLAGS)$"	>> cs-config
	@echo $"CSTHREAD.LIBS=$(CSTHREAD.LIBS)$"	>> cs-config
	@$(CSCONFIG.MAKEFRAG) >> cs-config
	@echo $"EOF$"					>> cs-config
	@echo $"}$"					>> cs-config
	@echo						>> cs-config
	@cat $(SRCDIR)/scripts/cs-config/cs-config.temppost >> cs-config

	@chmod +x cs-config

endif # ifeq ($(DO_CREATE_CSCONFIG),yes)

cs-configclean:
	-$(RM) $(CSCONFIG.EXE) 

endif
