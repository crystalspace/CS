# headercache.m4                                               -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# Text cache facility for C-style #define properties.  The cache is stored in
# the shell variable cs_header_text.
#
# CS_HEADER_APPEND(TEXT)
#	Append text to the C header text cache.  This is a cover for
#	CS_TEXT_CACHE_APPEND().
#
# CS_HEADER_PREPEND(TEXT)
#	Prepend text to the C header text cache.  This is a cover for
#	CS_TEXT_CACHE_PREPEND().
#
# CS_HEADER_PROPERTY(KEY, [VALUE])
#	Append a line of the form "#define KEY VALUE" to the C header text
#	cache.  If the VALUE argument is omitted, then the appended line has
#	the simplified form "#define KEY".
#
# CS_HEADER_OUTPUT(FILENAME)
#	Instruct config.status to write the C header text cache to the given
#	filename.  This is a cover for CS_TEXT_CACHE_OUTPUT().
#------------------------------------------------------------------------------
AC_DEFUN([CS_HEADER_APPEND], [CS_TEXT_CACHE_APPEND([cs_header_text], [$1])])
AC_DEFUN([CS_HEADER_PREPEND], [CS_TEXT_CACHE_PREPEND([cs_header_text], [$1])])
AC_DEFUN([CS_HEADER_PROPERTY],
[CS_HEADER_APPEND([@%:@define $1[]m4_ifval([$2], [ $2], [])
])])
AC_DEFUN([CS_HEADER_OUTPUT], [CS_TEXT_CACHE_OUTPUT([cs_header_text], [$1])])
