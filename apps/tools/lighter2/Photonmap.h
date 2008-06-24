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

#ifndef __PHOTONMAP_H__
#define __PHOTONMAP_H__

#include "common.h"

namespace lighter
{
  struct Photon
  {
    // The current color and energy of the photon
    csColor color;
    // The direction the photon is heading
    csVector3 direction;
    // the position of the photon in space
    csVector3 position;

    // the squared distance to the search point
    float distance;

    Photon *left;
    Photon *right;

    Photon(const csColor& color, const csVector3& dir,
           const csVector3& pos);
    ~Photon();
  };

  class PhotonMap
  {
  public:
    enum Direction { DIRX, DIRY, DIRZ };

    /**
     * Default Constructor
     * You must pass in the number of photons we are going to use in order
     * to initialize the heap correctly.
     */
    PhotonMap();

    /**
     * Default Deconstructor
     */
    ~PhotonMap();

    /**
    * Add Photon
    * Adds a photon to the heap data structure contained within
    * the photon map.
    * /param color - The color or energy of the photon
    * /param dir - The direction the given photon was traveling
    * /param pos - The position of the photon
    */
    void addPhoton(const csColor& color, 
                   const csVector3& dir, 
                   const csVector3& pos);

    /**
    * Sample Color
    * Samples the color at the given position and the given radius.
    * /param pos - the position to get a sample for
    * /param radius - the radius to sample
    * /param normal - the normal to check for, anything radically different
    *                 will be not be added
    */
    csColor sampleColor(const csVector3& pos,  float radius, 
                        const csVector3& normal);

    /**
    * NearestNeighbor
    * Searches the tree for the nearest neighbors and adds them to a
    * priority heap.
    * /param pos - the position to search for nearest neighbors
    * /param radius - the maximum radius to search for
    * /param number - the max number of elements to return
    * /return the nearest number of photons as a heap
    */
   csArray<Photon*> nearestNeighbor(const csVector3& pos, float radius, 
                                         int number);

    // helper for function for the heap
    static bool photonComp(const Photon* a, const Photon* b);

  private:
      

    // The root of the tree
    Photon *root;

  };

};

#endif