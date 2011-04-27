# jamcache.m4                                                  -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
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
# Text cache facility for Jam-style properties.  The cache is stored in
# the shell variable cs_jamfile_text.
#
# CS_JAMCONFIG_APPEND(TEXT)
#	Append text to the Jam text cache.  This is a cover for
#	CS_TEXT_CACHE_APPEND().
#
# CS_JAMCONFIG_PREPEND(TEXT)
#	Prepend text to the Jam text cache.  This is a cover for
#	CS_TEXT_CACHE_PREPEND().
#
# CS_JAMCONFIG_PROPERTY(KEY, VALUE, [OPTIONS])
#	Append, by default, a line of the form "KEY ?= VALUE" to the Jam text
#	cache.  OPTIONS is a comma-separated list of keywords which alter the
#	format of the emitted line. The following options are understood:
#	    append - Employ += appending assignment.
#	    atomic - Emit VALUE as an atomic (quote-enclosed) string.
#	    conditional - Employ ?= optional assignment (the default).
#	    default - Alias for "conditional".
#	    unconditional - Employ = unconditional assignment.
#	For backward compatibility, if OPTIONS is not one of the above keywords
#	and is not the empty string, then "append" is assumed.  Furthermore, if
#	the macro is invoked with a non-empty fourth argument, then
#	"unconditional" is assumed.  Note that if VALUE references other Jam
#	variables, for example $(OBJS), then be sure to protect the value with
#	AS_ESCAPE().  For example:
#	CS_JAMCONFIG_PROPERTY([ALLOBJS], [AS_ESCAPE([$(OBJS) $(LIBOBJS)])])
#
# CS_JAMCONFIG_OUTPUT(FILENAME)
#	Instruct config.status to write the Jam text cache to the given
#	filename.  This is a cover for CS_TEXT_CACHE_OUTPUT().
#------------------------------------------------------------------------------
AC_DEFUN([CS_JAMCONFIG_APPEND],
    [CS_TEXT_CACHE_APPEND([cs_jamconfig_text], [$1])])

AC_DEFUN([CS_JAMCONFIG_PREPEND],
    [CS_TEXT_CACHE_PREPEND([cs_jamconfig_text], [$1])])

AC_DEFUN([CS_JAMCONFIG_PROPERTY],
    [CS_JAMCONFIG_APPEND(
[$1 dnl
CS_MEMBERSHIP_ANY([append], [$3], [+=],
[CS_MEMBERSHIP_ANY([unconditional], [$3], [=],
[CS_MEMBERSHIP_ANY([conditional, default], [$3], [?=],
[CS_MEMBERSHIP_ANY([atomic], [$3], [?=], dnl Backward compatibility. 
[m4_ifval([$3], [+=], dnl Backward compatibility.
[m4_ifval([$4], [=], dnl Backward compatibility.
[?=])])])])])]) dnl
_CS_JAMCONFIG_QUOTE([$2], CS_MEMBERSHIP_ANY([atomic], [$3], [\"])) ;
])])
AC_DEFUN([_CS_JAMCONFIG_QUOTE], [$2$1$2])

AC_DEFUN([CS_JAMCONFIG_OUTPUT],
    [CS_TEXT_CACHE_OUTPUT([cs_jamconfig_text], [$1])])
