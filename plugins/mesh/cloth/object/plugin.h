/*
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

#ifndef __TEST_H__
#define __TEST_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/tesselat.h"
#include "imesh/object.h"
#include "imesh/genmesh.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "igeom/objmodel.h"
#include "ivideo/vbufmgr.h"

class csMaterialHandle;
struct G3DTriangleMesh;
struct iObjectRegistry;
struct iGraphics3D;
struct iGraphics2D;
struct iMaterialWrapper;
struct iMeshObject;
struct ClothIntegrator;

  /////////////////////////////////////////////////////////////////////////////////
 //-------------csStuffObject declaration --------------------------------------//
/////////////////////////////////////////////////////////////////////////////////    
//     This Class implements:
//       
//                     iMeshObject:
//                       essence of any mesh object implementation, isnt?
//                       
//                     iVertexBufferManagerClient:
//                       well, someone has to handle the buffers
//                       
//                     iObjectModel:    
//                       ugh, frustvis requires this, and eventually it will be needed
//                       for collision detection
//                     
//                     i???State:
//                       well, the first reason i stumbled across for implementing this is:
//                       how are you supposed to assign materials to the meshobject from the
//                       client application?  
////////////////////////////////////////////////////////////////////////////////////////////
class csStuffObject : public iMeshObject 
{
	
G3DTriangleMesh mesh;
ClothIntegrator* Integrator;
iObjectRegistry* Obj_Reg;
bool setup;
//----------------------geometrical data
int num_vertices;
int max_vertices;
                csVector3 max_radius;
		csVector3 shift;
int Xsize;
int Ysize;
float time;
//---------------------drawing data
csVector3* vertices;
csVector2* texels;
iMaterialWrapper* material;
csColor* colors;
 uint MixMode;
iVertexBuffer* vbuf;
  iVertexBufferManager* vbufmgr;
  
   iMeshObjectDrawCallback* vis_cb;

  //------------------BEGIN iVertexBufferManagerClient implementation------------------///
  //
  
  struct eiVertexBufferManagerClient : public iVertexBufferManagerClient
  {
    SCF_DECLARE_EMBEDDED_IBASE (csStuffObject);
    virtual void ManagerClosing ();
  }scfiVertexBufferManagerClient;
  friend struct eiVertexBufferManagerClient;

  //----------------< Local methods directly connected to this interface >--------//
  //
  void SetupVertexBuffer ();  
  
  //--------------------------END iVertexBufferManagerClient implementation--------//
  
  iMeshObjectFactory* factory;
  iBase* logparent;
  csBox3 camera_bbox;
  csBox3 object_bbox;

SCF_DECLARE_IBASE;     

 //-------------------------< core & initialization methods >----------------------//
  csStuffObject(iMeshObjectFactory* fact);

  virtual ~csStuffObject();

  bool Initialize(iObjectRegistry* obj_reg);  //this should call/do the setup?

  //------------------------< mesh initialization and run-time evolution >---------//
  void SetupMesh();
  void UpdateMesh();
  void GetRadius(csVector3& r, csVector3& c);
  
  //-----------------------| BEGIN MeshObject Impl |-------------------------------//

  virtual bool DrawTest(iRenderView *rview, iMovable *movable);
  virtual bool Draw(iRenderView *rview, iMovable *movable, csZBufMode zbufmode);



  virtual iMeshObjectFactory *GetFactory () const;
  virtual void UpdateLighting (iLight **,int, iMovable *);
  virtual void SetVisibleCallback(iMeshObjectDrawCallback *);
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const;
  virtual void NextFrame (unsigned int);
  virtual bool WantToDie () const;
  virtual void HardTransform (const csReversibleTransform &);
  virtual bool SupportsHardTransform () const;
  virtual bool HitBeamOutline (const csVector3 &, const csVector3 &, csVector3 &, float *);
  virtual bool HitBeamObject (const csVector3 &, const csVector3 &, csVector3 &, float *);
  virtual void SetLogicalParent (iBase *);
  virtual iBase *GetLogicalParent () const;

     void GetObjectBoundingBox(csBox3& bbox, int type = CS_BBOX_NORMAL);

   //-------------------| BEGIN iObjectModel implementation |----------------------//
  class ObjectModel : public iObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csStuffObject);
    virtual long GetShapeNumber () const { return 0; }
    virtual iPolygonMesh* GetPolygonMesh () { return NULL; }
    virtual iPolygonMesh* GetSmallerPolygonMesh () { return NULL; }
    virtual iPolygonMesh* CreateLowerDetailPolygonMesh (float) { return NULL; }
    virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL)
    {
      scfParent->GetObjectBoundingBox (bbox, type);
      	printf(" exiting ObjectModel:GOBB \n");
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      rad = scfParent->max_radius;
      cent.Set (scfParent->shift);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel *GetObjectModel () { return &scfiObjectModel; };

   //--------------------| BEGIN iGeneralMeshState implementation |----------------//
    struct eiGeneralMeshState : public iGeneralMeshState
  {
	  SCF_DECLARE_EMBEDDED_IBASE (csStuffObject);
  /// Set material of mesh.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) { scfParent->material=material; };
  /// Get material of mesh.
  virtual iMaterialWrapper* GetMaterialWrapper () const { return scfParent->material; };
  /// Set mix mode.
  virtual void SetMixMode (uint mode) { scfParent->MixMode=mode; };
  /// Get mix mode.
  virtual uint GetMixMode () const { return scfParent->MixMode; };

  /// Set lighting.
  virtual void SetLighting (bool l) {};
  /// Is lighting enabled.
  virtual bool IsLighting () const { return false; };
  /// Set the color to use. Will be added to the lighting values.
  virtual void SetColor (const csColor& col) {};
  /// Get the color.
  virtual csColor GetColor () const { return csColor(0,0,0); };
  /**
   * Set manual colors. If this is set then lighting will be ignored
   * and so will the color set with SetColor(). In this case you can
   * manipulate the color array manually by calling GetColors().
   */
  virtual void SetManualColors (bool m) {};
  /// Are manual colors enabled?
  virtual bool IsManualColors () const { return false; };
} scfiGeneralMeshState;
  friend class eiGeneralMeshState;
};


//////////////////////////////////////////////////////////////////////////////////////
////       BEGIN StuffFactory class
//////////////////////////////////////////////////////////////////////////////////////
class StuffFactory : public iMeshObjectFactory
{
public:
iObjectRegistry* object_reg;

SCF_DECLARE_IBASE;
 StuffFactory(iBase* parent);
 virtual ~StuffFactory();
  virtual bool Initialize(iObjectRegistry* iO_R);
  virtual iMeshObject* NewInstance();

  virtual void HardTransform (const csReversibleTransform &);
  virtual bool SupportsHardTransform() const;
  virtual void SetLogicalParent(iBase *);
  virtual iBase *GetLogicalParent () const;

 struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (StuffFactory);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

class StuffMeshObjectType : public iMeshObjectType
{
public:
  iObjectRegistry* object_reg;

  SCF_DECLARE_IBASE;

  /// Constructor.
  StuffMeshObjectType (iBase*);
  /// Destructor.
  virtual ~StuffMeshObjectType ();
  /// Draw.
  virtual iMeshObjectFactory* NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg)
  {
    StuffMeshObjectType::object_reg = object_reg;
    return true;
  }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(StuffMeshObjectType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};


#endif // __TEST_H__
