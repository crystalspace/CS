#!/bin/sh
#------------------------------------------------------------------------------
# mergeres.sh
# Merge one or more windows .rc files into one; fix some file paths.
#------------------------------------------------------------------------------

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
    
    # Replace icon file paths.  Though not very flexible, it will do it as
    # long there isn't demand for any other type of resources...
    newpath=`echo "${fixup}${rcpath}" | sed -e "s/\//\\\\\\\\\\\\//g"`

    # Older sed commands do not understand alternation (`|' operator), thus
    # \(ICON\|icon\) fails, so use two expressions instead.
    expression1="s:\(.*\) ICON \\\"\(.*\)\\\":\1 ICON \\\"${newpath}\2\\\":g"
    expression2="s:\(.*\) icon \\\"\(.*\)\\\":\1 icon \\\"${newpath}\2\\\":g"
    cat ${rcfile} | sed -e "${expression1}" -e "${expression2}" >> ${outfile}
    echo >> ${outfile}
done
