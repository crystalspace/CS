#! /bin/sh
#------------------------------------------------------------------------------
# mkdef.sh
# Make a dllwrap .def file given classes published by a .csplugin file.
#------------------------------------------------------------------------------

infile=$1
outfile=$2

# Implementation note: Do not change the layout/formatting of the sed
# invocation.  It is purposely split over multiple lines (using backslash +
# newline) in order to ensure that the entries in the generated .def file are
# each placed on a separate line.

echo "EXPORTS" > $outfile
echo "  plugin_compiler" >> $outfile
sed '/<implementation>/!d;s:[ 	]*<implementation>\(..*\)</implementation>:  \1_scfInitialize\
  \1_scfFinalize\
  \1_Create:' < $infile >> $outfile
