
#include "cssysdef.h"
#include "cloth.h"
#include "igeom/polymesh.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/tesselat.h"
#include "imesh/object.h"
#include "imesh/clothmesh.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "igeom/objmodel.h"
#include "ivideo/vbufmgr.h"

/*
 * bool Cloth::AddConstraint( int v0, int v1 , Constraint** edge ) { if
 * (v0==v1) { *edge=NULL; return false; }; Constraint* first; Constraint* 
 * p; p=first=(Constraint*)Edges->GetFirstItem(); do { if ((p->v0==v0) ||
 * (p->v0==v1)) { if ((p->v1==v0) || (p->v1==v1)) { *edge=p; return
 * false; }; }; p=(Constraint*)Edges->GetNextItem(); } while (p!=first);
 * p=new Constraint( v0 , v1 ); Edges->AddItem( (void*) p ); *edge=p;
 * return true; };
 */

bool
Cloth::AddConstraint(int v0, int v1, Constraint ** edge, int *pos)
{
    if (v0 == v1) 
      {
	*edge = NULL;
	return false;
    };
    Constraint     *p;
    int             size = Edges->Length();
    for (int i = 0; i < size; i++) 
      {
	p = (Constraint *) Edges->Get(i);
	if ((p->v0 == v0) || (p->v0 == v1)) 
          {
	    if ((p->v1 == v0) || (p->v1 == v1)) 
              {
		*edge = p;
		*pos = i;
		return false;
	    };
	};
    };
    p = new Constraint(v0, v1);
    *pos = Edges->Push((void *) p);
    *edge = p;
    return true;
};

bool
Cloth::AddShearConstraint(int v0, int v1, Constraint ** edge, int *pos)
{
    if (v0 == v1) 
      {
	*edge = NULL;
	return false;
    };
    Constraint     *p;
    int             size = Shear_Neighbours->Length();
    for (int i = 0; i < size; i++) 
      {
	p = (Constraint *) Shear_Neighbours->Get(i);
	if ((p->v0 == v0) || (p->v0 == v1)) 
          {
	    if ((p->v1 == v0) || (p->v1 == v1)) 
              {
		*edge = p;
		*pos = i;
		return false;
	    };
	};
    };
    p = new Constraint(v0, v1);
    *pos = Shear_Neighbours->Push((void *) p);
    *edge = p;
    return true;
};

