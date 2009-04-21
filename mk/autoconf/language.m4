# language.m4                                                  -*- Autoconf -*-
#==============================================================================
# Copyright (C)2008 by Eric Sunshine <sunshine@sunshineco.com>
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
# CS_TR_SH_Lang(ARG)
# CS_TR_SH_lang(ARG)
# CS_TR_SH_LANG(ARG)
#	Similar to AS_TR_SH(), but converts "+" in ARG to "x" rather than "p"
#	since "x" is more suitable in a context dealing with programming
#	languages. For instance, CS_TR_SH_Lang([C++]) becomes "Cxx", rather
#	than the less idiomatic "Cpp" as would be the case with AS_TR_SH().
#	Useful for composing shell variable names pertaining to a particular
#	language. For instance, if the language upon which a macro operates is
#	in $1, then the macro may compose a shell variable name using
#	CS_TR_SH_lang([cs_cv_prog_$1_feature]).  CS_TR_SH_Lang() performs no
#	case-folding on ARG. CS_TR_SH_lang() folds ARG to lowercase.
#	CS_TR_SH_LANG() folds ARG to uppercase.
#------------------------------------------------------------------------------
AC_DEFUN([CS_TR_SH_Lang], [AS_TR_SH(m4_translit([$1],[+],[x]))])
AC_DEFUN([CS_TR_SH_lang], [AS_TR_SH(m4_translit([$1],[+A-Z],[xa-z]))])
AC_DEFUN([CS_TR_SH_LANG], [AS_TR_SH(m4_translit([$1],[+a-z],[XA-Z]))])



#------------------------------------------------------------------------------
# CS_LANG_RESOLVE([LANGUAGE], [FALLBACK])
#	A convenience for macros which optionally accept a language specifier.
#	Returns LANGUAGE if non-empty, else returns FALLBACK.  If FALLBACK is
#	not provided, then FALLBACK is assumed to be "C".
#------------------------------------------------------------------------------
AC_DEFUN([CS_LANG_RESOLVE], [m4_default([$1],m4_default([$2],[C]))])
