/*
 * Cloth mesh object plugin Copyl3ft (C) 2002 by Charles Quarra
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
 */

#ifndef __CLOTHOBJ_H__
#define __CLOTHOBJ_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/tesselat.h"
#include "csutil/refarr.h"
#include "imesh/object.h"
#include "imesh/clothmesh.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "igeom/objmodel.h"
#include "ivideo/vbufmgr.h"

class           csMaterialHandle;
struct G3DTriangleMesh;
struct iObjectRegistry;
struct iGraphics3D;
struct iGraphics2D;
struct iMaterialWrapper;
struct iMeshObject;
class           Integrator;
class           Cloth;

  // ///////////////////////////////////////////////////////////////////////////////
 // -------------csStuffObject declaration
 // --------------------------------------//
// /////////////////////////////////////////////////////////////////////////////// 
// 
// 
// This Class implements:
// 
// iMeshObject:
// essence of any mesh object implementation, isnt?
// 
// iVertexBufferManagerClient:
// well, someone has to handle the buffers
// 
// iObjectModel: 
// ugh, frustvis requires this, and eventually it will be needed
// for collision detection
// 
// i???State:
// well, the first reason i stumbled across for implementing this is:
// how are you supposed to assign materials to the meshobject from the
// client application? 
// //////////////////////////////////////////////////////////////////////////////////////////
class           csStuffObject:public iMeshObject 
{
    G3DTriangleMesh mesh;
    Integrator     *Dynamics;
    Cloth          *Fabric;
    iObjectRegistry *Obj_Reg;
    bool            setup;
    // ----------------------geometrical data
    int             num_vertices;
    int             max_vertices;
    csVector3       max_radius;
    csVector3       shift;

    csTicks         time;
    // ---------------------drawing data
    csVector3      *vertices;
    csVector2      *texels;
    iMaterialWrapper *material;
    csColor        *colors;
    uint            MixMode;
                    csRef < iVertexBuffer > vbuf;
    iVertexBufferManager *vbufmgr;
    iMeshObjectDrawCallback *vis_cb;

    // ------------------BEGIN iVertexBufferManagerClient
    // implementation------------------///
    // 

    struct eiVertexBufferManagerClient:public iVertexBufferManagerClient {
	SCF_DECLARE_EMBEDDED_IBASE(csStuffObject);
	virtual void    ManagerClosing();
    } scfiVertexBufferManagerClient;
    friend struct eiVertexBufferManagerClient;

    // ----------------< Local methods directly connected to this
    // interface >--------//
    // 
    void            SetupVertexBuffer();

    // --------------------------END iVertexBufferManagerClient
    // implementation--------//

    csRef < iMeshObjectFactory > factory;
    csRef < iClothFactoryState > fact_state;
    iBase          *logparent;
    csBox3          camera_bbox;
    csBox3          object_bbox;

    SCF_DECLARE_IBASE;

    // -------------------------< core & initialization methods
    // >----------------------//
    csStuffObject(iMeshObjectFactory * fact);
    virtual ~ csStuffObject();
    bool            Initialize(iObjectRegistry * obj_reg);	// this
    // should
    // call/do 
    // the
    // setup?

    // ------------------------< mesh initialization and run-time
    // evolution >---------//
    void            SetupMesh();
    void            GetRadius(csVector3 & r, csVector3 & c);

    // -----------------------| BEGIN MeshObject Impl
    // |-------------------------------//
    virtual bool    DrawTest(iRenderView * rview, iMovable * movable);
    virtual bool    Draw(iRenderView * rview, iMovable * movable,
			 csZBufMode zbufmode);

