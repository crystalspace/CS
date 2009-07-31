# compiler.m4                                                  -*- Autoconf -*-
#=============================================================================
# Copyright (C)2003-2009 by Eric Sunshine <sunshine@sunshineco.com>
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
#       desired.  This will also set the CMD.CC, COMPILER.CFLAGS,
#       COMPILER.C.TYPE, and (as an historic anomaly) COMPILER.TYPE variables
#       in Jamconfig.  The shell variable cs_compiler_name_c is also exported.
#-----------------------------------------------------------------------------
AC_DEFUN([_CS_PROG_CC_CFLAGS_FILTER],[
    CFLAGS="$CFLAGS" # Filter undesired flags
])
AC_DEFUN([CS_PROG_CC],[
    AC_REQUIRE([_CS_PROG_CC_CFLAGS_FILTER])
    AC_REQUIRE([AC_PROG_CC])
    AS_IF([test -n "$CC"],[
	CS_EMIT_BUILD_PROPERTY([CMD.CC], [$CC])
	CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS], [$CPPFLAGS $CFLAGS], [+])
	_CS_COMPILER_NAME([$CC], [C], [$ac_compiler_gnu])
	AC_MSG_CHECKING([for C compiler version])
	_CS_COMPILER_VERSION([$CC], [C])
	AS_IF([test -z "$_CS_COMPILER_VERSION_SH([C])"],
	    [AC_MSG_RESULT([unknown])],
	    [AC_MSG_RESULT([$_CS_COMPILER_VERSION_SH([C])])])
	
	# Check if compiler recognizes -pipe directive.
	CS_EMIT_BUILD_FLAGS([if $CC accepts -pipe], [cs_cv_prog_cc_pipe],
	    [CS_CREATE_TUPLE([-pipe])], [C], [COMPILER.CFLAGS], [append])

        # Check if compiler recognizes Sparc v9 CPU. Ugly to pollute
        # project-agnostic C compiler check, but it is needed by assembly code
        # implementing Sparc atomic threading operations, and it should not
        # hurt if the option is not recognized.
	CS_EMIT_BUILD_FLAGS([if $CC handles Sparc v9],
            [cs_cv_prog_cc_sparc_v9],
            [CS_CREATE_TUPLE([-mcpu=v9])], [C], [COMPILER.CFLAGS], [append])
    ])
])

#-----------------------------------------------------------------------------
# CS_PROG_CXX
#       Detects the C++ compiler.  Also takes care of the CXXFLAGS, CPPFLAGS
#       and CXX environment variables.  This will filter out all -g and -O from
#       the CXXFLAGS variable because Autoconf's -g and -O defaults are not
#       always desired.  This will also set the CMD.C++, COMPILER.C++FLAGS,
#       COMPILER.C++.TYPE, and (as an historic anomaly) COMPILER.TYPE variables
#       in Jamconfig. The shell variable cs_compiler_name_cxx is also exported.
#-----------------------------------------------------------------------------
AC_DEFUN([_CS_PROG_CXX_CFLAGS_FILTER],[
    CXXFLAGS="$CXXFLAGS" # Filter undesired flags
])
AC_DEFUN([CS_PROG_CXX],[
    AC_REQUIRE([_CS_PROG_CXX_CFLAGS_FILTER])
    AC_REQUIRE([AC_PROG_CXX])
    AS_IF([test -n "$CXX"],[
	CS_EMIT_BUILD_PROPERTY([CMD.C++], [$CXX])
	CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS], [$CPPFLAGS $CXXFLAGS], [+])
	_CS_COMPILER_NAME([$CXX], [C++], [$ac_compiler_gnu])
	AC_MSG_CHECKING([for C++ compiler version])
	_CS_COMPILER_VERSION([$CXX], [C++])
	AS_IF([test -z "$_CS_COMPILER_VERSION_SH([C++])"],
	    [AC_MSG_RESULT([unknown])],
	    [AC_MSG_RESULT([$_CS_COMPILER_VERSION_SH([C++])])])

        # Check if compiler can be instructed to produce position-independent-code
        # (PIC).  This feature is required by some platforms when building plugin
        # modules and shared libraries.
	CS_COMPILER_PIC([C++], [cs_cv_prog_cxx_pic],
	    [CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS.PIC],
		[$cs_cv_prog_cxx_pic])])
    ])
])

