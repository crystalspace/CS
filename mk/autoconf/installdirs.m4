#-----------------------------------------------------------------------------
# installdirs.m4 (c) Matze Braun <matze@braunis.de>
# Macros for outputing the installation paths which autoconf gathers into the
# Jamconfig file.
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
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# CS_OUTPUT_INSTALLDIRS
#   Transforms the installation dirs which are gathered by autoconf and sets
#   properties in the Jamconfig file for them. We deal with stuff like
#   variable references inside the paths (often the paths contain ${prefix})
#   and with correct quoting here.
#   The script will set the prefix, exec_prefix,
#   INSTALLDIR.APPLICATION, INSTALLDIR.SBIN, INSTALLDIR.LIBEXEC,
#   INSTALLDIR.DATA, INSTALLDIR.MAP, INSTALLDIR.CONFIG, INSTALLDIR.SHAREDSTATE
#   INSTALLDIR.LOCALSTATE, INSTALLDIR.PLUGIN, INSTALLDIR.DOC
#   INSTALLDIR.LIBRARY, INSTALLDIR.INCLUDE, INSTALLDIR.OLDINCLUDE,
#   INSTALLDIR.INFO, INSTALLDIR.MAN
#-----------------------------------------------------------------------------
AC_DEFUN([CS_OUTPUT_INSTALLDIRS],[
# Handle the case when no prefix is given. And the special case when a path
# contains more than 2 slashes, these paths seem to be correct but jam fails
# on them.
AS_IF([test $prefix = NONE],
    [jam_prefix="$ac_default_prefix"],
    [jam_prefix=`echo "$prefix" | sed -e 's:///*:/:g'`])
AS_IF([test $exec_prefix = NONE],
    [jam_exec_prefix="AS_ESCAPE([$(prefix)])"],
    [jam_exec_prefix=`echo "$exec_prefix" | sed -e 's:///*:/:g'`])

CS_JAMCONFIG_PROPERTY([prefix], [CS_PREPARE_INSTALLPATH([$jam_prefix])])
CS_JAMCONFIG_PROPERTY([exec_prefix], [CS_PREPARE_INSTALLPATH([$jam_exec_prefix])])

CS_JAMCONFIG_PROPERTY([bindir],[CS_PREPARE_INSTALLPATH([$bindir])])
CS_JAMCONFIG_PROPERTY([sbindir],[CS_PREPARE_INSTALLPATH([$sbindir])])
CS_JAMCONFIG_PROPERTY([libexecdir],[CS_PREPARE_INSTALLPATH([$libexecdir])])
CS_JAMCONFIG_PROPERTY([datadir],[CS_PREPARE_INSTALLPATH([$datadir])])
CS_JAMCONFIG_PROPERTY([sysconfdir], [CS_PREPARE_INSTALLPATH([$sysconfdir])])
CS_JAMCONFIG_PROPERTY([sharedstatedir], [CS_PREPARE_INSTALLPATH([$sharedstatedir])])
CS_JAMCONFIG_PROPERTY([localstatedir], [CS_PREPARE_INSTALLPATH([$localstatedir])])
CS_JAMCONFIG_PROPERTY([libdir], [CS_PREPARE_INSTALLPATH([$libdir])])
CS_JAMCONFIG_PROPERTY([includedir], [CS_PREPARE_INSTALLPATH([$includedir])])
CS_JAMCONFIG_PROPERTY([oldincludedir], [CS_PREPARE_INSTALLPATH([$oldincludedir])])
CS_JAMCONFIG_PROPERTY([infodir], [CS_PREPARE_INSTALLPATH([$infodir])])
CS_JAMCONFIG_PROPERTY([mandir], [CS_PREPARE_INSTALLPATH([$mandir])])

])

#-----------------------------------------------------------------------------
# CS_PREPARE_INSTALLPATH
#   Transform variables of the form ${bla} to $(bla) inside the string and
#   correctly quotes backslashes.
#   This is needed if you want to output some of the paths that autoconf
#   creates to the Jamconfig file.
#-----------------------------------------------------------------------------
AC_DEFUN([CS_PREPARE_INSTALLPATH],
dnl We need all the strange \\\\ quoting here, because the command will be
dnl inserted into a "" block and sed needs quoting as well
[`echo "$1" | sed -e 's/\${\([[a-zA-Z_][a-zA-Z_]]*\)}/$(\1)/g' -e 's/\\\\/\\\\\\\\/g'`])

