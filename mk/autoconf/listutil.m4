# listutil.m4                                                  -*- Autoconf -*-
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
# CS_MEMBERSHIP_ANY(ITEMS, LIST, [IS-MEMBER], [IS-NOT-MEMBER])
#	Test ITEMS for membership in LIST, both of which are comma-separated m4
#	lists. If any members of ITEMS are present in LIST, then IS-MEMBER is
#	expanded once, regardless of how many matches are found. If no members
#	of ITEMS are in LIST, then IS-NOT-MEMBER is expanded. As a convenience,
#	the m4 variable 'cs_is_member_matches' is set to a comma-separated list
#	of elements from ITEMS which exist in LIST.
#------------------------------------------------------------------------------
AC_DEFUN([CS_MEMBERSHIP_ANY],
[m4_define([cs_is_member_matches], [])dnl
m4_define([_cs_is_member_found], [])dnl
m4_foreach([_cs_is_member_list_item], [$2],dnl
[m4_foreach([_cs_is_member_item], [$1],dnl
[m4_if(_cs_is_member_item, _cs_is_member_list_item,dnl
[m4_define([cs_is_member_matches],dnl
cs_is_member_matches[]m4_ifval(dnl
cs_is_member_matches, [[,]])_cs_is_member_item)dnl
m4_define([_cs_is_member_found], [yes])])])])dnl
m4_ifval(_cs_is_member_found, [$3], [$4])])
