#=============================================================================
# Copyright (C)2006 by Frank Richter
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
#=============================================================================

#-----------------------------------------------------------------------------
# CS_CHECK_BINUTILS_2_17
#     Certain features (among these, the --as-needed linker flag and splitting
#     debug information) need relatively recent binutils versions to work
#     properly. This test checks the binutils version (by probing the version
#     of ld) and sets the cs_cv_binutils_2_17 variable to 'yes' if the
#     version is at least 2.17 (or some earlier prerelease found to work well
#     enough).
#-----------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_BINUTILS_2_17],
    [CS_CHECK_TOOLS([LD], [ld])
    AS_IF([test -n "$LD"],
        [# Apple linker is not GNU-compatible. Further, on Leopard, -v emits
        # to stderr rather than stdout. Handle these anomalies.
        AS_IF([echo `ld -v 2>&1` | grep GNU 2>&1 > /dev/null],
	    [# binutils versions come in the flavors X.Y as well as X.Y.Z
	    CS_CHECK_PROG_VERSION([binutils], [$LD -v], [2.17], [9.9|.9|.9|.9|.9],
		[cs_cv_binutils_true_2_17=yes],
		[cs_cv_binutils_true_2_17=no])
	    AS_IF([test "$cs_cv_binutils_true_2_17" = yes],
		[cs_cv_binutils_2_17=yes])
	    AS_IF([test -z "$cs_cv_binutils_2_17"],
		[CS_CHECK_PROG_VERSION([binutils], [$LD -v], [2.16.91], 
		    [9.9.9|.9|.9|.9], [cs_cv_binutils_2_17=yes])])
	    AS_IF([test -z "$cs_cv_binutils_2_17"],
		[cs_cv_binutils_2_17=no])])])])
