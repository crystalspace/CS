#!/bin/sh
#------------------------------------------------------------------------------
# compres.sh
# compile one or more windows .rc files into one file
#------------------------------------------------------------------------------

files=

# Distributing resources over multiple files doesn't work,
# so merge everything into only file and compile this one.
debug=$1
shift
outfile=$1
shift
sources=$*

echo > ${outfile}.rc

incpath="--include-dir include"

for rcfile in ${sources}; do
    rcbase=`echo ${rcfile} | sed -e "s/.*\///"`
    rcpath=`echo ${rcfile} | sed -e "s/${rcbase}$//"`
    basedir=`echo ${rcpath} | sed -e "s/[A-Za-z0-9_ ~][A-Za-z0-9_ ~]*/\.\./g"`
    
    cat ${rcfile} >> ${outfile}.rc
    echo >> ${outfile}.rc
    incpath="${incpath} --include-dir ${rcpath}"
done

if [ "x$debug" != "xdebug" ]; then
    wrflags=
else
    wrflags="-DCS_DEBUG"
fi

windres ${wrflags} ${incpath} -i ${outfile}.rc -o ${outfile} -I rc -O coff

rm -f ${outfile}.rc
