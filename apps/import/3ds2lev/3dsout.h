#ifndef _3DSOUT_H
#define _3DSOUT_H

// includes for lib3ds
#include "lib3ds/camera.h"
#include "lib3ds/file.h"
#include "lib3ds/io.h"
#include "lib3ds/light.h"
#include "lib3ds/material.h"
#include "lib3ds/matrix.h"
#include "lib3ds/mesh.h"
#include "lib3ds/node.h"
#include "lib3ds/vector.h"

// Added by LucaPancallo 2000.09.28
void OutpHeadCS(FILE *o, Lib3dsFile *p3dsFile);
void OutpObjectsCS(FILE * o, Lib3dsFile *p3dsFile, bool lighting);

#endif
