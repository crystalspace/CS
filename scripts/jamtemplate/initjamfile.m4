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

