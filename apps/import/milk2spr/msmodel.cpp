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

#include "cssysdef.h"
#include "msmodel.h"
#include "csutil/scfstr.h"
#include "csutil/xmltiny.h"


bool MsModel::IsFileMsModel(const char* msfile)
{
  FILE *file = fopen (msfile, "rt");
  if(!file)
    return false;
  char szLine[256];

  if (fgets (szLine, 256, file) != NULL)
  {
     if (strcmp(szLine,"// MilkShape 3D ASCII\n") == 0)
     {
       fclose (file);
       return true;
     }
  }
  fclose (file);
  return false;
}


MsModel::MsModel(const char* msfile) : sError(NULL)
{
  frames = NULL;
  nbFrames = 0;
  triangleList = NULL;
  strcpy(material,"spark");
  strcpy(materialFile,"spark.gif");
  joints = NULL;
  nbJoints = 0;
  
  frameDuration = FRAME_DURATION_DEFAULT;
  
  clearError();
  ReadMsFile(msfile);
}

MsModel::MsModel(const char* msfile,float i_frameDuration) : sError(NULL)
{
  frames = NULL;
  nbFrames = 0;
  triangleList = NULL;
  strcpy(material,"spark");
  strcpy(materialFile,"spark.gif");
  joints = NULL;
  nbJoints = 0;
  
  frameDuration = i_frameDuration;
  
  clearError();
  ReadMsFile(msfile);
}

MsModel::~MsModel()
{
  if (sError != NULL)
    free(sError);
  clearMemory();
  free(material);
  free(materialFile);
}

void MsModel::clearMemory()
{
  if(frames!=NULL)
  {
    int i;
    for(i=0;i<nbFrames;i++)
    {
      if(frames[i] != NULL)
      {
        delete frames[i];
      }
    }
  }
  if(triangleList!=NULL)
  {
    delete triangleList;
  }
  frames = NULL;
  triangleList = NULL;
  nbFrames = 0;
  
  strcpy(material,"spark");
  strcpy(materialFile,"spark.gif");
    
  if(joints !=NULL)
  {
    int i;
    for(i=0;i<nbJoints;i++)
    {
      if(joints[i] != NULL)
      {
        delete joints[i];
      }
    }
  }
  joints = NULL;
  nbJoints = 0;
}

bool MsModel::setError(const char* errorstring, FILE* closethis)
{
  if (closethis != 0)
    fclose(closethis);

  if (sError != NULL)
    free(sError);

  if (errorstring == NULL)
    sError = strdup("Unknown error");
  else
    sError = strdup(errorstring);

  bError = true;
  return false;
}

void MsModel::clearError()
{
  setError("No error");
  bError = false;
}

