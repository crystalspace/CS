################################################################################
#
#           This is the root makefile for the Crystal Space engine
#
################################################################################

.PHONY: help all doc api depend clean cleanlib cleandep distclean libs \
	drivers drivers2d drivers3d snddrivers netdrivers \
	linux solaris irix beos os2gcc os2wcc djgpp freebsd macosxs openstep \
	nextstep amiga win32vc

# The default values for configuration variables
TARGET=unknown
SYSMAKEFILE=mk/system/unknown.mak
USE_DLL=yes
MODE=optimize
USE_NASM=no

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
	@echo $"  make linux        Prepare for building under and for $(DESCRIPTION.linux)$"
	@echo $"  make solaris      Prepare for building under and for $(DESCRIPTION.solaris)$"
	@echo $"  make irix         Prepare for building under and for $(DESCRIPTION.irix)$"
	@echo $"  make freebsd      Prepare for building under and for $(DESCRIPTION.freebsd)$"
	@echo $"  make beos         Prepare for building under and for $(DESCRIPTION.beos)$"
	@echo $"  make os2gcc       Prepare for building under and for $(DESCRIPTION.os2gcc)$"
	@echo $"  make os2wcc       Prepare for building under and for $(DESCRIPTION.os2wcc)$"
	@echo $"  make djgpp        Prepare for building under and for $(DESCRIPTION.djgpp)$"
	@echo $"  make macosxs      Prepare for building under and for $(DESCRIPTION.macosxs)$"
	@echo $"  make openstep     Prepare for building under and for $(DESCRIPTION.openstep)$"
	@echo $"  make nextstep     Prepare for building under and for $(DESCRIPTION.nextstep)$"
	@echo $"  make amiga        Prepare for building under and for $(DESCRIPTION.amiga)$"
	@echo $"  make win32vc      Prepare for building under and for $(DESCRIPTION.win32vc)$"
	@echo $"  -*- Modifiers -*-$"
else
	@echo $"  Configured for $(DESCRIPTION.$(TARGET)) with the following modifiers:$"
	@echo $"  USE_DLL=$(USE_DLL) MODE=$(MODE) $(SYSMODIFIERS)$"
	@echo $"$"
	@echo $"  Other platforms are: linux, solaris, irix, freebsd, beos, os2gcc, os2wcc,$"
	@echo $"  macosxs, openstep, nextstep, amiga, djgpp or win32vc.$"
	@echo $"$"
	@echo $"  -*- Modifiers -*-$"
endif
	@echo $"  USE_DLL=yes$|no    Build dynamic/static modules (drivers, plugins)$"
	@echo $"  MODE=optimize$|debug$|profile  Select how to compile everything.$"
ifdef SYSMODIFIERSHELP
	@$(SYSMODIFIERSHELP)
endif
	@echo $"  Example: make linux USE_DLL=yes MODE=debug$"
	@echo $"+-----------------------------------------------------------------------------+$"
ifneq ($(PROC),invalid)
	@$(DRVHELP)
	@echo $"+-----------------------------------------------------------------------------+$"
	@$(LIBHELP)
	@echo $"+-----------------------------------------------------------------------------+$"
	@$(APPHELP)
	@echo $"+-----------------------------------------------------------------------------+$"
	@echo $"  make apps         Make all applications$"
	@echo $"  make libs         Make all static libraries$"
	@echo $"  make drivers      Make all drivers$"
	@echo $"  make drivers2d    Make all supported 2D graphics drivers$"
	@echo $"  make drivers3d    Make all supported 3D graphics drivers (renderers)$"
	@echo $"  make snddrivers   Make all supported sound drivers$"
	@echo $"  make netdrivers   Make all supported sound drivers$"
	@echo $"  make depend       Make dependencies (recommended!)$"
	@echo $"  make doc          Make documentation using Doc++$"
	@echo $"  make api          Make API documentation using Doc++$"
	@echo $"  make clean        Clean all files generated during build$"
	@echo $"  make cleanlib     Clean all dynamic libraries$"
	@echo $"  make cleandep     Clean all dependency rule files$"
	@echo $"  make distclean    Clean everything$"
	@echo $"+-----------------------------------------------------------------------------+$"

depend:
	@$(MAKE) --no-print-directory -f mk/cs.mak $@ DO_DEPEND=yes

doc api clean cleanlib cleandep distclean:
	@$(MAKE) --no-print-directory -f mk/cs.mak $@

# Create volatile.h before anything other if it does not exist
mk/user.mak: include/volatile.h
include/volatile.h: config.mak $(SYSMAKEFILE) $(LIBRARY_SUBMAKEFILES) \
	$(DRIVER_SUBMAKEFILES) $(APPLICATION_SUBMAKEFILES)
	@echo $",------=======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=======------$"
	@echo $"| Rebuilding $@$"
	@echo $"`----------==============xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx==============----------$"
	@echo $"/* This file is automatically generated, do not change manually! */$">$@
	@echo $"#ifndef __VOLATILE_H__$">>$@
	@echo $"#define __VOLATILE_H__$">>$@
	@$(MAKE_VOLATILE_H)
	@echo $"#endif // __VOLATILE_H__$">>$@

endif

# Prepare for specific system
# WARNING: Try to avoid quotes in most important "echo" statements
# since several systems (OS/2, DOS and WIN32) have a "echo" that does
# literal output, i.e. they do not strip quotes from string.
linux solaris irix beos os2gcc os2wcc djgpp freebsd macosxs openstep nextstep \
amiga win32vc unknown:
	@echo TARGET = $@>config.mak
	@echo SYSMAKEFILE = mk/system/$@.mak>>config.mak
	@echo MODE = $(MODE)>>config.mak
	@echo USE_DLL = $(USE_DLL)>>config.mak
	@echo "+---------------------------------------------------------------------------+"
	@echo "| System-dependent configuration pass."
	@echo "+---------------------------------------------------------------------------+"
	@$(MAKE) --no-print-directory -f mk/system/$@.mak MAKESECTION=configure
	@echo "+---------------------------------------------------------------------------+"
	@echo "| Makefiles are now configured for $(DESCRIPTION.$@)."
	@echo "+---------------------------------------------------------------------------+"

MAKESECTION=roottargets
include mk/subs.mak
