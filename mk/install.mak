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
#    a platform specific location (e.g. System folder).
# TO_INSTALL.SCRIPTS: files will be placed under appropriately named
#    subdirectories of scripts/
#
# Always copied:
# INSTALL_INCLUDE.FILES: does not exist, the entire include/ hierarchy is
#    copied to include/.  (max 6 levels deep)
# INSTALL_DOCS.FILES: also does not exist, the docs/html dir is copied (all
#    html) to docs/html, as well as all subdirs (8 deep) with gif, jpg, png,
#    css; also docs/README.html is copied.
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
.PHONY: install_config install_data install_plugin install_lib \
  install_bin install_include install_root install_doc install_script

define INSTALL_COMPONENT
	@echo $(SEPARATOR)
	@echo $"  Installing Crystal Space SDK (component: $@)$"
	@echo $"  INSTALL_DIR=$(INSTALL_DIR)$"
	@echo $(SEPARATOR)
	+@$(MAKE) $(RECMAKEFLAGS) -f $(SRCDIR)/mk/cs.mak $@ DO_INSTALL=yes
endef

install install_config install_data install_plugin install_lib \
  install_bin install_include install_root install_doc install_script:
	$(INSTALL_COMPONENT)

uninstall: uninstexe
	uninst $(INSTALL_DIR)/install.log

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# This section is specially protected by DO_INSTALL in order to prevent the
# lengthy $(wildcard) operations from impacting _all_ other build targets.
# DO_INSTALL is only defined when the top-level 'install' target is invoked.

ifeq ($(DO_INSTALL),yes)

INSTALL_INCLUDE.DIR = $(INSTALL_DIR)/include
INSTALL_DOCS.DIR = $(INSTALL_DIR)/docs
INSTALL_SCRIPTS.DIR = $(INSTALL_DIR)/scripts
INSTALL_DATA.DIR = $(INSTALL_DIR)/data
INSTALL_CONFIG.DIR = $(INSTALL_DATA.DIR)/config
INSTALL_EXE.DIR = $(INSTALL_DIR)/bin
INSTALL_LIB.DIR = $(INSTALL_DIR)/lib
ifneq (.,$(OUTDLL))
  INSTALL_DLL.DIR = $(INSTALL_DIR)/$(OUTDLL)
else
  INSTALL_DLL.DIR = $(INSTALL_LIB.DIR)
endif


INSTALL_LOG = $(INSTALL_DIR)/install.log

INSTALL_DEPTH=* */* */*/* */*/*/* */*/*/*/* */*/*/*/*/* */*/*/*/*/*/* \
  */*/*/*/*/*/*/*

# For the 'include/' hierarchy, only the header files and Swig input files
# detected here will be copied.
INSTALL_INCLUDE.FILES = $(wildcard $(addprefix $(SRCDIR)/include/,\
  $(foreach s,.h .i,$(addsuffix $s,$(INSTALL_DEPTH)))))

