#!/bin/sh

# Make sure we're in the correct directory
if [ ! -f configure.ac ]; then
  echo "Please start this script from CS root directory"
  exit 1
fi

MACROFILE=aclocal.m4
MACROFILES=mk/autoconf
echo "creating $MACROFILE from macros out of $MACROFILES/*.m4"
rm -f $MACROFILE
for i in `find $MACROFILES -name "*.m4"`; do
	cat $i >> $MACROFILE
done
echo autoconf
autoconf

