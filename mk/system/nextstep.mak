#==============================================================================
#
#	Copyright (C)1999 by Eric Sunshine <sunshine@sunshineco.com>
#
# The contents of this file are copyrighted by Eric Sunshine.  This work is
# distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  You may distribute this file provided that this
# copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
#
#==============================================================================
#-------------------------------------------------------------------------------
# nextstep.mak
#
#	Build-time configuration options for the NextStep platform.
#
#-------------------------------------------------------------------------------

NEXT.TARGET=nextstep
NEXT.TARGET.DESCRIPTION=$(NEXT.TARGET)
NEXT.FLAVOR=NEXTSTEP
NEXT.DESCRIPTION=NextStep
NEXT.ARCHS=i386 m68k sparc hppa
NEXT.SOURCE_DIRS=nextstep
NEXT.INCLUDE_DIRS=
NEXT.CFLAGS.GENERAL=-Wall $(CFLAGS.D)_POSIX_SOURCE $(CFLAGS.D)__STRICT_ANSI__
NEXT.CFLAGS.DEBUG=
NEXT.CFLAGS.DLL=
NEXT.LIBS=$(LFLAGS.l)NeXT_s $(LFLAGS.l)sys_s
NEXT.LFLAGS.GENERAL=
NEXT.LFLAGS.EXE=-u libNeXT_s -u libsys_s
NEXT.LFLAGS.DLL=-nostdlib -r

NEXT.FRIEND=yes
include mk/system/next.mak
NEXT.FRIEND=no
