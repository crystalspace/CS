#!/bin/sh
#==============================================================================
# Check for presence of OpenGL and determine its features.
#
# IMPORTS
#    CXX
#	Shell or environment variable used to compile a program.
#    X11_PATH
#	Shell or environment variable specifying root of X11 installation if
#	present.
#    msg_*()
#	Functions for reporting progress to users.
#
# EXPORTS
#    OPENGL_PATH
#	Makefile variable emitted to the standard output stream.  Value is the
#	base path of the OpenGL installation, otherwise the variable is not
#	set.
#    CSGL_EXT_STATIC_ASSERTION
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if OpenGL extensions are _not_ supported, otherwise the variable
#	is not set.
#==============================================================================

precondition '-n "${CXX}"'
precondition '${X11_OK} -eq 0 -o ${X11_OK} -eq 1'

# Find the OpenGL header directory.

msg_checking "for OpenGL"

OPENGL_PATH=''
if [ -d /usr/local/include/GL ]; then
  OPENGL_PATH="/usr/local/include"
elif [ -d /usr/include/GL ]; then
  OPENGL_PATH="/usr/include"
elif [ -d /usr/openwin/include/GL ]; then
  OPENGL_PATH="/usr/openwin/include"
elif [ ${X11_OK} -eq 1 -a -d ${X11_PATH}/include/GL ]; then
  OPENGL_PATH="${X11_PATH}/include"
fi

if [ -n "${OPENGL_PATH}" ]; then
  echo "OPENGL_PATH = ${OPENGL_PATH}"
  msg_result "${OPENGL_PATH}"
else
  msg_result "no"
fi

# Determine if OpenGL extensions are supported.

if [ -n "${OPENGL_PATH}" ]; then
  msg_checking "for OpenGL extensions"

  cat << EOF > gltest.cpp
  #define GLX_GLXEXT_PROTOTYPES
  #include <GL/glx.h>
  int main()
  {
    (void)glXGetProcAddressARB((GLubyte const*)"");
    return 0;
  }
EOF

  ${CXX} -c gltest.cpp -I${OPENGL_PATH} 2>/dev/null
  if [ $? -eq 0 ]; then
    msg_result "yes"
  else
    echo "CSGL_EXT_STATIC_ASSERTION=yes"
    msg_result "no"
  fi

  rm -f gltest.cpp gltest.o gltest.obj gltest.exe gltest
fi
