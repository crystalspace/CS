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
# openstep.mak
#
#	Build-time configuration options for the OpenStep platform.
#
# *NOTE*
#	The OpenStep 4.2 Objective-C++ compiler has a register allocation
#	bug which causes it to corrupt the virtual-table pointer of classes
#	whose parent class (and sometimes parent's parent class) have very
#	small inline constructors.  This bug only manifests itself when the
#	constructors are *not* inlined, such as when compiling for debug.
#	When inlined, they work correctly.  To work around this problem, we
#	must force use of inline functions even when compiling for debug.
#	Consequently, NEXT.CFLAGS.DEBUG includes the -finline-functions
#	directive.
#
#-------------------------------------------------------------------------------

NEXT.TARGET=openstep
NEXT.TARGET.DESCRIPTION=$(NEXT.TARGET)
NEXT.FLAVOR=OPENSTEP
NEXT.DESCRIPTION=OpenStep
NEXT.ARCHS=i386 m68k sparc
NEXT.SOURCE_DIRS=openstep
NEXT.INCLUDE_DIRS=-FAppKit -FFoundation
NEXT.CFLAGS.GENERAL=-Wmost -D_POSIX_SOURCE -D__STRICT_ANSI__
NEXT.CFLAGS.DEBUG=-finline-functions
NEXT.LIBS=
NEXT.LFLAGS.GENERAL=-framework AppKit -framework Foundation

NEXT.FRIEND=yes
include mk/system/next.mak
NEXT.FRIEND=no
