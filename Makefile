################################################################################
#
#           This is the root makefile for the Crystal Space engine
#
################################################################################

.PHONY: help banner showplatforms showconfig platforms all depend configure \
  configbanner clean cleanlib cleandep distclean libs plugins drivers \
  drivers2d drivers3d snddrivers netdrivers meshes softcanvas

# The following two symbols are intended to be used in "echo" commands.
# config.mak can override them depending on configured platform's requirements.
"='
|=|
-include config.mak

# parameters for recursive make calls
RECMAKEFLAGS=--no-print-directory --no-builtin-rules
MAKEFLAGS+=r

include mk/user.mak
include mk/common.mak

# Find all available platform targets
SYSMAKEFILES=$(wildcard libs/cssys/*/*.mak)
SYSTARGETS=$(basename $(notdir $(SYSMAKEFILES)))

# The initial driver and application targets help text
SYSHELP = \
  echo $"The makefile system can be configured for the following platforms:$"
DRIVERHELP = \
  echo $"The following Crystal Space drivers can be built:$"
PLUGINHELP = \
  echo $"The following Crystal Space plug-in modules can be built:$"
APPHELP = \
  echo $"The following Crystal Space applications can be built:$"
LIBHELP = \
  echo $"The following Crystal Space libraries can be built:$"
DOCHELP = \
  echo $"The following Crystal Space documentation targets can be invoked:$"
define PSEUDOHELP
  echo $"The following pseudo targets can be invoked:$"
  echo $"  make all          Make all important targets$"
  echo $"  make apps         Make all applications$"
  echo $"  make libs         Make all static libraries$"
  echo $"  make plugins      Make all plug-in modules including drivers$"
  echo $"  make meshes       Make all mesh plugins$"
  echo $"  make drivers      Make all drivers$"
  echo $"  make drivers2d    Make all supported 2D graphics drivers$"
  echo $"  make drivers3d    Make all supported 3D graphics drivers (renderers)$"
  echo $"  make snddrivers   Make all supported sound drivers$"
  echo $"  make netdrivers   Make all supported network drivers$"
  echo $"  make depend       Make dependencies (recommended!)$"
  echo $"  make clean        Clean all files generated during build$"
  echo $"  make cleanlib     Clean all dynamic libraries$"
  echo $"  make cleandep     Clean all dependency rule files$"
  echo $"  make distclean    Clean everything$"
  echo $"  make platforms    List the available target platforms$"
endef
define SYSMODIFIERSHELP
  echo $(SEPARATOR)
  echo $"The following optional configuration modifiers may be specified:$"
  echo $"$"
  echo $"  USE_PLUGINS=yes$|no$"
  echo $"      Build drivers/plugins as dynamic/static modules$"
  echo $"  MODE=optimize$|debug$|profile$"
  echo $"      Select one of three available compilation modes$"
endef

# If there is no target defined (makefile system was not configured),
# find the available targets and present the list to the user.
ifeq ($(TARGET),)

MAKESECTION=confighelp
include mk/install.mak
-include $(SYSMAKEFILES)

help: banner showplatforms

else

MAKESECTION=rootdefines
include mk/subs.mak

help: banner showconfig driverhelp pluginhelp libhelp apphelp dochelp \
  pseudohelp

dep:
	@$(MAKE) $(RECMAKEFLAGS) -f mk/cs.mak $@ DO_DEPEND=yes

depend: cleandep dep

clean cleanlib cleandep distclean:
	@$(MAKE) $(RECMAKEFLAGS) -f mk/cs.mak $@

unknown:
	$(RM) config.mak
	$(RM) include/volatile.h

platforms:
	@echo $(SEPARATOR)
	@$(MAKE) $(RECMAKEFLAGS) showplatforms TARGET="" TARGET_MAKEFILE=""

showconfig:
	@echo $"Configured for $(DESCRIPTION.$(TARGET)) with the following modifiers:$"
	@echo $"  USE_PLUGINS=$(USE_PLUGINS)$"
	@echo $"  MODE=$(MODE)$"
	@$(SYSMODIFIERS)
	@echo $(SEPARATOR)

driverhelp:
	@$(DRIVERHELP)
	@echo $(SEPARATOR)

pluginhelp:
	@$(PLUGINHELP)
	@echo $(SEPARATOR)

libhelp:
	@$(LIBHELP)
	@echo $(SEPARATOR)

apphelp:
	@$(APPHELP)
	@echo $(SEPARATOR)

dochelp:
	@$(DOCHELP)
	@echo $(SEPARATOR)

pseudohelp:
	@$(PSEUDOHELP)
	@echo $(SEPARATOR)

endif

banner:
	@echo $(SEPARATOR)
	@echo $"  Before compiling Crystal Space examine mk/user.mak and see if settings$"
	@echo $"  are suited to your system.$"
	@echo $(SEPARATOR)

showplatforms:
	@$(SYSHELP)
	@$(SYSMODIFIERSHELP)
	@echo $"$"
	@echo $"Example: make linux USE_PLUGINS=yes MODE=debug$"
	@echo $(SEPARATOR)

# Prepare for specific platform.
# WARNING: Try to avoid quotes in most important "echo" statements
# since several platforms (OS/2, DOS and WIN32) have a "echo" that does
# literal output, i.e. they do not strip quotes from string.
$(SYSTARGETS):
	@echo TARGET = $@>config.tmp
	@echo TARGET_MAKEFILE = $(filter %$@.mak,$(SYSMAKEFILES))>>config.tmp
	@$(MAKE) $(RECMAKEFLAGS) ROOTCONFIG=config configure \
		TARGET=$@ TARGET_MAKEFILE=$(filter %$@.mak,$(SYSMAKEFILES))

ifeq ($(ROOTCONFIG),config)

# Force config.tmp to be always rebuilt
.PHONY: config.tmp

configure: configbanner config.tmp
	@$(MAKE) $(RECMAKEFLAGS) ROOTCONFIG=volatile configure

configbanner:
	@echo $(SEPARATOR)
	@echo $"  Configuring makefiles for $(DESCRIPTION.$(TARGET)) [$(MODE)]$"
	@echo $(SEPARATOR)

config.tmp:
	@echo MODE = $(MODE)>>config.tmp
	@echo USE_PLUGINS = $(USE_PLUGINS)>>config.tmp
	@$(SYSCONFIG)
	$(subst DEST,config.mak,$(UPD))

endif
ifeq ($(ROOTCONFIG),volatile)

include mk/volatile.mak

configure: volatile.tmp

volatile.tmp: config.mak
	@$(DO.MAKE.VOLATILE)
	$(subst DEST,include/volatile.h,$(UPD))

endif

MAKESECTION=roottargets
include mk/subs.mak
# DO NOT DELETE
