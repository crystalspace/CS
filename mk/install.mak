#==============================================================================
# Installation Makefile
#
# Module makefiles may append resources to the following variables (using +=):
#
# TO_INSTALL.ROOT:   files will be added in the $(INSTALL_DIR)/ dir itself.
# TO_INSTALL.EXE:    files will be put in bin/
# TO_INSTALL.DATA:   files will be put in data/
# TO_INSTALL.CONFIG: files will be put in data/config/
# TO_INSTALL.STATIC_LIBS:  files will be put in lib/
# TO_INSTALL.DYNAMIC_LIBS: files could be put in lib/, but could also end up in
#    a platform specific location (e.g.  System folder).
#
# Always copied:
# TO_INSTALL.INCLUDE: does not exist, the entire include/ hierarchy is copied
#    to include/.  (max 6 levels deep)
# TO_INSTALL.DOCS: also does not exist, the docs/html dir is copied (all html)
#    to docs/html, as well as all subdirs (6 deep) with gif, jpg, png; also
#    docs/README.html is copied, and docs/pubapi is copied (all html, gif, css).
# TO_INSTALL.SCRIPTS: does not exist. scripts/python is copied.
#==============================================================================

#------------------------------------------------------------- all defines ---#
ifneq ($(findstring defines,$(MAKESECTION)),)

ifeq (,$(strip $(INSTALL_DIR)))
  INSTALL_DIR = /usr/local/crystal
endif

endif # ifneq ($(findstring defines,$(MAKESECTION)),)

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PSEUDOHELP += \
  $(NEWLINE)echo $"  make install      Install Crystal Space SDK$" \
  $(NEWLINE)echo $"  make uninstall    Uninstall Crystal Space SDK$"

SYSMODIFIERS += $(NEWLINE)echo $"  INSTALL_DIR=$(INSTALL_DIR)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: install uninstall

install:
	@echo $(SEPARATOR)
	@echo $"  Installing Crystal Space SDK$"
	@echo $"  INSTALL_DIR=$(INSTALL_DIR)$"
	@echo $(SEPARATOR)
	@$(MAKE) $(RECMAKEFLAGS) -f mk/cs.mak install_all DO_INSTALL=yes

uninstall: uninstexe
	uninst $(INSTALL_DIR)/install.log

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# This section is specially protected by DO_INSTALL in order to prevent the
# lengthy $(wildcard) operations from impacting _all_ other build targets.
# DO_INSTALL is only defined when the top-level 'install' target is invoked.

ifeq ($(DO_INSTALL),yes)

INSTALL_LOG = $(INSTALL_DIR)/install.log

