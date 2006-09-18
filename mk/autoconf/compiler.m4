# compiler.m4                                                  -*- Autoconf -*-
#=============================================================================
# Copyright (C)2003 by Matze Braun <matze@braunis.de>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
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
#       Also, it is checked whether the linker supports the --as-needed
#       command line option, and if so, it is employed. As some libraries
#       reportedly don't support that feature, you can put 
#       $cs_cv_prog_link_no_as_needed and $cs_cv_prog_link_as_needed around
#       the linker flags to disable this feature for a particular library.
#-----------------------------------------------------------------------------
AC_DEFUN([CS_PROG_CC],[
    CFLAGS="$CFLAGS" # Filter undesired flags
    AC_PROG_CC
    AS_IF([test -n "$CC"],[
	CS_EMIT_BUILD_PROPERTY([CMD.CC], [$CC])
	CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS], [$CPPFLAGS $CFLAGS], [+])
	
	# Check if compiler recognizes -pipe directive.
	CS_EMIT_BUILD_FLAGS([if $CC accepts -pipe], [cs_cv_prog_cc_pipe],
	  [CS_CREATE_TUPLE([-pipe])], [C], [COMPILER.CFLAGS], [+])
    ])
])

#-----------------------------------------------------------------------------
# CS_PROG_CXX
#       Detects the C++ compiler.  Also takes care of the CXXFLAGS, CPPFLAGS
#       and CXX environment variables.  This will filter out all -g and -O from
#       the CXXFLAGS variable because Autoconf's -g and -O defaults are not
#       always desired.  This will also set the CMD.C++ and COMPILER.C++FLAGS
#       variables in Jamconfig
#-----------------------------------------------------------------------------
AC_DEFUN([CS_PROG_CXX],[
    CXXFLAGS="$CXXFLAGS" # Filter undesired flags
    AC_PROG_CXX
    AS_IF([test -n "$CXX"],[
	CS_EMIT_BUILD_PROPERTY([CMD.C++], [$CXX])

	CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS], [$CPPFLAGS $CXXFLAGS], [+])

        # Check if compiler can be instructed to produce position-independent-code
        # (PIC).  This feature is required by some platforms when building plugin
        # modules and shared libraries.
	CS_COMPILER_PIC([C++], [cs_cv_prog_cxx_pic],
	    [CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS.PIC],
		[$cs_cv_prog_cxx_pic])])
    ])
])

#-----------------------------------------------------------------------------
# CS_PROG_LINK
#	Tries to determine a linker.  This is done by checking if a C++ or
#       Objecctive-C++ compiler is available in which case it is used for
#       linking; otherwise the C or Objective-C compiler is used.  This also
#       sets the CMD.LINK and COMPILER.LFLAGS variables in Jamconfig and
#       respects the LDFLAGS environment variable.  Finally, checks if linker
#	recognizes -shared and sets PLUGIN.LFLAGS; and checks if linker
#	recognizes -soname and sets PLUGIN.LFLAGS.USE_SONAME to "yes".
#-----------------------------------------------------------------------------
AC_DEFUN([CS_PROG_LINK],[
    AC_REQUIRE([CS_PROG_CXX])
    AS_IF([test -n "$CXX"],
	[CS_EMIT_BUILD_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.C++)])])],
	[CS_EMIT_BUILD_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.CC)])])])

    CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [$LDFLAGS], [+])

    # Check if compiler/linker recognizes -shared directive which is needed for
    # linking plugin modules.  Unfortunately, the Apple compiler (and possibly
    # others) requires extra effort.  Even though the compiler does not recognize
    # the -shared option, it nevertheless returns a "success" result after emitting
    # the warning "unrecognized option `-shared'".  Worse, even -Werror fails to
    # promote the warning to an error, so we must instead scan the compiler's
    # output for an appropriate diagnostic.
    CS_CHECK_BUILD_FLAGS([if -shared is accepted], [cs_cv_prog_link_shared],
	[CS_CREATE_TUPLE([-shared $cs_cv_prog_cxx_pic])], [C++],
	[CS_EMIT_BUILD_PROPERTY([PLUGIN.LFLAGS], [-shared], [+])], [],
	[], [], [], [shared])

    # Check if linker recognizes -soname which is used to assign a name internally
    # to plugin modules.
    CS_CHECK_BUILD([if -soname is accepted], [cs_cv_prog_link_soname], [],
	[CS_CREATE_TUPLE([-Wl,-soname,foobar])], [C++],
	[CS_EMIT_BUILD_PROPERTY([PLUGIN.LFLAGS.USE_SONAME], [yes])])
	
    # Check if binutils support response files
    rm -f conftest.resp
    echo "" > conftest.resp
    CS_CHECK_BUILD([if response files are accepted], [cs_cv_prog_link_respfile], [],
	[-Wl,@conftest.resp], [C++],
	[CS_EMIT_BUILD_PROPERTY([LINKER.RESPONSEFILES], [yes])])
    rm -f conftest.resp
    
    # Check if linker supports --as-needed.
    CS_EMIT_BUILD_FLAGS([if --as-needed is supported], 
        [cs_cv_prog_link_as_needed], [CS_CREATE_TUPLE([-Wl,--as-needed])], [C++],
	[CMD.LINK], [+])
    CS_CHECK_BUILD_FLAGS([if --no-as-needed is supported], 
        [cs_cv_prog_link_no_as_needed], [CS_CREATE_TUPLE([-Wl,--no-as-needed])], 
        [C++])])
])

#-----------------------------------------------------------------------------
# CS_CHECK_MNO_CYGWIN
#	Check whether the -mno-cygwin flag should be passed to the compiler.
#	If so, adjust the compiler env vars accordingly.
#
#	Peter Amstutz explains: "Because "gcc -mno-cygwin" is quite literally 
#	another compiler (it actually invokes /usr/lib/gcc/mingw-3.4.4/cc1.exe 
#	instead of /usr/lib/cygwin-3.4.4/cc1.exe) things like the paths to 
#	standard libraries change (out with cygwin headers, in with windows 
#	headers) and generally the surface behavior of the compiler changes 
#	radically.  As a result, simply augmenting $CXXFLAGS was problematic 
#	(autoconf tests that didn't have $CXXFLAGS would yield incorrect 
#	results) and it made more sense to have autoconf treat it as if it 
#	were a different compiler entirely."
#-----------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_MNO_CYGWIN],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    AC_MSG_CHECKING([whether to enable -mno-cygwin])
    AC_ARG_ENABLE([mno-cygwin],
        [AC_HELP_STRING([--enable-mno-cygwin],
            [Compile with the -mno-cygwin option. (default YES)])],
	[enable_m_no_cygwin=$enableval], [enable_m_no_cygwin=yes])
    AS_IF([test "$enable_m_no_cygwin" != "no"],
	[case $host_os in
	    cygwin*)
		AS_IF([test -n "$CC"], 
		    [CC="$CC -mno-cygwin"], 
		    [CC="gcc -mno-cygwin"])
		AS_IF([test -n "$CXX"], 
		    [CXX="$CXX -mno-cygwin"], 
		    [CXX="g++ -mno-cygwin"])
		CPP="$CC -E"
		CXXCPP="$CXX -E"
		cs_mno_cygwin=yes
		;;
	    *)
		cs_mno_cygwin=no
		;;
	esac
	], [cs_mno_cygwin=no])
    AC_MSG_RESULT([$cs_mno_cygwin])
    ])
