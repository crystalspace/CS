#!/bin/sh
#------------------------------------------------------------------------------
# mergeres.sh
# Merge one or more windows .rc files into one; fix some file paths.
#------------------------------------------------------------------------------

outfile=$1
shift
root=$1
shift
fixup=$1
shift
sources=$*

root=`echo ${root} | sed "s:\.:\\.:g"`

echo > ${outfile}

for rcfile in ${sources}; do
    rcbase=`echo ${rcfile} | sed "s:.*/::"`
    rcbase=`echo ${rcbase} | sed "s:\.:\\.:g"`
    rcpath=`echo ${rcfile} | sed "s:${root}/::"`
    rcpath=`echo ${rcpath} | sed "s:${rcbase}$::"`
    
    # Replace icon file paths.  Though not very flexible, it will do it as
    # long there isn't demand for any other type of resources...
    newpath=`echo "${fixup}/${rcpath}" | sed "s/\//\\\\\\\\\\\\//g"`

    # Older sed commands do not understand alternation (`|' operator), thus
    # \(ICON\|icon\) fails, so use two expressions instead.
    expression1="s:\(.*\) ICON \\\"\(.*\)\\\":\1 ICON \\\"${newpath}\2\\\":g"
    expression2="s:\(.*\) icon \\\"\(.*\)\\\":\1 icon \\\"${newpath}\2\\\":g"
    cat ${rcfile} | sed "${expression1};${expression2}" >> ${outfile}
    echo >> ${outfile}
done
