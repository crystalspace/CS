################################################################################
#
#           This is the root makefile for the Crystal Space engine
#
################################################################################

.PHONY: help all doc api depend clean cleanlib cleandep libs \
	drivers drivers2d drivers3d snddrivers netdrivers \
	linux solaris beos os2gcc os2wcc djgpp freebsd nextstep openstep \
	rhapsody amiga

# The default values for configuration variables
TARGET=unknown
SYSMAKEFILE=mk/system/unknown.mak
USE_DLL=yes
MODE=optimize
# The following two symbols are intended to be used in "echo" commands
# System-dependent makefiles can override them
"='
|=|
-include config.mak

# The initial driver and application targets help text
DRVHELP = echo $"The following Crystal Space drivers can be built:$"
APPHELP = echo $"The following Crystal Space applications can be built:$"
LIBHELP = echo $"The following Crystal Space libraries can be built:$"

MAKESECTION=rootdefines
include mk/user.mak
include mk/common.mak
include mk/subs.mak
 
help:
	@echo $"+-----------------------------------------------------------------------------+$"
	@echo $"  Before compiling Crystal Space examine mk/user.mak and see if settings$"
	@echo $"  are suited for your system. Note that you need at least one renderer and$"
	@echo $"  at least one 2D driver in order to be able to run the engine.$"
	@echo $"+-----------------------------------------------------------------------------+$"
ifeq ($(PROC),invalid)
	@echo $"Before anything else, you should configure the makefile system as follows:$"
	@echo $"  make linux        Prepare for building under and for Linux$"
	@echo $"  make solaris      Prepare for building under and for Solaris$"
	@echo $"  make freebsd      Prepare for building under and for FreeBSD$"
	@echo $"  make beos         Prepare for building under and for BeOS$"
	@echo $"  make os2gcc       Prepare for building under and for OS/2 using GCC$"
	@echo $"  make os2wcc       Prepare for building under and for OS/2 using Watcom C$"
	@echo $"  make djgpp        Prepare for building under and for DOS using DJGPP$"
	@echo $"  make nextstep     Prepare for building under and for NextStep 3.3$"
	@echo $"  make openstep     Prepare for building under and for OpenStep 4.2$"
	@echo $"  make rhapsody     Prepare for building under and for Rhapsody (MacOS/X) DR2$"
	@echo $"  make amiga        Prepare for building under and for Amiga using GCC$"
	@echo $"  -*- Modifiers -*-$"
	@echo $"  USE_DLL=yes$|no    Build dynamic/static modules (drivers, plugins)$"
	@echo $"  MODE=optimize$|debug$|profile  Select how to compile everything.$"
	@echo $"+-----------------------------------------------------------------------------+$"
else
	@echo $"  Configured for $(DESCRIPTION.$(TARGET)) USE_DLL=$(USE_DLL) MODE=$(MODE) $(SYSMODIFIERS)$"
	@echo $"  Other platforms are: linux, solaris, freebsd, beos, os2gcc, os2wcc, djgpp,$"
	@echo $"  nextstep, openstep, rhapsody, or amiga with the following modifiers:$"
	@echo $"  USE_DLL=yes$|no    Build dynamic/static modules (drivers, plugins)$"
	@echo $"  MODE=optimize$|debug$|profile  Select how to compile everything.$"
ifdef SYSMODIFIERSHELP
	@$(SYSMODIFIERSHELP)
endif
	@echo $"  For example: make linux USE_DLL=yes MODE=debug$"
	@echo $"+-----------------------------------------------------------------------------+$"
	@$(DRVHELP)
	@echo $"+-----------------------------------------------------------------------------+$"
	@$(LIBHELP)
	@echo $"+-----------------------------------------------------------------------------+$"
	@$(APPHELP)
	@echo $"+-----------------------------------------------------------------------------+$"
	@echo $"  make libs         Make all static libraries$"
	@echo $"  make drivers      Make all dynamic libraries (COM drivers)$"
	@echo $"  make drivers2d    Make all supported 2D graphics drivers$"
	@echo $"  make drivers3d    Make all supported 3D graphics drivers (renderers)$"
	@echo $"  make snddrivers   Make all supported sound drivers$"
	@echo $"  make netdrivers   Make all supported sound drivers$"
	@echo $"  make depend       Make dependencies (recommended!)$"
	@echo $"  make doc          Make documentation using Doc++$"
	@echo $"  make api          Make API documentation using Doc++$"
	@echo $"  make clean        Clean everything$"
	@echo $"  make cleanlib     Clean all dynamic libraries$"
	@echo $"  make cleandep     Clean all dependency rule files$"
	@echo $"+-----------------------------------------------------------------------------+$"

depend:
	$(MAKE) --no-print-directory -f mk/cs.mak $@ DO_DEPEND=yes

doc api clean cleanlib cleandep:
	$(MAKE) --no-print-directory -f mk/cs.mak $@

endif

# Prepare for specific system
# WARNING: Try to avoid quotes in most important "echo" statements
# since several systems (OS/2, DOS and WIN32) have a "echo" that does
# literal output, i.e. they do not strip quotes from string.
linux solaris beos os2gcc os2wcc djgpp freebsd nextstep openstep rhapsody amiga unknown:
	@echo # Automatically generated file, do not edit!>config.mak
	@echo TARGET = $@>>config.mak
	@echo SYSMAKEFILE = mk/system/$@.mak>>config.mak
	@echo MODE = $(MODE)>>config.mak
	@echo USE_DLL = $(USE_DLL)>>config.mak
	@echo "+==================================================================="
	@echo "| System-dependent configuration pass."
	@echo "+==================================================================="
	@$(MAKE) --no-print-directory -f mk/system/$@.mak configure MAKESECTION=configure
	@echo "+==================================================================="
	@echo "| Makefiles are now configured for $(DESCRIPTION.$@)."
	@echo "+==================================================================="

MAKESECTION=roottargets
include mk/subs.mak
