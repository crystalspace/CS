#-----------------------------------------------------------------------------
# compiler.m4                                                  -*- Autoconf -*-
#=============================================================================
# Copyright (C)2003 by Matze Braun <matze@braunis.de>
#=============================================================================

#-----------------------------------------------------------------------------
# Detection of C and C++ compilers and setting flags
#
# CS_PROG_CC
#       Detects the C compiler. Also takes care of the CFLAGS, CPPFLAGS and CC
#       environment variables. This will filter out all -g and -O from the
#       CFLAGS variable because autoconf comes with some stupid defaults,
#       while nearly all projects set their own defaults here.
#	This will also set the COMPILER.CFLAGS variable in the Jamconfig
# CS_PROG_CXX
#       Detects the C++ compiler. Also takes care of the CXXFLAGS, CPPFLAGS
#	and CC environment variables. This will filter out all -g and -O from
#	the CXXFLAGS variable because autoconf comes with some stupid defaults,
#       while nearly all projects set their own defaults here.
#	This will also set the CMD.C++, COMPILER.C++FLAGS variable in the
#	Jamconfig
# CS_PROG_LINK
#	Tries to determine a linker. This is done by looking if a c++ compiler
#	is availbale in this case this app is used, otherwise the c compiler
#	is used.
#	This also sets the CMD.LINK, COMPILER.LFLAGS variables in the
#	Jamconfig and respects the LDFLAGS environment variable.
#-----------------------------------------------------------------------------
AC_DEFUN([CS_PROG_CC],[
    AC_PROG_CC
    AS_IF([test -n "$CC"],[
	CS_JAMCONFIG_PROPERTY([CMD.CC], [$CC])

	CFLAGS=`echo "$CFLAGS" | sed 's/\-O.//g;s/\-g.//g'`
	CS_JAMCONFIG_PROPERTY([COMPILER.CFLAGS], [$CPPFLAGS $CFLAGS])
    ])
])

AC_DEFUN([CS_PROG_CXX],[
    AC_PROG_CXX
    AS_IF([test -n "$CXX"],[
	CS_JAMCONFIG_PROPERTY([CMD.C++], [$CXX])
    
        CXXFLAGS=`echo "$CXXFLAGS" | sed 's/\-O.//g;s/-g.//g'`
	CS_JAMCONFIG_PROPERTY([COMPILER.C++FLAGS], [$CPPFLAGS $CXXFLAGS])
    ])
])

AC_DEFUN([CS_PROG_LINK],[
    AS_IF([test -n "$CXX"],
	  [CS_JAMCONFIG_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.C++)])])],
	  [CS_JAMCONFIG_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.CC)])])])

    CS_JAMCONFIG_PROPERTY([COMPILER.LFLAGS], [$LDFLAGS])
])

