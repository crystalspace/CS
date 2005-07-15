# checkcswin32libs.m4                                         -*- Autoconf -*-
#==============================================================================
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

#------------------------------------------------------------------------------
# CS_CHECK_CSWIN32LIBS
# 	Check for the cs-win32libs package. It is a convenience archive made 
#	available to Crystal Space clients targetting Windows.  It contains 
#	common libraries, headers, and tools (such as zlib, libjpeg, cal3d, 
#	etc.) usable by MSVC, Mingw/MSYS, and Cygwin users as well as in cross-
#	compile environments.  It saves users the bother of having to install 
#	these packages manually one at a time.
#	The package provides a script 'cslibs-config' which can report the 
#	compiler and linker flags necessary to utilize the contained 
#	third-party libraries. If this script is found, the reported flags are
#	added to the global CFLAGS, CPPFLAGS and LDFLAGS variables.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_CSWIN32LIBS],
    [CS_CHECK_TOOLS([CSLIBS_CONFIG], [cslibs-config])
    AS_IF([test $ac_compiler_gnu = yes],
	[cs_cv_cslibs_compiler="--compiler gcc-`$CXX -dumpversion`"])
    AC_CACHE_CHECK([for cslibs package], [cs_cv_cslibs],
	[AS_IF([test -n "$CSLIBS_CONFIG"],
	    [cs_cv_cslibs=yes
	    cs_cv_cslibs_cflags=CS_RUN_PATH_NORMALIZE(
		[$CSLIBS_CONFIG --cflags $cs_cv_cslibs_compiler])
	    cs_cv_cslibs_lflags=CS_RUN_PATH_NORMALIZE(
		[$CSLIBS_CONFIG --lflags $cs_cv_cslibs_compiler])
	    cs_cv_cslibs_binpath=CS_RUN_PATH_NORMALIZE(
		[$CSLIBS_CONFIG --binpath $cs_cv_cslibs_compiler])],
	    [cs_cv_cslibs=no])])
    AS_IF([test $cs_cv_cslibs = yes],
	[CFLAGS="$CFLAGS $cs_cv_cslibs_cflags"
	CPPFLAGS="$CPPFLAGS $cs_cv_cslibs_cflags"
	LDFLAGS="$LDFLAGS $cs_cv_cslibs_lflags"
	PATH="$cs_cv_cslibs_binpath$PATH_SEPARATOR$PATH"])])
