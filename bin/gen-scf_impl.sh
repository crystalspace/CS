#!/bin/sh

TMPFILE=/tmp/scf_impl$$

gcc -E -x c -P -DSCF_IN_IMPLEMENTATION_H -Iinclude/ include/csutil/scf_implgen.h > $TMPFILE

echo "" > include/csutil/scf_implgen_p.h
IFS=$'\n'
set -f
for l in $(cat include/csutil/scf_implgen_p_template.h) ; do
  if echo "$l" | grep "** INSERT OUTPUT FROM GCC HERE **" > /dev/null ; then
    cat $TMPFILE >> include/csutil/scf_implgen_p.h
  else
    echo "$l" >> include/csutil/scf_implgen_p.h
  fi
done

rm $TMPFILE