bool MsModel::ReadMsFile(const char* msfile)
{
  FILE *file = fopen (msfile, "rt");
  if (!file)
    return setError("Couldn't open file.",file);
  
  clearMemory();
  
  char szLine[256];
  char szName[MS_MAX_NAME];
    
  // Scanning for the ID tag // MilkShape 3D ASCII
  if (!fgets (szLine, 256, file))
  {
    return setError("Unexpected ending of file.",file);
  }
  if (strcmp(szLine,"// MilkShape 3D ASCII\n") != 0)
  {
    return setError("Couldn't find: // MilkShape 3D ASCII",file);
  }
  
  //This is followed by a space
  if (!fgets (szLine, 256, file))
  {
    return setError("Unexpected ending of file.",file);
  }
  if (strlen(szLine)-1 != 0) 
  {
    return setError("Couldn't find space after ID tag.",file);
  }
  
  //The number of Frames
  //I wasn't able to find or generate a case with more than 1 frame.
  //Eventhough the number of frames is higher.
  if (!fgets (szLine, 256, file))
  {
    return setError("Unexpected ending of file.",file);
  }
  if (sscanf (szLine, "Frames: %d", &nbFrames) != 1)
  {
    return setError("Couldn't find the number of frames.",file);
  }

  int frameIndex;  
  frames = new struct Frame*[nbFrames];
  for(frameIndex = 0;frameIndex<nbFrames;frameIndex++)
  {
    frames[frameIndex] = NULL;
  }
  struct Frame** currentFrame = frames;
  struct TriangleList** currentTriangle = &triangleList;
  
  for(frameIndex = 0;frameIndex<nbFrames;frameIndex++)
  {
    //The number of the frame
    //I wasn't able to find or generate a case with more than 1 frame.
    //Eventhough the number of frames is higher.
    //So we just break the loop if we don't find another frame.
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    if (sscanf (szLine, "Frame: %s", szName) != 1)
    {
      break;
    }
    //This is followed by a space
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    if (strlen(szLine)-1 != 0)
    {
      return setError("Couldn't find space after number of the frame.",file);
    }
    
    //Setting the current frame correctly.
    //We are only going to use the first trianglelist,
    //Because CS only supports one trianglelist per mesh.
    currentFrame = frames + frameIndex;
    
    //Finding out how many meshes the MS model has.
    //Ms works with several meshes per model.
    //We can only use one mesh in CS,
    //or else we can't define a skeleton.
    int nbMeshes = 0;
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    if (sscanf (szLine, "Meshes: %d", &nbMeshes) != 1)
    {
      return setError("Couldn't find the number of meshes.",file);
    }
    // A variable for keeping a consistent way of keeping the
    // indices of the triangles pointed to the right vertex.
    long vertexCount = 0;
    for(int meshIndex = 0;meshIndex<nbMeshes;meshIndex++)
    {
      // We scan for the name, flags and material index,
      // of a mesh. 
      // I don't know what the flags mean.
      // There can only be one material per 3d sprite in CS
      // So we'll only use the first material.
      int meshFlags = 0;
      int meshMaterialIndex = 0;
      if (!fgets (szLine, 256, file))
      {
        return setError("Unexpected ending of file.",file);
      }
      if (sscanf (szLine, "\"%[^\"]\" %d %d",szName, &meshFlags, &meshMaterialIndex) != 3)
      {
        return setError("Couldn't scan for mesh name, flags and materialIndex.",file);
      }
      
      //Scanning for the number of vertices in a mesh.
      int nbVertices = 0;
      if (!fgets (szLine, 256, file))
      {
        return setError("Unexpected ending of file.",file);
      }      
      if (sscanf (szLine, "%d", &nbVertices) != 1)
      {
        return setError("Couldn't scan for number of indices.",file);
      }
      struct TempVertex tempVertices[nbVertices];
      for (int vertexIndex = 0; vertexIndex < nbVertices; vertexIndex++)
      {
        //Here we scan for vertexes,
        //We use the x, y, z, u and v coordinates in Vertex.
        //We alsow store the bone index for future reference.
        //The bone index is necessary for bulding the skeleton.
        //I don't know what the flags mean.
        if (!fgets (szLine, 256, file))
        {
          return setError("Unexpected ending of file.",file);
        }
        int vertexFlags = 0;
        if (sscanf (szLine, "%d %f %f %f %lf %lf %d",
              &vertexFlags ,
              &(tempVertices[vertexIndex].position.x), 
              &(tempVertices[vertexIndex].position.y), 
              &(tempVertices[vertexIndex].position.z),
              &(tempVertices[vertexIndex].u), 
              &(tempVertices[vertexIndex].v), 
              &(tempVertices[vertexIndex].boneIndex)
              ) != 7)
        {
          return setError("Couldn't scan vertex correctly.",file);
        }
      }
      
      // We don't use the normals.
      // Note: If you want to use the normals for the triangles,
      // don't forget that the indeces are alsow per mesh.
      // So you'll need a normalCount variable.
      // Scanning for the number of normals
      int nbNormals = 0;
      if (!fgets (szLine, 256, file))
      {
        return setError("Unexpected ending of file.",file);
      }
      if (sscanf (szLine, "%d", &nbNormals) != 1)
      {
        return setError("Couldn't scan for number of normals.",file);
      }
      struct TempNormal tempNormals[nbNormals];
      for (int normalIndex = 0; normalIndex < nbNormals; normalIndex++)
      {
        // We don't use normals in CS.
        if (!fgets (szLine, 256, file))
        {
          return setError("Unexpected ending of file.",file);
        }
        if (sscanf (szLine, "%f %f %f", 
                            &(tempNormals[normalIndex].normal.x), 
                            &(tempNormals[normalIndex].normal.y), 
                            &(tempNormals[normalIndex].normal.z)) != 3)
        {
          return setError("Couldn't scan for normals.",file);
        }
      }
      
      // Scanning for the number of triangles
      int nbTriangles = 0;
      if (!fgets (szLine, 256, file))
      {
        return setError("Unexpected ending of file.",file);
      }
      if (sscanf (szLine, "%d", &nbTriangles) != 1)
      {
        return setError("Couldn't scan for number of triangles.",file);
      }
      long newVertices = vertexCount;
      for (int triangleIndex = 0; triangleIndex < nbTriangles; triangleIndex++)
      {
        int triangleFlags  = 0;
        long index1        = 0;
        long index2        = 0;
        long index3        = 0;
        long normal1       = 0;
        long normal2       = 0;
        long normal3       = 0;
        int smoothingGroup = 0;
        if (!fgets (szLine, 256, file))
        {
          return setError("Unexpected ending of file.",file);
        }
        if (sscanf (szLine, "%d %ld %ld %ld %ld %ld %ld %d",
           &triangleFlags  ,
           &index1 , &index2 , &index3,
           &normal1, &normal2, &normal3, 
           &smoothingGroup 
           ) != 8)
        {
          return setError("Couldn't scan for triangle.",file);
        }
        // We only use the triangles of the first frame.
        // Because CS only allows one Triangle definition per sprite.
        if(frameIndex==0)
        {
          //Make first new point
          struct Vertex* tempV = new struct Vertex();
          tempV->position.x = tempVertices[index1].position.x;
          tempV->position.y = tempVertices[index1].position.y;
          tempV->position.z = tempVertices[index1].position.z;
          tempV->normal.x   = tempNormals[normal1].normal.x;
          tempV->normal.y   = tempNormals[normal1].normal.y;
          tempV->normal.z   = tempNormals[normal1].normal.z;
          tempV->u          = tempVertices[index1].u;
          tempV->v          = tempVertices[index1].v;
          tempV->boneIndex  = tempVertices[index1].boneIndex;
          *currentFrame = new struct Frame(tempV);
          currentFrame = &((*currentFrame)->frame);
          
          //Make second new point
          tempV = new struct Vertex();
          tempV->position.x = tempVertices[index2].position.x;
          tempV->position.y = tempVertices[index2].position.y;
          tempV->position.z = tempVertices[index2].position.z;
          tempV->normal.x   = tempNormals[normal2].normal.x;
          tempV->normal.y   = tempNormals[normal2].normal.y;
          tempV->normal.z   = tempNormals[normal2].normal.z;
          tempV->u          = tempVertices[index2].u;
          tempV->v          = tempVertices[index2].v;
          tempV->boneIndex  = tempVertices[index2].boneIndex;
          *currentFrame = new struct Frame(tempV);
          currentFrame = &((*currentFrame)->frame);
          
          //Make third new point
          tempV = new struct Vertex();
          tempV->position.x = tempVertices[index3].position.x;
          tempV->position.y = tempVertices[index3].position.y;
          tempV->position.z = tempVertices[index3].position.z;
          tempV->normal.x   = tempNormals[normal3].normal.x;
          tempV->normal.y   = tempNormals[normal3].normal.y;
          tempV->normal.z   = tempNormals[normal3].normal.z;
          tempV->u          = tempVertices[index3].u;
          tempV->v          = tempVertices[index3].v;
          tempV->boneIndex  = tempVertices[index3].boneIndex;
          *currentFrame = new struct Frame(tempV);
          currentFrame = &((*currentFrame)->frame);
          struct Triangle* temp = new Triangle();
          
          // We must be carefull here because the indices of the triangles 
          // are per mesh so we use the variable vertexCount.
          temp->index1 = newVertices;
          temp->index2 = newVertices + 1;
          temp->index3 = newVertices + 2;
          *currentTriangle = new struct TriangleList(temp);
          currentTriangle = &((*currentTriangle)->triangleList);
          newVertices += 3;
        }
      }
      
      // Updating VertexCount
      vertexCount += 3*nbTriangles;
    }
  }
  
  //If we exited the loop irregulary we allready scanned the next line.
  if(frameIndex==nbFrames)
  {
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
  }
  //There is a space after the description of the frames.
  if (strlen(szLine)-1!=0) 
  {
    return setError("Couldn't find space after the description of the frames.",file);
  }
  
  // We can only use one material per mesh in CS.
  // So we will only use the first one.
  int nbMaterials = 0;
  if (!fgets (szLine, 256, file))
  {
    return setError("Unexpected ending of file.",file);
  }
  if (sscanf (szLine, "Materials: %d", &nbMaterials) != 1)
  {
    return setError("Couldn't scan the number of materials.",file);
  }
  for (int materialIndex = 0; materialIndex < nbMaterials; materialIndex++)
  {
    // The name of the material.
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    if (sscanf (szLine, "\"%[^\"]\"", szName) != 1)
    {
      return setError("Couldn't scan the line for the name of the material.",file);
    }
    //We only use the first material.
    if(materialIndex==0)
    {
      strcpy(material,szName);
    }
      
    // Various features not supported by CS.
    // ambient
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    scalar_t ambient1, ambient2, ambient3, ambient4;
    if (sscanf (szLine, "%lf %lf %lf %lf", &ambient1, &ambient2, &ambient3, &ambient4) != 4)
    {
      return setError("Couldn't scan for ambient.",file);
    }
    
    // diffuse
    if (!fgets (szLine, 256, file))
    {
     return setError("Unexpected ending of file.",file);
    }
    scalar_t diffuse1, diffuse2, diffuse3, diffuse4;
    if (sscanf (szLine, "%lf %lf %lf %lf", &diffuse1, &diffuse2, &diffuse3, &diffuse4) != 4)
    {
     return setError("Couldn't scan for diffuse" ,file);
    }
    
    // specular
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    scalar_t specular1, specular2, specular3, specular4;
    if (sscanf (szLine, "%lf %lf %lf %lf", &specular1, &specular2, &specular3, &specular4) != 4)
    {
      return setError("Couldn't scan for specular.",file);
    }
    
    // emissive
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    scalar_t emissive1, emissive2, emissive3, emissive4;
    if (sscanf (szLine, "%lf %lf %lf %lf", &emissive1, &emissive2, &emissive3, &emissive4) != 4)
    {
      return setError("Couldn't scan for emissive.",file);
    }
    
    // shininess
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    scalar_t fShininess;
    if (sscanf (szLine, "%lf", &fShininess) != 1)
    {
      return setError("Couldn't scan for shininess",file);
    }
    
    // transparency
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    scalar_t fTransparency;
    if (sscanf (szLine, "%lf", &fTransparency) != 1)
    {
      return setError("Couldn't Scan for transparency." ,file);
    }
    
    // diffuse texture
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    if (sscanf (szLine, "\"%[^\"]\"", szName) != 1)
    {
      if (strcmp(szLine,"\"\"\n") == 0)
      {
        strcpy (szName, "spark.gif");
      }
      else
      {
        return setError("Couldn't extract diffuse texture.",file);
      }
    }
    //We only use the first material.
    if(materialIndex==0)
    {
      strcpy(materialFile,szName);
    }
    
    // alpha texture
    if (!fgets (szLine, 256, file))
    {
       return setError("Unexpected ending of file.",file);
    }
    if (sscanf (szLine, "\"%[^\"]\"", szName) != 1)
    {
      if (strcmp(szLine,"\"\"\n")==0) 
      {
        strcpy (szName, "");
      }
      else
      {
        return setError("Couldn't extract alpha texture.",file);
      }
    }
  }
  
  // A space after the materials.
  if (!fgets (szLine, 256, file))
  {
    return setError("Unexpected ending of file.",file);
  }
  if (strlen(szLine)-1!=0) 
  {
    return setError("Couldn't scan for the space after the materials." ,file);
  }
  
  
  // A line indicating the number of joints.
  if (!fgets (szLine, 256, file))
  {
    return setError("Unexpected ending of file.",file);
  }
  if (sscanf (szLine, "Bones: %d", &nbJoints) != 1)
  {
    return setError("Couldn't scan for the number of joints.",file);
  }
  joints = new struct Joint*[nbJoints];
  int i;
  for(i = 0;i<nbJoints;i++)
  {
    joints[i] = NULL;
  }
  char** parents = new char*[nbJoints];
  for(i = 0;i<nbJoints;i++)
  {
    parents[i]= new char[MS_MAX_NAME];
  }
  char** names = new char*[nbJoints];
  for(i = 0;i<nbJoints;i++)
  {
    names[i]= new char[MS_MAX_NAME];
  }
  
  for (int jointIndex = 0; jointIndex < nbJoints; jointIndex++)
  {
    //Scanning for the name of a joint.
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    if (sscanf (szLine, "\"%[^\"]\"", szName) != 1)
    {
      return setError("Couldn't scan for name of a joint.",file);
    }
    strcpy(names[jointIndex],szName);
    
    // Scanning for the name of the parent of a joint.
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    if(sscanf (szLine, "\"%[^\"]\"", szName)!=1)
    {
      if (strcmp("\"\"\n",szLine) == 0)
      {
        strcpy (szName, "no parent" );
      }
      else
      {
        return setError("Couldn't extract the parent of a joint.",file);
      }
    }
    strcpy(parents[jointIndex],szName);
    
    
    //Scanning for flags, position, rotation
    csVector3 Position, Rotation;
    int jointFlags = 0;
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    if (sscanf (szLine, "%d %f %f %f %f %f %f",
       &jointFlags,
       &(Position.x), &(Position.y), &(Position.z),
       &(Rotation.x), &(Rotation.y), &(Rotation.z)) != 7)
    {
      return setError("Couldn't scan for flags, position and rotation of a joint.",file);
    }
    struct Transform* transform = new Transform();
    transform->matrix = csZRotMatrix3(Rotation.z) *
                        csYRotMatrix3(-Rotation.y) * //Why oh why is this necessary, I have no clue. It must have to do with the differences in the axis systems of CS and MS.
                        csXRotMatrix3(Rotation.x);
    transform->move = Position;
    joints[jointIndex] = new Joint();
    joints[jointIndex]->transform = transform;
    joints[jointIndex]->index     = jointIndex;
    strcpy(joints[jointIndex]->name,names[jointIndex]);
    
    // Scanning for position key count.
    joints[jointIndex]->nbPositionKeys = 0;
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }  
    if (sscanf (szLine, "%d", &(joints[jointIndex]->nbPositionKeys)) != 1)
    {
      return setError("Couldn't scan for the number of position keys.",file);
    }
    joints[jointIndex]->positionKeys = new struct KeyData[joints[jointIndex]->nbPositionKeys];
    for (int positionKeyIndex = 0; positionKeyIndex < joints[jointIndex]->nbPositionKeys; positionKeyIndex++)
    {
      //Scanning for the positionKey time and position.
      scalar_t time = 0;
      scalar_t x    = 0;
      scalar_t y    = 0;
      scalar_t z    = 0;
      if (!fgets (szLine, 256, file))
      {
        return setError("Unexpected ending of file.",file);
      }
      if (sscanf (szLine, "%lf %lf %lf %lf", &time, &x , &y , &z ) != 4)
      {
        return setError("Couldn't scan for position key data.",file);
      }
      joints[jointIndex]->positionKeys[positionKeyIndex].time = time;
      joints[jointIndex]->positionKeys[positionKeyIndex].data = csVector3(x,y,z);
    }
    
    // Scanning for rotation key count
    joints[jointIndex]->nbRotationKeys = 0;
    if (!fgets (szLine, 256, file))
    {
      return setError("Unexpected ending of file.",file);
    }
    if (sscanf (szLine, "%d", &(joints[jointIndex]->nbRotationKeys)) != 1)
    {
      return setError("Couldn't scan for number rotation keys.",file);
    }
    joints[jointIndex]->rotationKeys = new struct KeyData[joints[jointIndex]->nbRotationKeys];
    for (int rotationKeyIndex = 0; rotationKeyIndex < joints[jointIndex]->nbRotationKeys; rotationKeyIndex++)
    {
      //Scanning for the positionKey time and rotation.
      scalar_t time = 0;
      scalar_t x    = 0;
      scalar_t y    = 0;
      scalar_t z    = 0;
      if (!fgets (szLine, 256, file))
      {
        return setError("Unexpected ending of file.",file);
      }
      if (sscanf (szLine, "%lf %lf %lf %lf", &time, &x , &y , &z ) != 4)
      {
        return setError("Couldn't scan for rotation key data.",file);
      }
      joints[jointIndex]->rotationKeys[rotationKeyIndex].time = time;
      joints[jointIndex]->rotationKeys[rotationKeyIndex].data = csVector3(x,y,z);
    }
  }
  
  //Initializing all children indices to -1.
  for(i = 0;i<nbJoints;i++)
  {
    joints[i]->children = new int[nbJoints];
    for(int j = 0;j<nbJoints;j++)
    {
      joints[i]->children[j]=-1;
    }
  }
  
  //Filling in the children
  for(i = 0;i<nbJoints;i++)
  {
    //find out if it's a root joint.
    if(strcmp(parents[i], "no parent" ) == 0)
    {
      joints[i]->parent = -1;
    }
    else
    {
      //Go through the names list untill you meet the parent.
      int j;
      for(j=0;strncmp(parents[i],names[j],strlen(parents[i]))!=0;j++);
      //Go through children list untill you meet an unused one(-1).
      int k;
      for(k=0;joints[j]->children[k]!=-1;k++);
      joints[j]->children[k]=i;
      joints[i]->parent = j;
    }
  }
  
  
  
  fclose (file);
  
  return true;
}

