#    Matze Braun <MatzeBraun@gmx.de>
#    Patrick McFarland (Diablo-D3) <unknown@panax.com>
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

# CS_PATH_CRYSTAL_HELPER
# Checks for Crystal Space paths and libs, This scripts tries first if it can
# find a cs-config in the actual path.  If yes it just uses that.  If not it
# checks if CRYSTAL environment variable is set, and after that if it tries to
# find Crystal Space.  If all else fails, it just looks in /usr/local/crystal.
# Emits an error if it can not locate Crystal Space or it was fed it a bad
# (optional) path.  Exports the variables CRYSTAL_VERSION, CRYSTAL_CFLAGS,
# CRYSTAL_LIBS, and CRYSTAL_INCLUDE_DIR.  Remember to do CFLAGS="$CFLAGS
# $CRYSTAL_CFLAGS" and LDFLAGS=$"LDFLAGS $CRYSTAL_LIBS" so you can use the
# information provided by this script.

dnl helper function
AC_DEFUN([CS_PATH_CRYSTAL_HELPER],
[# Get the cflags, libraries, and include-dir from the cs-config script.
AC_ARG_WITH([cs-prefix],
    [AC_HELP_STRING([--with-cs-prefix=PFX],
	[Prefix where Crystal Space is installed (optional)])],
    [CRYSTAL="$withval"], [])
AC_ARG_ENABLE([cstest],
    [AC_HELP_STRING([--disable-cstest],
	[Do not try to compile and run a test Crystal Space program])],
    [], [enable_cstest=yes])
AC_ARG_VAR([CRYSTAL], [Prefix Where Crystal Space is installed])
dnl If you really want to use autoconf 2.13 (not recommended) comment out the
dnl line above.

# Try to find an installed cs-config.

if test "x$CRYSTAL" != "x"
then
   my_IFS=$IFS; IFS=$PATH_SEPARATOR
   for cs_dir in $CRYSTAL 
   do
     if test -f ${cs_dir}/cs-config
     then
	CSCONF="${cs_dir}/cs-config"
	break
     else
	if test -f ${cs_dir}/bin/cs-config
	then
	   CSCONF="${cs_dir}/bin/cs-config"
	   break
	fi
     fi	
   done
   IFS=$my_IFS

   if test "x$CSCONF" = "x"
   then
     AC_MSG_WARN([Can not find cs-config in path you provided])
     no_cs=yes
   fi
fi

if test "x$CRYSTAL" = "x"
then
   CS_CHECK_TOOLS([CSCONF], [cs-config])
   if test "x$CSCONF" = "x"
   then
      CRYSTAL=/usr/local/crystal
      if test -f $CRYSTAL/bin/cs-config
      then
         CSCONF="$CRYSTAL/bin/cs-config"
      else
	 no_result=yes
	 no_cs=yes
      fi
   fi
fi

# Either we found cs by now, or we caused an error.  Now we define stuff, then
# check if we are running a new enough version

if test "x$no_cs" = "x"
then
    min_cs_version=ifelse([$1],[],[0.99],[$1])
    AC_MSG_CHECKING([for Crystal Space - version >= $min_cs_version])

    CRYSTAL_CFLAGS=CS_RUN_PATH_NORMALIZE([$CSCONF $csconf_args --cxxflags $4])
    CRYSTAL_LIBS=CS_RUN_PATH_NORMALIZE([$CSCONF $csconf_args --libs $4])
    CRYSTAL_INCLUDE_DIR=CS_RUN_PATH_NORMALIZE([$CSCONF --includedir $4])
    CRYSTAL_VERSION=`$CSCONF --version $4`

    cs_major_version=`$CSCONF $cs_args --version | \
       sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)?/\1/'`
    cs_minor_version=`$CSCONF $cs_args --version | \
       sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)?/\2/'`

    if test "x$CRYSTAL_LIBS" = "x"
    then
	no_cs=yes
    fi
fi

if test "x$no_cs" != "x"
then
    enable_cstest=no
fi

if test x$enable_cstest = xyes
then
   ac_save_CXXFLAGS="$CXXFLAGS"
   ac_save_LIBS="$LIBS"
   CXXFLAGS="$CXXFLAGS $CRYSTAL_CFLAGS"
   LIBS="$LIBS $CRYSTAL_LIBS"

   AC_LANG_SAVE()
   AC_LANG(C++)
   AC_TRY_RUN([
#include <cssysdef.h>
#include <stdio.h>
#include <csutil/util.h>

CS_IMPLEMENT_APPLICATION

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* HP/UX 9 writes to sscanf strings */
  tmp_version = csStrNew ("$min_cs_version");
  if (sscanf (tmp_version, "%d.%d", &major, &minor) != 2)
  {
     printf("%s, bad version string\n", "$min_cs_version");
     return 1;
  }

  if (($cs_major_version > major) ||
     (($cs_major_version == major) && ($cs_minor_version > minor)) ||
     (($cs_major_version == major) && ($cs_minor_version == minor)))
  {
    return 0;
  }
  else
  {
    printf("\n*** 'cs-config --version' returned %d.%d, but the minimum version\n", $cs_major_version, $cs_minor_version);
    printf("*** of Crystal Space required is %d.%d. If cs-config is correct, then it is\n", major, minor);
    printf("*** best to upgrade to the required version.\n");
    printf("*** If cs-config was wrong, set the environment variable CRYSTAL\n");
    printf("*** to point to the correct copy of cs-config, and remove the file\n");
    printf("*** config.cache before re-running configure\n");
    return 1;
  }
}
], [], [no_cs=yes], [echo $ac_n "cross compiling; assumed OK... $ac_c"])
  CXXFLAGS="$ac_save_CXXFLAGS"
  LIBS="$ac_save_LIBS"
  AC_LANG_RESTORE()
fi

if test "x$no_cs" = "x"
then
   AC_MSG_RESULT([$CRYSTAL_VERSION])
   ifelse([$2], [], [:], [$2])     
else
   if test x$no_result = x
   then
     AC_MSG_RESULT([no])
   fi
   CRYSTAL_CFLAGS=""
   CRYSTAL_VERSION=""
   CRYSTAL_LIBS=""
   CRYSTAL_INCLUDE_DIR=""
   ifelse([$3], [], [:], [$3])
fi
])