Cloth::Cloth(iClothFactoryState * mesh,
	     csVector3 & Sh, csBox3 & box, csVector3 grav)
{
    shift = &Sh;
    object_bbox = &box;
    gravity = grav;
    //printf(" CREATING the cloth  ... \n");
    int            i;
    vertices = NULL;
    triangles = NULL;
    csVector3      *verts = mesh->GetVertices();
    nverts = mesh->GetVertexCount();
    csTriangle     *tris = mesh->GetTriangles();
    int            tri_count = mesh->GetTriangleCount();

    Edges = new csBasicVector(nverts + 16, 16);
    Shear_Neighbours = new csBasicVector(nverts - 32, 16);
    nedges = 0;
    vertices = new csVector3[nverts];

    for (i = 0; i < nverts; i++) 
      {
	vertices[i].Set(verts[i].x, verts[i].y, verts[i].z);
    };

    ntris = 2 * tri_count;	// here we care about double-facing the
				// mesh
    triangles = new csTriangle[ntris];
    Triangle2EdgeRef = new int *[tri_count];	// this will be needed 
    // when 2nd neighbours are
    // considered
    // if dynamic refinement is considered,
    // then i think this can be deleted after
    // the creation of neighbour constraints 

    int             index = 0;
    int             v;
    Constraint     *current;
    bool            isNew;
    for (i = 0; i < tri_count; i++) 
      {
	triangles[2 * i].a = tris[i].a;
	triangles[2 * i].b = tris[i].b;
	triangles[2 * i].c = tris[i].c;
	// here triangles are mabe double-sided, shouldn't we check for
	// duplicated triangles?
	triangles[2 * i + 1].a = tris[i].a;
	triangles[2 * i + 1].b = tris[i].c;
	triangles[2 * i + 1].c = tris[i].b;

	Triangle2EdgeRef[i] = new int[3];
        
	isNew =
	    AddConstraint(tris[i].a, tris[i].b, &current,
			  &Triangle2EdgeRef[i][0]);
	if (isNew) 
          {
	    current->L0 = (verts[tris[i].a] - verts[tris[i].b]).Norm();
	    nedges++;
	};
        
	isNew =
	    AddConstraint(tris[i].b, tris[i].c, &current,
			  &Triangle2EdgeRef[i][1]);
	if (isNew) 
          {
	    current->L0 = (verts[tris[i].b] - verts[tris[i].c]).Norm();
	    nedges++;
	};
        
	isNew =
	    AddConstraint(tris[i].c, tris[i].a, &current,
			  &Triangle2EdgeRef[i][2]);
	if (isNew) 
          {
	    current->L0 = (verts[tris[i].c] - verts[tris[i].a]).Norm();
	    nedges++;
	};
	
    };
    int            j = Edges->Length();
    int            k;
    Edge2TriangleRef = new int *[j];
    for (i = 0; i < j; i++) 
      {
	Edge2TriangleRef[i] = new int[2];
	Edge2TriangleRef[i][0] = -1;
	Edge2TriangleRef[i][1] = -1;
    };

    for (i = 0; i < tri_count; i++) 
      {
	for (k = 0; k < 3; k++) 
          {
	    j = Triangle2EdgeRef[i][k];
	    if (Edge2TriangleRef[j][0] == -1) 
              {
		Edge2TriangleRef[j][0] = i;
	    } else 
            {
		Edge2TriangleRef[j][1] = i;
	    };
	};
	delete[]Triangle2EdgeRef[i];
    };
    delete[]Triangle2EdgeRef;

    j = Edges->Length();
    int             s0;
    int             s1;
    int             tri[7];
    csBitArray      shared(6);
    int             m;
    // printf(" csBitArray? %u %u \n",sizeof(csBitArray) , sizeof( shared
    for (i = 0; i < j; i++) 
      {
	if (Edge2TriangleRef[i][1] != -1) 
          {
	    // the factor 2* comes from the fact that triangles are stored 
	    // double-sided 
	    tri[0] = triangles[2 * Edge2TriangleRef[i][0]].a;
	    tri[1] = triangles[2 * Edge2TriangleRef[i][0]].b;
	    tri[2] = triangles[2 * Edge2TriangleRef[i][0]].c;

	    tri[3] = triangles[2 * Edge2TriangleRef[i][1]].a;
	    tri[4] = triangles[2 * Edge2TriangleRef[i][1]].b;
	    tri[5] = triangles[2 * Edge2TriangleRef[i][1]].c;
	    // printf(" L:%u E2T %u T:%u,%u,%u : %u,%u,%u
	    // \n",j,Edge2TriangleRef[i][1], tri[0],tri[1],tri[2] ,
	    // tri[3],tri[4],tri[5] ); 
	    delete[]Edge2TriangleRef[i];
	    shared.Clear();

	    for (k = 0; k < 6; k++) 
              {
		for (m = k + 1; m < 6; m++) 
                  {
		    if (tri[k] == tri[m]) 
                      {
			shared.SetBit(k);
			shared.SetBit(m);
		    };
		};
	    };
	    if (!shared.IsBitSet(0)) { 	s0 = tri[0];  }
            else if (!shared.IsBitSet(1)) { s0 = tri[1]; } 
            else { s0 = tri[2]; };
            
            if (!shared.IsBitSet(3)) { s1 = tri[3]; }
            else if (!shared.IsBitSet(4)) { s1 = tri[4]; }
            else { s1 = tri[5]; };

	    // printf(" vertex %u and %u are the winners! " , s0 , s1);
	    isNew = AddShearConstraint(s0, s1, &current, &m);
	    if (isNew)  { current->L0 = (verts[s0] - verts[s1]).Norm();  };
	};
    };

    delete[]Edge2TriangleRef;

};				// END Cloth::Cloth

Cloth::~Cloth()
{
    if (triangles)  delete[]triangles; 
    Constraint     *p;
    p = (Constraint *) Edges->Pop();
    do 
      {
        delete          p;
        p = (Constraint *) Edges->Pop();
    } while (p != NULL);
    delete          Edges;
};

void
Cloth::ReallocFields(Constraint ** Struct_F, int *size0,
		     Constraint ** Shear_F, int *size1)
{
    // we should keep a local copy of these pointers 
    // (Struct_F and Shear_F are intended to be from Integrator)
    *size0 = Edges->Length();
    Constraint     *NewFields = new Constraint[*size0];
    int             count = 0;
    Constraint     *p;
    p = (Constraint *) Edges->Pop();
    do
      {
	NewFields[count].v0 = p->v0;
	NewFields[count].v1 = p->v1;
	NewFields[count].L0 = p->L0;
	delete          p;
	count++;
	p = (Constraint *) Edges->Pop();
    } while (count < *size0);
    delete          Edges;
    *Struct_F = NewFields;
    *size1 = Shear_Neighbours->Length();
    NewFields = new Constraint[*size1];
    count = 0;

    p = (Constraint *) Shear_Neighbours->Pop();
    do
      {
	NewFields[count].v0 = p->v0;
	NewFields[count].v1 = p->v1;
	NewFields[count].L0 = p->L0;
	delete          p;
	count++;
	p = (Constraint *) Shear_Neighbours->Pop();
    } while (count < *size1);
    delete          Shear_Neighbours;
    *Shear_F = NewFields;
};
