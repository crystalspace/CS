/*
  Crystal Space Quake Milk Shape ASCII convertor
  Copyright (C) 2002 by Steven Geens <steven.geens@student.kuleuven.ac.be>
  Based upon:
  Crystal Space Quake MDL/MD2 convertor

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __MSMODEL_H__
#define __MSMODEL_H__

#include "cstypes.h"
#include <stdio.h>
#include "csgeom/transfrm.h"
#include "csgeom/quaterni.h"

#define MS_MAX_NAME             32
#define MS_MAX_PATH             256
#define FRAME_DURATION_DEFAULT  1.0

typedef double scalar_t;       // Scalar value,

struct iDocumentNode;


struct Vertex              // Vertex
{
  class csVector3 position;
  scalar_t u;
  scalar_t v;
  int boneIndex;
};

struct Frame
{
  struct Vertex* vertex;
  struct Frame* frame;
  public:
  Frame(struct Vertex* inVertex)
  {
    vertex = inVertex;
    frame = NULL;
  }
  ~Frame()
  {
    if(frame!=NULL)
    {
      delete frame;
    }
  }
};

struct Triangle
{
  long int index1;
  long int index2;
  long int index3;
};

struct TriangleList
{
  struct Triangle* triangle;
  struct TriangleList* triangleList;
  public:
  TriangleList(struct Triangle* inTriangle)
  {
    triangle = inTriangle;
    triangleList = NULL;
  }
  ~TriangleList()
  {
    if(triangleList!=NULL)
    {
      delete triangleList;
    }
  }
};

struct Transform
{
  class csVector3 move;
  class csMatrix3 matrix;
};

struct KeyData
{
  scalar_t time;
  csVector3 data;
};

struct Joint
{
  struct Transform* transform;
  int parent;
  int* children;
  int index;
  char name[MS_MAX_NAME];
  ~Joint()
  {
    delete[] children;
  }
  int nbPositionKeys;
  struct KeyData* positionKeys;
  int nbRotationKeys;
  struct KeyData* rotationKeys;
};

class MsModel
{
private:
  bool bError;
  char* sError;
  
  struct Frame** frames;
  int nbFrames;
  struct TriangleList* triangleList;
  char material[MS_MAX_NAME];
  char materialFile[MS_MAX_NAME];
  struct Joint** joints;
  int nbJoints;
  
  float frameDuration;
  
  void printJoint(struct Joint* joint, csRef<iDocumentNode> parent);
  void transform(csReversibleTransform* trans, int boneIndex,int parent);
  
protected:
  void clearError();
  bool setError(const char* errorstring, FILE* closethis = 0);
  void clearMemory();


public:
  static bool IsFileMsModel(const char* msfile);
  
  MsModel(const char* msfile);
  MsModel(const char* msfile,float i_frameDuration);
  virtual ~MsModel();
  void dumpstats(FILE*);
  const char* getErrorString() const { return sError; }
  bool getError() const { return bError; }

  bool ReadMsFile(const char* msfile);
  bool WriteSPR(const char *spritename);
};

#endif // __M2S_BASE_H__
