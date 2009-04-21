# visibility.m4                                                -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003-2008 by Eric Sunshine <sunshine@sunshineco.com>
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
# CS_VISIBILITY_FLAG_INLINES_HIDDEN([ACTION-IF-SUPPORTED],
#                                   [ACTION-IF-NOT-SUPPORTED])
#
# Check if "hidden" visibilty for C++ inline functions is supported. If it is
# supported, then the appropriate compiler switch is assigned to the shell
# variable cs_prog_cxx_visibility_inlines_hidden and ACTION-IF-SUPPORTED is
# invoked, otherwise the shell variable is cleared and ACTION-IF-NOT-SUPPORTED
# is invoked.
#
# IMPLEMENTATION NOTES
#
# There is a bug in gcc 3.4.x and at least up to 4.1.x where
# -fvisibility-inlines-hidden on several architectures in combination with
# -fPIC and -shared flags causes the linker to fail with a bogus errors. One
# manifestation of the error states that the target library needs to be built
# with -fPIC even when it has been built using that option.  In other cases,
# the linker complains about undefined references to various inline methods in
# external libraries.  Furthermore, on such installations, if Crystal Space is
# built with -fvisibility-inlines-hidden, then clients linking against Crystal
# Space also experience one of these bogus errors.  Thus far, the most reliable
# way to detect this gcc bug is to test for the anomaly by manipulating a
# std::basic_string<> in a test program linked with -fPIC and -shared, and then
# avoid -fvisibility-inlines-hidden if the test program fails to link.
# Practical experience has shown that this test is not 100% reliable for all
# cases, but it does seem to catch most, and does a better job than hard-coded
# platform and compiler version checks.  References:
#
#     http://dev.gentoo.org/~kugelfang/pappy-visibility.txt
#     http://www.gnu.org/software/gcc/gcc-4.0/changes.html#visibility
#     http://www.nedprod.com/programs/gccvisibility.html
#     http://bugs.gentoo.org/show_bug.cgi?id=78720
#     http://trac.crystalspace3d.org/trac/CS/ticket/333
#
# Furthermore, there seems to be a bug on MacOS/X with gcc 4.0.x where use of
# -fvisibility-inlines-hidden causes link problems with external clients which
# do not specify this flag.  For this reason, we also disable this flag on
# MacOS/X.  In particular, clients receive errors of this sort:
#
#    /usr/bin/ld: libcrystalspace.a(scf.o) malformed object, illegal
#        reference for -dynamic code (reference to a coalesced section
#        (__TEXT,__textcoal_nt) from section (__TEXT,__text) relocation
#        entry (1))
#
# An additional issue arises when cross-compiling from Linux to Windows using
# the i586-mingw32msvc-g++ compiler. In this case, during actual compilation of
# certain templates, the compiler annoyingly complains that the "visibility
# attribute is not supported in this configuration", despite the fact that the
# -fvisibility-inlines-hidden option is otherwise silently accepted by the
# compiler.  A simple test case involving a templated inline method is employed
# to detect this anomaly.
#------------------------------------------------------------------------------
AC_DEFUN([CS_VISIBILITY_FLAG_INLINES_HIDDEN],
[AC_REQUIRE([_CS_VISIBILITY_PREPARE])
AC_REQUIRE([CS_CHECK_HOST])
AC_REQUIRE([CS_CHECK_STL])
CS_COMPILER_PIC([C++], [cs_cv_prog_cxx_pic])

# Check if compiler recognizes an option to set inline visibility to hidden.
CS_CHECK_BUILD_FLAGS([for inline visibility flag],
    [cs_cv_prog_cxx_visibility_inlines_hidden],
    [CS_CREATE_TUPLE([-fvisibility-inlines-hidden])], [C++], [], [],
    [$cs_cv_prog_cxx_enable_errors])
cs_prog_cxx_visibility_inlines_hidden=$cs_cv_prog_cxx_visibility_inlines_hidden

# Check if compiler silently accepts option but later complains that
# "visibility attribute is not supported in this configuration".
AS_IF([test -n "$cs_prog_cxx_visibility_inlines_hidden"],
    [CS_CHECK_BUILD(
        [if configuration supports $cs_prog_cxx_visibility_inlines_hidden],
        [cs_cv_prog_cxx_visibility_inlines_hidden_supported],
        [AC_LANG_PROGRAM(
            [template <typename T> struct X { inline void f() {} };],
            [X<int>().f();])],
        [], [C++],
        [cs_cv_prog_cxx_visibility_inlines_hidden_supported=yes],
        [cs_cv_prog_cxx_visibility_inlines_hidden_supported=no],
        [],
        [$cs_prog_cxx_visibility_inlines_hidden \
        $cs_cv_prog_cxx_enable_errors])
    AS_IF([test $cs_cv_prog_cxx_visibility_inlines_hidden_supported != yes],
        [cs_prog_cxx_visibility_inlines_hidden=''])])

# Check if hidden visibility handling is buggy.
AS_IF([test -n "$cs_prog_cxx_visibility_inlines_hidden"],
    [AC_CACHE_CHECK([if $cs_prog_cxx_visibility_inlines_hidden is buggy],
	[cs_cv_prog_cxx_visibility_inlines_hidden_buggy],
	[AS_IF([test $ac_compiler_gnu = yes && test $cs_cv_libstl = yes],
	    [AS_IF([test $cs_host_target = macosx],
                [cs_cv_prog_cxx_visibility_inlines_hidden_buggy=yes],
		[CS_BUILD_IFELSE(
		    [AC_LANG_PROGRAM(
			[[#include <string>]],
			[std::string s; s = "";])],
		    [CS_CREATE_TUPLE(
                        [$cs_prog_cxx_visibility_inlines_hidden \
			$cs_cv_prog_cxx_pic], [$cs_cv_prog_link_shared])],
		    [C++],
		    [cs_cv_prog_cxx_visibility_inlines_hidden_buggy=no],
		    [cs_cv_prog_cxx_visibility_inlines_hidden_buggy=yes],
		    [$cs_cv_libstl_cflags],
		    [$cs_cv_libstl_lflags],
		    [$cs_cv_libstl_libs])])],
            [cs_cv_prog_cxx_visibility_inlines_hidden_buggy=no])])
    AS_IF([test $cs_cv_prog_cxx_visibility_inlines_hidden_buggy = yes],
    	[cs_prog_cxx_visibility_inlines_hidden=''])])

AS_IF([test -n "$cs_prog_cxx_visibility_inlines_hidden"], [$1], [$2])])



#------------------------------------------------------------------------------
# CS_VISIBILITY_FLAG_HIDDEN([LANGUAGE], [CACHE-VAR], [ACTION-IF-SUPPORTED],
#		[ACTION-IF-NOT-SUPPORTED])
#	Check if setting default symbol visibilty to "hidden" is supported.  If
#	so, then the appropriate compiler switch is assigned to CACHE-VAR, or
#	to cs_cv_prog_{language}_visibility_hidden if CACHE-VAR is omitted, and
#	ACTION-IF-SUPPORTED is invoked. Otherwise, the cache variable is
#	cleared and ACTION-IF-NOT-SUPPORTED is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_VISIBILITY_FLAG_HIDDEN],
[_CS_CHECK_VISIBILITY([hidden], [$1], [$2], [$3], [$4])])



#------------------------------------------------------------------------------
# CS_VISIBILITY_FLAG_DEFAULT([LANGUAGE], [CACHE-VAR], [ACTION-IF-SUPPORTED],
#		[ACTION-IF-NOT-SUPPORTED])
#	Check if setting default symbol visibilty to "default" is supported.
#	If so, then the appropriate compiler switch is assigned to CACHE-VAR,
#	or to cs_cv_prog_{language}_visibility_default if CACHE-VAR is omitted,
#	and ACTION-IF-SUPPORTED is invoked. Otherwise, the cache variable is
#	cleared and ACTION-IF-NOT-SUPPORTED is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_VISIBILITY_FLAG_DEFAULT],
[_CS_CHECK_VISIBILITY([default], [$1], [$2], [$3], [$4])])



