#==============================================================================
#
#	Copyright (C)1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
#
# The contents of this file are copyrighted by Eric Sunshine.  This work is
# distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  You may distribute this file provided that this
# copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
#
#==============================================================================
#-------------------------------------------------------------------------------
# openstep.mak
#
#	Build-time configuration options for the OpenStep platform.
#
# *NOTE*
#	The OpenStep 4.2 Objective-C++ compiler has a register allocation bug
#	which causes it to corrupt the virtual-table pointer of classes whose
#	parent class (and sometimes parent's parent class) have very small
#	constructors or very small inline constructors.  This bug only
#	manifests itself when the constructors are not inlined and when no
#	optimization is in effect, such as when compiling for debug.  When the
#	constructors are inlined or when even minimal optimization is in
#	effect, they work correctly.  To work around this problem, we enable
#	minimal optimization (-O) and force use of inline functions
#	(-finline-functions) when compiling for debug.
#
#-------------------------------------------------------------------------------

NEXT.TARGET=openstep
NEXT.TARGET.DESCRIPTION=$(NEXT.TARGET)
NEXT.FLAVOR=OPENSTEP
NEXT.DESCRIPTION=OpenStep
NEXT.DESCRIPTION.OS=OpenStep
NEXT.ARCHS=i386 m68k sparc
NEXT.SOURCE_DIRS=openstep
NEXT.INCLUDE_DIRS=
NEXT.CFLAGS.GENERAL=-Wmost $(CFLAGS.D)_POSIX_SOURCE $(CFLAGS.D)__STRICT_ANSI__
NEXT.CFLAGS.OPTIMIZE=-O4 -finline-functions
NEXT.CFLAGS.DEBUG=-finline-functions -O
NEXT.CFLAGS.DLL=
NEXT.LIBS=
NEXT.LFLAGS.GENERAL=-framework AppKit -framework Foundation
NEXT.LFLAGS.EXE=
NEXT.LFLAGS.DLL=-bundle -undefined suppress

NEXT.FRIEND=yes
include libs/cssys/next/next.mak
NEXT.FRIEND=no
