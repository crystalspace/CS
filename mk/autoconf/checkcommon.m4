#==============================================================================
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
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_CHECK_COMMON_TOOLS_LINK([EMITTER])
#	Checks for common tools related to linking. Results of the checks are
#	recorded with CS_EMIT_BUILD_PROPERTY() via the optional EMITTER. If
#	EMITTER is omitted, then CS_EMIT_BUILD_PROPERTY()'s default emitter is
#	employed.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_TOOLS_LINK],
    [# The default RANLIB in Jambase is wrong on some platforms, and is also
    # unsuitable during cross-compilation, so value is set unconditionally.
    AC_PROG_RANLIB
    CS_EMIT_BUILD_PROPERTY([RANLIB], [$RANLIB], [unconditional], [], [$1])
    
    CS_CHECK_TOOLS([DLLTOOL], [dlltool])
    CS_EMIT_BUILD_PROPERTY([CMD.DLLTOOL], [$DLLTOOL], [], [], [$1])
    
    CS_CHECK_TOOLS([DLLWRAP], [dllwrap])
    AS_IF([test "$cs_mno_cygwin" = "yes"],
      [DLLWRAP="$DLLWRAP --target i386-mingw32"])
    CS_EMIT_BUILD_PROPERTY([CMD.DLLWRAP], [$DLLWRAP], [], [], [$1])
    
    CS_CHECK_TOOLS([WINDRES], [windres])
    CS_EMIT_BUILD_PROPERTY([CMD.WINDRES], [$WINDRES], [], [], [$1])
    
    CS_CHECK_TOOLS([STRINGS], [strings])
    CS_EMIT_BUILD_PROPERTY([CMD.STRINGS], [$STRINGS], [], [], [$1])

    CS_CHECK_TOOLS([OBJCOPY], [objcopy])
    CS_EMIT_BUILD_PROPERTY([CMD.OBJCOPY], [$OBJCOPY], [], [], [$1])
    
    CS_CHECK_LIBTOOL
    CS_EMIT_BUILD_PROPERTY([LIBTOOL], [$LIBTOOL], [], [], [$1])
    CS_EMIT_BUILD_PROPERTY([APPLE_LIBTOOL], [$APPLE_LIBTOOL], [], [], [$1])])


#------------------------------------------------------------------------------
# CS_CHECK_COMMON_TOOLS_RELAYTOOL([PATH], [EMITTER])
#	Checks for relaytool. Prepends PATH to the search path.
#	The result of the check is recorded with CS_EMIT_BUILD_PROPERTY() via 
#	the optional EMITTER. If EMITTER is omitted, then
#	CS_EMIT_BUILD_PROPERTY()'s default emitter is employed.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_TOOLS_RELAYTOOL],
    [AC_ARG_ENABLE([relaytool], [AC_HELP_STRING([--enable-relaytool],
	[enable use of relaytool for some libraries (default YES)])])
	AS_IF([test -z "$enable_relaytool"], 
	    [enable_relaytool=yes])
    AS_IF([test "$enable_relaytool" != "no"],
	[CS_PATH_TOOL([RELAYTOOL], [relaytool], [], [$1])
	CS_CHECK_PROGS([BASH], [bash])
	AS_IF([test -n "$BASH"],
	    [CS_EMIT_BUILD_PROPERTY([CMD.RELAYTOOL],
	        [$BASH $RELAYTOOL], [atomic], [], [$2])])])])


#------------------------------------------------------------------------------
# CS_CHECK_COMMON_TOOLS_BASIC([EMITTER])
#	Checks for basic tools for building things. Results of the checks are
#	recorded with CS_EMIT_BUILD_PROPERTY() via the optional EMITTER. If
#	EMITTER is omitted, then CS_EMIT_BUILD_PROPERTY()'s default emitter is
#	employed.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_TOOLS_BASIC],
    [CS_CHECK_MKDIR
    CS_EMIT_BUILD_PROPERTY([CMD.MKDIR], [$MKDIR], [], [], [$1])
    CS_EMIT_BUILD_PROPERTY([CMD.MKDIRS], [$MKDIRS], [], [], [$1])

    CS_CHECK_PROGS([INSTALL], [install])
    CS_EMIT_BUILD_PROPERTY([INSTALL], [$INSTALL], [], [], [$1])
    AC_PROG_LN_S
    CS_EMIT_BUILD_PROPERTY([LN_S], [$LN_S], [], [], [$1])])


