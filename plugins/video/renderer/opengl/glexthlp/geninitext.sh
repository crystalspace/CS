#!/bin/sh

cat glexthlp/initextbody | sed -e 's/<ext>/'$1'/' >> $2