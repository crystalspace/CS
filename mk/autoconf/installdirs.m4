#-----------------------------------------------------------------------------
# installdirs.m4 (c) Matze Braun <matze@braunis.de>
# Macros for outputing the installation paths which autoconf gathers into the
# Jamconfig file.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# CS_OUTPUT_INSTALLDIRS
#   Transforms the installation dirs which are gathered by autoconf and sets
#   properties in the Jamconfig file for them. We deal with stuff like
#   variable references inside the paths (often the paths contain ${prefix})
#   and with correct quoting here.
#   The script will set the INSTALLDIR.PREFIX, INSTALLDIR.EXEC_PREFIX,
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
    [jam_exec_prefix="AS_ESCAPE([$(INSTALLDIR.PREFIX)])"],
    [jam_exec_prefix=`echo "$exec_prefix" | sed -e 's:///*:/:g'`])

CS_JAMCONFIG_PROPERTY([INSTALLDIR.PREFIX],
    [CS_PREPARE_INSTALLPATH([$jam_prefix])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.EXEC_PREFIX],
    [CS_PREPARE_INSTALLPATH([$jam_exec_prefix])])

# Hack: Unfortunately, values of the other directories often contain references
# to prefix and exec_prefix in lowercase, thus we duplicate the above values.
CS_JAMCONFIG_PROPERTY([prefix], [AS_ESCAPE([$(INSTALLDIR.PREFIX)])])
CS_JAMCONFIG_PROPERTY([exec_prefix], [AS_ESCAPE([$(INSTALLDIR.EXEC_PREFIX)])])

# Hack: Improve Autoconf's default paths a bit.
docdir="$datadir/doc/$PACKAGE_NAME";
AS_IF([test "$includedir" = '${prefix}/include'],
      [includedir="AS_ESCAPE([$(prefix)])/include/$PACKAGE_NAME"])
AS_IF([test "$datadir" = '${prefix}/share'],
      [datadir="AS_ESCAPE([$(prefix)])/share/$PACKAGE_NAME"])
AS_IF([test "$sysconfdir" = '${prefix}/etc'],
      [sysconfdir="AS_ESCAPE([$(prefix)])/etc/$PACKAGE_NAME"])

mapdir="$datadir/maps" ;
plugindir="$libdir/$PACKAGE_NAME" ;

CS_JAMCONFIG_PROPERTY([INSTALLDIR.APPLICATION],[CS_PREPARE_INSTALLPATH([$bindir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.SBIN],[CS_PREPARE_INSTALLPATH([$sbindir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.LIBEXEC],[CS_PREPARE_INSTALLPATH([$libexecdir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.DATA],[CS_PREPARE_INSTALLPATH([$datadir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.MAP], [CS_PREPARE_INSTALLPATH([$mapdir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.CONFIG], [CS_PREPARE_INSTALLPATH([$sysconfdir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.SHAREDSTATE], [CS_PREPARE_INSTALLPATH([$sharedstatedir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.LOCALSTATE], [CS_PREPARE_INSTALLPATH([$localstatedir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.PLUGIN], [CS_PREPARE_INSTALLPATH([$plugindir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.DOC], [CS_PREPARE_INSTALLPATH([$docdir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.LIBRARY], [CS_PREPARE_INSTALLPATH([$libdir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.INCLUDE], [CS_PREPARE_INSTALLPATH([$includedir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.OLDINCLUDE], [CS_PREPARE_INSTALLPATH([$oldincludedir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.INFO], [CS_PREPARE_INSTALLPATH([$infodir])])
CS_JAMCONFIG_PROPERTY([INSTALLDIR.MAN], [CS_PREPARE_INSTALLPATH([$mandir])])

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

