/*
   instpath.cpp

    Created by Matt Reda <mreda@mac.com>
    Copyright (c) 2002 Matt Reda. All rights reserved.
 */

#define CS_SYSDEF_PROVIDE_PATH
#include "cssysdef.h"


extern "C" int NeXTGetInstallPath(char *oInstallPath, size_t iBufferSize, char pathSep);


bool csGetInstallPath(char *oInstallPath, size_t iBufferSize)
{
  return NeXTGetInstallPath(oInstallPath, iBufferSize, PATH_SEPARATOR);
};
