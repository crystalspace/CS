#----------------------------------------------------------------------------
#  CS_CHECK_INSTALLDIRS
#	Determine installation locations.
#----------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_INSTALLDIRS],[
	AS_IF([test "$prefix" == "NONE"],
	  [prefix="$ac_default_prefix"])
	AS_IF([test "$exec_prefix" == "NONE"],
	  [exec_prefix="\$(INSTALLDIR.PREFIX)"])
		
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.PREFIX], [$prefix])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.EXEC_PREFIX], [$exec_prefix])

	# values of the other dirs often contain refs to prefix and
	# exec_prefix in lowercase :-/
	CS_JAMCONFIG_PROPERTY([prefix], [AS_ESCAPE([$(INSTALLDIR.PREFIX)])])
	CS_JAMCONFIG_PROPERTY([exec_prefix], [AS_ESCAPE([$(INSTALLDIR.EXEC_PREFIX)])])

	# modify autoconf default paths
	docdir="$datadir/doc/$PACKAGE_NAME";
	AS_IF([test "$includedir" = '${prefix}/include'],
	      [includedir="\$(prefix)/include/$PACKAGE_NAME"])
	AS_IF([test "$datadir" = '${prefix}/share'],
	      [datadir="\$(prefix)/share/$PACKAGE_NAME"])
	AS_IF([test "$sysconfdir" = '${prefix}/etc'],
	      [sysconfdir="\$(prefix)/etc/$PACKAGE_NAME"])
	
	# construct some own paths
	mapdir="$datadir/maps" ;
	plugindir="$libdir/$PACKAGE_NAME" ;
		  
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.APPLICATION], [$bindir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.SBIN], [$sbindir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.LIBEXEC], [$libexecdir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.DATA], [$datadir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.MAP], [$mapdir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.CONFIG], [$sysconfdir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.SHAREDSTATE], [$sharedstatedir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.LOCALSTATE], [$localstatedir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.PLUGIN], [$plugindir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.DOC], [$docdir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.LIBRARY], [$libdir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.INCLUDE], [$includedir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.OLDINCLUDE], [$oldincludedir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.INFO], [$infodir])
	CS_JAMCONFIG_PROPERTY([INSTALLDIR.MAN], [$mandir])
])