dnl CS_PATH_CRYSTAL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND
dnl    [, LIBS]]]])
dnl Test for Crystal Space, and set CRYSTAL_VERSION, CRYSTAL_CFLAGS,
dnl CRYSTAL_LIBS, and CRYSTAL_INCLUDE_DIR.  Also exports these variables via
dnl AC_SUBST().
dnl
AC_DEFUN([CS_PATH_CRYSTAL],[
CS_PATH_CRYSTAL_HELPER([$1],[$2],[$3],[$4])
AC_SUBST([CRYSTAL_CFLAGS])
AC_SUBST([CRYSTAL_LIBS])
AC_SUBST([CRYSTAL_VERSION])
AC_SUBST([CRYSTAL_INCLUDE_DIR])
])

dnl CS_PATH_CRYSTAL_JAM([MINIMUM-VERSION, [ACTION-IF-FOUND
dnl    [, ACTION-IF-NOT-FOUND [, LIBS]]]])
dnl Test for Crystal Space, and set CRYSTAL_VERSION, CRYSTAL_CFLAGS,
dnl CRYSTAL_LIBS, and CRYSTAL_INCLUDE_DIR.  Also exports these variabls via
dnl CS_JAMCONFIG_PROPERTY() as the Jam variables CRYSTAL.VERSION,
dnl CRYSTAL.CFLAGS, CRYSTAL.LFLAGS, and CRYSTAL.INCLUDE_DIR.
dnl
AC_DEFUN([CS_PATH_CRYSTAL_JAM],[
CS_PATH_CRYSTAL_HELPER([$1],[$2],[$3],[$4])
CS_JAMCONFIG_PROPERTY([CRYSTAL.CFLAGS], [$CRYSTAL_CFLAGS])
CS_JAMCONFIG_PROPERTY([CRYSTAL.LFLAGS], [$CRYSTAL_LIBS])
CS_JAMCONFIG_PROPERTY([CRYSTAL.VERSION], [$CRYSTAL_VERSION])
CS_JAMCONFIG_PROPERTY([CRYSTAL.INCLUDE_DIR], [$CRYSTAL_INCLUDE_DIR])
])