void MsModel::dumpstats(FILE* s)
{
  fprintf(s, "\nMS ASCII Model (TXT) Statistics:\n");
  fprintf(s, "Number of Joints: %d",nbJoints);
}

bool MsModel::WriteSPR(const char* spritename)
{
  FILE *f;
  char *spritefilename;

  if (spritename == NULL || strlen(spritename) == 0)
  {
    fprintf(stderr, "Unable to save: NULL sprite name\n");
    return false;
  }

  spritefilename = new char [strlen(spritename) + 5];
  strcpy(spritefilename, spritename);
  strcat(spritefilename, ".lib");

  if ((f = fopen(spritefilename, "w")) == NULL)
  {
    fprintf(stderr, "Cannot open sprite file %s for writing\n", spritename);
    return false;
  }
  
  //Make the inverse transforms.
  csReversibleTransform* inv = new csReversibleTransform[nbJoints];
  csReversibleTransform* orig = new csReversibleTransform[nbJoints];
  if(nbJoints>0)
  {
    int i;
    for(i = 0;i<nbJoints;i++)
    {
      csMatrix3 m = joints[i]->transform->matrix;
      csVector3 v = joints[i]->transform->move;
      orig[i] = csReversibleTransform (m, -m.GetInverse () * v);
      inv[i] = orig[i];
    }
    for(i = 0;i<nbJoints;i++)
    {
      //find out if it's a root joint.
      if(joints[i]->parent==-1)
      {
        transform(inv,i,-1);
      }
    }
    for(i = 0;i<nbJoints;i++)
    {
      inv[i] = inv[i].GetInverse();
    }
  }
  csRef<iDocumentSystem> xml (csPtr<iDocumentSystem> (
    	new csTinyDocumentSystem ()));
  csRef<iDocument> doc = xml->CreateDocument ();
  csRef<iDocumentNode> root = doc->CreateRoot ();
  csRef<iDocumentNode> library = root->CreateNodeBefore (
    	CS_NODE_ELEMENT, NULL);
  library->SetValue ("library");
  //fprintf(f, "LIBRARY \n(\n");
  // begin hard work now
  csRef<iDocumentNode> textures;
  textures = library->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  textures->SetValue ("textures");
  csRef<iDocumentNode> texture;
  texture = textures->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  texture->SetValue ("texture");
  texture->SetAttribute("name",material);
  csRef<iDocumentNode> file;
  file = texture->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  file->SetValue ("file");
  csRef<iDocumentNode> text;
  text = file->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValue (materialFile);
  //fprintf(f, "TEXTURES (\n");
  //fprintf(f, "  TEXTURE '%s' ( FILE(/lib/std/%s))\n",material,materialFile);
  //fprintf(f, ")\n");
  
  csRef<iDocumentNode> materials;
  materials = library->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  materials->SetValue ("materials");
  csRef<iDocumentNode> materialnode;
  materialnode = materials->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  materialnode->SetValue ("material");
  materialnode->SetAttribute("name",material);
  texture = materialnode->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  texture->SetValue ("texture");
  text = texture->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValue (material);
  //fprintf(f, "MATERIALS (\n");
  //fprintf(f, "  MATERIAL '%s' ( TEXTURE('%s'))\n",material,material);
  //fprintf(f, ")\n");
  
  csRef<iDocumentNode> meshfact;
  meshfact = library->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  meshfact->SetValue ("meshfact");
  meshfact->SetAttribute("name",spritename);
  csRef<iDocumentNode> plugin;
  plugin = meshfact->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  plugin->SetValue ("plugin");
  text = plugin->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValue ("crystalspace.mesh.loader.factory.sprite.3d");
  csRef<iDocumentNode> params;
  params = meshfact->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  params->SetValue ("params");
  materialnode = params->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  materialnode->SetValue ("material");
  text = materialnode->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValue (material);
  //fprintf(f, "MESHFACT '%s' \n(\n", spritename);
  //fprintf(f, "  PLUGIN ('crystalspace.mesh.loader.factory.sprite.3d')\n");
  //fprintf(f, "  PARAMS \n  (\n");
  //fprintf(f, "    MATERIAL ('%s')\n", material);
  
  if(frames!=NULL)
  {
    int i;
    for(i = 0; i < nbFrames;i++)
    {
      if(frames[i]!=NULL)
      {
        csRef<iDocumentNode> frame;
        frame = params->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
        frame->SetValue ("frame");
        frame->SetAttributeAsInt("name",i);
        //fprintf(f, "    FRAME '%d' \n    (\n", i);
        struct Frame* currentFrame = frames[i];
        while(currentFrame!=NULL)
        {
          if(nbJoints>0)
          {
            currentFrame->vertex->position = inv[currentFrame->vertex->boneIndex]*currentFrame->vertex->position;
            currentFrame->vertex->normal   = inv[currentFrame->vertex->boneIndex].GetT2O()*currentFrame->vertex->normal;
          }
          csRef<iDocumentNode> v;
          v = frame->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
          v->SetValue ("v");
          v->SetAttributeAsFloat("x",currentFrame->vertex->position.x);
          v->SetAttributeAsFloat("y",currentFrame->vertex->position.y);
          v->SetAttributeAsFloat("z",currentFrame->vertex->position.z);
          v->SetAttributeAsFloat("u",currentFrame->vertex->u);
          v->SetAttributeAsFloat("v",currentFrame->vertex->v);
          v->SetAttributeAsFloat("nx",currentFrame->vertex->normal.x);
          v->SetAttributeAsFloat("ny",currentFrame->vertex->normal.y);
          v->SetAttributeAsFloat("nz",currentFrame->vertex->normal.z);
          //fprintf(f, "      V (%f,%f,%f:%f,%f)\n", currentFrame->vertex->position.x, 
          //                                         currentFrame->vertex->position.y, 
          //                                         currentFrame->vertex->position.z, 
          //                                         currentFrame->vertex->u, 
          //                                         currentFrame->vertex->v);
          currentFrame = currentFrame->frame;
        }
        //fprintf(f, "    )\n");
      }
    }
  }
  csRef<iDocumentNode> action;
  action = params->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  action->SetValue ("action");
  action->SetAttribute("name","default");
  //fprintf(f, "    ACTION 'default' (");  
  if(frames!=NULL)
  {
    int i;
    for(i = 0; i < nbFrames;i++)
    {
      if(frames[i]!=NULL)
      {
        csRef<iDocumentNode> f;
        f = action->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
        f->SetValue ("f");
        f->SetAttributeAsInt("name",i);
        f->SetAttributeAsInt("delay",(int)(frameDuration*1000.0));
        //fprintf(f, "F('%d',%d) ",i,(int)(frameDuration*1000.0));
      }
    }
  }
  //fprintf(f, " )\n");
  
  struct TriangleList* currentTri = triangleList;
  while(currentTri !=NULL)
  {
    csRef<iDocumentNode> t;
    t = params->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    t->SetValue ("t");
    t->SetAttributeAsInt("v1",currentTri->triangle->index1);
    t->SetAttributeAsInt("v2",currentTri->triangle->index2);
    t->SetAttributeAsInt("v3",currentTri->triangle->index3);
    //fprintf(f, "    TRIANGLE (%ld,%ld,%ld)\n",currentTri->triangle->index1, 
    //                                          currentTri->triangle->index2, 
    //                                          currentTri->triangle->index3);
    currentTri = currentTri ->triangleList;
  }
  

  if(nbJoints>0)
  {
    csRef<iDocumentNode> skeleton;
    skeleton = params->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    skeleton->SetValue ("skeleton");
    skeleton->SetAttribute("name","skeleton");
    //fprintf(f,"    SKELETON 'skeleton' ( \n");
    int i;
    for(i = 0;i<nbJoints;i++)
    {
      //find out if it's a root joint.
      if(joints[i]->parent==-1)
      {
        printJoint(joints[i],skeleton);
      }
    }
    //fprintf(f,"    )\n");
  }

  //fprintf(f, "  )\n)\n");
  
  //loading the motions.
  if(nbJoints > 0 )
  {
    csRef<iDocumentNode> addon;
    addon = library->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    addon->SetValue ("addon");
    plugin = addon->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    plugin->SetValue ("plugin");
    text = plugin->CreateNodeBefore (CS_NODE_TEXT, NULL);
    text->SetValue ("crystalspace.motion.loader.default");
    params = addon->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    params->SetValue ("params");
    csRef<iDocumentNode> motion;
    motion = params->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    motion->SetValue ("motion");
    motion->SetAttribute("name","default");
    csRef<iDocumentNode> duration;
    duration = motion->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    duration->SetValue ("duration");
    csRef<iDocumentNode> time;
    time = duration->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    time->SetValue ("time");
    text = time->CreateNodeBefore (CS_NODE_TEXT, NULL);
    text->SetValueAsFloat ( ((float)nbFrames)*frameDuration);
    csRef<iDocumentNode> loop;
    loop = duration->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    loop->SetValue ("loop");
    //fprintf(f, "ADDON (\n");
    //fprintf(f, "  PLUGIN ('crystalspace.motion.loader.default')\n");
    //fprintf(f, "  PARAMS (\n");
    //fprintf(f, "    MOTION 'default' (\n");  
    //fprintf(f, "      DURATION '%f' ( LOOP() )\n",((float)nbFrames)*frameDuration); 
   
    int i, j, k;
    for(i = 0; i < nbJoints;i++)
    {
      if(joints[i]->nbPositionKeys>0||joints[i]->nbRotationKeys>0)
      {
        //Some rotation and position keys might be at the same time.
        //I never encounter a case in which a rotation or position key was on his own.
        bool* rotUsed = new bool[joints[i]->nbRotationKeys];
        for(k = 0;k < joints[i]->nbRotationKeys;k++) rotUsed[k] = false;
        
        csRef<iDocumentNode> bone;
        bone = motion->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
        bone->SetValue ("bone");
        bone->SetAttribute("name",joints[i]->name);
        //fprintf(f, "      BONE '%s' (\n",joints[i]->name); 
        for(j = 0;j < joints[i]->nbPositionKeys;j++)
        {
          bool found = false;
          for(k = 0;k < joints[i]->nbRotationKeys;k++)
          {
            if(joints[i]->rotationKeys[k].time==joints[i]->positionKeys[j].time)
            {
              csMatrix3 matrixy = csZRotMatrix3(joints[i]->rotationKeys[k].data.z) *
                                  csYRotMatrix3(-joints[i]->rotationKeys[k].data.y) * 
                                  csXRotMatrix3(joints[i]->rotationKeys[k].data.x);
              csReversibleTransform transy = orig[i]*csReversibleTransform(matrixy,-matrixy.GetInverse()*joints[i]->positionKeys[j].data);
              csQuaternion qy = csQuaternion(transy.GetO2T());
              csVector3 vy = - transy.GetO2T() * transy.GetO2TTranslation();
              
              csRef<iDocumentNode> frame;
              frame = bone->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
              frame->SetValue ("frame");
              time = frame->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
              time->SetValue ("time");
              text = time->CreateNodeBefore (CS_NODE_TEXT, NULL);
              text->SetValueAsFloat (joints[i]->positionKeys[j].time*frameDuration);
              csRef<iDocumentNode> pos;
              pos = frame->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
              pos->SetValue ("pos");
              pos->SetAttributeAsFloat("x",vy.x);
              pos->SetAttributeAsFloat("y",vy.y);
              pos->SetAttributeAsFloat("z",vy.z);
              csRef<iDocumentNode> rot;
              rot = frame->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
              rot->SetValue ("rot");
              csRef<iDocumentNode> q;
              q = rot->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
              q->SetValue ("q");
              q->SetAttributeAsFloat("x",qy.x);
              q->SetAttributeAsFloat("y",qy.y);
              q->SetAttributeAsFloat("z",qy.z);
              q->SetAttributeAsFloat("r",qy.r);
              //fprintf(f, "        FRAME '%f' ( POS(%f,%f,%f) ROT(Q(%f,%f,%f,%f)))\n",
              //        joints[i]->positionKeys[j].time*frameDuration, 
              //        vy.x,
              //        vy.y,
              //        vy.z,
              //        qy.x,
              //        qy.y,
              //        qy.z,
              //        qy.r);
              rotUsed[k]=true;
              found = true;
              break; //There can only match 1
            }
          }
          if(!found)
          {
            csRef<iDocumentNode> frame;
            frame = bone->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
            frame->SetValue ("frame");
            time = frame->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
            time->SetValue ("time");
            text = time->CreateNodeBefore (CS_NODE_TEXT, NULL);
            text->SetValueAsFloat (joints[i]->positionKeys[j].time*frameDuration);
            csRef<iDocumentNode> pos;
            pos = frame->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
            pos->SetValue ("pos");
            pos->SetAttributeAsFloat("x",joints[i]->positionKeys[j].data.x);
            pos->SetAttributeAsFloat("y",joints[i]->positionKeys[j].data.y);
            pos->SetAttributeAsFloat("z",joints[i]->positionKeys[j].data.z);
            //fprintf(f, "        FRAME '%f' ( POS(%f,%f,%f))\n",
            //        joints[i]->positionKeys[j].time*frameDuration, 
            //        joints[i]->positionKeys[j].data.x,
            //        joints[i]->positionKeys[j].data.y,
            //        joints[i]->positionKeys[j].data.z);
          }
        }
        for(k = 0;k < joints[i]->nbRotationKeys;k++)
        {
          if(!rotUsed[k])
          {
            csMatrix3 matrixy = csZRotMatrix3(joints[i]->rotationKeys[k].data.z) *
                                  csYRotMatrix3(-joints[i]->rotationKeys[k].data.y) * 
                                  csXRotMatrix3(joints[i]->rotationKeys[k].data.x);
            csQuaternion qy = csQuaternion(matrixy);
            
            csRef<iDocumentNode> frame;
            frame = bone->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
            frame->SetValue ("frame");
            time = frame->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
            time->SetValue ("time");
            text = time->CreateNodeBefore (CS_NODE_TEXT, NULL);
            text->SetValueAsFloat (joints[i]->rotationKeys[j].time*frameDuration);
            csRef<iDocumentNode> rot;
            rot = frame->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
            rot->SetValue ("rot");
            csRef<iDocumentNode> q;
            q = rot->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
            q->SetValue ("q");
            q->SetAttributeAsFloat("x",qy.x);
            q->SetAttributeAsFloat("y",qy.y);
            q->SetAttributeAsFloat("z",qy.z);
            q->SetAttributeAsFloat("r",qy.r);
            //fprintf(f, "        FRAME '%f' ( ROT(Q(%f,%f,%f,%f)))\n",
            //        joints[i]->rotationKeys[k].time*frameDuration, 
            //        qy.x,
            //        qy.y,
            //        qy.z,
            //        qy.r);
          }
        }
        //fprintf(f, "      )\n");
      } 
    }
    //fprintf(f, "    )\n");
    //fprintf(f, "  )\n");
    //fprintf(f, ")\n");
  }
  
  //fprintf(f, ")\n");
  iString* string = new scfString();
  doc->Write(string);
  fprintf(f,string->GetData());
  fclose(f);
  return true;
}