# Install docs/html into INSTALL_DIR/docs/html; copy docs/README.html and
# docs/history.* into INSTALL_DIR/docs.
INSTALL_DOCS.FILES = \
  $(SRCDIR)/docs/README.html \
  $(SRCDIR)/docs/history.txt \
  $(SRCDIR)/docs/history.old \
  $(wildcard $(SRCDIR)/docs/html/*/*.html \
    $(addprefix $(SRCDIR)/docs/html,\
      $(addsuffix .jpg,$(INSTALL_DEPTH)) \
      $(addsuffix .gif,$(INSTALL_DEPTH)) \
      $(addsuffix .png,$(INSTALL_DEPTH)) \
      $(addsuffix .css,$(INSTALL_DEPTH))))

# Macro to copy a potentially deeply nested file from the source tree to the
# installation directory while preserving the nesting.  Creates the destination
# directory if needed.  The file to be copied must be stored in a variable
# named F.  Also takes care of recording the installation in $(INSTALL_LOG).
# IMPLEMENTATION NOTE: The empty line in this macro is important since it
# results in inclusion of a newline.  This is desirable because we expect this
# macro to be invoked from $(foreach) for a set of files, and we want the
# expansion to be a series of copy commands, one per line.
define INSTALL.DEEP_COPY
  @test -r $(INSTALL.DEEP_DIR) || { $(CMD.MKDIRS) $(INSTALL.DEEP_DIR); \
    echo $"$(INSTALL.DEEP_DIR)/deleteme.dir$" >> $(INSTALL_LOG); }
  $(CP) $(F) $(INSTALL_DIR)/$(subst $(SRCDIR)/,,$(F))
  @echo $"$(INSTALL_DIR)/$(subst $(SRCDIR)/,,$(F))$" >> $(INSTALL_LOG)

endef
INSTALL.DEEP_DIR = \
  $(patsubst %/,%,$(dir $(INSTALL_DIR)/$(subst $(SRCDIR)/,,$(F))))

# Macro to copy a plugin module to the installation directory.  Creates the
# destination directory if needed.  The plugin to be copied must be stored in a
# variable named F.  This default implementation copies the named file to the
# installation directory for plugins along with a second file of the same name
# but with extension .csplugin, if the .csplugin file exists.  This is suitable
# for platforms where a plugin's meta information resides in a separate file
# alongside the plugin itself, or when it is embedded directly into the plugin
# file.  Other platforms with different requirements may have to override this
# command with a more suitable one.  For instance, on MacOS/X, meta information
# might be bundled inside the plugin's "wrapper", rather than living alongside
# the plugin.  It is also the responsibility of this command to make an
# appropriate entry (or multiple entries) in the install log, $(INSTALL_LOG),
# for each installed plugin.  This macro assumes that the target directory tree
# already exists.  IMPLEMENTATION NOTE: The empty line in this macro is
# important since it results in inclusion of a newline.  This is desirable
# because we expect this macro to be invoked from $(foreach) for a set of
# files, and we want the expansion to be a series of copy commands, one per
# line.
ifeq (,$(strip $(INSTALL.DO_PLUGIN)))
define INSTALL.DO_PLUGIN
  @test -r $(INSTALL_DLL.DIR) || { $(CMD.MKDIRS) $(INSTALL_DLL.DIR); \
    echo $"$(INSTALL_DLL.DIR)/deleteme.dir$" >> $(INSTALL_LOG); }
  $(CP) $(F) $(INSTALL_DLL.DIR)/$(notdir $(F))
  @echo $"$(INSTALL_DLL.DIR)/$(notdir $(F))$" >> $(INSTALL_LOG)
  if test -f $(INSTALL.CSPLUGIN); then \
  $(CP) $(INSTALL.CSPLUGIN) $(INSTALL_DLL.DIR)/$(INSTALL.CSPLUGIN.BASE); \
  echo $"$(INSTALL_DLL.DIR)/$(INSTALL.CSPLUGIN.BASE)$" >> $(INSTALL_LOG); \
  fi

endef
INSTALL.CSPLUGIN = $(patsubst %$(DLL),%.csplugin,$(F))
INSTALL.CSPLUGIN.BASE = $(notdir $(INSTALL.CSPLUGIN))
endif

# Macro to copy a library to the installation directory.  Creates the
# destination directory if needed.  The library to be copied must be stored in
# a variable named F.  Also takes care of recording the installation in
# $(INSTALL_LOG) and running $(RANLIB) on the installed library.
define INSTALL.DO_LIBRARY
  @test -r $(INSTALL_LIB.DIR) || { $(CMD.MKDIRS) $(INSTALL_LIB.DIR); \
    echo $"$(INSTALL_LIB.DIR)/deleteme.dir$" >> $(INSTALL_LOG); }
  $(CP) $(F) $(INSTALL_LIB.DIR)/$(notdir $(F))
  @echo $"$(INSTALL_LIB.DIR)/$(notdir $(F))$" >> $(INSTALL_LOG)
  $(RANLIB) $(INSTALL_LIB.DIR)/$(notdir $(F))

endef
ifeq (,$(RANLIB))
RANLIB = @:$(SPACE)
endif

# Macro to copy a resource to the root installation directory.  Creates the
# destination directory if needed.  The resource to be copied must be stored in
# a variable named F.  Also takes care of recording the installation in
# $(INSTALL_LOG).
define INSTALL.DO_ROOT
  @test -r $(INSTALL_DIR) || { $(CMD.MKDIRS) $(INSTALL_DIR); \
    echo $"$(INSTALL_DIR)/deleteme.dir$" >> $(INSTALL_LOG); }
  $(CP) $(F) $(INSTALL_DIR)/$(notdir $(F))
  @echo $"$(INSTALL_DIR)/$(notdir $(F))$" >> $(INSTALL_LOG)

endef

# Macro to copy an executable file to the installation directory.  Creates the
# destination directory if needed.  The executable to be copied must be stored
# in a variable named F.  Also takes care of recording the installation in
# $(INSTALL_LOG). Uses recursive copy (-r) to handle MacOS/X .app wrappers.
define INSTALL.DO_EXE
  @test -r $(INSTALL_EXE.DIR) || { $(CMD.MKDIRS) $(INSTALL_EXE.DIR); \
    echo $"$(INSTALL_EXE.DIR)/deleteme.dir$" >> $(INSTALL_LOG); }
  $(CP) -r $(F) $(INSTALL_EXE.DIR)/$(notdir $(F))
  @echo $"$(INSTALL_DIR)/bin/$(notdir $(F))$" >> $(INSTALL_LOG)

endef

endif # ifeq ($(DO_INSTALL),yes)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

# This section is specially protected by DO_INSTALL in order to prevent the
# lengthy $(wildcard) operations from impacting _all_ other build targets.
# DO_INSTALL is only defined when the top-level 'install' target is invoked.

ifeq ($(DO_INSTALL),yes)

.PHONY: install_config install_data install_plugin install_lib \
  install_bin install_include install_root install_logfile install_doc \
  install_script install_all

# Install log, itself, should also be deleted.
install_logfile:
	@echo $"$(INSTALL_LOG)$" >> $(INSTALL_LOG)

# Install configuration files.
install_config: $(TO_INSTALL.CONFIG)
	$(foreach F,$(TO_INSTALL.CONFIG),$(INSTALL.DEEP_COPY))

# Install data files.
install_data: $(TO_INSTALL.DATA)
	$(foreach F,$(TO_INSTALL.DATA),$(INSTALL.DEEP_COPY))

# Install dynamic libraries (plug-in modules).
# ***NOTE***
# Do not make the `install_plugin' target depend upon TO_INSTALL.DYNAMIC_LIBS.
# The current makefile mechanism is wholely incapable of correctly building out
# of date or missing plugin modules once the `install' target has been invoked,
# therefore the `install_plugin' target must not depend upon any plugin
# modules.  The technical reason for the inability to regenerate plugin modules
# at this late stage is that such modules must be built with the MAKE_DLL
# makefile variable set to `yes', yet by the time `install_plugin' is running,
# it is too late to set the MAKE_DLL variable.  Furthermore, this variable must
# only be set for plugin modules.  It can not be set for static libraries.
# Therefore, trying to use a blanket approach of globally enabling MAKE_DLL
# during the installation process will not succeed.
ifeq ($(USE_PLUGINS),yes)
install_plugin:
	$(foreach F,$(TO_INSTALL.DYNAMIC_LIBS),$(INSTALL.DO_PLUGIN))
endif

# Install static libraries.
install_lib: $(TO_INSTALL.STATIC_LIBS)
	$(foreach F,$(TO_INSTALL.STATIC_LIBS),$(INSTALL.DO_LIBRARY))

# Install executables.
install_bin: $(TO_INSTALL.EXE)
	$(foreach F,$(TO_INSTALL.EXE),$(INSTALL.DO_EXE))

# Install top-level files.
install_root: $(TO_INSTALL.ROOT)
	$(foreach F,$(TO_INSTALL.ROOT),$(INSTALL.DO_ROOT))

# Install headers.
install_include:
	$(foreach F,$(INSTALL_INCLUDE.FILES),$(INSTALL.DEEP_COPY))

# If the build directory differs from the source directory, we must take
# special care to install the generated $(builddir)/include/volatile.h.
ifneq ($(SRCDIR),.)
.PHONY: install_volatile
install_include: install_volatile
install_volatile:
	@test -r $(INSTALL_DIR)/include || \
	{ $(CMD.MKDIRS) $(INSTALL_DIR)/include; \
	echo $"$(INSTALL_DIR)/include/deleteme.dir$" >> $(INSTALL_LOG); }
	$(CP) include/volatile.h $(INSTALL_DIR)/include/volatile.h
	@echo $"$(INSTALL_DIR)/include/volatile.h$" >> $(INSTALL_LOG)
endif

# Install documentation.
install_doc:
	$(foreach F,$(INSTALL_DOCS.FILES),$(INSTALL.DEEP_COPY))

# Install Scripts
install_script: $(TO_INSTALL.SCRIPTS)
	$(foreach F,$(TO_INSTALL.SCRIPTS),$(INSTALL.DEEP_COPY))

.PHONY: install_build_lightmaps
install_build_lightmaps:
	@echo $"$"
	@echo $"Creating Lightmaps...$"
	@echo $"---------------------$"
	-@CRYSTAL=$"$(INSTALL_DIR)$" ; \
	export CRYSTAL ; \
	if test -f $(INSTALL_DIR)/bin/cslight.app/Contents/MacOS/cslight ; \
	then \
	$(INSTALL_DIR)/bin/cslight.app/Contents/MacOS/cslight -video=null flarge ; \
	$(INSTALL_DIR)/bin/cslight.app/Contents/MacOS/cslight -video=null partsys ; \
	else \
	$(INSTALL_DIR)/bin/cslight -video=null flarge ; \
	$(INSTALL_DIR)/bin/cslight -video=null partsys ; \
	fi
	@echo $"---------------------$"
	@echo $"If automatic lightmap creation failed (see above), use the$"
	@echo $"-relight option when viewing maps for the first time with$"
	@echo $"Walktest.$"
	@echo $"$"
	@echo $"Installation complete$"
	@echo $"---------------------$"
	@echo $"Now set up the following definition in your environment to$"
	@echo $"let Crystal Space programs know where their resources are.$"
	@echo $"CRYSTAL=$(INSTALL_DIR)$"

# The Big Kafoozy!
install_all: \
  install_data \
  install_config \
  install_plugin \
  install_lib \
  install_bin \
  install_include \
  install_doc \
  install_script \
  install_root \
  install_logfile

endif # ifeq ($(DO_INSTALL),yes)

endif # ifeq ($(MAKESECTION),targets)
