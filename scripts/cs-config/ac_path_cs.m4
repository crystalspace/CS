dnl @synopsis AC_PATH_CS
dnl
dnl Checks for Crystal Space paths and libs,
dnl This scripts tries first if it can find a cs-config in the actual path
dnl if yes it just uses that. If not it looke if CRYSTAL var is set, and after
dnl that if it tries to find Crystal Space in /usr/local/crystal
dnl
dnl @author Matze Braun <MatzeBraun@gmx.de>

dnl AC_PATH_CS ([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, LIBS]]])
AC_DEFUN(AC_PATH_CS,
[
AC_ARG_WITH(cs-prefix,[  --with-cs-prefix=PFX   Prefix where Crystal Space is installed (optional)],
        CRYSTAL="$withval", )
AC_ARG_ENABLE(cstest, [  --disable-cstest       Do not try to compile and run a test Crystal Space program], disable_cstest=yes, )

dnl try to find an installed cs-config
if test "x$CRYSTAL" = x ; then
  AC_PATH_PROG(CSCONF, cs-config)
  if test "x$CSCONF" = x ; then
    CRYSTAL=/usr/local/crystal
  fi
fi
if test "x$CSCONF" = x ; then
  if test -f $CRYSTAL/cs-config ; then
    CSCONF="$CRYSTAL/cs-config"
  else
    if test -f $CRYSTAL/bin/cs-config ; then
      CSCONF="$CRYSTAL/bin/cs-config"
    fi
  fi
fi

AC_MSG_CHECKING(for CS)
if test "x$CSCONF" = x ; then
  AC_MSG_RESULT(not found)
  echo "*** Couldn't find cs-config script!"
  no_cs=yes
else
  CS_CFLAGS=`$CSCONF --cflags $3`
  CS_CXXFLAGS=`$CSCONF --cxxflags $3`
  CS_LIBS=`$CSCONF --libs $3`
  CS_VERSION=`$CSCONF --version $3`
  AC_MSG_RESULT($CS_VERSION)
fi	

if test "x$no_cs" = x ; then
  AC_MSG_CHECKING(if a simple CS app links)
  if test "x$disable_cstest" = x ; then
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    ac_save_LIBS="$LIBS"
    ac_save_CXXFLAGS="$CXXFLAGS"
    LIBS="$CS_LIBS"
    CXXFLAGS="$CS_CXXFLAGS"
    AC_TRY_RUN([
/* FixMe I need a nice app here */
#include <cssysdef.h>

CS_IMPLEMENT_APPLICATION

int main()
{
	return 0;
}
],, no_cs=yes, [echo $ac_n "cross compiling; assumed OK... $ac_c"])
    CXXFLAGS="$ac_save_CXXFLAGS"
    LIBS="$ac_save_LIBS"
    AC_LANG_RESTORE
    if test "x$no_cs" = x ; then
      AC_MSG_RESULT(ok)
    else
      AC_MSG_RESULT(failed)
    fi    
  else
    AC_MSG_RESULT(test disabled)
  fi    
fi

if test "x$no_cs" = x ; then
  ifelse([$1], , :, [$1])
  AC_SUBST(CS_CFLAGS)
  AC_SUBST(CS_CXXFLAGS)
  AC_SUBST(CS_LIBS)
else
  ifelse([$2], , :, [$2])
fi
])

