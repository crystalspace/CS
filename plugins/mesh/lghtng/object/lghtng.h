/*
    Copyright (C) 2003 by Boyan Hristov

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

#ifndef __CS_LGHTNG_H__
#define __CS_LGHTNG_H__

#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"
#include "csgeom/objmodel.h"
#include "csutil/cscolor.h"
#include "csutil/refarr.h"
#include "csutil/randomgen.h"
#include "imesh/object.h"
#include "imesh/lghtng.h"
#include "imesh/particle.h"
#include "imesh/genmesh.h"
#include "ivideo/graph3d.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

struct iMaterialWrapper;
struct iCamera;
class csLightningMeshObjectFactory;

/**
 * Lightning mesh object.
 */
class csLightningMeshObject : public iMeshObject
{
  csLightningMeshObjectFactory *LightningFact;
  csRef<iMeshObject> GenMesh;
  csRef<iGeneralMeshState> GenState;
  csRef<iMeshObjectFactory> ifactory;
  iBase* logparent;
  csLightningMeshObjectFactory* factory;
  csFlags flags;

  csRef<iMaterialWrapper> material;
  uint MixMode;
  bool initialized;
  iMeshObjectDrawCallback* vis_cb;
  float length;  
  float wildness;
  float vibration;
  float bandwidth;
  csVector3 origin;
  csVector3 directional;
  int points;

  
  void SetupObject ();

  /// Get the bounding box in transformed space.
  void GetTransformedBoundingBox (long cameranr, long movablenr,
      const csReversibleTransform& trans, csBox3& cbox);

public:
  /// Constructor.
  csLightningMeshObject (csLightningMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csLightningMeshObject ();
  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& bbox);
  void GetRadius (csVector3& rad, csVector3& cent);

  ///--------------------- iMeshObject implementation ------------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return ifactory; }
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
  virtual csRenderMesh **GetRenderMeshes (int &n, iRenderView*, 
    iMovable*, uint32);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    if (cb) cb->IncRef ();
    if (vis_cb) vis_cb->DecRef ();
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }
  virtual void NextFrame (csTicks current_time, const csVector3& /*pos*/);
  virtual bool WantToDie () const { return false; }
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3&, const csVector3&,
        csVector3&, float*)
  { return false; }
  virtual bool HitBeamObject (const csVector3&, const csVector3&,
  	csVector3&, float*, int* = 0) { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  //------------------------- iObjectModel implementation ----------------
  class ObjectModel : public csObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csLightningMeshObject);
    virtual void GetObjectBoundingBox (csBox3& bbox)
    {
      scfParent->GetObjectBoundingBox (bbox);
    }
    virtual void SetObjectBoundingBox (const csBox3& bbox)
    {
      scfParent->SetObjectBoundingBox (bbox);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }
  virtual bool SetColor (const csColor&) { return false; }
  virtual bool GetColor (csColor&) const { return false; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void InvalidateMaterialHandles () { }
  /**
   * see imesh/object.h for specification. The default implementation
   * does nothing.
   */
  virtual void PositionChild (iMeshObject* child,csTicks current_time) { }

  //------------------------- iLightningState implementation ----------------
  class LightningState : public iLightningState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csLightningMeshObject);
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    { 
      return scfParent->material; 
    }
    virtual void SetMixMode (uint mode) { scfParent->MixMode = mode; }
    virtual uint GetMixMode () const { return scfParent->MixMode; }
    virtual void SetOrigin(const csVector3& pos) { scfParent->origin = pos; }
    virtual const csVector3& GetOrigin() const {return scfParent->origin;}
    virtual int GetPointCount() const
    {      
      return 0;
    }
    ///
    virtual float GetLength() const
    {
      return scfParent->length;
    }
    ///
    virtual void SetLength(float value)
    {
      scfParent->length = value;
    }
    ///
    virtual void SetPointCount(int n)
    {      
    }    
    ///
    virtual float GetWildness() const
    {
      return scfParent->wildness;
    }
    ///
    virtual void SetWildness(float value)
    {
    }
    ///
    virtual float GetVibration() const
    {
      return scfParent->vibration;
    }
    ///
    virtual void SetVibration(float value)
    {
    }
    ///
    virtual void SetDirectional(const csVector3 &pos)
    {
      scfParent->directional = pos;
    }
    ///
    virtual const csVector3& GetDirectional()
    {      
      return scfParent->directional;
    }
    virtual csTicks GetUpdateInterval () const
    {
      return 0;
    }
    virtual void SetUpdateInterval (csTicks value)
    {
      
    }
    virtual float GetBandWidth () const
    {
      return scfParent->bandwidth;
    }
    virtual void SetBandWidth (float value)
    {
      ///
    }
  } scfiLightningState;
  friend class LightningState;
};

/**
 * Factory for 2D sprites. This factory also implements iLightningFactoryState.
 */
class csLightningMeshObjectFactory : public iMeshObjectFactory
{
private:

  int MaxPoints;
  float glowsize;
  float vibrate;
  float wildness;
  float length;
  float bandwidth;
  csTicks update_interval;
  csTicks update_counter;
  csRef<iMaterialWrapper> material;
  csRandomGen rand;

  csRef<iMeshObjectFactory> GenMeshFact;
  csRef<iGeneralFactoryState> GenFactState;

