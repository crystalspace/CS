#!/bin/sh
# Test for OpemGL headers and features

if [ -d /usr/local/include/GL ]; then
  INC_OPENGL_PATH="/usr/local/include"
elif [ -d /usr/include/GL ]; then
  INC_OPENGL_PATH="/usr/include"
elif [ -d /usr/openwin/include/GL ]; then
  INC_OPENGL_PATH="/usr/openwin/include"
else
  INC_OPENGL_PATH="/usr/include"
fi

cat << EOF > gltest.cpp
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
int main()
{
  (void*)glXGetProcAddressARB((const GLubyte *)"YoMama");
  return 0;
}
EOF
${CXX} -c gltest.cpp -I${INC_OPENGL_PATH} 2>/dev/null || echo "CSGL_EXT_STATIC_ASSERTION=yes"

rm -f gltest.*
