/*
  Copyright (C) 2005 by Marten Svanfeldt

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
#include "lightmesh.h"

csMeshPatch::csMeshPatch ()
  : parentFace (0), area (0.0f), accStruct (0)
{
  for (uint i=0; i<4;i++)
  {
    vertexIndex[i] = -1;
    colorIndex[i] = -1;
    neighbour[i] = 0;
    child[i] = 0;
  }
}

csMeshPatch::~csMeshPatch ()
{
  FreeAccelerationStructs ();
  // Destroy children
  for (int i = 0; i < 4; i++)
  {
    delete child[i];
  }
}

// Lookup tables for the dirs
static int quadOppDir[] = {2, 3, 0, 1};

static int quadChildDir1[] = {0, 1, 2, 3};
static int quadChildDir2[] = {3, 0, 1, 2};

static int triChildDir1[] = {0, 1, 2};
static int triChildDir2[] = {1, 2, 0};

void csMeshPatch::SetupNeighbours ()
{
  int i = 0;
  if (IsQuad ())
  {
    //Setup "easy" interchild connections
    child[0]->neighbour[2] = child[1];
    child[0]->neighbour[3] = child[3];

    child[1]->neighbour[0] = child[0];
    child[1]->neighbour[3] = child[2];

    child[2]->neighbour[0] = child[3];
    child[2]->neighbour[1] = child[1];

    child[3]->neighbour[1] = child[0];
    child[3]->neighbour[2] = child[2];

    //setup neighbour->us/child connections
    for (i = 0; i < 4; i++)
    {
      if (neighbour[i] && neighbour[i]->HasChildren ())
      {
        int oppdir = quadOppDir[i];
        int ch1 = quadChildDir1[i];
        int ch2 = quadChildDir2[i];

        int ch1opp = quadChildDir1[oppdir];
        int ch2opp = quadChildDir2[oppdir];

        //fix neighbours children
        neighbour[i]->child[ch1opp]->neighbour[oppdir] = child[ch2];
        neighbour[i]->child[ch2opp]->neighbour[oppdir] = child[ch1];
        //and our children
        child[ch1]->neighbour[i] = neighbour[i]->child[ch2opp];
        child[ch2]->neighbour[i] = neighbour[i]->child[ch1opp];
      }
      else
      {
        child[quadChildDir1[i]]->neighbour[i] = 0;
        child[quadChildDir2[i]]->neighbour[i] = 0;
      }
    }
  }
  else
  {
    // IsTriangle () == true
    // the easy ones
    child[0]->neighbour[1] = child[3];
    child[1]->neighbour[2] = child[3];
    child[2]->neighbour[0] = child[3];
    child[3]->neighbour[0] = child[2];
    child[3]->neighbour[1] = child[0];
    child[3]->neighbour[2] = child[1];

    //the directions
    for (i = 0; i < 3; i++)
    {
      if (neighbour[i] && neighbour[i]->HasChildren ())
      {
        int ch1 = triChildDir1[i];
        int ch2 = triChildDir2[i]; 

        //fix neighbours children
        neighbour[i]->child[ch1]->neighbour[i] = child[ch2];
        neighbour[i]->child[ch2]->neighbour[i] = child[ch1];
        //and other way around
        child[ch1]->neighbour[i] = neighbour[i]->child[ch2];
        child[ch2]->neighbour[i] = neighbour[i]->child[ch1];
      }
      else
      {
        child[triChildDir1[i]]->neighbour[i] = 0;
        child[triChildDir2[i]]->neighbour[i] = 0;
      }
    }
  }
}

void csMeshPatch::Subdivide ()
{
  int i = 0;
  // If we have children, let them subdivide instead of us
  if (HasChildren ())
  {
    for (i = 0; i < 4; i++)
    {
      child[i]->Subdivide ();
    }
    return;
  }

  //some holders
  int centerIdx, midPointIdx[4];
  int centerCIdx, midPointCIdx[4];
  int oppdir, ch1, ch2;

  // Create the children
  for (i = 0; i < 4; i++)
  {
    child[i] = new csMeshPatch;
  }


  //Add mid if quad
  if (IsQuad ())
  {
    centerIdx = parentFace->AddVertex (center);
    csColor cntColor = GetColor (0) + GetColor (1) + GetColor (2) + GetColor (3);
    centerCIdx = parentFace->AddColor (cntColor*0.25f);

    //iterate over neighbours and see if we can get any midpoint 
    for (i = 0; i < 4; i++)
    {
      if (neighbour[i] && neighbour[i]->HasChildren ())
      {
        //have child, thus have a midpoint in our direction
        oppdir = quadOppDir[i];
        ch1 = quadChildDir1[oppdir];
        ch2 = quadChildDir2[oppdir];

        midPointIdx[i] = neighbour[i]->child[ch1]->vertexIndex[ch2];
        midPointCIdx[i] = neighbour[i]->child[ch1]->colorIndex[ch2];
      } 
      else
      {
        //we have to add it
        csVector3 mid = (GetVertex (quadChildDir1[i]) + 
                         GetVertex (quadChildDir2[i])) / 2;
        midPointIdx[i] = parentFace->AddVertex (mid);

        csColor cmid = (GetColor (quadChildDir1[i]) + 
                        GetColor (quadChildDir2[i])) / 2;
        midPointCIdx[i] = parentFace->AddColor (cmid);
      }
    }

    //Fix the indices
    child[0]->SetVertexIdx (vertexIndex[0], midPointIdx[1], centerIdx, midPointIdx[0]);
    child[1]->SetVertexIdx (midPointIdx[1], vertexIndex[1], midPointIdx[2], centerIdx);
    child[2]->SetVertexIdx (centerIdx, midPointIdx[2], vertexIndex[2], midPointIdx[3]);
    child[3]->SetVertexIdx (midPointIdx[0], centerIdx, midPointIdx[3], vertexIndex[3]);

    child[0]->SetColorIdx (colorIndex[0], midPointCIdx[1], centerCIdx, midPointCIdx[0]);
    child[1]->SetColorIdx (midPointCIdx[1], colorIndex[1], midPointCIdx[2], centerCIdx);
    child[2]->SetColorIdx (centerCIdx, midPointCIdx[2], colorIndex[2], midPointCIdx[3]);
    child[3]->SetColorIdx (midPointCIdx[0], centerCIdx, midPointCIdx[3], colorIndex[3]);
  }
  else
  {
    //iterate over neightbours, check for existing midpoints
    for (i = 0; i < 3; i++)
    {
      if (neighbour[i] && neighbour[i]->HasChildren ())
      {
        ch1 = triChildDir1[i];
        ch2 = triChildDir2[i];

        midPointIdx[i] = neighbour[i]->child[ch1]->vertexIndex[ch2];
        midPointCIdx[i] = neighbour[i]->child[ch1]->colorIndex[ch2];
      }
      else
      {
        //we have to add it
        csVector3 mid = (GetVertex (triChildDir1[i]) + 
                         GetVertex (triChildDir2[i])) / 2;
        midPointIdx[i] = parentFace->AddVertex (mid);

        csColor cmid = (GetColor (triChildDir1[i]) + 
                        GetColor (triChildDir2[i])) / 2;
        midPointCIdx[i] = parentFace->AddColor (cmid);
      }
    }

    //fix indices
    child[0]->SetVertexIdx (vertexIndex[0], midPointIdx[0], midPointIdx[2]);
    child[1]->SetVertexIdx (midPointIdx[0], vertexIndex[1], midPointIdx[1]);
    child[2]->SetVertexIdx (midPointIdx[2], midPointIdx[1], vertexIndex[2]);
    child[3]->SetVertexIdx (midPointIdx[1], midPointIdx[2], midPointIdx[0]);
  }
  
  //fix child<->neighbour connections
  SetupNeighbours ();

  for (i = 0; i < 4; i++)
  {
    child[i]->area = area / 4;
    child[i]->parent = this;
    child[i]->parentFace = parentFace;
    
    csVector3 c = (child[i]->GetVertex (0) + child[i]->GetVertex (1) + 
      child[i]->GetVertex (2) + child[i]->GetVertex (3)) / 4.0f;
    child[i]->center = c;
  }
}

void csMeshPatch::ConstructAccelerationStruct ()
{
  //First make sure there is no old struct
  if (accStruct) return;

  if (IsQuad ())
  {
    accStruct = new csMeshPatchAccStruct[2]; 
    csSetupAccStruct (accStruct[0], 
      parentFace->mesh->vertexList[vertexIndex[0]],
      parentFace->mesh->vertexList[vertexIndex[1]],
      parentFace->mesh->vertexList[vertexIndex[2]],
      parentFace->geoNormal);
    csSetupAccStruct (accStruct[1], 
      parentFace->mesh->vertexList[vertexIndex[0]],
      parentFace->mesh->vertexList[vertexIndex[2]],
      parentFace->mesh->vertexList[vertexIndex[3]],
      parentFace->geoNormal);
  }
  else
  {
    //IsTriangle()
    accStruct = new csMeshPatchAccStruct[1];
    csSetupAccStruct (accStruct[0], 
      parentFace->mesh->vertexList[vertexIndex[0]],
      parentFace->mesh->vertexList[vertexIndex[1]],
      parentFace->mesh->vertexList[vertexIndex[2]],
      parentFace->geoNormal);
  }
}

void csMeshPatch::FreeAccelerationStructs ()
{
  delete [] accStruct;
  accStruct = 0;
}

void csSetupAccStruct (csMeshPatchAccStruct &acc, const csVector3 &A, 
                       const csVector3 &B, const csVector3 &C, const csVector3 &normal)
{
  int k = 0; //normal maxdirection

  // Find max normal direction
  if (fabsf (normal.x) > fabsf (normal.y))
  {
    if (fabsf (normal.x) > fabsf (normal.z)) k = 0;
    else k = 2;
  }
  else
  {
    if (fabsf (normal.y) > fabsf (normal.z)) k = 1;
    else k = 2;
  }
  
  uint u = (k+1)%3;
  uint v = (k+1)%3;

  // precalc normal
  float nkinv = 1.0f/normal[k];
  acc.normal_u = normal[u] * nkinv;
  acc.normal_v = normal[v] * nkinv;
  acc.normal_d = (normal * A) * nkinv;


  csVector3 b = C - A;
  csVector3 c = B - A;

  float tmp = 1.0f/(b[u] * c[v] - b[v] * c[u]);

  // edge 1
  acc.b_nu = b[u] * tmp;
  acc.b_nv = -b[v] * tmp;
  acc.b_d = (b[v] * A[u] - b[u] * A[v]) * tmp;

  // edge 2
  acc.b_nu = c[u] * tmp;
  acc.b_nv = -c[v] * tmp;
  acc.b_d = (c[v] * A[u] - c[u] * A[v]) * tmp;
}
