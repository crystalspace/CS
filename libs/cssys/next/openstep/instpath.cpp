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
    if (iBufferSize == 0)
        return false;

    bool result = NeXTGetInstallPath(oInstallPath, iBufferSize, PATH_SEPARATOR);
    if (result == 0)
    {
        char *path = getenv ("CRYSTAL");
        if (!path || !*path)
        {
            oInstallPath [0] = '\0';
            return true;
        }

        size_t pl = strlen (path);
        // See if we have to add an ending path separator to the directory
        bool addsep = (path [pl - 1] != PATH_SEPARATOR);
        if (addsep)
            pl++;

        if (pl >= iBufferSize)
            pl = iBufferSize - 1;
        memcpy (oInstallPath, path, pl);
        if (addsep)
            oInstallPath [pl - 1] = PATH_SEPARATOR;
        oInstallPath [pl] = 0;
        return true;
    };
    
    return 1;
};
