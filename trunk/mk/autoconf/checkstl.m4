# checkstl.m4                                                  -*- Autoconf -*-
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
# CS_CHECK_STL([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#	Check if C++ Standard Template Library (STL) is available. If found,
#	shell variable cs_cv_libstl is set to 'yes' and variables
#	cs_cv_libstl_cflags, cs_cv_libstl_lflags, and cs_cv_libstl_libs are
#	populated appropriately and ACTION-IF-FOUND is invoked.  If not found,
#	cs_cv_libstl is set to 'no', the other variables are cleared, and
#	ACTION-IF-NOT-FOUND is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_STL],
    [CS_CHECK_BUILD([for STL], [cs_cv_libstl],
	[AC_LANG_PROGRAM(
	    [[#include <map>
	    #include <string>]],
	    [[std::map<std::string,int> m; m.begin();]])],
	[], [C++], [$1], [$2])])
