# compiler.m4                                                  -*- Autoconf -*-
#=============================================================================
# Copyright (C)2003 by Matze Braun <matze@braunis.de>
#=============================================================================

#-----------------------------------------------------------------------------
# Detection of C and C++ compilers and setting flags
#
# CS_PROG_CC
#       Detects the C compiler.  Also takes care of the CFLAGS, CPPFLAGS and CC
#       environment variables.  This will filter out all -g and -O from the
#       CFLAGS variable because Autoconf's -g and -O defaults are not always
#       desired.  This will also set the CMD.CC and COMPILER.CFLAGS variables
#       in Jamconfig
# CS_PROG_CXX
#       Detects the C++ compiler.  Also takes care of the CXXFLAGS, CPPFLAGS
#       and CXX environment variables.  This will filter out all -g and -O from
#       the CXXFLAGS variable because Autoconf's -g and -O defaults are not
#       always desired.  This will also set the CMD.C++ and COMPILER.C++FLAGS
#       variables in Jamconfig
# CS_PROG_LINK
#	Tries to determine a linker.  This is done by checking if a C++ or
#       Objecctive-C++ compiler is availbale in which case it is used for
#       linking; otherwise the C or Objective-C compiler is used.  This also
#       sets the CMD.LINK and COMPILER.LFLAGS variables in Jamconfig and
#       respects the LDFLAGS environment variable.  Finally, checks if linker
#	recognizes -shared and sets PLUGIN.LFLAGS; and checks if linker
#	recognizes -soname and sets PLUGIN.LFLAGS.USE_SONAME to "yes".
#-----------------------------------------------------------------------------
AC_DEFUN([CS_PROG_CC],[
    AC_PROG_CC
    AS_IF([test -n "$CC"],[
	CS_JAMCONFIG_PROPERTY([CMD.CC], [$CC])

	CFLAGS=`echo "$CFLAGS" | sed 's/\-O.//g;s/\-g.//g'`
	CS_JAMCONFIG_PROPERTY([COMPILER.CFLAGS], [$CPPFLAGS $CFLAGS], [+])
    ])
])

AC_DEFUN([CS_PROG_CXX],[
    AC_PROG_CXX
    AS_IF([test -n "$CXX"],[
	CS_JAMCONFIG_PROPERTY([CMD.C++], [$CXX])

        CXXFLAGS=`echo "$CXXFLAGS" | sed 's/\-O.//g;s/-g.//g'`
	CS_JAMCONFIG_PROPERTY([COMPILER.C++FLAGS], [$CPPFLAGS $CXXFLAGS], [+])
    ])
])

AC_DEFUN([CS_PROG_LINK],[
    AS_IF([test -n "$CXX"],
	[CS_JAMCONFIG_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.C++)])])],
	[CS_JAMCONFIG_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.CC)])])])

    CS_JAMCONFIG_PROPERTY([COMPILER.LFLAGS], [$LDFLAGS], [+])

    AC_CACHE_CHECK([if -shared is accepted], [cs_cv_prog_link_shared],
	[dnl Cheat: Make AC_LINK_IFELSE send errors to conftest.err
	m4_pushdef([AC_TRY_EVAL], [_AC_EVAL_STDERR([$$1])])
	CS_BUILD_IFELSE([], [CS_CREATE_TUPLE([-shared])], [C++],
	    [AS_IF([grep nrecognize conftest.err >/dev/null 2>&1],
		[cs_cv_prog_link_shared=no], [cs_cv_prog_link_shared=yes])],
	    [cs_cv_prog_link_shared=no])
	rm -f conftest.err
	m4_popdef([AC_TRY_EVAL])])
    AS_IF([test $cs_cv_prog_link_shared = yes],
	[CS_JAMCONFIG_PROPERTY([PLUGIN.LFLAGS], [-shared], [+])])

    CS_CHECK_BUILD_FLAGS([is -soname is recgonized], [cs_cv_prog_link_soname],
	[CS_CREATE_TUPLE([-Wl,-soname,foobar])], [],
	[CS_JAMCONFIG_PROPERTY([PLUGIN.LFLAGS.USE_SONAME], [yes])])
])
