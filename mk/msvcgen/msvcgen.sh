#!/bin/sh
# Use this script to generate msvc project files from the Jamfiles

jam $@ -sJAMCONFIG=mk/msvcgen/Jamconfig -sMSVC_VERSION=7 msvcgen
jam $@ -sJAMCONFIG=mk/msvcgen/Jamconfig -sMSVC_VERSION=6 msvcgen

