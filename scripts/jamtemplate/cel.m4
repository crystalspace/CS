# cel.m4                                                       -*- Autoconf -*-
#==============================================================================
# CEL detection macros
# Copyright (C)2005 by Eric Sunshine <sunshine@sunshineco.com>
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
#==============================================================================
AC_PREREQ([2.56])

m4_define([cel_min_version_default], [0.99])

#------------------------------------------------------------------------------
# CS_PATH_CEL_CHECK([MINIMUM-VERSION], [ACTION-IF-FOUND],
#                   [ACTION-IF-NOT-FOUND], [REQUIRED-LIBS], [OPTIONAL-LIBS])
#	Checks for Crystal Entity Layer (CEL) paths and libraries by consulting
#	cel-config. It first looks for cel-config in the paths mentioned by
#	$CEL, then in the paths mentioned by $PATH, and then in
#	/usr/local/cel/bin.  Emits an error if it can not locate cel-config, if
#	the CEL test program fails, or if the available version number is
#	unsuitable.  Exports the variables CEL_CONFIG_TOOL, CEL_AVAILABLE,
#	CEL_VERSION, CEL_CFLAGS, CEL_LIBS, CEL_INCLUDE_DIR, and
#	CEL_AVAILABLE_LIBS.  If the check succeeds, then CEL_AVAILABLE will be
#	'yes', and the other variables set to appropriate values. If it fails,
#	then CEL_AVAILABLE will be 'no', and the other variables empty.  If
#	REQUIRED-LIBS is specified, then it is a list of CEL libraries which
#	must be present, and for which appropriate compiler and linker flags
#	will be reflected in CEL_CFLAGS and CEL_LFLAGS. If OPTIONAL-LIBS is
#	specified, then it is a list of CEL libraries for which appropriate
#	compiler and linker flags should be returned if the libraries are
#	available.  It is not an error for an optional library to be
#	absent. The client can check CEL_AVAILABLE_LIBS for a list of all
#	libraries available for this particular installation of CEL.  The
#	returned list is independent of REQUIRED-LIBS and OPTIONAL-LIBS.  Use
#	the results of the check like this: CFLAGS="$CFLAGS $CEL_CFLAGS" and
#	LDFLAGS="$LDFLAGS $CEL_LIBS"
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CEL_CHECK],
[AC_REQUIRE([CS_PATH_CRYSTAL_CHECK])
AC_ARG_WITH([cel-prefix], 
    [AC_HELP_STRING([--with-cel-prefix=CEL_PREFIX],
	[specify location of CEL installation; this is the \$prefix value used
        when installing the SDK])],
    [CEL="$withval"
    export CEL])
AC_ARG_VAR([CEL], [Prefix where CEL is installed])
AC_ARG_ENABLE([celtest], 
    [AC_HELP_STRING([--enable-celtest], 
	[verify that the CEL SDK is actually usable (default YES)])], [],
	[enable_celtest=yes])

# Try to find an installed cel-config.
cel_path=''
AS_IF([test -n "$CEL"],
    [my_IFS=$IFS; IFS=$PATH_SEPARATOR
    for cel_dir in $CEL; do
	AS_IF([test -n "$cel_path"], [cel_path="$cel_path$PATH_SEPARATOR"])
	cel_path="$cel_path$cel_dir$PATH_SEPARATOR$cel_dir/bin"
    done
    IFS=$my_IFS])

AS_IF([test -n "$cel_path"], [cel_path="$cel_path$PATH_SEPARATOR"])
cel_path="$cel_path$PATH$PATH_SEPARATOR/usr/local/cel/bin"

AC_PATH_TOOL([CEL_CONFIG_TOOL], [cel-config], [], [$cel_path])

AS_IF([test -n "$CEL_CONFIG_TOOL"],
    [cfg="$CEL_CONFIG_TOOL"

    CS_CHECK_PROG_VERSION([CEL], [$cfg --version],
	[m4_default([$1],[cel_min_version_default])], [9.9|.9],
	[cel_sdk=yes], [cel_sdk=no])

    AS_IF([test $cel_sdk = yes],
	[cel_liblist="$4"
	cel_optlibs=CS_TRIM([$5])
	AS_IF([test -n "$cel_optlibs"],
	    [cel_optlibs=`$cfg --available-libs $cel_optlibs`
	    cel_liblist="$cel_liblist $cel_optlibs"])
	CEL_VERSION=`$cfg --version $cel_liblist`
	CEL_CFLAGS=CS_RUN_PATH_NORMALIZE([$cfg --cflags $cel_liblist])
	CEL_LIBS=CS_RUN_PATH_NORMALIZE([$cfg --lflags $cel_liblist])
	CEL_INCLUDE_DIR=CS_RUN_PATH_NORMALIZE(
	    [$cfg --includedir $cel_liblist])
	CEL_AVAILABLE_LIBS=`$cfg --available-libs`
	CEL_STATICDEPS=`$cfg --static-deps`
	AS_IF([test -z "$CEL_LIBS"], [cel_sdk=no])])],
    [cel_sdk=no])

AS_IF([test "$cel_sdk" = yes && test "$enable_celtest" = yes],
    [CS_CHECK_BUILD([if CEL SDK is usable], [cel_cv_cel_sdk],
	[AC_LANG_PROGRAM(
	    [#include <cssysdef.h>
	    #include <physicallayer/entity.h>],
	    [/* TODO write a suitable test */])],
	[CS_CREATE_TUPLE([$CEL_CFLAGS],[],[$CEL_LIBS])], [C++],
	[], [cel_sdk=no], [],
	[$CRYSTAL_CFLAGS], [], [$CRYSTAL_LIBS])])

AS_IF([test "$cel_sdk" = yes],
   [CEL_AVAILABLE=yes
   $2],
   [CEL_AVAILABLE=no
   CEL_CFLAGS=''
   CEL_VERSION=''
   CEL_LIBS=''
   CEL_INCLUDE_DIR=''
   $3])
])


