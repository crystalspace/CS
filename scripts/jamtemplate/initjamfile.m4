#============================================================================
# Copyright (C)2003 by Matze Braun <matzebraun@users.sourceforge.net>
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
#============================================================================

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
