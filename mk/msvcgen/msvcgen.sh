#!/bin/sh
# Use this script to generate msvc project files from the Jamfiles

if test "$1" = "--help"; then
    echo "  mk/msvcgen/msvcgen.sh [6|7]"
    echo "Generates msvc projectfiles. You may specify version 6 or 7 if you"
    echo "only want to generate for a specific version (default is both)"
    exit 0
fi

DO_MSVC6=no
DO_MSVC7=no
if test "$1" = 7; then
    DO_MSVC7=yes
    shift
fi
if test "$1" = 6; then
    DO_MSVC6=yes
    shift
fi
if test "$DO_MSVC6" = no -a "$DO_MSVC7" = no; then
    DO_MSVC6=yes
    DO_MSVC7=yes
fi

if test "$DO_MSVC6" = yes; then
    jam $@ -sJAMCONFIG=mk/msvcgen/Jamconfig -sMSVC_VERSION=6 msvcgen
fi
if test "$DO_MSVC7" = yes; then
    jam $@ -sJAMCONFIG=mk/msvcgen/Jamconfig -sMSVC_VERSION=7 msvcgen
fi
