#!/bin/sh
#------------------------------------------------------------------------------
# mergeres.sh
# merge one or more windows .rc files into one; fix some file paths
#------------------------------------------------------------------------------

files=

outfile=$1
shift
fixuppath=$1
shift
sources=$*

echo > ${outfile}

fixup=`echo ${fixuppath} | sed -e "s/[A-Za-z0-9_ ~][A-Za-z0-9_ ~]*/\.\./g"`

for rcfile in ${sources}; do
    rcbase=`echo ${rcfile} | sed -e "s/.*\///"`
    rcpath=`echo ${rcfile} | sed -e "s/${rcbase}$//"`
    
    # replace icon file paths.
    # tho not very flexible... but will do it as long there isn't demand
    # for any other type of resources...
    newpath=`echo "${fixup}${rcpath}" | sed -e "s/\//\\\\\\\\\\\\//g"`
    expression="s/\(.*\) \(ICON\|icon\) \(.*\)/\1 ICON \"${newpath}\3\"/"
    cat ${rcfile} | sed -e "${expression}" >> ${outfile}
    echo >> ${outfile}
done
