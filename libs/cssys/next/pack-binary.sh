#!/bin/sh
#==============================================================================
#
#	Copyright (C)1999 by Eric Sunshine <sunshine@sunshineco.com>
#
# The contents of this file are copyrighted by Eric Sunshine.  This work is
# distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  You may distribute this file provided that this
# copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
#
#==============================================================================
#------------------------------------------------------------------------------
# pack-binary.sh
#
#	Create Crystal Space binary distribution packages for MacOS/X Server,
#	OpenStep, and NextStep.
#
#	Run this script from the directory which contains the "cryst" source
#	directory.  It takes a single, optional argument which is the ambient
#	light level for use with the cryst.cfg file.  Typically MacOS/X Server
#	package should use 60 for this value, while OpenStep and NextStep
#	packages should use 40.  The default, if not specified, is 40.
#
# TODO
#	Upgrade to reflect the new directory structure used with Crystal Space
#	beta 13.  Currently it expects the directory structure from beta 12.
#
#------------------------------------------------------------------------------

SRC=./cryst
PACK=${HOME}/cryst
PROGS="cryst mazed blocks"
DATA="blocks.zip large.zip MazeD.zip"
DOCS="blocks.txt config.txt console.txt faq.html"
SRCFILES="autoexec.cfg blocks.cfg coord cryst.cfg csCOM.cfg MazeD.cfg \
	standard.zip"
READMEFILES="README src/system/next/README.Binaries"

AMBIENT=40
if [ $# -gt 0 ]; then
    AMBIENT=$1
    shift;
    fi
echo "Using ambient value: ${AMBIENT}"

COPY_FILES()
    {
    DESC=$1; shift
    FROM=$1; shift
    TO=$1;   shift
    n=0
    echo -n "Grabbing ${DESC} files:"
    for f in $*; do
	n=`expr ${n} + 1`
	if [ ${n} -eq 5 ]; then
	    echo ""
	    echo -n "   "
	    fi
	echo -n " ${f}"
	cp ${SRC}/${FROM}/${f} ${PACK}/${TO}
	done
    echo ""
    }

PATCH_FILE()
    {
    FILE=${PACK}/$1; shift
    DESC=$1; shift
    echo -n " ${DESC}"
    mv ${FILE} ${FILE}.old
    sed "s:${PATTERN}:${REPLACE}:" < ${FILE}.old > ${FILE}
    rm ${FILE}.old
    }

if [ ! -d ${SRC} ]; then
    echo "Can not locate source directory: ${SRC}"
    exit 1
    fi

echo "Creating package: ${PACK}"
rm -rf ${PACK}

mkdir ${PACK}
mkdir ${PACK}/data
mkdir ${PACK}/docs

COPY_FILES "readme" ./ ./ ${READMEFILES}
COPY_FILES "data" data data ${DATA}
COPY_FILES "documentation" docs docs ${DOCS}
COPY_FILES "support" src ./ ${SRCFILES}
COPY_FILES "program" src ./ ${PROGS}

echo -n "Stripping program files:"
for f in ${PROGS}; do
    echo -n " ${f}"
    strip ${PACK}/${f}
    done
echo ""

echo -n "Patching cryst.cfg:"
PATTERN="WORLDFILE="
REPLACE="WORLDFILE=data/"
PATCH_FILE cryst.cfg "paths"

PATTERN="AMBIENT_WHITE=20"
REPLACE="AMBIENT_WHITE=${AMBIENT}"
PATCH_FILE cryst.cfg "brightness"
echo ""

echo -n "Patching MazeD.cfg:"
PATTERN="../data"
REPLACE="data"
PATCH_FILE MazeD.cfg "paths"
echo ""

echo -n "Patching blocks.cfg:"
PATTERN="../data"
REPLACE="data"
PATCH_FILE blocks.cfg "paths"
echo ""

mv ${PACK}/README.Binaries ${PACK}/README.NeXT
echo -n "Patching README.NeXT:"
PATTERN="./cryst large.zip"
REPLACE="./cryst data/large.zip"
PATCH_FILE README.NeXT "paths"
echo ""

echo -n "Fixing permissions: "

echo -n " directories"
find ${PACK} -type d -exec chmod 755 {} \;

echo -n " data"
find ${PACK} ! -type d -exec chmod 644 {} \;

echo -n " programs"
for f in ${PROGS}; do
    chmod 755 ${PACK}/${f}
    done
echo ""
