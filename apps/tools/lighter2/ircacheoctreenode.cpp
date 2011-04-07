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

#include "common.h"
#include "ircacheoctreenode.h"
#include "irradiancecache.h"

#include "statistics.h"

namespace lighter
{
  double OctreeSampleNode::alpha = 0.1;

  OctreeSampleNode::OctreeSampleNode(IrradianceSample** parentArray,
    const double newAlpha)
  {
    if(newAlpha > 0.0)
    {
      OctreeSampleNode::alpha = newAlpha;
    }

    masterArray = parentArray;

    isLeaf = true;

    center[0] = center[1] = center[2] = 0;
    size = 0;

    child[0] = child[1] = child[2] = child[3] = NULL;
    child[4] = child[5] = child[6] = child[7] = NULL;
  }

  OctreeSampleNode::~OctreeSampleNode()
  {
    for(size_t i=0; i<8; i++)
      delete child[i];
  }

  bool OctreeSampleNode::Shadowed(const IrradianceSample *A,
                            const IrradianceSample *B)
  {
    // Compute vector from A to B
    float AB[3] = {
      A->pos[0] - B->pos[0],
      A->pos[1] - B->pos[1],
      A->pos[2] - B->pos[2]
    };

    // Compute average of Normals
    float NN[3] = {
      (A->norm[0] + B->norm[0])/2.0,
      (A->norm[1] + B->norm[1])/2.0,
      (A->norm[2] + B->norm[2])/2.0
    };

    // Compute dot product and test (eq 6 in Ward et al.)
    float dot = AB[0]*NN[0] + AB[1]*NN[1] + AB[2]*NN[2];
    return (dot >= 0.0f);
  }

  // Only the pos and norm from A are used
  // but all values in B are used
  float OctreeSampleNode::Weight(const IrradianceSample *A,
                              const IrradianceSample *B)
  {
    // Determine distance between A and B
    float AB[3] = {
      A->pos[0] - B->pos[0],
      A->pos[1] - B->pos[1],
      A->pos[2] - B->pos[2]
    };
    float len = sqrtf(AB[0]*AB[0] + AB[1]*AB[1] + AB[2]*AB[2]);

    // Determine dot product of normals
    float dot = A->norm[0]*B->norm[0] + 
      A->norm[1]*B->norm[1] + A->norm[2]*B->norm[2];

    // Compute weight according to Jensen pg. 141, eq. 11.5
    return 1.0/(len/B->mean + sqrtf(1.0f - dot));
  }

  void OctreeSampleNode::SetBoundingBox(
    const float newMin[3], const float newMax[3])
  {
    float maxDim = 0;
    for(size_t i=0; i<3; i++)
    {
      if(newMax[i] - newMin[i] > maxDim)
        maxDim = newMax[i] - newMin[i];
      center[i] = (newMin[i] + newMax[i])/2.0;
    }
    size = maxDim;
  }

  void OctreeSampleNode::FindSamples(const IrradianceSample *samp,
                          NearestSamples* &nearest)
  {
    globalStats.photonmapping.irCacheLookups++;

    // Check all samples at this node
    for(size_t i=0; i<samples.GetSize(); i++)
    {
      IrradianceSample *Pi = &((*masterArray)[samples[i]]);
      if(Pi && !Shadowed(samp, Pi))
      {
        float weight = Weight(samp, Pi);
        if(weight > 1.0/alpha)
        {
          #pragma omp critical
          {
            nearest->samples[nearest->count] = Pi;
            nearest->weights[nearest->count] = weight;
            nearest->count++;
          }
        }
      }
    }

    // Check children
    if(!isLeaf)
    {
      // Compute squared size for children
      float childSizeSq = child[0]->size;
      childSizeSq *= childSizeSq;

      // Compute distance to center of each child
      #pragma omp parallel for
      for(size_t i=0; i<8; i++)
      {
        float tmp[3], distSq;
        tmp[0] = child[i]->center[0] - samp->pos[0];
        tmp[1] = child[i]->center[1] - samp->pos[1];
        tmp[2] = child[i]->center[2] - samp->pos[2];
        distSq = tmp[0]*tmp[0] + tmp[1]*tmp[1] + tmp[2]*tmp[2];

        // Check distance to center & recurse
        if(distSq <= childSizeSq)
        {
          child[i]->FindSamples(samp, nearest);
        }
      }
    }
  }

