#! /bin/sh

if test ! -f configure.ac ; then
  echo "*** Please invoke this script from directory containing configure.ac."
  exit 1
fi

MACROFILE=aclocal.m4
MACRODIR=mk/autoconf

rm -f $MACROFILE
for i in $MACRODIR/*.m4 ; do
  cat $i >> $MACROFILE
done

autoconf $@
rc=$?
if test $rc -eq 0 ; then
  rm -f $MACROFILE
fi

# Work around for ^M character emited by autoconf 2.63-
# which causes line-ending problem for SVN.
cr=`echo X | tr X '\\015'`
sed "s/^ac_cr='$cr'$/ac_cr=\`echo X | tr X '\\\015'\`/" < configure > configure.tmp
mv configure.tmp configure
chmod a+x configure

exit $rc