    virtual iMeshObjectFactory *GetFactory() const;
    virtual void    UpdateLighting(iLight **, int, iMovable *);
    virtual void    SetVisibleCallback(iMeshObjectDrawCallback *);
    virtual iMeshObjectDrawCallback *GetVisibleCallback() const;
    virtual void    NextFrame(unsigned int, const csVector3& /*pos*/);
    virtual bool    WantToDie() const;
    virtual void    HardTransform(const csReversibleTransform &);
    virtual bool    SupportsHardTransform() const;
    virtual bool    HitBeamOutline(const csVector3 &, const csVector3 &,
				   csVector3 &, float *);
    virtual bool    HitBeamObject(const csVector3 &, const csVector3 &,
				  csVector3 &, float *);
    virtual void    SetLogicalParent(iBase *);
    virtual iBase  *GetLogicalParent() const;
    void            GetObjectBoundingBox(csBox3 & bbox, int type =
					 CS_BBOX_NORMAL);

    // -------------------| BEGIN iObjectModel implementation
    // |----------------------//
class           ObjectModel:public iObjectModel 
{
  SCF_DECLARE_EMBEDDED_IBASE(csStuffObject);
  virtual long    GetShapeNumber() const { return 0; }
  virtual iPolygonMesh *GetPolygonMeshColldet() { return NULL; }
  virtual iPolygonMesh *GetPolygonMeshViscull() { return NULL; }
  virtual csPtr < iPolygonMesh > CreateLowerDetailPolygonMesh(float) { return NULL; }
  virtual void    GetObjectBoundingBox(csBox3 & bbox, int type =
					     CS_BBOX_NORMAL) 
  { scfParent->GetObjectBoundingBox(bbox, type); }
  virtual void    GetRadius(csVector3 & rad, csVector3 & cent) 
  {
    rad = scfParent->max_radius;
    cent.Set(scfParent->shift);
  }
  virtual void AddListener (iObjectModelListener*)
  {
    // @@@ TODO
  }
  virtual void RemoveListener (iObjectModelListener*)
  {
    // @@@ TODO
  }

} scfiObjectModel;
friend class    ObjectModel;

virtual iObjectModel *GetObjectModel() { return &scfiObjectModel; };

virtual bool SetColor(const csColor &) { return false; }
virtual bool GetColor(csColor &) const { return false; }
virtual bool SetMaterialWrapper(iMaterialWrapper * mat)
       { material = mat; return true; }
virtual iMaterialWrapper *GetMaterialWrapper() const { return material; }

// --------------------| BEGIN iClothMeshState implementation// |----------------// 
bool LightsEnabled;
struct csClothMeshState:public iClothMeshState 
  {
    SCF_DECLARE_EMBEDDED_IBASE(csStuffObject);
	// / Set material of mesh.
    virtual void    SetMaterialWrapper(iMaterialWrapper * material) {
    scfParent->material = material;
  };
	// / Get material of mesh.
virtual iMaterialWrapper *GetMaterialWrapper() const { return scfParent->material; };
	// / Set mix mode.
virtual void    SetMixMode(uint mode) { scfParent->MixMode = mode; };
	// / Get mix mode.
virtual uint    GetMixMode() const { return scfParent->MixMode; };
	// / Set lighting.
virtual void    SetLighting(bool l) { scfParent->LightsEnabled = l; };
	// / Is lighting enabled.
virtual bool    IsLighting() const { return scfParent->LightsEnabled; };
	// / Set the color to use. Will be added to the lighting values.
virtual void    SetColor(const csColor &) { };
	// / Get the color.
virtual csColor GetColor() const { return csColor(0, 0, 0); };
  /**
   * Set manual colors. If this is set then lighting will be ignored
   * and so will the color set with SetColor(). In this case you can
   * manipulate the color array manually by calling GetColors().
   */
virtual void    SetManualColors(bool) { };
	// / Are manual colors enabled?
virtual bool    IsManualColors() const { return false; };
 } scfiClothMeshState;
 friend struct csClothMeshState;
};

// ////////////////////////////////////////////////////////////////////////////////////
// // BEGIN StuffFactory class
// ////////////////////////////////////////////////////////////////////////////////////
// //// Things Allocated under the responsability of StuffFactory:
// / StuffFactory::factory_vertices
// / StuffFactory::factory_texels
// //// StuffFactory::factory_colors
// //// StuffFactory::factory_triangles
// /
// / the idea is that at startup each cloth meshobject gets its initial
// state from
// / its factory, and when it needs to reset it (btw, which interface
// could allow me
// / to reset to factory settings?

