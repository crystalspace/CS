#ifndef _3DSOUT_H
#define _3DSOUT_H

#include <stdarg.h>

// includes for lib3ds
#include <lib3ds/camera.h>
#include <lib3ds/file.h>
#include <lib3ds/io.h>
#include <lib3ds/light.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/vector.h>

class Writer
{
public:
    Writer(const char* fname);
    Writer(FILE* f); 
    ~Writer();

    void Indent(int sp=2);
    void UnIndent(int sp=2);

    void WriteL (const char* line, ...);
    void Write (const char* line, ...);
    void WriteV (const char* line, va_list args);
protected:
    bool indented;
    int indentlevel;
    FILE* file;    
};

class CSWriter : public Writer
{
public:
    CSWriter(const char* filename, Lib3dsFile* data3d);
    CSWriter(FILE* f, Lib3dsFile* data3d);
    ~CSWriter();

    bool WriteFile();
    
    void OutpHeaderCS();
    void OutpObjectsCS(bool lighting);
protected:
    Lib3dsFile* p3dsFile;
};

#endif

