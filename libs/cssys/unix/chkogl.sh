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
#    GL.AVAILABLE
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if OpenGL SDK is present and usable, otherwise the variable is
#	not set.
#    GL.CFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	compiler flags required for OpenGL.
#    GL.LFLAGS
#	Makefile variable emitted to the standard output stream.  Value is the
#	linker flags required for OpenGL.
#    CSGL_EXT_STATIC_ASSERTION
#	Makefile variable emitted to the standard output stream.  Value is
#	"yes" if OpenGL extensions are _not_ supported, otherwise the variable
#	is not set.
#==============================================================================

precondition '-n "${CXX}"'
precondition '${X11_OK} -eq 0 -o ${X11_OK} -eq 1'

# Find the OpenGL header directory.

msg_checking "for OpenGL"

GL_OK=1
if [ -d /usr/local/include/GL ]; then
  OPENGL_HPATH="/usr/local/include"
  OPENGL_LPATH="/usr/local/lib"
elif [ -d /usr/include/GL ]; then
  OPENGL_HPATH="/usr/include"
  OPENGL_LPATH="/usr/lib"
elif [ -d /usr/openwin/include/GL ]; then
  OPENGL_HPATH="/usr/openwin/include"
  OPENGL_LPATH="/usr/openwin/lib"
elif [ ${X11_OK} -eq 1 -a -d ${X11_PATH}/include/GL ]; then
  OPENGL_HPATH="${X11_PATH}/include"
  OPENGL_LPATH="${X11_PATH}/lib"
else
  OPENGL_HPATH=''
  OPENGL_LPATH=''
  GL_OK=0
fi

if [ ${GL_OK} -eq 1 ]; then
  cat << EOGL
GL.AVAILABLE = yes
GL.CFLAGS = \$(X_CFLAGS) -I${OPENGL_HPATH}
GL.LFLAGS = \$(X_PRE_LIBS) \$(X_LIBS) -lXext -lX11 \$(X_EXTRA_LIBS)
GL.LFLAGS += -L${OPENGL_LPATH}
ifneq (\$(USE_MESA),1)
  GL.LFLAGS += -lGL
else
  GL.LFLAGS += -lMesaGL
  ifdef MESA_PATH
    GL.CFLAGS += -I\$(MESA_PATH)/include
    GL.LFLAGS += -I\$(MESA_PATH)/lib
  endif
endif
EOGL
  msg_result "${OPENGL_HPATH}"
else
  msg_result "no"
fi

# Determine if OpenGL extensions are supported.

if [ ${GL_OK} -eq 1 ]; then
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

  ${CXX} -c gltest.cpp -I${OPENGL_HPATH} 2>/dev/null
  if [ $? -eq 0 ]; then
    msg_result "yes"
  else
    echo "CSGL_EXT_STATIC_ASSERTION=yes"
    msg_result "no"
  fi

  rm -f gltest.cpp gltest.o gltest.obj gltest.exe gltest
fi
