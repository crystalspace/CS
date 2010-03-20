# checkcswin32libs.m4                                         -*- Autoconf -*-
#==============================================================================
# Copyright (C)2005,2006 by Eric Sunshine <sunshine@sunshineco.com>
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

#------------------------------------------------------------------------------
# CS_CHECK_CSWIN32LIBS
# 	Check for the cs-win32libs package. It is a convenience archive made 
#	available to Crystal Space clients targetting Windows.  It contains 
#	common libraries, headers, and tools (such as zlib, libjpeg, cal3d, 
#	etc.) usable by MSVC, Mingw/MSYS, and Cygwin users as well as in cross-
#	compile environments.  It saves users the bother of having to install 
#	these packages manually one at a time.
#
#	The package provides a script 'cslibs-config' which can report the
#	compiler and linker flags necessary to utilize the contained
#	third-party libraries. If this script is found, the reported flags and
#	paths are added to the CFLAGS, CPPFLAGS, LDFLAGS, PATH, and
#	PKG_CONFIG_PATH shell variables.
#
#       This macro exports the following shell variables containing the results
#       of the test:
#
#           cs_cv_cslibs ('yes' or 'no')
#           cs_cv_cslibs_cflags
#           cs_cv_cslibs_lflags
#           cs_cv_cslibs_binpath
#           cs_cv_cslibs_incpath (header search list)
#           cs_cv_cslibs_pcpath (pkgconfig search list)
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_CSWIN32LIBS],
    [AC_REQUIRE([AC_CANONICAL_BUILD])
    AC_REQUIRE([AC_CANONICAL_HOST])
    # ensure PKGCONFIG is set since we may meddle with it
    AC_REQUIRE([_CS_CHECK_PKG_CONFIG_PREPARE_PATH]) 
    AC_REQUIRE([CS_CHECK_MNO_CYGWIN])
    case $host_os in
    mingw*|cygwin*)
        # Always look for a cs-win32libs version that begins with the host
	# tuple, even if the default host is used
	AS_IF([test "$ac_cv_host" == "$ac_cv_build"],
	    [CS_CHECK_TOOLS([CSLIBS_CONFIG], [$ac_cv_host-cslibs-config])])
	AS_IF([test -z "$CSLIBS_CONFIG"],
            [CS_CHECK_TOOLS([CSLIBS_CONFIG], [cslibs-config])])
        AS_IF([test $ac_compiler_gnu = yes],
	    [cs_cv_cslibs_compiler="--compiler gcc-`$CXX -dumpversion | 
	      sed 's/\([[0-9]]\?\)\.\([[0-9]]\?\)\.[[0-9]]\?/\1.\2/'`"])
        AC_CACHE_CHECK([for cslibs package], [cs_cv_cslibs],
	    [AS_IF([test -n "$CSLIBS_CONFIG"],
	       [cs_cv_cslibs=yes
               AS_IF([test "x$cs_mno_cygwin" = "xyes"],
                    [AS_IF(
			[$CSLIBS_CONFIG -mno-cygwin --cflags >/dev/null 2>&1],
                        [cs_cv_cslibs_compiler="-mno-cygwin $cs_cv_cslibs_compiler"])])
	       cs_cv_cslibs_cflags=CS_RUN_PATH_NORMALIZE(
		    [$CSLIBS_CONFIG --cflags $cs_cv_cslibs_compiler])
	       cs_cv_cslibs_lflags=CS_RUN_PATH_NORMALIZE(
		    [$CSLIBS_CONFIG --lflags $cs_cv_cslibs_compiler])
	       cs_cv_cslibs_binpath=CS_RUN_PATH_NORMALIZE(
		    [$CSLIBS_CONFIG --binpath $cs_cv_cslibs_compiler])
               AS_IF([$CSLIBS_CONFIG --incpath >/dev/null 2>&1],
                    [cs_cv_cslibs_incpath=CS_RUN_PATH_NORMALIZE(
                        [$CSLIBS_CONFIG --incpath $cs_cv_cslibs_compiler])],
                    [cs_cv_cslibs_incpath=''])
               AS_IF([$CSLIBS_CONFIG --pcpath >/dev/null 2>&1],
                    [cs_cv_cslibs_pcpath=CS_RUN_PATH_NORMALIZE(
                        [$CSLIBS_CONFIG --pcpath $cs_cv_cslibs_compiler])],
                    [cs_cv_cslibs_pcpath=''])],
	       [cs_cv_cslibs=no])])
        AS_IF([test $cs_cv_cslibs = yes],
	    [CFLAGS="$CFLAGS $cs_cv_cslibs_cflags"
	    CPPFLAGS="$CPPFLAGS $cs_cv_cslibs_cflags"
	    LDFLAGS="$LDFLAGS $cs_cv_cslibs_lflags"
	    PATH="$cs_cv_cslibs_binpath$PATH_SEPARATOR$PATH"
	    AS_IF([test -n "$cs_cv_cslibs_pcpath"],
		[PKG_CONFIG_PATH="$cs_cv_cslibs_pcpath$PATH_SEPARATOR$PKG_CONFIG_PATH"
		export PKG_CONFIG_PATH])])
        ;;
    esac])
