

#include "common.h"

#include "Photonmap.h"
#include "lighter.h"

namespace lighter
{
  PhotonMap::PhotonMap()
  {
    root = 0;
  }

  PhotonMap::~PhotonMap()
  {

  }

  void PhotonMap::addPhoton(const csColor& color, const csVector3& dir,
                            const csVector3& pos)
  {
    // check to see if we are the first photon to be added
    if (!root)
    {
      root = new Photon(color, dir, pos);
      return;
    }

    // lets find the node we should be attached to
    Photon *current = root;
    int direction = 0;
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
           return;
        }
      }

      direction = (direction + 1) % 3;
    }
  }

  csColor PhotonMap::sampleColor(const csVector3& pos,  float radius, 
                                 const csVector3& normal)
  {
    return csColor();
  }

  csArray<Photon*> nearestNeighbor(const csVector3& pos, float radius, 
                                            int number)
  {
    //std::priority_queue<Photon*, std::vector<Photon*>, PhotonMap::photonComp> que;
    return csArray<Photon*>();
  }

  bool PhotonMap::photonComp(const Photon* a, const Photon* b)
  {
    return a->distance < b->distance;
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
    if (left != 0) delete left;
    if (right != 0) delete right;
  }
}
