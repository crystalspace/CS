#==============================================================================
#
#	Copyright (C)1999-2001 by Eric Sunshine <sunshine@sunshineco.com>
#
# The contents of this file are copyrighted by Eric Sunshine.  This work is
# distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  You may distribute this file provided that this
# copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
#
#==============================================================================
#------------------------------------------------------------------------------
# nextstep.mak
#
#	Build-time configuration options for the NextStep platform.
#
# *NOTE*
#	The NextStep 3.3 Objective-C++ compiler has a code generation bug
#	which is triggered by CS/libs/csengine/covtree.cpp and which results
#	in invalid computation of the coverage tree.  The problem occurs when
#	the compiler applies strength-reduction optimization, thus we work
#	around the problem by specifying -fno-strength-reduce.
#
# *NOTE*
#	On non-NeXT platforms, symbols within dynamically loaded modules are
#	considered private unless explicitly exported.  On NextStep, however,
#	all symbols within a plug-in module are public.  Since many Crystal
#	Space plug-in modules link with various Crystal Space static libraries
#	name collisions will surely occur.  To work around this problem, we
#	make an addition to DO.SHARED.PLUGIN.POSTAMBLE so that it strips all
#	unnecessary public symbols from the plug-in module.  This is fairly
#	decent insurance against symbolic collisions between modules even when
#	they link against the same libraries.  The only serious drawback to
#	this approach is that it makes symbolic debugging of plug-in modules
#	impossible.  This approach is different than the one used by the
#	MacOS/X Server and OpenStep ports of Crystal Space.  For those ports,
#	it is possible to instruct the dynamic linker (DYLD) to ignore
#	duplicate symbols at load time, thus there is no need to strip symbols
#	from the plug-in modules.  Unfortunately, however, the NextStep RLD
#	facility does not provide a mechanism for ignoring duplicate symbols at
#	load time.  It is possible, though, to attempt loading a module and
#	then parse the error stream if the load fails.  For each duplicate
#	symbol mentioned in the error stream, call rld_forget_symbol() then
#	attempt to reload the module.  Unfortunately, there are two problems
#	with this approach which make it unusable.  First, RLD is sufficiently
#	buggy that it actually crashes the program if rld_forget_symbol() is
#	called after an unsuccessful load.  Second, it is apparently impossible
#	to "forget" symbols which have been linked into the application itself,
#	thus there is no way to resolve a collision between a symbol in the
#	application and one in a plug-in module.  Thus, we must resort to the
#	DO.SHARED.PLUGIN.POSTAMBLE technique of stripping the plug-in of all of
#	its public symbols before ever attempting to load it in the first
#	place.
#
#------------------------------------------------------------------------------

NEXT.TARGET=nextstep
NEXT.TARGET.DESCRIPTION=$(NEXT.TARGET)
NEXT.FLAVOR=NEXTSTEP
NEXT.DESCRIPTION=NextStep
NEXT.DESCRIPTION.OS=NextStep
NEXT.ARCHS=i386 m68k sparc hppa
NEXT.SOURCE_DIRS=nextstep
NEXT.INCLUDE_DIRS=
NEXT.CFLAGS.GENERAL=-Wall $(CFLAGS.D)_POSIX_SOURCE $(CFLAGS.D)__STRICT_ANSI__
NEXT.CFLAGS.OPTIMIZE=-O4 -finline-functions -fno-strength-reduce
NEXT.CFLAGS.DEBUG=
NEXT.CFLAGS.DLL=
NEXT.LIBS=$(LFLAGS.l)NeXT_s $(LFLAGS.l)sys_s
NEXT.LFLAGS.GENERAL=
NEXT.LFLAGS.EXE=-u libNeXT_s -u libsys_s
NEXT.LFLAGS.DLL=-nostdlib -r

NEXT.FRIEND=yes
include libs/cssys/next/next.mak
NEXT.FRIEND=no

ifeq ($(MAKESECTION),postdefines)
PUBDLLSYM = $(OUTOS)$(basename $(notdir $@)).sym
DO.SHARED.PLUGIN.POSTAMBLE += \
  ; echo "_$(basename $(notdir $@))_GetClassTable" > $(PUBDLLSYM) \
  ; $(STRIP) -s $(PUBDLLSYM) -u $(NEXT.STRIP_FLAGS) $@
endif
