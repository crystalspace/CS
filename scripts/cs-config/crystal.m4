# AM_PATH_CS
#
# Checks for Crystal Space paths and libs,
# This scripts tries first if it can find a cs-config in the actual path
# if yes it just uses that. If not it looke if CRYSTAL var is set, and after
# that if it tries to find Crystal Space and then it just looks in 
# /usr/local/crystal. Causes an error if it cant find it or you fed it a bad
# (optional) path. Remember to do CFLAGS="$CFLAGS $CS_CFLAGS" and so forth for
# CS_LIBS, CS_CXXFLAGS, and CS_LIBS so you can use the information provided
# by this script!

# Matze Braun <MatzeBraun@gmx.de>
# Patrick McFarland (Diablo-D3) <unknown@panax.com>

dnl AM_PATH_CS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, LIBS]]]])
dnl Test for CS, and define CS_VERSION, CS_LONGVERSION, CS_CFLAGS, 
dnl CS_CXXFLAGS, and CS_LIBS
dnl

AC_DEFUN(AM_PATH_CS,
[dnl 
dnl Get the cflags and libraries from the cs-config script
dnl
AC_ARG_WITH(cs-prefix,[  --with-cs-prefix=PFX   Prefix where Crystal Space is installed (optional)],
        CRYSTAL="$withval", )
AC_ARG_ENABLE(cstest, [  --disable-cstest       Do not try to compile and run a test Crystal Space program], disable_cstest=yes, )
dnl AC_ARG_VAR([CRYSTAL],     [Prefix Where Crystal Space is installed])
dnl ^ doesnt work because too many people still use autoconf 2.13 instead of
dnl autoconf 2.5x. Damn them. -- Diablo-D3

dnl try to find an installed cs-config

if test "x$CRYSTAL" != x ; then
   if test -f $CRYSTAL/cs-config ; then
      CSCONF="$CRYSTAL/cs-config"
   else
      if test -f $CRYSTAL/bin/cs-config ; then
         CSCONF="$CRYSTAL/bin/cs-config"
      else
         AC_MSG_ERROR(Can't find cs-config in path you provided)
      fi
   fi	
fi

if test "x$CRYSTAL" = x ; then
   AC_PATH_PROG(CSCONF, cs-config)
   if test "x$CSCONF" = x ; then 
      CRYSTAL=/usr/local/crystal
      if test -f $CRYSTAL/bin/cs-config ; then
         CSCONF="$CRYSTAL/bin/cs-config"
      else
         echo "*** The cs-config script installed by Crystal Space could not be found"
         echo "*** If Crystal Space was installed in PREFIX, make sure PREFIX/bin is in"
         echo "*** your path, or set the CRYSTAL environment variable to the full path"
         echo "*** to cs-config."
         AC_MSG_ERROR(Can't find cs-config)
      fi
   fi
fi


# Well, either we found cs by now, or we caused an error.
# Now we define stuff, then check if we are running a new enough version

min_cs_version=ifelse([$1], ,0.93,$1)
AC_MSG_CHECKING(for Crystal Space - version >= $min_cs_version)
CS_CFLAGS=`$CSCONF $csconf_args --cflags $4`
CS_CXXFLAGS=`$CSCONF $csconf_args --cxxflags $4`
CS_LIBS=`$CSCONF $csconf_args --libs $4`
CS_VERSION=`$CSCONF --version $4`
CS_LONGVERSION=`$CSCONF --longversion $4`

cs_major_version=`$CSCONF $cs_args --version | \
   sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
cs_minor_version=`$CSCONF $cs_args --version | \
   sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`

if test "x$enable_cstest" = "xyes" ; then
   ac_save_CFLAGS="$CFLAGS"
   ac_save_LIBS="$LIBS"
   CFLAGS="$CFLAGS $CS_CFLAGS"
   LIBS="$LIBS $CS_LIBS"

dnl Godamity-damn *nix sh sucks.

rm -f conf.cstest
AC_TRY_RUN([
#include <cssysdef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

CS_IMPLEMENT_APPLICATION

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.cstest");
  */
  { FILE *fp = fopen("conf.cstest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_cs_version");
  if (sscanf(tmp_version, "%d.%d", &major, &minor) != 3) {
     printf("%s, bad version string\n", "$min_cs_version");
     exit(1);
   }

   if (($cs_major_version > major) ||
      (($cs_major_version == major) && ($cs_minor_version > minor)) ||
      (($cs_major_version == major) && ($cs_minor_version == minor))
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

],, no_cs=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi

  if test "x$no_cs" = x ; then
     AC_MSG_RESULT($CS_LONGVERSION)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
       if test -f conf.cstest ; then
        :
       else
          echo "*** Could not run Crystal Space test program, checking why..."
          CFLAGS="$CFLAGS $CS_CFLAGS"
          LIBS="$LIBS $CS_LIBS"
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     CS_CFLAGS=""
     CS_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(CS_CFLAGS)
  AC_SUBST(CS_CXXFLAGS)
  AC_SUBST(CS_LIBS)
  AC_SUBST(CS_VERSION)
  AC_SUBST(CS_LONGVERSION)
  rm -f conf.cstest
  # "If sex was gpl, everyone would want to join in" -- Diablo-D3
])