#------------------------------------------------------------------------------
# CS_PATH_CEL_HELPER([MINIMUM-VERSION], [ACTION-IF-FOUND],
#                        [ACTION-IF-NOT-FOUND], [REQUIRED-LIBS],
#                        [OPTIONAL-LIBS])
#	Deprecated: Backward compatibility wrapper for CS_PATH_CEL_CHECK().
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CEL_HELPER],
[CS_PATH_CEL_CHECK([$1],[$2],[$3],[$4],[$5])])


#------------------------------------------------------------------------------
# CS_PATH_CEL([MINIMUM-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                 [REQUIRED-LIBS], [OPTIONAL-LIBS])
#	Convenience wrapper for CS_PATH_CEL_CHECK() which also invokes
#	AC_SUBST() for CEL_AVAILABLE, CEL_VERSION, CEL_CFLAGS,
#	CEL_LIBS, CEL_INCLUDE_DIR, and CEL_AVAILABLE_LIBS.
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CEL],
[CS_PATH_CEL_CHECK([$1],[$2],[$3],[$4],[$5])
AC_SUBST([CEL_AVAILABLE])
AC_SUBST([CEL_VERSION])
AC_SUBST([CEL_CFLAGS])
AC_SUBST([CEL_LIBS])
AC_SUBST([CEL_INCLUDE_DIR])
AC_SUBST([CEL_AVAILABLE_LIBS])
AC_SUBST([CEL_STATICDEPS])])


#------------------------------------------------------------------------------
# CS_PATH_CEL_EMIT([MINIMUM-VERSION], [ACTION-IF-FOUND],
#                      [ACTION-IF-NOT-FOUND], [REQUIRED-LIBS], [OPTIONAL-LIBS],
#                      [EMITTER])
#	Convenience wrapper for CS_PATH_CEL_CHECK() which also emits
#	CEL_AVAILABLE, CEL_VERSION, CEL_CFLAGS, CEL_LIBS,
#	CEL_INCLUDE_DIR, and CEL_AVAILABLE_LIBS as the build properties
#	CEL.AVAILABLE, CEL.VERSION, CEL.CFLAGS, CEL.LIBS,
#	CEL.INCLUDE_DIR, and CEL.AVAILABLE_LIBS, respectively, using
#	EMITTER.  EMITTER is a macro name, such as CS_JAMCONFIG_PROPERTY or
#	CS_MAKEFILE_PROPERTY, which performs the actual task of emitting the
#	property and value. If EMITTER is omitted, then
#	CS_EMIT_BUILD_PROPERTY()'s default emitter is used.
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CEL_EMIT],
[CS_PATH_CEL_CHECK([$1],[$2],[$3],[$4],[$5])
_CS_PATH_CEL_EMIT([CEL.AVAILABLE],[$CEL_AVAILABLE],[$6])
_CS_PATH_CEL_EMIT([CEL.VERSION],[$CEL_VERSION],[$6])
_CS_PATH_CEL_EMIT([CEL.CFLAGS],[$CEL_CFLAGS],[$6])
_CS_PATH_CEL_EMIT([CEL.LFLAGS],[$CEL_LIBS],[$6])
_CS_PATH_CEL_EMIT([CEL.INCLUDE_DIR],[$CEL_INCLUDE_DIR],[$6])
_CS_PATH_CEL_EMIT([CEL.AVAILABLE_LIBS],[$CEL_AVAILABLE_LIBS],[$6])
_CS_PATH_CEL_EMIT([CEL.STATICDEPS],[$CEL_STATICDEPS],[$6])
])

AC_DEFUN([_CS_PATH_CEL_EMIT],
[CS_EMIT_BUILD_PROPERTY([$1],[$2],[],[],[$3])])


#------------------------------------------------------------------------------
# CS_PATH_CEL_JAM([MINIMUM-VERSION], [ACTION-IF-FOUND],
#                     [ACTION-IF-NOT-FOUND], [REQUIRED-LIBS], [OPTIONAL-LIBS])
#	Deprecated: Jam-specific backward compatibility wrapper for
#	CS_PATH_CEL_EMIT().
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CEL_JAM],
[CS_PATH_CEL_EMIT([$1],[$2],[$3],[$4],[$5],[CS_JAMCONFIG_PROPERTY])])
