#!/bin/sh
#==============================================================================
#
#	Copyright (C)1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
#
# The contents of this file are copyrighted by Eric Sunshine.  This work is
# distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  You may distribute this file provided that this
# copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
#
#==============================================================================
#------------------------------------------------------------------------------
# packnext.sh
#
#	Create Crystal Space binary distribution packages for MacOS/X Server,
#	OpenStep, and NextStep.
#
#	Run this script from the directory which contains the "CS" source
#	directory.  It takes a single, optional argument which is the ambient
#	light level for use with the engine.cfg file.  Typically MacOS/X Server
#	package should use 70 for this value, while OpenStep and NextStep
#	packages should use 50.  The default, if not specified, is 50.
#
# To-Do
#	Update to support plug-in modules.  Need to include .csplugin files
#	and strip applications and plug-ins with appropriate flags for both
#	NextStep, OpenStep, and MacOS/X Server.
#       Many of the configuration files have changed.  Augment the list here
#       so that it includes all necessary files.
#------------------------------------------------------------------------------

SRC=./CS
DST=${1-"outgoing"}
PACK=${DST}/CS
PROGS="walktest mazed blocks metademo"
DATA="standard.zip flarge.zip maze.zip room.zip MazeD.zip blocks.zip"
DOCS="blocks.txt config.txt console.txt keys.txt README.NeXT"
SRCFILES="autoexec.cfg blocks.cfg walktest.cfg softrndr.cfg coord MazeD.cfg \
	metademo.cfg scf.cfg vfs.cfg engine.cfg"
READMEFILES="README INSTALL.NeXT-Binary"

AMBIENT=50
if [ $# -gt 1 ]; then
    AMBIENT=$2
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
    chmod u+w ${FILE}.old
    sed "s:${PATTERN}:${REPLACE}:" < ${FILE}.old > ${FILE}
    rm ${FILE}.old
    }

if [ ! -d ${SRC} ]; then
    echo "Can not locate source directory: ${SRC}"
    exit 1
    fi

echo "Creating package: ${PACK}"
if [ ! -d ${DST} ]; then
    mkdir ${DST}
    fi

rm -rf ${PACK}
mkdir ${PACK}
mkdir ${PACK}/data
mkdir ${PACK}/docs

COPY_FILES "readme" docs ./ ${READMEFILES}
COPY_FILES "data" data data ${DATA}
COPY_FILES "documentation" docs docs ${DOCS}
COPY_FILES "support" ./ ./ ${SRCFILES}
COPY_FILES "program" ./ ./ ${PROGS}

echo -n "Stripping program files:"
for f in ${PROGS}; do
    echo -n " ${f}"
    strip ${PACK}/${f}
    done
echo ""

echo -n "Patching engine.cfg:"
PATTERN="Lighting.Ambient.White=20"
REPLACE="Lighting.Ambient.White=${AMBIENT}"
PATCH_FILE engine.cfg "brightness"
echo ""

mv ${PACK}/INSTALL.NeXT-Binary ${PACK}/INSTALL.NeXT
echo -n "Patching INSTALL.NeXT:"
PATTERN="./walktest \(-*[a-z]* *\)large"
REPLACE="./walktest \1data/large"
PATCH_FILE INSTALL.NeXT "paths"
PATTERN="large"
REPLACE="flarge"
PATCH_FILE INSTALL.NeXT "names "
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
