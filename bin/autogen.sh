#! /bin/sh

# Correct working directory?
if test ! -f configure.ac ; then
  echo "*** Please invoke this script from directory containing configure.ac."
  exit 1
fi

MACROFILE=aclocal.m4
MACRODIR=mk/autoconf

# Creating jam configure file
rm -f $MACROFILE
for i in $MACRODIR/jam/*.m4 ; do
  cat $i >> $MACROFILE
done
# cheating a bit to make autoconf use configure-jam.ac
mv configure.ac configure-make.ac
cp configure-jam.ac configure.ac
autoconf
mv configure-make.ac configure.ac
mv configure configure-jam
rm -f $MACROFILE

# Creating normal configure file
rm -f $MACROFILE
for i in $MACRODIR/*.m4 ; do
  cat $i >> $MACROFILE
done
autoconf
rm -f $MACROFILE
