#! /bin/sh
#------------------------------------------------------------------------------
# mkdef.sh
# Make a dllwrap .def file given classes published by a .csplugin file.
#------------------------------------------------------------------------------

infile=$1
outfile=$2

echo "EXPORTS" > $outfile
echo "  plugin_compiler" >> $outfile
sed '/<implementation>/!d;s:[ 	]*<implementation>\(..*\)</implementation>:  \1_scfInitialize\
  \1_scfFinalize\
  \1_Create:' < $infile >> $outfile
