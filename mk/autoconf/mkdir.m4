# mkdir.m4                                                     -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# Replacement for AS_MKDIR_P() from m4sugar/m4sh.m4 which fixes two problems
# which are present at least as recently as Autoconf 2.57.  This bug, along
# with a patch, was submitted to the Autoconf GNATS database by Eric Sunshine
# as #227.
#
# 1) Removes bogus "-p" directory which the stock AS_MKDIR_P() leaves laying
#    around in the working directory if the mkdir command does not recognize
#    the -p option.
# 2) Takes advantage of the older "mkdirs" program if it exists and if "mkdir
#    -p" does not work.
#------------------------------------------------------------------------------
m4_defun([_AS_MKDIR_P_PREPARE],
[if mkdir -p . 2>/dev/null; then
  as_mkdir_p='mkdir -p'
elif mkdirs . 2>/dev/null; then
  as_mkdir_p='mkdirs'
else
  as_mkdir_p=''
fi
test -r ./-p && rm -rf ./-p
])# _AS_MKDIR_P_PREPARE

m4_define([AS_MKDIR_P],
[AS_REQUIRE([_$0_PREPARE])dnl
{ if test -n "$as_mkdir_p"; then
    $as_mkdir_p $1
  else
    as_dir=$1
    as_dirs=
    while test ! -d "$as_dir"; do
      as_dirs="$as_dir $as_dirs"
      as_dir=`AS_DIRNAME("$as_dir")`
    done
    test ! -n "$as_dirs" || mkdir $as_dirs
  fi || AS_ERROR([cannot create directory $1]); }
])# AS_MKDIR_P
