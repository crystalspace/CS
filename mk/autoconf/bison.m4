# Check for bison version

dnl AC_CHECK_BISON([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for good Bison version
dnl
AC_DEFUN(AC_CHECK_BISON,
[dnl
min_bsn_version=ifelse([$1], ,1.35,$1)
AC_MSG_CHECKING(for bison - version >= $min_bsn_version)
no_bsn=""
bsn_config_major_version=`$BISONBIN -V | \
    sed '/[[0-9]][[0-9]]*\.[[0-9]][[0-9]]*/!d;s/.*\([[0-9]][[0-9]]*\)\.[[0-9]][[0-9]]*.*/\1/'`
bsn_config_minor_version=`$BISONBIN -V | \
    sed '/[[0-9]][[0-9]]*\.[[0-9]][[0-9]]*/!d;s/.*[[0-9]][[0-9]]*\.\([[0-9]][[0-9]]*\).*/\1/'`
bsn_min_major_version=`echo $min_bsn_version | \
    sed 's/\([[0-9]][[0-9]]*\)\.[[0-9]][[0-9]]*/\1/'`
bsn_min_minor_version=`echo $min_bsn_version | \
    sed 's/[[0-9]][[0-9]]*\.\([[0-9]][[0-9]]*\)/\1/'`
bsn_config_is_lt=""
if test $bsn_config_major_version -lt $bsn_min_major_version ; then
  bsn_config_is_lt=yes
else
  if test $bsn_config_major_version -eq $bsn_min_major_version ; then
    if test $bsn_config_minor_version -lt $bsn_min_minor_version ; then
      bsn_config_is_lt=yes
    fi
  fi
fi
if test x$bsn_config_is_lt = xyes ; then
  no_bsn=yes
fi
if test x$no_bsn = x ; then
   AC_MSG_RESULT(yes)
   ifelse([$2], , :, [$2])
else
   AC_MSG_RESULT([no (version $bsn_config_major_version.$bsn_config_minor_version)])
#   if test x$bsn_config_is_lt = xyes ; then
#     echo "*** Your installed version of $BISONBIN is too old."
#   fi
   ifelse([$3], , :, [$3])
fi
])