#-----------------------------------------------------------------------------
# _CS_COMPILER_NAME(COMPILER, LANGUAGE, [IS_GNU])
#	Attempt to glean COMPILER's name and export it as shell variable
#	cs_compiler_name_{language} and as build properties
#	COMPILER.LANGUAGE.TYPE and (as an historic anomaly) COMPILER.TYPE.
#	LANGUAGE is typically either C or C++ and specifies the compiler's
#	language.  If provided, IS_GNU should be 'yes' or 'no', and is employed
#	as a hint when determining the compiler's name. The output of this
#	macro is perhaps useful for display purposes, but should not generally
#	be used for decision making since name-gleaning is unreliable.  For
#	decision-making, it is better to perform actual compiler tests rather
#	than basing a decision upon the compiler's name.
#
# *IMPLEMENTATION NOTES*
#
#	Since there is no obvious generic way to determine a compiler's name,
#	this implementation emits "GCC" if IS_GNU is 'yes', otherwise it
#	attempts to fashion a name by invoking AS_TR_CPP(COMPILER), but this is
#	only a very rough and unreliable approximation of the compiler's name.
#	For instance, if a user configures the project with a non-GNU compiler
#	by setting the environment variable CXX="ccache c++comp -blazing-fast",
#	then the gleaned compiler name will be CCACHE_CPPCOMP__BLAZING_FAST
#	rather than "C++COMP" or "CXXCOMP" as one might expect.
#
#	When fashioning the shell variable name cs_compiler_name_{language},
#	"+" in LANGUAGE is replaced with "x", rather than with "p" as would be
#	the case normally when AS_TR_SH() is employed. The use of "x", however,
#	is more sensible in this context, since a language of "C++" translates
#	to "cxx", which is the expected variable name in the context of
#	compilers.
#-----------------------------------------------------------------------------
AC_DEFUN([_CS_COMPILER_NAME],
    [AS_IF([test "m4_default([$3],[no])" = yes],
        [_CS_COMPILER_NAME_SH([$2])=GCC],
        [_CS_COMPILER_NAME_SH([$2])=AS_TR_CPP([$1])])
    CS_EMIT_BUILD_PROPERTY([COMPILER.$2.TYPE], [$_CS_COMPILER_NAME_SH([$2])])
    CS_EMIT_BUILD_PROPERTY([COMPILER.TYPE], [$_CS_COMPILER_NAME_SH([$2])],
        [], [], [], [Y])])

AC_DEFUN([_CS_COMPILER_NAME_SH],
    [cs_compiler_name_[]AS_TR_SH(m4_translit([$1],[+A-Z],[xa-z]))])



#-----------------------------------------------------------------------------
# _CS_COMPILER_VERSION(COMPILER, LANGUAGE)
#-----------------------------------------------------------------------------
AC_DEFUN([_CS_COMPILER_VERSION],
    [case $_CS_COMPILER_NAME_SH([$2]) in
        GCC)
            _CS_COMPILER_VERSION_SH([$2])=`$1 -dumpversion`
	    CS_EMIT_BUILD_PROPERTY([COMPILER.$2.VERSION],
	        [$_CS_COMPILER_VERSION_SH([$2])])
	    CS_EMIT_BUILD_PROPERTY([COMPILER.VERSION],
	        [$_CS_COMPILER_VERSION_SH([$2])],
		[], [], [], [Y])
            _compiler_version_list=`echo $_CS_COMPILER_VERSION_SH([$2]) | sed -e 's/\./ /g'`
	    CS_EMIT_BUILD_PROPERTY([COMPILER.$2.VERSION_LIST],
	        [$_compiler_version_list])
	    CS_EMIT_BUILD_PROPERTY([COMPILER.VERSION_LIST],
	        [$_compiler_version_list],
		[], [], [], [Y])
            ;;
    esac])

AC_DEFUN([_CS_COMPILER_VERSION_SH],
    [cs_compiler_version_[]AS_TR_SH(m4_translit([$1],[+A-Z],[xa-z]))])



