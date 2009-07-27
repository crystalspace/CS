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

#include <vector>
#include <string>
using namespace std;

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
    
    // The direction for this node
    int planeDir;

    Photon *left;
    Photon *right;

    // Overload for the less than operator for use with the comparator
    bool operator< (const Photon& right) const;

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
    void AddPhoton(const csColor& color, 
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
    csColor SampleColor(const csVector3& pos,  float radius, 
                        const csVector3& normal, const csVector3& dir = csVector3());

   /**
   * NearestNeighbor
   * Searches the tree for the nearest neighbors and adds them to a
   * priority que. The idea is to use the radius to judge if we are 
   * going to search a branch of the tree or not.
   *
   * /param pos - the position to search for nearest neighbors
   * /param radius - the maximum radius to search for
   * /param number - the max number of elements to return
   * /return the nearest number of photons as a heap
   */
   csArray<Photon> NearestNeighbor(const csVector3& pos, float radius, 
                                         int number);

   /**
    * SaveToFile
    * Write out the photons in this photon map to a binary file for use
    * outside of lighter2.  This is particularly useful for visualizing
    * the photon map for debugging purposes.
    *
    * The file will have the format:
    *    <size_t: count><photon: p>*count
    * Each 'photon' in the file will be:
    *    <byte: red><byte: green><byte: blue>
    *    <float: dirX><float: dirY><float: dirZ>
    *    <float: posX><float: posY><float: posZ>
    * Note, the line breaks are not part of the file format.
    *
    * /param filename - the fully qualified name of the file to save to.
    **/
   void SaveToFile(string filename);

  private:
    // helper function to delete the tree
    void DeleteNodes(Photon *p);

    // number of photons to sample
    int photonsPerSample;

    // The root of the tree
    Photon *root;

    // Flat array of all the photons
    vector<Photon*> photonArray;
  };

};

#endif