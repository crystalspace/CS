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
# macosxs.mak
#
#	Build-time configuration options for the MacOS/X Server 1.0
#	(Rhapsody) platform.
#
# BUG *1*
#	The MacOS/X Server Objective-C++ compiler has a register allocation
#	bug which causes it to corrupt the virtual-table pointer of classes
#	whose parent class (and sometimes parent's parent class) have very
#	small constructors or very small inline constructors.  This bug only
#	manifests itself when the constructors are not inlined and when no
#	optimization is in effect, such as when compiling for debug.  When the
#	constructors are inlined or when even minimal optimization is in
#	effect, they work correctly.  To work around this problem, we enable
#	minimal optimization (-O) and force use of inline functions
#	(-finline-functions) when compiling for debug.
#
# BUG *2*
#	The MacOS/X Server Objective-C++ PowerPC compiler has an instruction
#	scheduling optimization bug which causes it (the compiler) to hang
#	indefinitely when compiling libs/csgeom/polyclip.cpp.  Specifically,
#	the compiler chokes on the very first conditional statement which
#	appears within the master loop in edgeclip.inc; which is included by
#	boxclip.inc, which in turn is included by polyclip.cpp.  The actual
#	buggy optimization is 'schedule-insns2', thus we must disable it.
#	Since this optimization is typically enabled by '-O2', we only need to
#	disable it manually in NEXT.CFLAGS.OPTIMIZE; not in NEXT.CFLAGS.DEBUG.
#
# BUG *3*
#	The $(subst) calls in DO.MAKE.VOLATILE work around a bug in GNU 'make'
#	on MacOS/X Server.  Specifically, 'make' corrupts MAKE_VOLATILE_H (and
#	probably other variables) by truncating values which are appended to
#	it with +=.  In this case we see instances of volatile.t and
#	volatile.tm, which is clearly incorrect.  To work around the problem,
#	we manually translate .t and .tm back into .tmp.  Do not replace the
#	$(subst) calls with $(patsubst) since patsubst is incapable of
#	transforming the sort of strings which MAKE_VOLATILE_H contains.
#
#------------------------------------------------------------------------------

NEXT.TARGET=macosxs
NEXT.TARGET.DESCRIPTION=$(NEXT.TARGET)$(SPACE)
NEXT.FLAVOR=MACOSXS
NEXT.DESCRIPTION=MacOS/X Server
NEXT.DESCRIPTION.OS=MacOS/X
NEXT.ARCHS=i386 ppc
NEXT.SOURCE_DIRS=macosxs openstep
NEXT.INCLUDE_DIRS=-FAppKit -FFoundation
NEXT.CFLAGS.GENERAL=-Wmost
NEXT.CFLAGS.OPTIMIZE=-O3 -finline-functions -fno-schedule-insns2
NEXT.CFLAGS.DEBUG=-O -finline-functions
NEXT.CFLAGS.DLL=
NEXT.LIBS=
NEXT.LFLAGS.GENERAL=-framework AppKit -framework Foundation
NEXT.LFLAGS.EXE=
NEXT.LFLAGS.DLL=-bundle -undefined suppress

NEXT.FRIEND=yes
include libs/cssys/next/next.mak
NEXT.FRIEND=no

override DO.MAKE.VOLATILE=\
  $(subst volatile.t,volatile.tmp,\
  $(subst volatile.tm,volatile.t,\
  $(subst volatile.tmp,volatile.t,$(VOLATILE_H.ALL))))
