/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#include "csgeom/transfrm.h"

#include "vertexsetup.h"

CS_PLUGIN_NAMESPACE_BEGIN(Particles)
{

  // Different ways to compute the XY coordinates
  class ConstantCameraDir
  {
  public:
    CS_FORCEINLINE ConstantCameraDir ()
    {}

    CS_FORCEINLINE void Init (const csReversibleTransform& o2c,
                              const csVector3& commonDir)
    {
      x = o2c.GetRight ();
      y = o2c.GetUp ();
    }

    CS_FORCEINLINE void Update (const csParticle& particle)
    {}

    CS_FORCEINLINE const csVector3& GetX () const
    {
      return x;
    }

    CS_FORCEINLINE const csVector3& GetY () const
    {
      return y;
    }

  private:
    csVector3 x, y;
  };

  class ExactCameraDir
  {
  public:
    CS_FORCEINLINE ExactCameraDir ()
    {}

    CS_FORCEINLINE void Init (const csReversibleTransform& o2c,
      const csVector3& commonDir)
    {
      camCenter = o2c.GetOrigin ();
      camUp = o2c.GetUp ();
    }

    CS_FORCEINLINE void Update (const csParticle& particle)
    {
      const csVector3 camDir = particle.position - camCenter;

      // Calculate x and y dir
      x = (camUp % camDir).Unit ();
      y = (camDir % x).Unit ();
    }

    CS_FORCEINLINE const csVector3& GetX () const
    {
      return x;
    }

    CS_FORCEINLINE const csVector3& GetY () const
    {
      return y;
    }

  private:
    csVector3 camCenter, camUp;
    csVector3 x, y;
  };

  class CommonUpConstantCameraDir
  {
  public:
    CS_FORCEINLINE CommonUpConstantCameraDir ()
    {}

    CS_FORCEINLINE void Init (const csReversibleTransform& o2c,
      const csVector3& commonDir)
    {
      x = (commonDir % o2c.GetFront ()).Unit ();
      y = commonDir;
    }

    CS_FORCEINLINE void Update (const csParticle& particle)
    {
    }

    CS_FORCEINLINE const csVector3& GetX () const
    {
      return x;
    }

    CS_FORCEINLINE const csVector3& GetY () const
    {
      return y;
    }

  private:    
    csVector3 x, y, camCenter;
  };

  class CommonUpExactCameraDir
  {
  public:
    CS_FORCEINLINE CommonUpExactCameraDir ()
    {}

    CS_FORCEINLINE void Init (const csReversibleTransform& o2c,
      const csVector3& commonDir)
    {
      camCenter = o2c.GetOrigin ();
      y = commonDir;
    }

    CS_FORCEINLINE void Update (const csParticle& particle)
    {
      const csVector3 camDir = particle.position - camCenter;

      // Calculate x and y dir

      x = (y % camDir).Unit ();
    }

    CS_FORCEINLINE const csVector3& GetX () const
    {
      return x;
    }

    CS_FORCEINLINE const csVector3& GetY () const
    {
      return y;
    }

  private:    
    csVector3 x, y, camCenter;
  };

  class IndividualUpExactCameraDir
  {
  public:
    CS_FORCEINLINE IndividualUpExactCameraDir ()
    {}

    CS_FORCEINLINE void Init (const csReversibleTransform& o2c,
      const csVector3& commonDir)
    {
      camCenter = o2c.GetOrigin ();
    }

    CS_FORCEINLINE void Update (const csParticle& particle)
    {
      const csVector3 camDir = particle.position - camCenter;

      // Calculate x and y dir
      float vnSq = particle.linearVelocity.SquaredNorm ();
      if (vnSq == 0)
        y.Set (0.0f, 1.0f, 0.0f);
      else
        y = particle.linearVelocity / sqrtf(vnSq);

      x = (y % camDir).Unit ();
    }

    CS_FORCEINLINE const csVector3& GetX () const
    {
      return x;
    }

    CS_FORCEINLINE const csVector3& GetY () const
    {
      return y;
    }

  private:    
    csVector3 x, y, camCenter;
  };

  class IndividualOrientation
  {
    public:
    CS_FORCEINLINE IndividualOrientation ()
    {}

    CS_FORCEINLINE void Init (const csReversibleTransform& o2c,
      const csVector3& commonDir)
    {
    }

    CS_FORCEINLINE void Update (const csParticle& particle)
    {
      const csMatrix3 base = particle.orientation.GetMatrix ();
      x = base.Col1 ();
      y = base.Col2 ();
    }

    CS_FORCEINLINE const csVector3& GetX () const
    {
      return x;
    }

    CS_FORCEINLINE const csVector3& GetY () const
    {
      return y;
    }

  private:    
    csVector3 x, y;
  };

  class IndividualOrientationForward
  {
    public:
    CS_FORCEINLINE IndividualOrientationForward ()
    {}

    CS_FORCEINLINE void Init (const csReversibleTransform& o2c,
      const csVector3& commonDir)
    {
      camCenter = o2c.GetOrigin ();
    }

    CS_FORCEINLINE void Update (const csParticle& particle)
    {
      const csVector3 camDir = particle.position - camCenter;

      const csMatrix3 base = particle.orientation.GetMatrix ();
      if (base.Col3 () * camDir > 0)
        x = base.Col1 ();
      else
        x = -base.Col1 ();
       
      
      y = base.Col2 ();
    }

    CS_FORCEINLINE const csVector3& GetX () const
    {
      return x;
    }

    CS_FORCEINLINE const csVector3& GetY () const
    {
      return y;
    }

  private:    
    csVector3 x, y, camCenter;
  };


  // Different size computers

  class ConstantParticleSize
  {
  public:
    CS_FORCEINLINE ConstantParticleSize ()
    {}

    CS_FORCEINLINE void Init (const csVector2& s)
    {
      size = s;
    }

    CS_FORCEINLINE void Update (const csParticle& particle, 
      const csParticleAux& aux)
    {}

    CS_FORCEINLINE float GetX () const
    {
      return size.x;
    }

    CS_FORCEINLINE float GetY () const
    {
      return size.y;
    }

  private:
    csVector2 size;
  };

  class IndividualParticleSize
  {
  public:
    CS_FORCEINLINE IndividualParticleSize ()
    {}

    CS_FORCEINLINE void Init (const csVector2& s)
    {}

    CS_FORCEINLINE void Update (const csParticle& particle, 
      const csParticleAux& aux)
    {
      size = aux.particleSize;
    }

    CS_FORCEINLINE float GetX () const
    {
      return size.x;
    }

    CS_FORCEINLINE float GetY () const
    {
      return size.y;
    }

  private:
    csVector2 size;
  };


  


  template<class ParticleDirT, class ParticleSizeT>
  class BaseVertexSetup : public iVertexSetup
  {
  public:
    CS_FORCEINLINE_TEMPLATEMETHOD
    BaseVertexSetup ()
    {}

    CS_FORCEINLINE_TEMPLATEMETHOD
    virtual void Init (const csReversibleTransform& o2c, const csVector3& commonDir,
      const csVector2& particleSize)
    {
      partDir.Init (o2c, commonDir);
      partSize.Init (particleSize);
    }

    CS_FORCEINLINE_TEMPLATEMETHOD
    void Update (const csParticle& particle, const csParticleAux& aux)
    {
      partDir.Update (particle);
      partSize.Update (particle, aux);
    }
   
  protected:
    typedef BaseVertexSetup<ParticleDirT, ParticleSizeT> Base;
    ParticleDirT partDir;
    ParticleSizeT partSize;
  };

  // Normal unrotated vertices
  template<class ParticleDirT, class ParticleSizeT>
  class UnrotatedVertexSetup : public BaseVertexSetup<ParticleDirT, ParticleSizeT>
  {
  public:
    CS_FORCEINLINE_TEMPLATEMETHOD
    UnrotatedVertexSetup ()
      : BaseVertexSetup<ParticleDirT, ParticleSizeT> ()
    {}

    CS_FORCEINLINE_TEMPLATEMETHOD
    void SetupVertices (const csParticleBuffer particleBuffer,
      csVector3* vertexBuffer)
    {
      for (size_t pidx = 0; pidx < particleBuffer.particleCount; ++pidx)
      {
        const csParticle& particle = particleBuffer.particleData[pidx];
        const csParticleAux& aux = particleBuffer.particleAuxData[pidx];

        this->Update (particle, aux);
        
        const csVector3 partX = this->partDir.GetX () * this->partSize.GetX ();
        const csVector3 partY = this->partDir.GetY () * this->partSize.GetY ();

        const csVector3& particleCenter = particle.position;

        vertexBuffer[0] = particleCenter - partX + partY;
        vertexBuffer[1] = particleCenter + partX + partY;
        vertexBuffer[2] = particleCenter + partX - partY;
        vertexBuffer[3] = particleCenter - partX - partY;

        vertexBuffer+=4;
      }
    }
  };

  // Rotated vertices
  template<class ParticleDirT, class ParticleSizeT>
  class RotatedVertexSetup : public BaseVertexSetup<ParticleDirT, ParticleSizeT>
  {
  public:
    CS_FORCEINLINE_TEMPLATEMETHOD
    RotatedVertexSetup ()
      : BaseVertexSetup<ParticleDirT, ParticleSizeT> ()
    {}

    CS_FORCEINLINE_TEMPLATEMETHOD
    void SetupVertices (const csParticleBuffer particleBuffer,
      csVector3* vertexBuffer)
    {
      for (size_t pidx = 0; pidx < particleBuffer.particleCount; ++pidx)
      {
        const csParticle& particle = particleBuffer.particleData[pidx];
        const csParticleAux& aux = particleBuffer.particleAuxData[pidx];

        this->Update (particle, aux);

        csVector3 tmpV;
        float rot;
        particle.orientation.GetAxisAngle (tmpV, rot);
        const float r = rot + PI/4;
        const float s = sinf(r);
        const float c = cosf(r);

        const float pX = this->partSize.GetX ();
        const float pY = this->partSize.GetX ();

        csVector3 partX = this->partDir.GetX () * (pX*c - pY*s);
        csVector3 partY = this->partDir.GetY () * (pX*s + pY*c);

        const csVector3& particleCenter = particle.position;

        vertexBuffer[0] = particleCenter - partX + partY;
        vertexBuffer[2] = particleCenter + partX - partY;

        partX = this->partDir.GetX () * (-pX*c - pY*s);
        partY = this->partDir.GetY () * (-pX*s + pY*c);

        vertexBuffer[1] = particleCenter - partX + partY;
        vertexBuffer[3] = particleCenter + partX - partY;

        vertexBuffer+=4;
      }
    }
  };


  
  template<class PartDir, class PartSize>
  iVertexSetup* GetVertexSetupFunc2 (csParticleRotationMode rotMode)
  {
    if (rotMode == CS_PARTICLE_ROTATE_VERTICES)
    {
      return new RotatedVertexSetup<PartDir, PartSize> ();
    }
    else
    {
      return new UnrotatedVertexSetup<PartDir, PartSize> ();
    }

    return 0;
  }

  template<class PartSize>
  iVertexSetup* GetVertexSetupFunc1 (csParticleRotationMode rotMode, 
    csParticleRenderOrientation orient)
  {
    switch (orient)
    {
    case CS_PARTICLE_CAMERAFACE:
      {
        return GetVertexSetupFunc2<ExactCameraDir, PartSize> (rotMode);
      }
      break;
    case CS_PARTICLE_CAMERAFACE_APPROX:
      {
        return GetVertexSetupFunc2<ConstantCameraDir, PartSize> (rotMode);
      }
      break;
    case CS_PARTICLE_ORIENT_COMMON:
      {
        return GetVertexSetupFunc2<CommonUpExactCameraDir, PartSize> (rotMode);
      }
      break;
    case CS_PARTICLE_ORIENT_COMMON_APPROX:
      {
        return GetVertexSetupFunc2<CommonUpConstantCameraDir, PartSize> (rotMode);
      }
      break;
    case CS_PARTICLE_ORIENT_VELOCITY:
      {
        return GetVertexSetupFunc2<IndividualUpExactCameraDir, PartSize> (rotMode);
      }
      break;
    case CS_PARTICLE_ORIENT_SELF:
      {
        return GetVertexSetupFunc2<IndividualOrientation, PartSize> (rotMode);
      }
      break;
    case CS_PARTICLE_ORIENT_SELF_FORWARD:
      {
        return GetVertexSetupFunc2<IndividualOrientationForward, PartSize> (rotMode);
      }
      break;
    }

    return 0;
  }


  // Function to get a pointer to a vertex setup
  iVertexSetup* GetVertexSetupFunc (csParticleRotationMode rotMode, 
    csParticleRenderOrientation orient, bool individualSize)
  {
    if (individualSize)
    {
      return GetVertexSetupFunc1<IndividualParticleSize> (rotMode, orient);
    }
    else
    {
      return GetVertexSetupFunc1<ConstantParticleSize> (rotMode, orient);
    }

    return 0;
  }

}
CS_PLUGIN_NAMESPACE_END(Particles)