#------------------------------------------------------------------------------
# CS_CHECK_COMMON_TOOLS_DOC_TEXINFO([EMITTER])
#	Checks for tools to generate documentation from texinfo files. Results
#	of the checks are recorded with CS_EMIT_BUILD_PROPERTY() via the
#	optional EMITTER. If EMITTER is omitted, then
#	CS_EMIT_BUILD_PROPERTY()'s default emitter is employed.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_TOOLS_DOC_TEXINFO],
    [CS_CHECK_PROGS([TEXI2DVI], [texi2dvi])
    CS_EMIT_BUILD_PROPERTY([CMD.TEXI2DVI], [$TEXI2DVI], [], [], [$1])

    CS_CHECK_PROGS([TEXI2PDF], [texi2pdf])
    CS_EMIT_BUILD_PROPERTY([CMD.TEXI2PDF], [$TEXI2PDF], [], [], [$1])

    CS_CHECK_PROGS([DVIPS], [dvips])
    CS_EMIT_BUILD_PROPERTY([CMD.DVIPS], [$DVIPS], [], [], [$1])

    CS_CHECK_PROGS([DVIPDF], [dvipdf])
    CS_EMIT_BUILD_PROPERTY([CMD.DVIPDF], [$DVIPDF], [], [], [$1])

    CS_CHECK_PROGS([MAKEINFO], [makeinfo])
    CS_EMIT_BUILD_PROPERTY([CMD.MAKEINFO], [$MAKEINFO], [], [], [$1])])


#------------------------------------------------------------------------------
# CS_CHECK_COMMON_TOOLS_DOC_DOXYGEN([EMITTER])
#	Checks for tools to generate source documentation via doxygen. Results
#	of the checks are recorded with CS_EMIT_BUILD_PROPERTY() via the
#	optional EMITTER. If EMITTER is omitted, then
#	CS_EMIT_BUILD_PROPERTY()'s default emitter is employed.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_TOOLS_DOC_DOXYGEN],
    [CS_CHECK_PROGS([DOXYGEN], [doxygen])
    CS_EMIT_BUILD_PROPERTY([CMD.DOXYGEN], [$DOXYGEN], [], [], [$1])

    CS_CHECK_TOOLS([DOT], [dot])
    CS_EMIT_BUILD_PROPERTY([CMD.DOT], [$DOT], [], [], [$1])])


#------------------------------------------------------------------------------
# CS_CHECK_COMMON_TOOLS_ICONS([EMITTER])
#	Checks for tools required by the icon generation rules from
#	icons.jam. Results of the checks are recorded with
#	CS_EMIT_BUILD_PROPERTY() via the optional EMITTER. If EMITTER is
#	omitted, then CS_EMIT_BUILD_PROPERTY()'s default emitter is employed.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_TOOLS_ICONS],
    [# rsvg: for svg to png conversion
    CS_CHECK_PROGS([RSVG], [rsvg])
    CS_EMIT_BUILD_PROPERTY([CMD.RSVG], [$RSVG], [], [], [$1])

    # icotool: for creating Win32 ICO files
    CS_CHECK_PROGS([ICOTOOL], [icotool])
    CS_EMIT_BUILD_PROPERTY([CMD.ICOTOOL], [$ICOTOOL], [], [], [$1])

    # convert: for various image manipulations from both the svg conversion and
    #  ICO creation.
    CS_CHECK_PROGS([CONVERT], [convert])
    CS_EMIT_BUILD_PROPERTY([CMD.CONVERT], [$CONVERT], [], [], [$1])])


#------------------------------------------------------------------------------
# CS_CHECK_COMMON_LIBS([EMITTER])
#       Check for typical required libraries (libc, libm, libmx, libdl,
#	libnsl). Results of the checks are recorded under build key
#	"COMPILER.LFLAGS" via CS_EMIT_BUILD_PROPERTY() using the optional
#	EMITTER. If EMITTER is omitted, then CS_EMIT_BUILD_PROPERTY()'s default
#	emitter is employed.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_LIBS],
    [AC_LANG_PUSH([C])

    AC_CHECK_LIB([c], [fopen])
    AS_IF([test $ac_cv_lib_c_fopen = yes],
	[CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lc], [append],[],[$1])])

    AC_CHECK_LIB([m], [pow], [cs_cv_libm_libs=-lm], [cs_cv_libm_libs=])
    AC_CHECK_LIB([m], [cosf], [cs_cv_libm_libs=-lm])
    AS_IF([test $ac_cv_lib_m_pow = yes || test $ac_cv_lib_m_cosf = yes],
	[CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lm], [append],[],[$1])])

    AC_CHECK_LIB([mx], [cosf])
    AS_IF([test $ac_cv_lib_mx_cosf = yes],
	[CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lmx], [append],[],[$1])])

    AC_CHECK_LIB([dl], [dlopen], [cs_cv_libdl_libs=-ldl], [cs_cv_libdl_libs=])
    AS_IF([test $ac_cv_lib_dl_dlopen = yes],
	[CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-ldl], [append],[],[$1])])

    AC_CHECK_LIB([nsl], [gethostbyname])
    AS_IF([test $ac_cv_lib_nsl_gethostbyname = yes],
	[CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [-lnsl], [append],[],[$1])])

    AC_LANG_POP([C])
])
