#==============================================================================
# packageinfo.m4
# Copyright (C)2003 by Matthias Braun <matze@braunis.de>
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
# Macors for setting general info on the package, such as name and version
# numbers and propagate them to the generated make and Jam property files.
#==============================================================================

#------------------------------------------------------------------------------
# CS_PACKAGEINFO([LONGNAME], [COPYRIGHT, [HOMEPAGE])
#	Set additional information for the package.  Note that the version
#	number of your application should only contain numbers, because on
#	Windows you can only set numerical values in some of the file
#	properties (such as versioninfo .rc files).
#------------------------------------------------------------------------------
AC_DEFUN([CS_PACKAGEINFO],
    [PACKAGE_LONGNAME="[$1]"
    PACKAGE_COPYRIGHT="[$2]"
    PACKAGE_HOMEPAGE="[$3]"
])


#------------------------------------------------------------------------------
# CS_EMIT_PACKAGEINFO(TARGET)
#	Emit extended package information to a designated target.  If TARGET is
#	"jam", then information is emitted via CS_JAMCONFIG_PROPERTY macros.
#	If TARGET is "make", then information is emitted via
#	CS_MAKEFILE_PROPERTY macros.
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_PACKAGEINFO],
    [_CS_EMIT_PACKAGEINFO([$1], [PACKAGE.NAME], [$PACKAGE_NAME])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE.VERSION], [$PACKAGE_VERSION])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE.STRING], [$PACKAGE_STRING])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE.BUGREPORT], [$PACKAGE_BUGREPORT])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE.LONGNAME], [$PACKAGE_LONGNAME])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE.HOMEPAGE], [$PACKAGE_HOMEPAGE])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE.COPYRIGHT], [$PACKAGE_COPYRIGHT])
    m4_define([cs_ver_components], m4_translit(AC_PACKAGE_VERSION, [.], [ ]))
    m4_case([$1],
    [make], [CS_MAKEFILE_PROPERTY([PACKAGE.VERSION.LIST], cs_ver_components)],
    [jam], [CS_JAMCONFIG_APPEND([PACKAGE.VERSION.LIST ?= cs_ver_components ;
])],
    [_CS_EMIT_PACKAGEINFO_FATAL([$1])])])

AC_DEFUN([_CS_EMIT_PACKAGEINFO],
    [m4_case([$1],
	[make], [CS_MAKEFILE_PROPERTY([$2], [$3], [$4])],
	[jam], [CS_JAMCONFIG_PROPERTY([$2], [$3], [$4])],
	[_CS_EMIT_PACKAGEINFO_FATAL([$1])])])

AC_DEFUN([_CS_EMIT_PACKAGEINFO_FATAL],
    [AC_FATAL([CS_EMIT_PACKAGEINFO: unrecognized emitter ([$1])])])