INSTALL_DEPTH=* */* */*/* */*/*/* */*/*/*/* */*/*/*/*/*

# For the 'include/' hierarchy, only the header files detected here
# will be copied
INSTALL_INCLUDE.FILES = \
  $(wildcard $(addprefix include/,$(addsuffix .h,$(INSTALL_DEPTH))))

# Given all .h files in 'include/', take their directory parts, sort those,
# and remove trailing '/', then add the INSTALL_DIR prefix
INSTALL_INCLUDE.DIR = $(addprefix $(INSTALL_DIR)/, \
  $(patsubst %/, %,$(sort $(dir $(INSTALL_INCLUDE.FILES)))))
INSTALL_INCLUDE.DESTFILES = $(addprefix $(INSTALL_DIR)/, \
  $(patsubst %/, %,$(sort $(INSTALL_INCLUDE.FILES))))

# Install docs/html into INSTALL_DIR/docs/html, docs/pubapi to
# INSTALL_DIR/docs/pubapi and copy docs/README.html & docs/history.{txt|old}.
INSTALL_DOCS.FILES = docs/README.html docs/history.txt docs/history.old \
  $(wildcard docs/html/*.html \
    $(addprefix docs/html,\
      $(addsuffix .jpg,$(INSTALL_DEPTH)) \
      $(addsuffix .gif,$(INSTALL_DEPTH)) \
      $(addsuffix .png,$(INSTALL_DEPTH))) \
    $(addprefix docs/pubapi/,*.html *.gif *.css))
INSTALL_DOCS.DIR = $(addprefix $(INSTALL_DIR)/, \
  $(patsubst %/,%,$(sort $(dir $(INSTALL_DOCS.FILES)))))
INSTALL_DOCS.DESTFILES = \
  $(addprefix $(INSTALL_DIR)/,$(sort $(INSTALL_DOCS.FILES)))

# Files to install for scripts.
INSTALL_SCRIPTS.FILES = $(wildcard scripts/python/*.py)
INSTALL_SCRIPTS.DIR = $(addprefix $(INSTALL_DIR)/, \
  $(patsubst %/,%,$(sort $(dir $(INSTALL_SCRIPTS.FILES)))))
INSTALL_SCRIPTS.DESTFILES = \
  $(addprefix $(INSTALL_DIR)/,$(sort $(INSTALL_SCRIPTS.FILES)))

# Static library and plugin directories.
INSTALL_LIB.DIR = $(INSTALL_DIR)/lib
ifneq (.,$(OUTDLL))
  INSTALL_DLL.DIR = $(INSTALL_DIR)/$(OUTDLL)
else
  INSTALL_DLL.DIR = $(INSTALL_LIB.DIR)
endif

# Data directories.
INSTALL_DATA.DIR = $(addprefix $(INSTALL_DIR)/, \
  $(patsubst %/,%,$(sort $(dir $(TO_INSTALL.DATA)))))

# Command to copy a potentially deeply nested file to the installation
# directory even while preserving the nesting.  The file to be copied must be
# stored in a variable named F.  Assumes that the target directory tree
# already exists.  The empty line in this macro is important since it results
# in inclusion of a newline.  This is desirable because we expect this macro
# to be invoked from $(foreach) for a set of files, and we want the expansion
# to be a series of copy commands, one per line.
define INSTALL.DEEP_COPY
  $(CP) $(F) $(INSTALL_DIR)/$(patsubst ./%,%,$(F))

endef

# Command to run "ranlib" on an installed static library archive.  The archive
# to be processed must be stored in a variable named F.  The empty line in this
# macro is important since it results in inclusion of a newline.  This is
# desirable because we expect this macro to be invoked from $(foreach) for a
# set of archives, and we want the expansion to be a series of "ranlib"
# commands, one per line.
define INSTALL.RANLIB
  $(CMD.RANLIB) $(F)

endef

endif # ifeq ($(DO_INSTALL),yes)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

# This section is specially protected by DO_INSTALL in order to prevent the
# lengthy $(wildcard) operations from impacting _all_ other build targets.
# DO_INSTALL is only defined when the top-level 'install' target is invoked.

ifeq ($(DO_INSTALL),yes)

.PHONY: install_config install_data install_dynamiclibs install_staticlibs \
  install_exe install_include install_root install_logfile install_docs \
  install_scripts install_all

# Rules for creating installation directories.
$(INSTALL_DIR) $(INSTALL_DIR)/bin $(INSTALL_LIB.DIR) $(INSTALL_INCLUDE.DIR) \
$(INSTALL_DATA.DIR) $(INSTALL_DIR)/data/config:
	$(MKDIRS)

ifneq (.,$(OUTDLL))
$(INSTALL_DLL.DIR):
	$(MKDIRS)
endif

# Install log, itself, should also be deleted.
install_logfile:
	@echo $(INSTALL_LOG) >> $(INSTALL_LOG)

# Install configuration files.
install_config: $(TO_INSTALL.CONFIG) $(INSTALL_DIR)/data/config
	$(foreach F,$(TO_INSTALL.CONFIG),$(INSTALL.DEEP_COPY))
	@echo $(addprefix $(INSTALL_DIR)/data/config/, \
	  $(notdir $(TO_INSTALL.CONFIG))) >> $(INSTALL_LOG)

# Install data files.
install_data: $(INSTALL_DATA.DIR)
	$(foreach F,$(TO_INSTALL.DATA),$(INSTALL.DEEP_COPY))
	@echo $(addprefix $(INSTALL_DIR)/data/, \
	  $(notdir $(TO_INSTALL.DATA))) >> $(INSTALL_LOG)

# Install dynamic libraries (plug-in modules).
ifeq ($(USE_PLUGINS),yes)
install_dynamiclibs: $(INSTALL_DLL.DIR)
	$(CP) -r $(TO_INSTALL.DYNAMIC_LIBS) $(INSTALL_DLL.DIR)
	@echo $(addprefix $(INSTALL_DLL.DIR)/, \
	  $(notdir $(TO_INSTALL.DYNAMIC_LIBS))) >> $(INSTALL_LOG)
endif

# Install static libraries.
install_staticlibs: $(INSTALL_LIB.DIR)
	$(CP) $(TO_INSTALL.STATIC_LIBS) $(INSTALL_LIB.DIR)
ifneq (,$(CMD.RANLIB))
	$(foreach F,$(addprefix $(INSTALL_LIB.DIR)/,\
	$(notdir $(TO_INSTALL.STATIC_LIBS))),$(INSTALL.RANLIB))
endif
	@echo $(addprefix $(INSTALL_LIB.DIR)/, \
	  $(notdir $(TO_INSTALL.STATIC_LIBS))) >> $(INSTALL_LOG)

# Install executables.
install_exe: $(INSTALL_DIR)/bin
	$(CP) -r $(TO_INSTALL.EXE) $(INSTALL_DIR)/bin
	@echo $(addprefix $(INSTALL_DIR)/bin/, \
	  $(notdir $(TO_INSTALL.EXE))) >> $(INSTALL_LOG)

# Install top-level files.
install_root: $(INSTALL_DIR)
	$(CP) $(TO_INSTALL.ROOT) $(INSTALL_DIR)
	@echo $(addprefix $(INSTALL_DIR)/, \
	  $(notdir $(TO_INSTALL.ROOT))) >> $(INSTALL_LOG)

# Install headers.
$(INSTALL_INCLUDE.DESTFILES): $(INSTALL_DIR)/% : %
	$(CP) $< $@
	@echo $@ >> $(INSTALL_LOG)

install_include: $(INSTALL_DIR)/include $(INSTALL_INCLUDE.DIR) \
  $(INSTALL_INCLUDE.DESTFILES)

# Install documentation.
$(INSTALL_DOCS.DIR): 
	$(MKDIRS)
	@echo $@/deleteme.dir >> $(INSTALL_LOG)

$(INSTALL_DOCS.DESTFILES): $(INSTALL_DIR)/docs/% : docs/%
	$(CP) $< $@
	@echo $@ >> $(INSTALL_LOG)

install_docs: $(INSTALL_DIR)/docs $(INSTALL_DOCS.DIR) \
  $(INSTALL_DOCS.DESTFILES)

# Install Scripts
$(INSTALL_DIR)/scripts $(INSTALL_SCRIPTS.DIR): 
	$(MKDIRS)
	@echo $@/deleteme.dir >> $(INSTALL_LOG)

$(INSTALL_SCRIPTS.DESTFILES): $(INSTALL_DIR)/scripts/% : scripts/%
	$(CP) $< $@
	@echo $@ >> $(INSTALL_LOG)

install_scripts: $(INSTALL_DIR)/scripts $(INSTALL_SCRIPTS.DIR) \
  $(INSTALL_SCRIPTS.DESTFILES)

# The Big Kafoozy!
# ***NOTE***
# Do not make the `install_all' target depend upon TO_INSTALL.DYNAMIC_LIBS.
# The current makefile mechanism is incapable of correctly building out of date
# or missing plugin modules once the `install' target has been invoked,
# therefore the `install_all' target must not depend upon any plugin modules.
# The technical reason for the inability to regenerate plugin modules at this
# late stage is that such modules must be built with the MAKE_DLL makefile
# variable set to `yes', yet by the time `install_all' is running, it is too
# late to set the MAKE_DLL variable.  Furthermore, this variable must only be
# set for plugin modules.  It can not be set for static libraries.  Therefore,
# trying to use a blanket approach of globally enabling MAKE_DLL during the
# installation process will not succeed.
install_all: \
  $(TO_INSTALL.ROOT) \
  $(TO_INSTALL.CONFIG) \
  $(TO_INSTALL.DATA) \
  $(TO_INSTALL.EXE) \
  $(TO_INSTALL.STATIC_LIBS) \
  $(INSTALL_DIR) \
  install_data \
  install_config \
  install_dynamiclibs \
  install_staticlibs \
  install_exe \
  install_include \
  install_docs \
  install_scripts \
  install_root \
  install_logfile
	@echo $"$"
	@echo $"Creating Lightmaps...$"
	@echo $"------------------------$"
	@echo $"If this fails, use the -relight option when viewing maps$"
	@echo $"for the first time with Walktest.$"
	-CRYSTAL=$"$(INSTALL_DIR)$" \
	if test -f $(INSTALL_DIR)/bin/cslight.app/Contents/MacOS/cslight ; \
	then \
	$(INSTALL_DIR)/bin/cslight.app/Contents/MacOS/cslight -video=null flarge ; \
	$(INSTALL_DIR)/bin/cslight.app/Contents/MacOS/cslight -video=null partsys ; \
	else \
	$(INSTALL_DIR)/bin/cslight -video=null flarge ; \
	$(INSTALL_DIR)/bin/cslight -video=null partsys ; \
	fi
	@echo $"$"
	@echo $"Installation complete$"
	@echo $"---------------------$"
	@echo $"Now set up the following definition in your environment to$"
	@echo $"let Crystal Space programs know where their resources are.$"
	@echo $"CRYSTAL=$(INSTALL_DIR)$"

endif # ifeq ($(DO_INSTALL),yes)

endif # ifeq ($(MAKESECTION),targets)
