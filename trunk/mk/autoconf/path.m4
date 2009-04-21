# path.m4                                                      -*- Autoconf -*-
#==============================================================================
# Copyright (C)2004,2008 by Eric Sunshine <sunshine@sunshineco.com>
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
# CS_PATH_NORMALIZE_EMBEDDED(STRING)
#	Normalize all paths embedded in STRING at run-time by transliterating
#	Windows/DOS backslashes to forward slashes.  Also collapses whitespace.
#	This is useful when applied to command output which may include
#	embedded Windows-style paths since the backslashes in those paths could
#	be incorrectly interpreted in the context of Makefiles or other
#	development utilities. For instance, a string such as "-Ic:\foo\inc
#	-Ic:\foo\local\inc" is normalized to "-Ic:/foo/inc -Ic:/foo/local/inc".
#
#	Usage: opts=CS_PATH_NORMALIZE_EMBEDDED([$opts])
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_NORMALIZE_EMBEDDED],
[`echo "x$1" | tr '\\\\' '/' | sed 's/^x//;s/   */ /g;s/^ //;s/ $//'`])



#------------------------------------------------------------------------------
# CS_PATH_NORMALIZE_OUTPUT(COMMAND)
#	Normalize all paths emitted by COMMAND by transliterating Windows/DOS
#	backslashes to forward slashes.  Also collapses whitespace.  This is
#	useful when the output of COMMAND may include embedded Windows-style
#	paths since the backslashes in those paths could be incorrectly
#	interpreted in the context of Makefiles or other development
#	utilities. For instance, if COMMAND "pkg-config --cflags foo" emits
#	"-Ic:\foo\inc -Ic:\foo\local\inc", the output is normalized to
#	"-Ic:/foo/inc -Ic:/foo/local/inc".
#
#	Usage: opts=CS_PATH_NORMALIZE_OUTPUT([$cmd])
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_NORMALIZE_OUTPUT],
[`AC_RUN_LOG([$1]) | tr '\\\\' '/' | sed 's/   */ /g;s/^ //;s/ $//'`])



#------------------------------------------------------------------------------
# CS_PATH_NORMALIZE(STRING)
#	DEPRECATED: Use CS_PATH_NORMALIZE_EMBEDDED() instead.
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_NORMALIZE], [CS_PATH_NORMALIZE_EMBEDDED([$1])])



#------------------------------------------------------------------------------
# CS_RUN_PATH_NORMALIZE(COMMAND)
#	DEPRECATED: Use CS_PATH_NORMALIZE_OUTPUT() instead.
#------------------------------------------------------------------------------
AC_DEFUN([CS_RUN_PATH_NORMALIZE], [CS_PATH_NORMALIZE_OUTPUT([$1])])



#------------------------------------------------------------------------------
# CS_PATH_CANONICALIZE(PATH)
#	Canonicalize PATH by transliterating Windows/DOS backslashes to forward
#	slashes. On MSYS/MinGW, also converts MSYS-style paths
#	(ex. /home/foo/bar/, /c/foo/bar) to proper Windows-style paths
#	(ex. c:/msys/home/foo/bar, c:/foo/bar). This is important because an
#	MSYS-style path is meaningful only to MSYS, but not to utilities which
#	might be invoked by a Makefile or other build tool.  Such utilities
#	invariably expect Windows-style paths.
#
#	Usage: path=CS_PATH_CANONICALIZE([$path])
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CANONICALIZE],
    [$(
    cs_indir=CS_PATH_NORMALIZE_EMBEDDED([$1])
    MSYS_AC_CANONICAL_PATH([cs_outdir], [$cs_indir])
    echo $cs_outdir
    )])



#------------------------------------------------------------------------------
# MSYS_AC_CANONICAL_PATH(VAR, PATHNAME)
#	Set VAR to the canonically resolved absolute equivalent of PATHNAME,
#	(which may be a relative path, and need not refer to any existing
#	entity).
#
#	On Win32-MSYS build hosts, the returned path is resolved to its true
#	native Win32 path name (but with slashes, not backslashes).
#
#	On any other system, it is simply the result which would be obtained if
#	PATHNAME represented an existing directory, and the pwd command was
#	executed in that directory.
#
# Author: Keith Marshall <keith.marshall@total.com>
# Source: http://article.gmane.org/gmane.comp.gnu.mingw.msys/2785
#------------------------------------------------------------------------------
AC_DEFUN([MSYS_AC_CANONICAL_PATH],
    [ac_dir="$2"
    pwd -W >/dev/null 2>&1 && ac_pwd_w="pwd -W" || ac_pwd_w=pwd
    until ac_val=`exec 2>/dev/null; cd "$ac_dir" && $ac_pwd_w`
    do
       ac_dir=`AS_DIRNAME(["$ac_dir"])`
    done
    ac_dir=`echo "$ac_dir" | sed 's?^[[./]]*??'`
    ac_val=`echo "$ac_val" | sed 's?/*$[]??'`
    $1=`echo "$2" | sed "s?^[[./]]*$ac_dir/*?$ac_val/?"';s?/*$[]??'`])



#------------------------------------------------------------------------------
# CS_PATH_INIT
#	Initialize commonly-used variable substitutions via AC_SUBST().
#	Present implementation publishes CS_TOP_SRCDIR and CS_TOP_BUILDDIR,
#	which are canonicalized counterparts (via CS_PATH_CANONICALIZE()) of
#	Autoconf's top_srcdir and top_builddir substitutions (ex: in source
#	file, use @CS_TOP_SRCDIR@ rather than @top_srcdir@).
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_INIT],
    [AC_SUBST([CS_TOP_SRCDIR], [CS_PATH_CANONICALIZE([$srcdir])])
    AC_SUBST([CS_TOP_BUILDDIR], [.])
    ])
