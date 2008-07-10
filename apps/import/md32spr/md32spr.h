/*
    Copyright (C) 2002 by Manjunath Sripadarao

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

#ifndef __MD32SPR_H
#define __MD32SPR_H

#include <stdarg.h>

#include "csutil/ref.h"
#include "csutil/csstring.h"
#include "csutil/stringarray.h"
#include "csutil/array.h"
#include "csgeom/vector3.h"
#include "csgeom/vector2.h"
#include "csgeom/matrix3.h"
#include "iutil/document.h"

struct iCommandLineParser;
struct iObjectRegistry;
struct iFile;
struct iDocumentNode;
struct iVFS;
struct iStringArray;
class csParser;

typedef enum ActionEnum {
  BOTH_DEATH1,
  BOTH_DEAD1,
  BOTH_DEATH2,
  BOTH_DEAD2,
  BOTH_DEATH3,
  BOTH_DEAD3,
  TORSO_GESTURE,
  TORSO_ATTACK,	        //	(MUST NOT CHANGE -- hand animation is synced to this)
  TORSO_ATTACK2,	//      (MUST NOT CHANGE -- hand animation is synced to this)
  TORSO_DROP,	        //	(MUST NOT CHANGE -- hand animation is synced to this)
  TORSO_RAISE,	        //	(MUST NOT CHANGE -- hand animation is synced to this)
  TORSO_STAND,
  TORSO_STAND2,
  LEGS_WALKCR,
  LEGS_WALK,
  LEGS_RUN,
  LEGS_BACK,
  LEGS_SWIM,
  LEGS_JUMP,
  LEGS_LAND,
  LEGS_JUMPB,
  LEGS_LANDB,
  LEGS_IDLE,
  LEGS_IDLECR,
  LEGS_TURN,
  NUM_ELEMENTS
} Actions;

typedef struct MD3Header {
  char ID[4];         //id of file, always "IDP3" 
  int  version;       //Version number, always 15 for MD3.
  char fileName[68];  //Sometimes Blank...68 chars 
  int  numFrames;     //Number of BoneFrames
  int  numTags;       //Number of 'tags' per BoneFrame
  int  numMeshes;     //Number of meshes or skins 
  int  numMaxSkins;   //Maximum number of unique skins used in md3 file 
  int  headerLength;  //Always equal to the length of this header 
  int  tagStart;      //Starting position of tag-structures 
  int  tagEnd;        //Ending position of tag-structures/starting position of mesh-structures 
  int  fileSize;      //Size of file 
} md3Header;

typedef struct MD3Bone {
  float Mins[3];
  float Maxs[3];
  float Position[3];
  float Scale;
  char Creator[16];
} md3Bone;

typedef struct MD3Tag {
  char name[64];
  float position[3];
  float rotation[3][3];
} md3Tag;

typedef struct MD3MeshHeader {
  char ID[4];          //id, must be IDP3 
  char name[68];       //name of mesh, 65 chars, 32 bit aligned == 68 chars
  int  numMeshFrames;  //number of meshframes in mesh 
  int  numSkins;       //number of skins in mesh 
  int  numVertices;     //number of vertices per MeshFrame 
  int  numTriangles;   //number of Triangles 
  int  triangleStart;  //starting position of Triangle data, relative to start of Mesh_Header 
  int  headerSize;     //size of header 
  int  texCoordStart;    //starting position of texvector data, relative to start of Mesh_Header 
  int  vertexStart;    //starting position of vertex data,relative to start of Mesh_Header 
  int  meshSize;       //size of mesh 
} md3MeshHeader;  

typedef struct MD3Skin {
  char name[68];
} md3Skin;

typedef struct MD3Triangle {
  int triangle[3];
} md3Triangle;

typedef struct MD3TexCoord {
  float texCoord[2];
} md3TexCoord;

typedef struct MD3Vertices {
  short vec[3];
  unsigned char envTex[2];
} md3Vertices;

typedef struct MD3Mesh {
  md3MeshHeader *meshHeader;
  md3Skin *skins;
  md3Triangle *triangles;
  md3TexCoord *texCoords;
  md3Vertices *vertices;
} md3Mesh;

typedef struct AnimInfo {
  char actionName[15];
  Actions action;
  int startFrame;
  int numFrames;
  int loopFrames;
  int fps;
} AnimInfo;

typedef class MD3Model {
 public:
  MD3Model(iObjectRegistry* object_reg);
  ~MD3Model();
  void Init(csRef<iVFS> *vfs);
  void ReportError (const char* description, ...);
  bool LoadMD3(void);
  bool LoadSkin(char *md3File);
  const char *GetFileName();
  bool ReadHeader(char **headerBuf);
  
 public:
  iObjectRegistry* object_reg;
  csRef<iVFS> *vfs;
  csString fileName;
  md3Header *header;
  md3Bone *bones;
  md3Tag *tags;
  md3Mesh *meshes;
  AnimInfo *animInfo;
  int numActions;
  static const int NUM_ACTIONS;
} md3Model;


class MD32spr {
 public:
  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csRef <iVFS> out;
  csRef<iCommandLineParser> cmdline;

  void ReportError (const char* description, ...);

  csRef<iDocumentNode> CreateValueNode (csRef<iDocumentNode>& parent,
        const char* name, const char* value);
  csRef<iDocumentNode> CreateValueNodeAsInt (csRef<iDocumentNode>& parent,
        const char* name, int value);
  csRef<iDocumentNode> CreateValueNodeAsFloat (csRef<iDocumentNode>& parent,
        const char* name, float value);


  bool ReadVfsDir ();
  bool LoadAnimation(char *md3File);
  void Main(void);
  void Write();
  void WriteGeneric(md3Model *model, csRef<iDocumentNode>&);
  void WriteTextures(const char *inPath, const char *outPath);
 public:
  MD32spr(iObjectRegistry* object_reg);
  ~MD32spr();

 private:
  FILE *fp;
  csRef<iStringArray> fileNames;
  csRef <iStringArray> weaponFiles;

  csString weaponDir;
  csString outZipName;
  csString mountPath;
  bool player;
  float scaleFactor;

  md3Model *headModel;
  md3Model *upperModel;
  md3Model *lowerModel;
  csArray<md3Model*> generic;

  static const int NUM_ACTIONS;
  int md3Files;

 private:
  void WriteXMLTags(md3Model *model, const char *fileName);
  void WriteXMLTextures(md3Model *model, csRef<iDocumentNode>&);
  void WriteXMLMaterials(md3Model *model, csRef<iDocumentNode>&);

};  




#endif 
