#------------------------------------------------------------------------------
# Determine host platform.  Recognized families: Unix, Windows, Mac OS X.
# Copyright (C)2003-2009 by Eric Sunshine <sunshine@sunshineco.com>
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
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# CS_CHECK_HOST_CPU([EMITTER])
#       Set the shell variable cs_host_cpu to a normalized form of the CPU name
#       returned by config.guess/config.sub.  Typically, Crystal Space's
#       conception of CPU name is the same as that returned by
#       config.guess/config.sub, but there may be exceptions, such as
#       normalizing all Intel x86 CPU names to the canonical "x86".  Also takes
#       the normalized name, uppercases it to form a name suitable for the C
#       preprocessor and publishes it as shell variable cs_host_cpu_normalized.
#       Additionally, sets the TARGET.PROCESSOR build property to the value of
#       cs_host_cpu_normalized via the supplied EMITTER. If EMITTER is omitted,
#       then CS_EMIT_BUILD_PROPERTY()'s default emitter is employed.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_HOST_CPU],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_cpu in
        [[Ii][3-9]86*|[Xx]86*]) cs_host_cpu=x86 ;;
        powerpc*) cs_host_cpu=powerpc ;;
        sparc*) cs_host_cpu=sparc ;;
        mips*) cs_host_cpu=mips ;;
        alpha*) cs_host_cpu=alpha ;;
        *) cs_host_cpu=$host_cpu ;;
    esac
    cs_host_cpu_normalized="AS_TR_CPP([$cs_host_cpu])"
    CS_EMIT_BUILD_PROPERTY([TARGET.PROCESSOR], [$cs_host_cpu_normalized],
        [], [], [$1])
    ])


#------------------------------------------------------------------------------
# CS_CHECK_HOST([EMITTER])
#       Sets the shell variables cs_host_target, cs_host_family,
#       cs_host_os_normalized, and cs_host_os_normalized_uc (normalized and
#       uppercase).  Emits appropriate CS_PLATFORM_UNIX, CS_PLATFORM_WIN32,
#       CS_PLATFORM_MACOSX via AC_DEFINE(). Note that these are not necessarily
#       mutually exclusive. For instance, on Mac OS X, both CS_PLATFORM_MACOSX
#       and CS_PLATFORM_UNIX are defined.  Furthermore, emits build variables
#       TARGET.OS and TARGET.OS.NORMALIZED via CS_EMIT_BUILD_PROPERTY() using
#       the provided EMITTER or CS_EMIT_BUILD_PROPERTY()'s default emitter if
#       EMITTER is omitted.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_HOST],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    CS_CHECK_HOST_CPU
    cs_host_os_normalized=''
    case $host_os in
        mingw*|cygwin*)
            cs_host_target=win32gcc
            cs_host_family=windows
            ;;
        darwin*)
            _CS_CHECK_HOST_DARWIN
            _CS_CHECK_UNIVERSAL_BINARY
            ;;
        *)
            # Everything else is assumed to be Unix or Unix-like.
            cs_host_target=unix
            cs_host_family=unix
	    ;;
    esac

    case $cs_host_family in
	windows)
            AC_DEFINE([CS_PLATFORM_WIN32], [],
		[Define when compiling for Win32])
	    AS_IF([test -z "$cs_host_os_normalized"],
		[cs_host_os_normalized='Win32'])
	    ;;
	unix)
            AC_DEFINE([CS_PLATFORM_UNIX], [],
		[Define when compiling for Unix and Unix-like (i.e. Mac OS X)])
	    AS_IF([test -z "$cs_host_os_normalized"],
		[cs_host_os_normalized='Unix'])
	    ;;
    esac

    cs_host_os_normalized_uc="AS_TR_CPP([$cs_host_os_normalized])"
    CS_EMIT_BUILD_PROPERTY([TARGET.OS], [$cs_host_os_normalized_uc],
        [], [], [$1])
    CS_EMIT_BUILD_PROPERTY([TARGET.OS.NORMALIZED], [$cs_host_os_normalized],
        [], [], [$1])
])

