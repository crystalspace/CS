# diagnose.m4                                                  -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_MSG_ERROR(ERROR-DESCRIPTION, [EXIT-STATUS])
#	A convenience wrapper for AC_MSG_ERROR() which invokes AC_CACHE_SAVE()
#	before aborting the script.  Saving the cache should make subsequent
#	re-invocations of the configure script faster once the user has
#	corrected the problem(s) which caused the failure.
#------------------------------------------------------------------------------
AC_DEFUN([CS_MSG_ERROR],
    [AC_CACHE_SAVE
    AC_MSG_ERROR([$1], [$2])])
