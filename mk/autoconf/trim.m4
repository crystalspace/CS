# trim.m4                                                      -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_TRIM(STRING)
#	Strip leading and trailing spaces from STRING and collapse internal
#	runs of multiple spaces to a single space.
#------------------------------------------------------------------------------
AC_DEFUN([CS_TRIM], [`echo x$1 | sed 's/^x//;s/   */ /g;s/^ //;s/ $//'`])
