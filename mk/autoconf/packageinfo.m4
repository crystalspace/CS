#=============================================================================
# packageinfo.m4 (c)2003 by Matthias Braun <matze@braunis.de>
# Macors for setting general info on the package, like name and version numbers
# and propagate those to the Jamconfig file.
#=============================================================================


#  CS_PACKAGE_INFO ( [LongName [, Copyright [, Homepage]]] )
#    Set additional info for the package
AC_DEFUN([CS_PACKAGEINFO], [
  PACKAGE_LONGNAME="[$1]"
  PACKAGE_COPYRIGHT="[$2]"
  PACKAGE_HOMEPAGE="[$3]"
])

# Tries to use the information gathered by autoconf for setting some jam
# variables. Note that the version number of your application should only
# contain numbers, because on win32 platform you can only set numerical values
# to the file properties (versionfino .rc files).
AC_DEFUN([CS_PACKAGEINFO_SETJAMCONFIG], [

CS_JAMCONFIG_PROPERTY([PACKAGE.NAME], [$PACKAGE_NAME])
CS_JAMCONFIG_PROPERTY([PACKAGE.VERSION], [$PACKAGE_VERSION])
CS_JAMCONFIG_PROPERTY([PACKAGE.STRING], [$PACKAGE_STRING])
CS_JAMCONFIG_PROPERTY([PACKAGE.BUGREPORT], [$PACKAGE_BUGREPORT])
CS_JAMCONFIG_PROPERTY([PACKAGE.LONGNAME], [$PACKAGE_LONGNAME])
CS_JAMCONFIG_PROPERTY([PACKAGE.HOMEPAGE], [$PACKAGE_HOMEPAGE])
CS_JAMCONFIG_PROPERTY([PACKAGE.COPYRIGHT], [$PACKAGE_COPYRIGHT])

# create a version which can be used in the win32 rules

#disable the [] quoting
changequote(,)
V1=`echo "$PACKAGE_VERSION" | sed -e '/[0-9]*/!c0' -e 's/\([0-9]*\).*/\1/'`
V2=`echo $PACKAGE_VERSION | sed -e '/[0-9]*\.[0-9]*/!c0' -e 's/\([0-9]*\)\.\([0-9]*\).*/\2/'`
V3=`echo $PACKAGE_VERSION | sed -e '/[0-9]*\.[0-9]*\.[0-9]*/!c0' -e 's/\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\).*/\3/'`
V4=`echo $PACKAGE_VERSION | sed -e '/[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/!c0' -e 's/^\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\).*/\4/'`
changequote([,])

CS_JAMCONFIG_APPEND([PACKAGE.VERSION.LIST ?= $V1 $V2 $V3 $V4 ;
])

])