class           StuffFactory:public iMeshObjectFactory 
{
  public:
    iObjectRegistry * object_reg;

    SCF_DECLARE_IBASE;
    StuffFactory(iBase * parent);
    virtual ~ StuffFactory();
    virtual bool    Initialize(iObjectRegistry * iO_R);
    virtual         csPtr < iMeshObject > NewInstance();

    virtual void    HardTransform(const csReversibleTransform &);
    virtual bool    SupportsHardTransform() const;
    virtual void    SetLogicalParent(iBase *);
    virtual iBase  *GetLogicalParent() const;
    // iClothFactoryState implementation
    // WARNING:::::::::::::::::WARNING:::::::::::::WARNING::::::::::::::
    // INITIAL REFINEMENT SHOULD BE DONE ON THE FACTORY MESHES!!!!!!!
    iMaterialWrapper *material;
    csVector3      *factory_vertices;
    csVector2      *factory_texels;
    csColor        *factory_colors;
    uint            num_vertices;
    csTriangle     *factory_triangles;
    uint            num_triangles;

    class           csClothFactoryState:public iClothFactoryState 
    {
      SCF_DECLARE_EMBEDDED_IBASE(StuffFactory);
	// / Set material of factory.
      virtual void    SetMaterialWrapper(iMaterialWrapper * material) 
      { scfParent->material = material; };
	// / Get material of factory.
      virtual iMaterialWrapper *GetMaterialWrapper() const 
      { return scfParent->material; };
	// / Set the number of vertices to use for this mesh.
      virtual void    SetVertexCount(int n) 
      {
        if (scfParent->factory_vertices) 
          { delete[]scfParent->factory_vertices;  };
        if (scfParent->factory_texels) 
          { delete[]scfParent->factory_texels; };
        if (scfParent->factory_colors) 
          { delete[]scfParent->factory_colors; };
        scfParent->factory_vertices = new csVector3[n];
	scfParent->factory_texels = new csVector2[n];
	scfParent->factory_colors = new csColor[n];
	scfParent->num_vertices = n;
      };
	// / Get the number of vertices for this mesh.
    virtual int     GetVertexCount() const 
      { return scfParent->num_vertices; };
  /**
   * Get the array of vertices. It is legal to modify the vertices
   * in this array. The number of vertices in this array will be
   * equal to the number of vertices set.
   */
    virtual csVector3 *GetVertices() 
      { return scfParent->factory_vertices; };
  /**
   * Get the array of texels. It is legal to modify the texels in this
   * array. The number of texels in this array will be equal to
   * the number of vertices set.
   */
    virtual csVector2 *GetTexels() 
      { return scfParent->factory_texels; };
  /**
   * Get the array of normals. It is legal to modify the normals in this
   * array. The number of normals in this array will be equal to the
   * number of vertices set. Note that modifying the normals is only
   * useful when manual colors are not enabled and lighting is enabled
   * because the normals are used for lighting.
   */
    virtual csVector3 *GetNormals() 
      {  return NULL; };
    // Set the number of triangles to use for this mesh.
    virtual void    SetTriangleCount(int n) 
      {
        if (scfParent->factory_triangles) 
          { delete[]scfParent->factory_triangles; };
        scfParent->factory_triangles = new csTriangle[n];
        scfParent->num_triangles = n;
      };
	// / Get the number of triangles for this mesh.
     virtual int     GetTriangleCount() const 
      { return scfParent->num_triangles; };
  /**
   * Get the array of triangles. It is legal to modify the triangles in this
   * array. The number of triangles in this array will be equal to
   * the number of triangles set.
   */
     virtual csTriangle *GetTriangles() 
      { return scfParent->factory_triangles; };
  /**
   * Get the array of colors. It is legal to modify the colors in this
   * array. The number of colors in this array will be equal to the
   * number of vertices set. Note that modifying the colors will not do
   * a lot if manual colors is not enabled (SetManualColors).
   */
     virtual csColor *GetColors() 
      { return scfParent->factory_colors; };
  /**
   * After making a significant change to the vertices or triangles you
   * probably want to let this object recalculate the bounding boxes
   * and such. This function will invalidate the internal data structures
   * so that they are recomputed.
   */
     virtual void    Invalidate() 
      {	};
  /**
   * Automatically calculate normals based on the current mesh.
   */
     virtual void    CalculateNormals() 
      {	};
  /**
   * Automatically generate a square cloth
   */
     virtual void    GenerateFabric(uint Xsize, uint Ysize) 
      {
        SetVertexCount((Xsize + 1) * (Ysize + 1));
        SetTriangleCount(4 * Xsize * Ysize);
        uint            nverts = scfParent->num_vertices;
        
	float           xindex = 0.0;
        float           yindex = 0.0;
        uint            i;
	uint            j;
	uint            k;
        for (i = 0; i < nverts; i++) 
          {
            scfParent->factory_vertices[i].z = 0.0;	// (xindex-3)*(yindex-3);
            scfParent->factory_vertices[i].x = xindex;
	    scfParent->factory_vertices[i].y = yindex;
            scfParent->factory_texels[i].Set(xindex / 2.1f,
						 yindex / 2.1f);
	    scfParent->factory_colors[i].Set(1.0f, 1.0f, 1.0f);
	    xindex += 2.0 / Xsize;
	    if (((i + 1) % (Xsize + 1)) == 0) 
              {
                xindex = 0.0;
                yindex += 2.0 / Ysize;
		};
	    };
         k = 0;
         for (i = 0; i < Xsize; i++) 
           {
             for (j = 0; j < Ysize; j++) 
               {
                 scfParent->factory_triangles[k].a = i + (Xsize + 1) * j;	
                 // ahh lot of funs with triangles
		 scfParent->factory_triangles[k].b = i + 1 + (Xsize + 1) * j;
		 scfParent->factory_triangles[k].c = i + (Xsize + 1) * (j + 1);
		 k++;

		 scfParent->factory_triangles[k].a = i + (Xsize + 1) * (j + 1);
		 scfParent->factory_triangles[k].b = i + 1 + (Xsize + 1) * j;
                 scfParent->factory_triangles[k].c = i + 1 + (Xsize + 1) * (j + 1);
                 k++;
                    // -----------now anti-clock-wise for drawing the
		    // other side
		 scfParent->factory_triangles[k].a = i + (Xsize + 1) * j;
                 // ahh lot of funs with triangles
                 scfParent->factory_triangles[k].c = i + 1 + (Xsize + 1) * j;
                 scfParent->factory_triangles[k].b = i + (Xsize + 1) * (j + 1);
                 k++;

		 scfParent->factory_triangles[k].a = i + (Xsize + 1) * (j + 1);
                 scfParent->factory_triangles[k].c = i + 1 + (Xsize + 1) * j;
                 scfParent->factory_triangles[k].b = i + 1 + (Xsize + 1) * (j + 1);
                 k++;
                 
		};
	    };
	};			// END GenerateFabric

    } scfiClothFactoryState;
    friend class    csClothFactoryState;
};

class           StuffMeshObjectType:public iMeshObjectType 
{
  public:
    iObjectRegistry * object_reg;

    SCF_DECLARE_IBASE;

    // / Constructor.
    StuffMeshObjectType(iBase *);
    // / Destructor.
    virtual ~ StuffMeshObjectType();
    // / Draw.
    virtual         csPtr < iMeshObjectFactory > NewFactory();
    // / Initialize.
    bool            Initialize(iObjectRegistry * object_reg) {
	StuffMeshObjectType::object_reg = object_reg;
	return true;
    }
    struct eiComponent:public iComponent {
	SCF_DECLARE_EMBEDDED_IBASE(StuffMeshObjectType);
	virtual bool    Initialize(iObjectRegistry * object_reg) {
	    return scfParent->Initialize(object_reg);
	}
    } scfiComponent;
};

#endif				// __CLOTHPLG_H__