  uint MixMode;
  /// Lightning state info
  csVector3 origin;
  csVector3 directional;  
  iBase* logparent;  
  iMeshObjectType* lghtng_type;
  csFlags flags;

  void CalculateFractal (int left, int right, float lh, float rh, int xyz,
  	csVector3 *Vertices);
  void CalculateFractal ();

public:
  /// Constructor.
  csLightningMeshObjectFactory (iMeshObjectType *pParent,
  	iObjectRegistry *object_registry);
  /// Destructor.
  virtual ~csLightningMeshObjectFactory ();
  /// Get the material for this 2D sprite.
  iMaterialWrapper* GetMaterialWrapper () const { return material; }
  /// Get mixmode.
  uint GetMixMode () const { return MixMode; }
  /// Get the origin
  const csVector3& GetOrigin () const { return origin; }
  iMeshObjectFactory *GetMeshFactory () { return GenMeshFact; }
  const csVector3& GetDirectional () { return directional; }
  /// Get length
  const float GetLength () { return length; }
  /// Get number of points
  const int GetPointCount () { return MaxPoints; }
  /// Get vibration
  const float GetVibration () { return vibrate; }
  /// Get wildness
  const float GetWildness () { return wildness; }  
  /// 
  const float GetBandWidth () { return bandwidth; }

  //------------------------ iMeshObjectFactory implementation --------------
  SCF_DECLARE_IBASE;

  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual iObjectModel* GetObjectModel () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return lghtng_type; }
  virtual void NextFrame (csTicks CurrentTime);
  virtual void Invalidate ()
  {     
    if (GenMeshFact)
    {
      GenFactState = SCF_QUERY_INTERFACE(
          GenMeshFact, iGeneralFactoryState);      
      GenFactState->SetVertexCount(MaxPoints * 2);
      GenFactState->SetTriangleCount((MaxPoints - 1) * 2);
            
      csVector2 *Texels = GenFactState->GetTexels();
      csColor   *Colors = GenFactState->GetColors();
      //csVector3 *Vertices = GenFactState->GetVertices(); //@@@ Unused

      int i;
          
      for (i = 0; i < MaxPoints; i++ )
      {
        *Texels = csVector2 (i & 1, 0);
        Texels++;
        *Texels = csVector2 (i & 1, 1);
        Texels++;

        *Colors = csColor (1.f, 1.f, 1.f);
        Colors++;
        *Colors = csColor (1.f, 1.f, 1.f);
        Colors++;
      }

      csTriangle *Triangles = GenFactState->GetTriangles();
      
      for ( i = 0; i < (MaxPoints - 1) * 2; i += 2 )
      {
        Triangles->a = i;
        Triangles->b = i + 2;
        Triangles->c = i + 1;
        Triangles++;
        Triangles->a = i + 2;
        Triangles->b = i + 3;
        Triangles->c = i + 1;
        Triangles++;
      }

      CalculateFractal ();
      GenFactState->CalculateNormals ();
      GenFactState->Invalidate ();
    }
  }

  //------------------------- iLightningFactoryState implementation ----------------
  class LightningFactoryState : public iLightningFactoryState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csLightningMeshObjectFactory);

    csTicks update_counter;

    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    {
      scfParent->material = material;
    }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    { return scfParent->material; }
    virtual void SetMixMode (uint mode) { scfParent->MixMode = mode; }
    virtual uint GetMixMode () const { return scfParent->MixMode; }
    virtual void SetOrigin(const csVector3& pos) 
    {       
      scfParent->origin = pos; 
      scfParent->Invalidate();
    }
    virtual const csVector3& GetOrigin () const 
    {      
      return scfParent->origin;
    }

    virtual float GetLength () const
    {
      return scfParent->length;
    }
    virtual void SetLength (float value)
    {
      scfParent->length = value;
      scfParent->Invalidate ();
    }
    virtual int GetPointCount() const
    {
      return scfParent->MaxPoints;
    }
    virtual void SetPointCount (int n)
    {      
      scfParent->MaxPoints = n;
      scfParent->Invalidate ();
    }

    virtual float GetWildness () const
    {
      return scfParent->wildness;
    }
    virtual void SetWildness(float value)
    {
      scfParent->wildness = value;
    }
    
    virtual float GetVibration () const
    {
      return scfParent->vibrate;
    }
        
    virtual void SetVibration (float value)
    {
      scfParent->vibrate = value;
    }

    virtual void SetDirectional (const csVector3 &pos)
    {
      scfParent->directional = pos;
    }

    virtual const csVector3& GetDirectional ()
    {
      return scfParent->directional;
    }
    virtual csTicks GetUpdateInterval () const
    {
      return scfParent->update_interval;
    }
    virtual void SetUpdateInterval (csTicks value)
    {
      scfParent->update_interval = value;
    }
    virtual float GetBandWidth () const
    {
      return scfParent->bandwidth;
    }
    virtual void SetBandWidth (float value)
    {
      scfParent->bandwidth = value;
    }
  } scfiLightningFactoryState;
  friend class LightningFactoryState;
};

class csLightningMeshObjectType : public iMeshObjectType
{
  iObjectRegistry *Registry;
public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csLightningMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csLightningMeshObjectType ();
  /// New Factory.
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry *object_reg)
  {
    Registry = object_reg;
    return true;
  }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csLightningMeshObjectType);
    virtual bool Initialize (iObjectRegistry *object_reg) 
    {      
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // __CS_LGHTNG_H__