#-----------------------------------------------------------------------------
# CS_PROG_LINK
#	Tries to determine a linker.  This is done by checking if a C++ or
#       Objecctive-C++ compiler is available in which case it is used for
#       linking; otherwise the C or Objective-C compiler is used.  This also
#       sets the CMD.LINK and COMPILER.LFLAGS variables in Jamconfig and
#       respects the LDFLAGS environment variable.  Finally, checks if linker
#	recognizes -shared and sets PLUGIN.LFLAGS; and checks if linker
#	recognizes -soname and sets PLUGIN.LFLAGS.USE_SONAME to "yes".
#
#       Also, it is checked whether the linker supports the --as-needed
#       command line option, and if so, it is employed. As some libraries
#       reportedly don't support that feature, you can put 
#       $cs_cv_prog_link_no_as_needed and $cs_cv_prog_link_as_needed around
#       the linker flags to disable this feature for a particular library.
#-----------------------------------------------------------------------------
AC_DEFUN([CS_PROG_LINK],[
    AC_REQUIRE([CS_PROG_CXX])
    AS_IF([test -n "$CXX"],
	[CS_EMIT_BUILD_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.C++)])])],
	[CS_EMIT_BUILD_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.CC)])])])

    CS_CHECK_TOOLS([LD], [ld])
    CS_EMIT_BUILD_PROPERTY([CMD.LD], [$LD])
    
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
    rm -f conf$$.resp
    echo "" > conf$$.resp
    AC_CACHE_CHECK([if response files are accepted], [cs_cv_prog_link_respfile],
	[AC_LANG_PUSH([C++])
        cs_lflags_save="$LDFLAGS"
	LDFLAGS="-Wl,@conf$$.resp $cs_lflags_save"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([],[])],
	    [cs_cv_prog_link_respfile=yes],
	    [cs_cv_prog_link_respfile=no])
	LDFLAGS=$cs_lflags_save
    	AC_LANG_POP([C++])])
    AS_IF([test $cs_cv_prog_link_respfile = yes], 
    	[CS_EMIT_BUILD_PROPERTY([LINKER.RESPONSEFILES], [yes])])
    rm -f conf$$.resp
    
    # Check if linker supports --as-needed.
    AC_ARG_ENABLE([as-needed], 
	[AC_HELP_STRING([--enable-as-needed],
	    [Utilize --as-needed linker flag, if supported by linker and if
	    the used binutils version is recent enough to support it properly
	    (default YES)])])
    AS_IF([test -z "$enable_as_needed"], 
	[enable_as_needed=yes])
    AS_IF([test "$enable_as_needed" != "no"],
	[AC_REQUIRE([CS_CHECK_BINUTILS_2_17])
	AS_IF([test "$cs_cv_binutils_2_17" = "yes"],
	    [CS_EMIT_BUILD_FLAGS([if --as-needed is supported], 
		[cs_cv_prog_link_as_needed], [CS_CREATE_TUPLE([-Wl,--as-needed])], [C++],
		[CMD.LINK], [+])
	    CS_CHECK_BUILD_FLAGS([if --no-as-needed is supported], 
		[cs_cv_prog_link_no_as_needed], [CS_CREATE_TUPLE([-Wl,--no-as-needed])], 
		[C++])])])
    
    # Check if linker supports --gc-sections.
    AC_ARG_ENABLE([gc-sections], 
	[AC_HELP_STRING([--enable-gc-sections],
	    [Utilize --gc-sections linker flag for some targets (default YES)])])
    AS_IF([test -z "$enable_gc_sections"], 
	[enable_gc_sections=yes])
    AS_IF([test "$enable_gc_sections" != "no"],
	[CS_CHECK_BUILD_FLAGS([if --gc-sections is supported], 
	    [cs_cv_prog_link_gc_sections], 
	    [CS_CREATE_TUPLE([-Wl,--gc-sections])], 
	    [C++], 
	    [CS_EMIT_BUILD_PROPERTY([LINK.GC_SECTIONS], 
	        [$cs_cv_prog_link_gc_sections])])])
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