  void OctreeSampleNode::AddSample(const size_t newNode)
  {
    // Check if size is within specified limits
    float validRange =
      (*masterArray)[newNode].mean*OctreeSampleNode::alpha*4;

    if(validRange < SMALL_EPSILON)
    {
      CS_ASSERT(false);
      return;
    }

    if(size < validRange)
    {
      samples.Push(newNode);
    }

    // Otherwise, it doesn't go here
    else
    {
      // Split the node if it isn't already
      if(isLeaf) SplitNode();

      // Find child that bounds this new sample
      bool xPlus = ((*masterArray)[newNode].pos[0] >= center[0]);
      bool yPlus = ((*masterArray)[newNode].pos[1] >= center[1]);
      bool zPlus = ((*masterArray)[newNode].pos[2] >= center[2]);

      // Only three tests to find the proper octant
      if(xPlus)
      {
        if(yPlus)
        {
          if(zPlus)
          {
            child[0]->AddSample(newNode);
          }
          else
          {
            child[4]->AddSample(newNode);
          }
        }
        else
        {
          if(zPlus)
          {
            child[3]->AddSample(newNode);
          }
          else
          {
            child[7]->AddSample(newNode);
          }
        }
      }
      else
      {
        if(yPlus)
        {
          if(zPlus)
          {
            child[1]->AddSample(newNode);
          }
          else
          {
            child[5]->AddSample(newNode);
          }
        }
        else
        {
          if(zPlus)
          {
            child[2]->AddSample(newNode);
          }
          else
          {
            child[6]->AddSample(newNode);
          }
        }
      }
    }
  }

  void OctreeSampleNode::SplitNode()
  {
    // Sanity check (can't split a non-leaf node)
    if(!isLeaf) return;

    globalStats.photonmapping.irCacheSplits++;

    // Allocate children nodes
    for(size_t i=0; i<8; i++)
    {
      child[i] = new OctreeSampleNode(masterArray);
    }

    // Build child node bounding cubes
    float halfSize = size/2.0, quarterSize = size/4.0;

    // Octant 0 is (+, +, +)
    child[0]->size = halfSize;
    child[0]->center[0] = center[0] + quarterSize;
    child[0]->center[1] = center[1] + quarterSize;
    child[0]->center[2] = center[2] + quarterSize;

    // Octant 1 is (-, +, +)
    child[1]->size = halfSize;
    child[1]->center[0] = center[0] - quarterSize;
    child[1]->center[1] = center[1] + quarterSize;
    child[1]->center[2] = center[2] + quarterSize;

    // Octant 2 is (-, -, +)
    child[2]->size = halfSize;
    child[2]->center[0] = center[0] - quarterSize;
    child[2]->center[1] = center[1] - quarterSize;
    child[2]->center[2] = center[2] + quarterSize;

    // Octant 3 is (+, -, +)
    child[3]->size = halfSize;
    child[3]->center[0] = center[0] + quarterSize;
    child[3]->center[1] = center[1] - quarterSize;
    child[3]->center[2] = center[2] + quarterSize;

    // Octant 4 is (+, +, -)
    child[4]->size = halfSize;
    child[4]->center[0] = center[0] + quarterSize;
    child[4]->center[1] = center[1] + quarterSize;
    child[4]->center[2] = center[2] - quarterSize;

    // Octant 5 is (-, +, -)
    child[5]->size = halfSize;
    child[5]->center[0] = center[0] - quarterSize;
    child[5]->center[1] = center[1] + quarterSize;
    child[5]->center[2] = center[2] - quarterSize;

    // Octant 6 is (-, -, -)
    child[6]->size = halfSize;
    child[6]->center[0] = center[0] - quarterSize;
    child[6]->center[1] = center[1] - quarterSize;
    child[6]->center[2] = center[2] - quarterSize;

    // Octant 7 is (+, -, -)
    child[7]->size = halfSize;
    child[7]->center[0] = center[0] + quarterSize;
    child[7]->center[1] = center[1] - quarterSize;
    child[7]->center[2] = center[2] - quarterSize;

    // This node is no longer a leaf
    isLeaf = false;
  }

};