#------------------------------------------------------------------------------
# _CS_CHECK_VISIBILITY(VISIBILITY, [LANGUAGE], [CACHE-VAR],
#		[ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED])
#	Support macro for CS_VISIBILITY_FLAG_HIDDEN and
#	CS_VISIBILITY_FLAG_DEFAULT which performs the described functionality.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_CHECK_VISIBILITY],
[AC_REQUIRE([_CS_VISIBILITY_PREPARE])
CS_CHECK_BUILD_FLAGS([for $1 symbol visibility flag],
    [_CS_VISIBILITY_CACHE_VAR([$3], [$2], [visibility_$1])],
    [CS_CREATE_TUPLE([-fvisibility=$1])], [$2], [$4], [$5],
    [$CS_TR_SH_lang([cs_cv_prog_]CS_LANG_RESOLVE([$2])[_enable_errors])])])



#------------------------------------------------------------------------------
# CS_VISIBILITY_DECLARE_HIDDEN([LANGUAGE], [CACHE-VAR],
#		[ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED])
#	Check if declaring a symbol's visibilty as "hidden" directly in the
#	code is supported.  If so, then the appropriate language-specific
#	symbol qualifier is assigned to CACHE-VAR, or to
#	cs_cv_prog_{language}_declare_visibility_hidden if CACHE-VAR is
#	omitted, and ACTION-IF-SUPPORTED is invoked. Otherwise, the cache
#	variable is cleared and ACTION-IF-NOT-SUPPORTED is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_VISIBILITY_DECLARE_HIDDEN],
[_CS_CHECK_VISIBILITY_DECLARE([hidden], [$1], [$2], [$3], [$4])])



