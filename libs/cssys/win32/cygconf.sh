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

BIN_DIR=bin

# find out architecture
. ${BIN_DIR}/arch.sh

# Check for compiler
. ${BIN_DIR}/comptest.sh

# Check for tools
. ${BIN_DIR}/chktools.sh

# Analyse system headers
. ${BIN_DIR}/chkheadr.sh

# test for endianess
. ${BIN_DIR}/endtest.sh

# test for phyton
# Phyton doesn't work yet
#. ${BIN_DIR}/haspythn.sh

# find install dir
[ -z "${INSTALL_DIR}" ] && INSTALL_DIR=/usr/local/crystal
echo "INSTALL_DIR = ${INSTALL_DIR}"

exit 0
