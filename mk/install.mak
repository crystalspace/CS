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
#    to include/.  (max 3 levels deep now)
# TO_INSTALL.DOCS: also does not exist, the docs/html dir is copied (all html)
#    to docs/html, as well as all subdirs (2 deep) with gif, jpg, png; also
#    docs/README.html is copied, and docs/pubapi is copied (all html, gif, css).
# TO_INSTALL.SCRIPTS: does not exist. scripts/python is copied.
#==============================================================================

#------------------------------------------------------------- all defines ---#
ifneq ($(findstring defines,$(MAKESECTION)),)

INSTALL_DIR ?= /usr/local/crystal

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
	@echo $"  Installing Crystal Space SDK into $(INSTALL_DIR)$"
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

# For the 'include/' hierarchy, only the header files detected here
# will be copied
INSTALL_INCLUDE.FILES = $(wildcard $(addprefix include/,*.h */*.h */*/*.h))

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
  docs/html/*/*.jpg docs/html/*/*.gif docs/html/*/*.png  \
  docs/html/*/*/*.jpg docs/html/*/*/*.gif docs/html/*/*/*.png \
  docs/pubapi/*.html docs/pubapi/*.gif docs/pubapi/*.css )
INSTALL_DOCS.DIR1 = $(addprefix $(INSTALL_DIR)/, \
  $(patsubst %/,%,$(sort $(dir $(INSTALL_DOCS.FILES)))))

# Also include parent dirs in dirlist, for tutorial/mapcs 
INSTALL_DOCS.DIR = $(filter-out $(INSTALL_DIR), \
  $(sort $(INSTALL_DOCS.DIR1) $(patsubst %/,%,$(dir $(INSTALL_DOCS.DIR1)))))
INSTALL_DOCS.DESTFILES = \
  $(addprefix $(INSTALL_DIR)/,$(sort $(INSTALL_DOCS.FILES)))

# Files to install for scripts, includes parent dirs (for scripts/).
INSTALL_SCRIPTS.FILES = $(wildcard scripts/python/*.py)
INSTALL_SCRIPTS.DIR1 = $(addprefix $(INSTALL_DIR)/, \
  $(patsubst %/,%,$(sort $(dir $(INSTALL_SCRIPTS.FILES)))))
INSTALL_SCRIPTS.DIR = $(filter-out $(INSTALL_DIR), $(sort \
  $(INSTALL_SCRIPTS.DIR1) $(patsubst %/,%,$(dir $(INSTALL_SCRIPTS.DIR1)))))
INSTALL_SCRIPTS.DESTFILES = \
  $(addprefix $(INSTALL_DIR)/,$(sort $(INSTALL_SCRIPTS.FILES)))

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

# Rule for creating installation directories.
$(INSTALL_DIR) $(INSTALL_DIR)/bin $(INSTALL_DIR)/lib $(INSTALL_INCLUDE.DIR) \
$(INSTALL_DIR)/data $(INSTALL_DIR)/data/config:
	$(MKDIR)

# Install log, itself, should also be deleted.
install_logfile:
	@echo $(INSTALL_LOG) >> $(INSTALL_LOG)

# Install configuration files.
install_config: $(TO_INSTALL.CONFIG) $(INSTALL_DIR)/data/config
	$(CP) $(TO_INSTALL.CONFIG) $(INSTALL_DIR)/data/config
	@echo $(addprefix $(INSTALL_DIR)/data/config/, \
	  $(notdir $(TO_INSTALL.CONFIG))) >> $(INSTALL_LOG)

# Install data files.
install_data: $(INSTALL_DIR)/data
	$(CP) $(TO_INSTALL.DATA) $(INSTALL_DIR)/data
	@echo $(addprefix $(INSTALL_DIR)/data/, \
	  $(notdir $(TO_INSTALL.DATA))) >> $(INSTALL_LOG)

# Install dynamic libraries (plug-in modules).
install_dynamiclibs: $(INSTALL_DIR)/lib
	$(CP) $(TO_INSTALL.DYNAMIC_LIBS) $(INSTALL_DIR)/lib
	@echo $(addprefix $(INSTALL_DIR)/lib/, \
	  $(notdir $(TO_INSTALL.DYNAMIC_LIBS))) >> $(INSTALL_LOG)

# Install static libraries.
install_staticlibs: $(INSTALL_DIR)/lib
	$(CP) $(TO_INSTALL.STATIC_LIBS) $(INSTALL_DIR)/lib
	@echo $(addprefix $(INSTALL_DIR)/lib/, \
	  $(notdir $(TO_INSTALL.STATIC_LIBS))) >> $(INSTALL_LOG)

# Install executables.
install_exe: $(INSTALL_DIR)/bin
	$(CP) $(TO_INSTALL.EXE) $(INSTALL_DIR)/bin
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
	$(MKDIR)
	@echo $@/deleteme.dir >> $(INSTALL_LOG)

$(INSTALL_DOCS.DESTFILES): $(INSTALL_DIR)/docs/% : docs/%
	$(CP) $< $@
	@echo $@ >> $(INSTALL_LOG)

install_docs: $(INSTALL_DIR)/docs $(INSTALL_DOCS.DIR) \
  $(INSTALL_DOCS.DESTFILES)

# Install Scripts
$(INSTALL_SCRIPTS.DIR): 
	$(MKDIR)
	@echo $@/deleteme.dir >> $(INSTALL_LOG)

$(INSTALL_SCRIPTS.DESTFILES): $(INSTALL_DIR)/scripts/% : scripts/%
	$(CP) $< $@
	@echo $@ >> $(INSTALL_LOG)

install_scripts: $(INSTALL_DIR)/scripts $(INSTALL_SCRIPTS.DIR) \
  $(INSTALL_SCRIPTS.DESTFILES)

# The Big Kafoozy!
install_all: \
  $(TO_INSTALL.ROOT) \
  $(TO_INSTALL.CONFIG) \
  $(TO_INSTALL.DATA) \
  $(TO_INSTALL.EXE) \
  $(TO_INSTALL.STATIC_LIBS) \
  $(TO_INSTALL.DYNAMIC_LIBS) \
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
	@echo $"Installation complete.$"
	@echo $"Now define CRYSTAL=$(INSTALL_DIR) in your environment to tell$"
	@echo $"Crystal Space programs where to find their resource files.$"

endif # ifeq ($(DO_INSTALL),yes)

endif # ifeq ($(MAKESECTION),targets)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSMODIFIERSHELP += \
  $(NEWLINE)echo $"  INSTALL_DIR=path$" \
  $(NEWLINE)echo $"      Overrides the default installation destination.$" \
  $(NEWLINE)echo $"      Specify an absolute path (excluding the trailing /).$"

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#

ifeq ($(MAKESECTION)/$(ROOTCONFIG),rootdefines/config)

SYSCONFIG += $(NEWLINE)echo INSTALL_DIR = $(INSTALL_DIR)>>config.tmp

endif # ifeq ($(MAKESECTION)/$(ROOTCONFIG),rootdefines/config)
