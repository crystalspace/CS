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
# macosxs.mak
#
#	Build-time configuration options for the MacOS/X Server platform.
#
# *NOTE*
#	The MacOS/X Server Objective-C++ compiler has a register allocation
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
NEXT.FLAVOR=MACOSXS
NEXT.DESCRIPTION=MacOS/X Server
NEXT.ARCHS=i386 ppc
NEXT.SOURCE_DIRS=macosxs openstep
NEXT.INCLUDE_DIRS=-FAppKit -FFoundation
NEXT.CFLAGS.GENERAL=-Wmost
NEXT.CFLAGS.DEBUG=-finline-functions
NEXT.LIBS=
NEXT.LFLAGS.GENERAL=-framework AppKit -framework Foundation

include mk/system/next.mak
