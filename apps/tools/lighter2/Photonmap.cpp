

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
    photons.Push(new Photon(color, dir, pos));
    /*
    // check to see if we are the first photon to be added
    if (!root)
    {
      root = new Photon(color, dir, pos);
      root->planeDir = 0;
      return;
    }

    // lets find the node we should be attached to
    Photon *current = root;
    int direction = 1;
    while (current)
    {
      // check the left branch
      if (pos[direction] <= current->position[direction])
      {
        // check to see if there is a left branch
        if (current->left)
        {
          // update current and continue
          current = current->left;
        }
        else
        {
          current->left = new Photon(color, dir, pos);
          current->left->planeDir = direction;
          return;
        }
      }
      // check right 
      else if (pos[direction] > current->position[direction])
      {
        // check to see if there is a left branch
        if (current->right)
        {
          // update current and continue
          current = current->right;
        }
        else
        {
           current->right = new Photon(color, dir, pos);
           current->right->planeDir = direction;
           return;
        }
      }

      direction = (direction + 1) % 3;
    }
    */
  }


  csColor PhotonMap::SampleColor(const csVector3& pos,  float radius, 
                                 const csVector3& normal)
  {
    CS::Utility::PriorityQueue<Photon> que;
    csArray<Photon> nearest;
    for (size_t num=0; num < photons.GetSize(); ++num)
    {
      csVector3 dist = pos - photons[num]->position;
      photons[num]->distance = dist.SquaredNorm();
      que.Insert(*photons[num]);
    }

    // remove as many element as it takes
    while (nearest.GetSize() <= photonsPerSample && !que.IsEmpty())
    {
      nearest.Push(que.Pop());
    }
    
    //csArray<Photon> nearest = NearestNeighbor(pos, radius, photonsPerSample);
    csColor final(0,0,0);

    // loop through the nearest photons and based on their normal 
    // add their color to the final returned value
    for (size_t num = 0; num < nearest.GetSize(); num++)
    {
      csVector3 pnorm = nearest[num].direction;
      float app = normal*pnorm;

      // Check to make sure the two vectors aren't radically different or 
      // possibly on the other side of a surface
      if (!(app < .5))
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
      final = final * area * (1.0 / (0.5f * nearest.GetSize()));
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
      if (InRange(current->position, pos, radius, current->planeDir)
        && current->right
        && current->left)
      {
        parents.Push(current->right);
        parents.Push(current->left);
      }
      // else if we need check if its on the right or left
      else if (pos[current->planeDir] <= 
        current->position[current->planeDir] 
        && current->left)
      {
        // add just the left
        parents.Push(current->left);
      }
      // at this point it has to be on the right so just add the right
      else if (current->right)
      {
        parents.Push(current->right);
      }
    }

    // remove as many element as it takes
    while (finalOut.GetSize() <= number && !que.IsEmpty())
    {
      finalOut.Push(que.Pop());
    }
    return finalOut;
  }

  bool PhotonMap::InRange(const csVector3& tar, const csVector3& pos, 
                          const float& distance, const int& direction)
  {
    // get the points distance relative to the plane
    csVector3 dist = pos - tar;
    csVector3 normal;

    // based on the direction set the normal
    if (direction == DIRX)
    {
      normal = csVector3(1, 0, 0);
    }
    else if (direction == DIRY)
    {
      normal = csVector3(0, 1, 0);
    }
    else
    {
      normal = csVector3(0, 0, 1);
    }

    // now take the dot product of the normal and the point 
    // for total the total distance to the plane
    float tDist = dist.x*normal.x + dist.y*normal.y + dist.z*normal.z;
    
    // check to see if that distance is less than the search distance
    return distance > tDist;
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
