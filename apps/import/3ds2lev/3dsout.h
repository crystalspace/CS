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

class csDVector3;
class csDPlane;

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
  
  void WriteHeader();
  void WriteFooter();
  void WriteObjects(bool lighting);
  void WriteVertices(Lib3dsMesh* mesh);
  void WriteFaces(Lib3dsMesh* mesh, bool lighting, unsigned int numMesh);

  void SetScale(float x, float y, float z);
  void SetTranslate(float x, float y, float z);
protected:
  inline bool CombineTriangle (Lib3dsMesh* mesh, csDPlane*& plane, int* poly,
      int& plen, int trinum);
    
  Lib3dsFile* p3dsFile;
  int* newpointmap;
  csDVector3* vectors;
  csDPlane* planes;
  bool* used;
  float xscale, yscale, zscale;
  float xrelocate, yrelocate, zrelocate;
};

#endif