#------------------------------------------------------------------------------
# CS_VISIBILITY_DECLARE_DEFAULT([LANGUAGE], [CACHE-VAR],
#		[ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED])
#	Check if declaring a symbol's visibilty as "default" directly in the
#	code is supported.  If so, then the appropriate language-specific
#	symbol qualifier is assigned to CACHE-VAR, or to
#	cs_cv_prog_{language}_declare_visibility_default if CACHE-VAR is
#	omitted, and ACTION-IF-SUPPORTED is invoked. Otherwise, the cache
#	variable is cleared and ACTION-IF-NOT-SUPPORTED is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_VISIBILITY_DECLARE_DEFAULT],
[_CS_CHECK_VISIBILITY_DECLARE([default], [$1], [$2], [$3], [$4])])



#------------------------------------------------------------------------------
# _CS_CHECK_VISIBILITY_DECLARE(VISIBILITY, [LANGUAGE], [CACHE-VAR],
#		[ACTION-IF-SUPPORTED], [ACTION-IF-NOT-SUPPORTED])
#	Support macro for CS_VISIBILITY_DECLARE_HIDDEN and
#	CS_VISIBILITY_DECLARE_DEFAULT which performs the described
#	functionality.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_CHECK_VISIBILITY_DECLARE],
[CS_SYMBOL_QUALIFIER([how to declare $1 visibility],
    [_CS_VISIBILITY_CACHE_VAR([$3], [$2], [declare_visibility_$1])],
    [__attribute__((visibility("$1")))], [], [$2], [$4], [$5])])



#------------------------------------------------------------------------------
# _CS_WARNING_CACHE_VAR([CACHE-VAR], [LANGUAGE], DEFAULT-CACHE-VAR-SUFFIX)
#	Helper macro which returns the client-supplied cache variable name or
#	composes one from LANGUAGE and DEFAULT-CACHE-VAR-SUFFIX if not
#	provided.  LANGUAGE typically is `C' or `C++' (defaulting to `C' if not
#	provided).
#------------------------------------------------------------------------------
AC_DEFUN([_CS_VISIBILITY_CACHE_VAR],
[m4_default([$1], [CS_TR_SH_lang([cs_cv_prog_]CS_LANG_RESOLVE([$2])[_$3])])])



#------------------------------------------------------------------------------
# _CS_VISIBILITY_PREPARE
#------------------------------------------------------------------------------
AC_DEFUN([_CS_VISIBILITY_PREPARE],
[CS_COMPILER_ERRORS([C])
CS_COMPILER_ERRORS([C++])])