#------------------------------------------------------------------------------
# _CS_CHECK_HOST_DARWIN([EMITTER])
#       Both Mac OS X and Darwin are identified via $host_os as "darwin".  We
#       need a way to distinguish between the two.  If Carbon.h is present,
#       then assume Mac OX S; if not, assume Darwin.  If --with-x=yes was
#       invoked, and Carbon.h is present, then assume that user wants to
#       cross-build for Darwin even though build host is Mac OS X.
#
# IMPLEMENTATION NOTES
#
#       1. The QuickTime 7.0 installer removes <CarbonSound/CarbonSound.h>,
#       which causes #include <Carbon/Carbon.h> to fail
#       unconditionally. Re-installing the QuickTime SDK should restore the
#       header, however not all developers know to do this, so we work around
#       the problem of the missing CarbonSound.h by #defining __CARBONSOUND__
#       in the test in order to prevent Carbon.h from attempting to #include
#       the missing header.
#
#       2. At least one Mac OS X user switches between gcc 2.95 and gcc 3.3
#       with a script which toggles the values of CC, CXX, and CPP.
#       Unfortunately, CPP was being set to run the preprocessor directly
#       ("cpp", for instance) rather than running it via the compiler ("gcc
#       -E", for instance).  The problem with running the preprocessor directly
#       is that __APPLE__ and __GNUC__ are not defined, which causes the
#       Carbon.h check to fail.  We avoid this problem by supplying a non-empty
#       fourth argument to AC_CHECK_HEADER(), which causes it to test compile
#       the header only (which is a more robust test), rather than also testing
#       it via the preprocessor.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_CHECK_HOST_DARWIN],
    [AC_REQUIRE([CS_PROG_CC])
    AC_REQUIRE([CS_PROG_CXX])

    AC_DEFINE([__CARBONSOUND__], [],
	[Avoid problem caused by missing <Carbon/CarbonSound.h>])
    AC_CHECK_HEADER([Carbon/Carbon.h],
	[cs_host_macosx=yes], [cs_host_macosx=no], [/* force compile */])

    AS_IF([test $cs_host_macosx = yes],
	[AC_MSG_CHECKING([for --with-x])
	AS_IF([test "${with_x+set}" = set && test "$with_x" = "yes"],
	    [AC_MSG_RESULT([yes (assume Darwin)])
	    cs_host_macosx=no],
	    [AC_MSG_RESULT([no])])])

    AS_IF([test $cs_host_macosx = yes],
	[cs_host_target=macosx
	cs_host_family=unix
	cs_host_os_normalized='MacOS/X'
        AC_DEFINE([CS_PLATFORM_MACOSX], [],
	    [Define when compiling for MacOS/X])

	AC_CACHE_CHECK([for Objective-C compiler], [cs_cv_prog_objc],
	    [cs_cv_prog_objc="$CC"])
	CS_EMIT_BUILD_PROPERTY([CMD.OBJC], [$cs_cv_prog_objc], [], [], [$1])
	AC_CACHE_CHECK([for Objective-C++ compiler], [cs_cv_prog_objcxx],
	    [cs_cv_prog_objcxx="$CXX"])
	CS_EMIT_BUILD_PROPERTY([CMD.OBJC++], [$cs_cv_prog_objcxx],
            [], [], [$1])],

	[cs_host_target=unix
	cs_host_family=unix])])


#------------------------------------------------------------------------------
# _CS_CHECK_UNIVERSAL_BINARY([EMITTER])
#       Check if Mac OS X universal binaries should be built. This option is
#       enabled by default on Mac OS X only if the MacOSX10.4u universal SDK is
#       installed and if not cross-compiling. Otherwise, it is disabled.  Emits
#       CS_UNIVERSAL_BINARY via AC_DEFINE() and CS_HEADER_PROPERTY(), and
#       augments CFLAGS, CXXFLAGS, and LDFLAGS appropriately when universal
#       binaries are enabled. Also emits augmented build properties
#       COMPILER.CFLAGS, COMPILER.C++FLAGS, and COMPILER.LFLAGS via
#       CS_EMIT_BUILD_PROPERTY() using the optional EMITTER or
#       CS_EMIT_BUILD_PROPERTY()'s default emitter if EMITTER is omitted.
#------------------------------------------------------------------------------
m4_define([CS_UNIVERSAL_ARCHS], [i386, ppc])
m4_define([CS_UNIVERSAL_SDK], [/Developer/SDKs/MacOSX10.4u.sdk])
m4_define([CS_UNIVERSAL_ARCHS_LIST],
    m4_foreach([arch], [CS_UNIVERSAL_ARCHS], [[-arch] arch ]))
m4_define([CS_UNIVERSAL_CFLAGS],
    [-isysroot CS_UNIVERSAL_SDK -mmacosx-version-min=10.4])
m4_define([CS_UNIVERSAL_CXXFLAGS], [CS_UNIVERSAL_CFLAGS])
m4_define([CS_UNIVERSAL_LDFLAGS], [-Wl,-syslibroot,CS_UNIVERSAL_SDK])

AC_DEFUN([_CS_CHECK_UNIVERSAL_BINARY],
    [AS_IF([test "$cs_host_target" = macosx &&
	   test "$cross_compiling" != yes &&
	   test -d "CS_UNIVERSAL_SDK"],
	[cs_universal_binary_default=yes], [cs_universal_binary_default=no])
    AC_MSG_CHECKING([whether to build universal binaries])
    AC_ARG_ENABLE([universal-binary],
	[AC_HELP_STRING([--enable-universal-binary],
	    [build Mac OS X universal binaries (default YES on Mac OS X with
            universal SDK installed, else NO)])],
	[cs_universal_binary=$enableval],
	[cs_universal_binary=$cs_universal_binary_default])
    AC_MSG_RESULT([$cs_universal_binary])
    AS_IF([test "$cs_universal_binary" = yes],
	[AC_DEFINE([CS_UNIVERSAL_BINARY], [1],
	    [Define if building universal binaries on MacOSX.])
	CFLAGS="$CFLAGS CS_UNIVERSAL_CFLAGS CS_UNIVERSAL_ARCHS_LIST"
	CXXFLAGS="$CXXFLAGS CS_UNIVERSAL_CXXFLAGS CS_UNIVERSAL_ARCHS_LIST"
	LDFLAGS="$LDFLAGS CS_UNIVERSAL_LDFLAGS CS_UNIVERSAL_ARCHS_LIST"
	CS_HEADER_PROPERTY([CS_UNIVERSAL_BINARY])
	CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS],
            [CS_UNIVERSAL_CFLAGS CS_UNIVERSAL_ARCHS_LIST], [append])
	CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS],
            [CS_UNIVERSAL_CXXFLAGS CS_UNIVERSAL_ARCHS_LIST], [append])
	CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS],
            [CS_UNIVERSAL_LDFLAGS CS_UNIVERSAL_ARCHS_LIST], [append])])])
