/*
    Copyright (C) 2008 by Pavel Krajcevski

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

#ifndef __CS_IMESH_WATERMESH_H__
#define __CS_IMESH_WATERMESH_H__

/**\file
 * Water mesh object
 */ 

#include "csutil/scf.h"
#include "csutil/scf_implementation.h"

/**\addtogroup meshplugins
 * @{ */

/**
 * The water mesh is a simple mesh representation for a horizontal square. It
 * is best used in conjunction with a shader able to properly simulate the behavior
 * of water. The mesh supports ocean rendering with extremely basic LOD.
 *
 * The different properties that can be set for the waves are given by the 
 * amplitudes, phases, frequencies, and directions of combinations of sine and
 * cosine waves used to permute the vertices. These values get passed to the proper
 * shader via shader variables which permute the vertices in shader vertex programs.
 *
 * These are the wave properties as outlined in section 4.1 in this paper by
 * Jerry Tessendorf, presented in 2004, which uses ideas from Fournier and 
 * Reeves' paper from Siggraph '86 Proceedings:
 *
 * http://www.finelightvisualtechnology.com/docs/coursenotes2004.pdf
 * http://www.iro.umontreal.ca/~poulin/fournier/papers/p75-fournier.pdf
 *	
 * TODO: Move from Gerstner waves to a statistical model representation using
 * Fast Fourier Transforms as described in section 4.3 of Tessendorf's paper.
 *   
 */

struct iWaterFactoryState : public virtual iBase
{
  SCF_INTERFACE (iWaterFactoryState, 2, 0, 0);

  /**
   * Types of water meshes. Note, that setting the type as
   * WATER_TYPE_OCEAN ignores the values specified by 
   * iWaterFactoryState::SetLength(), iWaterFactoryState::SetWidth()
   * and iWaterFactoryState::SetGranularity()
   */
  enum waterMeshType {
	/// Local water, used for a finite amount.
    WATER_TYPE_LOCAL,
    /// Ocean water, used for an infinite amount.
    WATER_TYPE_OCEAN
  };

  /**
   * Invalidates the render buffers. This function should be
   * called when there has been a significant change to the
   * dimensions and or granularity of the water mesh.
   */
  virtual void Invalidate () = 0;

  /// Set the length of the mesh in the x direction.
  virtual void SetLength(uint length) = 0;
  /// Get the length of the mesh in the x direction.
  virtual uint GetLength() = 0;

  /// Set the width of the mesh in the z direction.
  virtual void SetWidth(uint width) = 0;
  /// Get the width of the mesh in the z direction.
  virtual uint GetWidth() = 0;

  /**
   * Get the granularity of the mesh. This determines the
   * number of vertices that get used per unit length. A
   * higher granularity gives nicer effects for wave simulation
   * and fluid dynamics, but is more expensive to draw. Default
   * value is 1.
   */
  virtual void SetGranularity(uint granularity) = 0;
  /// Get the granularity of the mesh. See iWaterFactoryState::SetGranularity();
  virtual uint GetGranularity() = 0;

  /// Set the murkiness (alpha) of the water.
  virtual void SetMurkiness(float murk) = 0;
  /// Get the murkiness (alpha) of the water.
  virtual float GetMurkiness() = 0;
	
  /// Set the water type.
  virtual void SetWaterType(waterMeshType type) = 0;

  /// Returns true if the factory represents an ocean mesh.
  virtual bool isOcean() = 0;

  // TODO: Implement a way to "add" a wave function instead of having three hardcoded.

  /// Set the wave amplitudes.
  virtual void SetAmplitudes(float amp1, float amp2, float amp3) = 0;

  /// Set the wave frequencies.
  virtual void SetFrequencies(float freq1, float freq2, float freq3) = 0;

  /// Set the wave phases.
  virtual void SetPhases(float phase1, float phase2, float phase3) = 0;

  /// Set the wave directions.
  virtual void SetDirections(csVector2 dir1, csVector2 dir2, csVector2 dir3) = 0;

  // Size must be a power of two. Not yet implemented.
  virtual csRef<iTextureWrapper> MakeFresnelTex(int size) = 0;
};

/**
 * This interface describes the API for the water mesh object.
 */
struct iWaterMeshState : public virtual iBase
{
  SCF_INTERFACE (iWaterMeshState, 1, 0, 0);

  /// Specify the normal map to be used with this object.
  virtual void SetNormalMap(iTextureWrapper *map) = 0;

  /// Get a pointer to the normal map to be used with this object.
  virtual iTextureWrapper *GetNormalMap() = 0;
};

/** @} */

#endif // __CS_IMESH_WATERMESH_H__
