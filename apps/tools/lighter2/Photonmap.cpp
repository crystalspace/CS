/*
  Copyright (C) 2008 by Greg Hoffman

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

#include <stdio.h>
#include "common.h"

#include "Photonmap.h"
#include "lighter.h"
#include <csutil/priorityqueue.h>

namespace lighter
{

  PhotonMap::PhotonMap()
  {
    root = 0;
    // photons per sample will need to be set from config eventually
    photonsPerSample = 30;
  }

  PhotonMap::~PhotonMap()
  {
    // remove the tree
    DeleteNodes(root);
  }

  void PhotonMap::DeleteNodes(Photon *p)
  {
    // check for null so we know when to stop
    if (!p)
    {
      return;
    }

    DeleteNodes(p->left);
    DeleteNodes(p->right);
    delete p;
  }

  void PhotonMap::AddPhoton(const csColor& color, const csVector3& dir,
                            const csVector3& pos)
  {
    // Create the new photon struct and store in the flat array
    Photon* newPhoton = new Photon(color, dir, pos);
    photonArray.push_back(newPhoton);
    
    // Add the photon to the kd-Tree

    // Is this the first photon (make the root)
    if (!root)
    {
      root = newPhoton;
      root->planeDir = 0;
      return;
    }

    // Traverse the tree to find where the photon should be attached
    Photon *current = root;
    int direction = 0;
    while (current)
    {
      // Traverse the left branch of this node
      if (pos[direction] <= current->position[direction])
      {
        // Is the left branch occupied?
        if (current->left)
        {
          // update current and continue
          current = current->left;
        }
        else
        {
          // Add this photon as a leaf on the left branch
          current->left = newPhoton;
          current->left->planeDir = (direction + 1) % 3;
          return;
        }
      }
      
      // Traverse right branch of this node 
      else if (pos[direction] > current->position[direction])
      {
        // Is the right branch occupied
        if (current->right)
        {
          // update current and continue
          current = current->right;
        }
        else
        {
           // Add this photon as a leaf on the right branch
           current->right = newPhoton;
           current->right->planeDir = (direction + 1) % 3;
           return;
        }
      }

      direction = (direction + 1) % 3;
    }
    
  }

  void PhotonMap::SaveToFile(string filename)
  {
    // Open file for writing in binary mode
    FILE* fout = fopen(filename.c_str(), "wb");
    if(fout != NULL)
    {
      // Write the number of photons
      size_t count = photonArray.size();
      fwrite(&count, sizeof(size_t), 1, fout);

      // Write each photon
      for(size_t i=0; i<count; i++)
      {
        // Get pointer to photon
        Photon* curPhoton = photonArray[i];

        // Write the color
        unsigned char curColor[3] = {
          (int)floor(curPhoton->color.red*255.0 + 0.5),
          (int)floor(curPhoton->color.green*255.0 + 0.5),
          (int)floor(curPhoton->color.blue*255.0 + 0.5)
        };

        fwrite(curColor, sizeof(unsigned char), 3, fout);

        // Write the direction
        float curDir[3] = {
          curPhoton->direction.x,
          curPhoton->direction.y,
          curPhoton->direction.z
        };

        fwrite(curDir, sizeof(float), 3, fout);

        // Write the position
        float curPos[3] = {
          curPhoton->position.x,
          curPhoton->position.y,
          curPhoton->position.z
        };

        fwrite(curPos, sizeof(float), 3, fout);
      }

      // Close the file
      fclose(fout);
    }
  }

  csColor PhotonMap::SampleColor(const csVector3& pos,  float radius, 
                                 const csVector3& normal, const csVector3& dir)
  {
    csArray<Photon> nearest = NearestNeighbor(pos, radius, photonsPerSample);
    csColor final(0,0,0);

    // loop through the nearest photons and based on their normal 
    // add their color to the final returned value
    for (size_t num = 0; num < nearest.GetSize(); num++)
    {
      csVector3 pnorm = nearest[num].direction;
      float app = normal*pnorm;

      // Check to make sure the two vectors aren't radically different or 
      // possibly on the other side of a surface
      if (!(app < 0.0))
      {
        // The wpc can be changed for filtering, default for now
        float wpc = 1.0;
        final.red += nearest[num].color.red*wpc;
        final.green += nearest[num].color.green*wpc;
        final.blue += nearest[num].color.blue*wpc;
      }
    }

    // Now as long as we have photons returned multiply by area / PI
    if (nearest.GetSize() != 0)
    {
      float area = (1.0 / (radius * radius * PI));
      //final = final * area * (1.0 / (0.5f * nearest.GetSize()));
      final = final * area * (1.0 / (1.0f * photonsPerSample));
    }
    return final;
    
  }

  csArray<Photon> PhotonMap::NearestNeighbor(const csVector3& pos, 
                                             float radius, 
                                             int number)
  {
    CS::Utility::PriorityQueue<Photon> que;
    csArray<Photon> finalOut;
    csArray<Photon*> parents;
    float radSquared = radius * radius;

    // Traverse the tree in a depth first manner
    parents.Push(root);
    while (parents.GetSize() > 0)
    {
      Photon *current = parents.Pop();
      
      // check to see if the photon is in range with squared distances
      csVector3 dist = current->position - pos;
      float totalDist = dist*dist;

      if (totalDist < radSquared)
      {
        Photon p(current->color, current->direction, current->position);
        p.distance = totalDist;
        que.Insert(p);
      }

      // check to see if we hit the intersection plane, if we do then
      // we need to add both sides of the tree to the parent tree.
      float tDistance = current->position[current->planeDir] - pos[current->planeDir];
      float tDisSq = tDistance * tDistance;
      float cdir = current->position[current->planeDir];
      float cdir2 = pos[current->planeDir];
      if (cdir2 < cdir)
      {
        if (current->left)
        {
          parents.Push(current->left);
        }
        if (tDisSq < radSquared && current->right)
        {
          parents.Push(current->right);
        }
      }
      else
      {
        if (current->right)
        {
          parents.Push(current->right);
        }

        if (tDisSq < radSquared && current->left)
        {
          parents.Push(current->left);
        }
      }
    }

    // remove as many element as it takes
    while (finalOut.GetSize() <= number && !que.IsEmpty())
    {
      finalOut.Push(que.Pop());
    }
    return finalOut;
  }

  bool Photon::operator < (const Photon& right) const
  {
    // This is backwards of what we want, but this way the smallest
    // elements will be the top on the priority que
    return distance > right.distance;
  }

  Photon::Photon(const csColor& color, const csVector3& dir,
                 const csVector3& pos)
  {
    left = right = 0;
    this->color = color;
    this->direction = dir;
    this->position = pos;
  }

  Photon::~Photon()
  {
    //if (left != 0) delete left;
    //if (right != 0) delete right;
  }
}
