# emit.m4                                                      -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003,2004 by Eric Sunshine <sunshine@sunshineco.com>
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
# CS_EMIT_BUILD_PROPERTY(KEY, VALUE, [APPEND], [EMPTY-OKAY], [EMITTER])
#	A utility function which invokes CS_JAMCONFIG_PROPERTY() if VALUE is
#	not the empty string (after leading and trailing whitespace is
#	stripped). If EMPTY-OKAY is not an empty string, then the property is
#	emitted even if VALUE is empty; that is, it is emitted unconditionally.
#	If APPEND is the empty string, then the value is set via "?=";
#	otherwise it is appended to the existing value of the Jam variable via
#	"+=". EMITTER is a macro name, such as CS_JAMCONFIG_PROPERTY or
#	CS_MAKEFILE_PROPERTY, which performs the actual task of emitting the
#	KEY/VALUE tuple; it should also accept APPEND as an optional third
#	argument. If EMITTER is omitted, CS_JAMCONFIG_PROPERTY is used.
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_BUILD_PROPERTY],
    [cs_build_prop_val="$2"
    cs_build_prop_val=CS_TRIM([$cs_build_prop_val])
    m4_ifval([$4],
	[CS_JAMCONFIG_PROPERTY([$1], [$cs_build_prop_val], [$3])],
	AS_IF([test -n "$cs_build_prop_val"],
	    [m4_default([$5],[CS_JAMCONFIG_PROPERTY])(
		[$1], [$cs_build_prop_val], [$3])]))])



#------------------------------------------------------------------------------
# CS_EMIT_BUILD_RESULT(CACHE-VAR, PREFIX, [EMITTER])
#	Record the results of CS_CHECK_BUILD() or CS_CHECK_LIB_WITH() via Jam
#	variables in the Jam text cache.  If CACHE-VAR indicates that the build
#	succeeded, then the following properties are emitted:
#
#	PREFIX.AVAILABLE = yes
#	PREFIX.CFLAGS = $CACHE-VAR_cflags
#	PREFIX.LFLAGS = $CACHE-VAR_lflags $CACHE-VAR_libs
#
#	EMITTER is a macro name, such as CS_JAMCONFIG_PROPERTY or
#	CS_MAKEFILE_PROPERTY, which performs the actual task of emitting the
#	KEY/VALUE tuple. If EMITTER is omitted, CS_JAMCONFIG_PROPERTY is used.
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_BUILD_RESULT],
    [AS_IF([test "$$1" = yes],
	[CS_EMIT_BUILD_PROPERTY([$2.AVAILABLE], [yes], [], [], [$3])
	CS_EMIT_BUILD_PROPERTY([$2.CFLAGS], [$$1_cflags], [], [], [$3])
	CS_EMIT_BUILD_PROPERTY([$2.LFLAGS], [$$1_lflags $$1_libs],
	    [], [], [$3])])])



#------------------------------------------------------------------------------
# CS_EMIT_BUILD_FLAGS(MESSAGE, CACHE-VAR, FLAGS, [LANGUAGE], JAM-VARIABLE,
#                     [APPEND], [ACTION-IF-RECOGNIZED],
#                     [ACTION-IF-NOT-RECOGNIZED], [EMITTER])
#	A convenience wrapper for CS_CHECK_BUILD_FLAGS() which also records the
#	results via CS_EMIT_BUILD_PROPERTY().  Checks if the compiler or linker
#	recognizes a command-line option.  MESSAGE is the "checking" message.
#	CACHE-VAR is the shell cache variable which receives the flag
#	recognized by the compiler or linker, or "no" if the flag was not
#	recognized.  FLAGS is a whitespace- delimited list of build tuples
#	created with CS_CREATE_TUPLE().  Each tuple from FLAGS is attempted in
#	order until one is found which is recognized by the compiler.  After
#	that, no further flags are checked.  LANGUAGE is typically either C or
#	C++ and specifies which compiler to use for the test.  If LANGUAGE is
#	omitted, C is used.  JAM-VARIABLE is the name of the variable to insert
#	into the Jam text cache if a usable flag is encountered.  If APPEND is
#	not the empty string, then the flag is appended to the existing value
#	of the Jam variable.  If the command-line option was recognized, then
#	ACTION-IF-RECOGNIZED is invoked, otherwise ACTION-IF-NOT-RECOGNIZED is
#	invoked.  EMITTER is a macro name, such as CS_JAMCONFIG_PROPERTY or
#	CS_MAKEFILE_PROPERTY, which performs the actual task of emitting the
#	KEY/VALUE tuple; it should also accept APPEND as an optional third
#	argument. If EMITTER is omitted, CS_JAMCONFIG_PROPERTY is used.
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_BUILD_FLAGS],
    [CS_CHECK_BUILD_FLAGS([$1], [$2], [$3], [$4],
	[CS_EMIT_BUILD_PROPERTY([$5], [$$2], [$6], [], [$9])
	    $7],
	[$8])])
