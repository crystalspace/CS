#============================================================================
# Copyright (C)2003 by Matze Braun <MatzeBraun@gmx.de>
#
# Autoconf Macros for use with CrystalSpace jam builds. (Use in combination
# with jam.m4 and textcache.m4)
#============================================================================

#------------------------------------------------------------------------------
# Text cache facility for Jamconfig properties.  The cache is stored in
# the shell variable cs_jamfile_text.
#
# CS_JAMCONFIG_APPEND(TEXT)
#       Append text to the jamfile text cache.  This is a cover for
#       CS_TEXT_CACHE_APPEND().
#
# CS_JAMCONFIG_PREPEND(TEXT)
#       Prepend text to the jamfile text cache.  This is a cover for
#       CS_TEXT_CACHE_PREPEND().
#
# CS_JAMCONFIG_PROPERTY(KEY, VALUE, [APPEND])
#       Append a line of the form "KEY = VALUE" to the makefile text cache.
#       If the APPEND argument is not the empty string, then VALUE is appended to
#       the existing value of KEY using the form "KEY += VALUE".  Note that if
#       VALUE references other makefile variables, for example $(OBJS), then
#       be sure to protect the value with AS_ESCAPE().  For example:
#       CS_JAMCONFIG_PROPERTY([ALLOBJS], [AS_ESCAPE([$(OBJS) $(LIBOBJS)])])
#
# CS_JAMCONFIG_OUTPUT(FILENAME)
#       Instruct config.status to write the makefile text cache to the given
#       filename.  This is a cover for CS_TEXT_CACHE_OUTPUT().
#------------------------------------------------------------------------------
AC_DEFUN([CS_JAMCONFIG_APPEND],
        [CS_TEXT_CACHE_APPEND([cs_jamconfig_text], [$1])])
AC_DEFUN([CS_JAMCONFIG_PREPEND],
	[CS_TEXT_CACHE_PREPEND([cs_jamconfig_text], [$1])])
AC_DEFUN([CS_JAMCONFIG_PROPERTY],
	[CS_JAMCONFIG_APPEND([$1 m4_ifval([$3], [+=], [=]) \"$2\" ;
])])
AC_DEFUN([CS_JAMCONFIG_OUTPUT],
	 [CS_TEXT_CACHE_OUTPUT([cs_jamconfig_text], [$1],
	 [sed -e's/\${\([[a-zA-Z_]]\+\)}/$(\1)/g' -e's/\\/\\\\/g'])])

#----------------------------------------------------------------------------
#  CS_SUBST
#	CS version of AC_SUBST. Does the same as AC_SUBST but additonally adds
#	the substitutions to the Jamconfig file
#----------------------------------------------------------------------------
AC_DEFUN([CS_SUBST],
	[AC_SUBST([$1])
	 CS_JAMCONFIG_PROPERTY([$1],[${$1}])])

#----------------------------------------------------------------------------
#  CS_INIT_JAMFILE
#    This rule let's config.status create a wrapper Jamfile in case configure
#    has been invoked from a directory outside the source directory
#----------------------------------------------------------------------------
AC_DEFUN([CS_INIT_JAMFILE],
    [AC_CONFIG_COMMANDS([Jamfile test],
      [AS_IF([test "$ac_top_srcdir" != "."],
	[echo Installing Jamfile wrapper.
	 echo "# This file was automatically create by config.status" > Jamfile
	 echo "TOP ?= $ac_top_srcdir ;" >> Jamfile
	 echo "BUILDTOP ?= . ;" >> Jamfile
	 echo "include \$(TOP)/Jamfile ;" >> Jamfile])])])