void MsModel::printJoint(struct Joint* joint,csRef<iDocumentNode> parent)
{
  csRef<iDocumentNode> child;
  child = parent->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  child->SetValue("limb");
  child->SetAttribute("name",joint->name);
  //fprintf(f, "      LIMB '%s' ( \n" , joint->name);
  
  //bool vertices=false;
  if(frames!=NULL)
  {
    int i;
    for(i = 0; i < nbFrames;i++)
    {
      if(frames[i]!=NULL)
      {
        struct Frame* currentFrame = frames[i];
        long vertexCount=0;
        while(currentFrame!=NULL)
        {
          if(currentFrame->vertex->boneIndex == joint->index)
          {
            csRef<iDocumentNode> v;
            v = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
            v->SetValue("v");
            csRef<iDocumentNode> text;
            text = v->CreateNodeBefore (CS_NODE_TEXT, NULL);
            text->SetValueAsInt (vertexCount);
            //if(!vertices)
            //{
            //   fprintf(f, "        VERTICES(%ld",vertexCount);
            //}
            //else
            //{
            //   fprintf(f, "\n          ,%ld",vertexCount);
            //}
            //vertices = true;
          }
          currentFrame = currentFrame->frame;
          vertexCount++;
        }
        // We only need 1 one frame because all the frames should 
        // describe the same vertices. And MS models can have different numbers of
        // vertices per frame.
        break;
      }
    }
  }
  //if(vertices)
  //{
  //  fprintf(f, ")\n");
  //}
  csRef<iDocumentNode> transform;
  transform = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  transform->SetValue("transform");
  
  csRef<iDocumentNode> matrix;
  matrix = transform->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  matrix->SetValue("matrix");
  
  csRef<iDocumentNode> mxx;
  mxx = matrix->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  mxx->SetValue("m11");
  csRef<iDocumentNode> text;
  text = mxx->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (joint->transform->matrix.m11);
  
  mxx = matrix->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  mxx->SetValue("m12");
  text = mxx->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (joint->transform->matrix.m12);
  
  mxx = matrix->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  mxx->SetValue("m13");
  text = mxx->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (joint->transform->matrix.m13);
  
  mxx = matrix->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  mxx->SetValue("m21");
  text = mxx->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (joint->transform->matrix.m21);
  
  mxx = matrix->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  mxx->SetValue("m22");
  text = mxx->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (joint->transform->matrix.m22);
  
  mxx = matrix->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  mxx->SetValue("m23");
  text = mxx->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (joint->transform->matrix.m23);
  
  mxx = matrix->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  mxx->SetValue("m31");
  text = mxx->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (joint->transform->matrix.m31);
  
  mxx = matrix->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  mxx->SetValue("m32");
  text = mxx->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (joint->transform->matrix.m32);
  
  mxx = matrix->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  mxx->SetValue("m33");
  text = mxx->CreateNodeBefore (CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (joint->transform->matrix.m33);
  
  csRef<iDocumentNode> v;
  v = transform->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  v->SetValue("v");
  v->SetAttributeAsFloat("x",joint->transform->move.x);
  v->SetAttributeAsFloat("y",joint->transform->move.y);
  v->SetAttributeAsFloat("z",joint->transform->move.z);
  
  //fprintf(f, "        TRANSFORM( MATRIX( %f, %f, %f, %f, %f, %f, %f, %f, %f ) ",
  //        joint->transform->matrix.m11, joint->transform->matrix.m12, joint->transform->matrix.m13, 
  //        joint->transform->matrix.m21, joint->transform->matrix.m22, joint->transform->matrix.m23, 
  //        joint->transform->matrix.m31, joint->transform->matrix.m32, joint->transform->matrix.m33);
  //fprintf(f, "V( %f, %f, %f  )" ,         
  //        joint->transform->move.x, joint->transform->move.y, joint->transform->move.z);
  //fprintf(f, " )\n");
  int i;
  for(i=0;joint->children[i]!=-1;i++)
  {
    printJoint(joints[joint->children[i]],child);
  }
  
  //fprintf(f, "      )\n");
}

void MsModel::transform(csReversibleTransform* trans, int boneIndex,int parent)
{
  if(parent != -1)
  {
    trans[boneIndex] = trans[parent]*trans[boneIndex];
  }
  int i;
  for(i=0;joints[boneIndex]->children[i]!=-1;i++)
  {
    transform(trans,joints[joints[boneIndex]->children[i]]->index,boneIndex);
  }
}
