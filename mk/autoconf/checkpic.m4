# checkpic.m4                                                  -*- Autoconf -*-
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
# CS_CHECK_COMPILER_PIC([LANGUAGE], [CACHE-VAR], [ACTION-IF-FOUND],
#                       [ACTION-IF-NOT-FOUND])
#	Check if compiler can be instructed to produce
#	position-independent-code (PIC).  This feature is required by some
#	platforms when building plugin modules and shared libraries.  If
#	LANGUAGE is not provided, then `C' is assumed (other options include
#	`C++').  If CACHE-VAR is provided, then it is assigned the result of
#	the check. If it is not provided, then CACHE-VAR defaults to the name
#	"cs_cv_prog_compiler_pic".  If a PIC-enabling option (such as `-fPIC')
#	is discovered, then it is assigned to CACHE-VAR and ACTION-IF-FOUND is
#	invoked; otherwise "no" is assigned to CACHE-VAR and
#	ACTION-IF-NOT-FOUND is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMPILER_PIC],
    [CS_CHECK_BUILD_FLAGS(
	[if m4_default([$1],[C]) compiler supports PIC generation],
	[m4_default([$2],[cs_cv_prog_compiler_pic])],
	[CS_CREATE_TUPLE([-fPIC])], [$1], [$3], [$4])])

