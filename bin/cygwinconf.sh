#! /bin/sh
#
# This script file is used to autodetect some parameters
# needed for building Crystal Space on various Unix flavours.
#
# Arguments: $1 is user-preferred install dir, argument can be left out
#
# The configuration (a makefile fragment) is piped to stdout
# and errors are piped to stderr.
#

INSTALL_DIR=$1

# Find the directory where script is located
SCRIPT_NAME=`basename $0`
SCRIPT_DIR=`expr $0 : "\(.*\)/${SCRIPT_NAME}"`
[ -z "${SCRIPT_DIR}" ] && SCRIPT_DIR="./"
SCRIPT_DIR=`(cd ${SCRIPT_DIR} > /dev/null; pwd)` # Convert to absolute path.

# find out architecture
. ${SCRIPT_DIR}/arch.sh

# Check for compiler
. ${SCRIPT_DIR}/comptest.sh

# Check for tools
. ${SCRIPT_DIR}/chktools.sh

# Analyse system headers
. ${SCRIPT_DIR}/chkheadr.sh

# test for endianess
. ${SCRIPT_DIR}/endtest.sh

# test for phyton
# Phyton doesn't work yet
#. ${SCRIPT_DIR}/haspythn.sh

# find install dir
[ -z "${INSTALL_DIR}" ] && INSTALL_DIR=/usr/local/crystal
echo "INSTALL_DIR = ${INSTALL_DIR}"

exit 0
