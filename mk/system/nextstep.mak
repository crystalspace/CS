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
NEXT.FLAVOR=NEXTSTEP
NEXT.DESCRIPTION=NextStep
NEXT.ARCHS=m68k i386 sparc hppa
NEXT.SOURCE_DIRS=nextstep
NEXT.INCLUDE_DIRS=
NEXT.CFLAGS=-Wall -DNO_BOOL_TYPE -D_POSIX_SOURCE -D__STRICT_ANSI__
NEXT.LIBS=-lNeXT_s -lsys_s

include mk/system/next.mak
