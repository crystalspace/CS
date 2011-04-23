/*
  Copyright (C) 2009 by Seth Berrier

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

#ifndef __IRCACHEOCTREENODE_H__
#define __IRCACHEOCTREENODE_H__

namespace lighter
{
  struct IrradianceSample;

  struct NearestSamples
  {
    size_t count;
    float *weights;
    IrradianceSample **samples;
  };

  class OctreeSampleNode
  {
  private:
    // Data
    csArray<size_t> samples;
    OctreeSampleNode* child[8];
    bool isLeaf;

    // Bounding CUBE for this octant (center and edge length)
    float center[3], size;

    // General data for all nodes
    IrradianceSample** masterArray;
    static double alpha;

    // Helper functions
    void SplitNode();
    static bool Shadowed(const IrradianceSample *A, const IrradianceSample *B);
    static float Weight(const IrradianceSample *A, const IrradianceSample *B);

  public:
    OctreeSampleNode(IrradianceSample** parentArray,
      const double newAlpha = -1.0);
    ~OctreeSampleNode();

    void AddSample(const size_t newNode);
    void SetBoundingBox(const float newMin[3], const float newMax[3]);
    void FindSamples(const IrradianceSample *samp, NearestSamples* &nearest);
  };

};

#endif // __IRCACHEOCTREENODE_H__
