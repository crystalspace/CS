# path.m4                                                      -*- Autoconf -*-
#==============================================================================
# Copyright (C)2004 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_PATH_NORMALIZE(STRING)
#	Normalize a pathname at run-time by transliterating Windows/DOS
#	backslashes to forward slashes.
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_NORMALIZE], [`echo x$1 | tr '\\\\' '/' | sed 's/^x//'`])
