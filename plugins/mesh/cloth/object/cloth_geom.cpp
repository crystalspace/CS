
#include "cssysdef.h"
#include "cloth.h"
#include "igeom/polymesh.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/tesselat.h"
#include "csutil/bitarray.h" 
#include "imesh/object.h"
#include "imesh/clothmesh.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "igeom/objmodel.h"
#include "ivideo/vbufmgr.h"

/*
bool Cloth::AddConstraint( int v0, int v1 , Constraint** edge )
{
	if (v0==v1) 
	{
		*edge=NULL;
		return false;
	};
	Constraint* first;
	Constraint* p;
	p=first=(Constraint*)Edges->GetFirstItem();
	do
	{
		if ((p->v0==v0) || (p->v0==v1)) 
		{
			if ((p->v1==v0) || (p->v1==v1))
			{
				*edge=p;				
				return false;
			};
		};				
		p=(Constraint*)Edges->GetNextItem();
	} while (p!=first);
	p=new Constraint( v0 , v1 );
	Edges->AddItem( (void*) p );
	*edge=p;
	return true;
};*/

bool Cloth::AddConstraint ( int v0 , int v1 , Constraint** edge )
{
	if (v0==v1) 
	{
		*edge=NULL;
		return false;
	};
	Constraint* p;
	int size    = Edges->Length();  
	for (int i=0; i < size ; i++ )
	{
		p = (Constraint*) Edges -> Get( i ); 
		if ((p->v0==v0) || (p->v0==v1)) 
		{
			if ((p->v1==v0) || (p->v1==v1))
			{ 
				*edge=p;				
				return false;
			};
		};		
	};
	p=new Constraint( v0 , v1 );
	Edges -> Push ( (void*) p );
	*edge = p;
	return true;	
};

Cloth::Cloth( iClothFactoryState* mesh,
         csVector3&    Sh,
         csBox3&       box,
         csVector3     grav ) 
{
	shift       = &Sh;
	object_bbox = &box;
	gravity     = grav;
	printf( " CREATING the cloth... \n");	
	uint i;
	vertices  = NULL;
	triangles = NULL;
	printf( " CREATING the clothA... \n");
	csVector3* verts          = mesh->GetVertices     ();
	printf( " CREATING the clothB... \n");
	nverts                    = mesh->GetVertexCount  ();
	csTriangle* tris          = mesh->GetTriangles    ();
	uint	tri_count         = mesh->GetTriangleCount();
	
	//for (i = 0; i < polycnt ; i++)	{ tri_count += polygons[i].num_vertices - 2; };
			printf( " CREATING the cloth2... \n");
	Edges            = new csBasicVector( nverts , 16 );
	nedges           = 0;
	vertices         = new csVector3[ nverts ];
		
	for (i = 0; i < nverts ; i++)
	{
		vertices[i].Set( verts[i].x , verts[i].y , verts[i].z );
	};
	
	ntris            = 2*tri_count;   //here we care about double-facing the mesh
	triangles        = new csTriangle   [ ntris ];
	//memcpy ( triangles , tris , sizeof(csTriangle)*ntris );
	//Triangle2EdgeRef = new Constraint**    [ tri_count ];    this will be needed
	                                                       //  when 2nd neighbours are
                                                           //  considered
	//int* vidx;
	printf( " CREATING the cloth3... \n");
	int index=0;
	int tri;
	int v;
	Constraint* a;
	Constraint* b;
	Constraint* c;
	bool isNew;
	
	for (i=0; i<tri_count; i++) 
	{
		triangles[ 2*i ].a = tris[i].a;
		triangles[ 2*i ].b = tris[i].b;
		triangles[ 2*i ].c = tris[i].c;
		
		triangles[ 2*i + 1 ].a = tris[i].a;
		triangles[ 2*i + 1 ].b = tris[i].c;
		triangles[ 2*i + 1 ].c = tris[i].b;
		
		
		isNew = AddConstraint( tris[i].a , tris[i].b , &a );
		if (isNew) 	{ a -> L0 =( verts[ tris[i].a ] - verts[ tris[i].b ] ).Norm();  nedges++; };
			
		isNew = AddConstraint( tris[i].b , tris[i].c , &b );
		if (isNew) { b -> L0 = ( verts[ tris[i].b ] - verts[ tris[i].c ] ).Norm(); nedges++; };
			
		isNew = AddConstraint( tris[i].c , tris[i].a , &c );
		if (isNew) { c -> L0 = ( verts[ tris[i].c ] - verts[ tris[i].a ] ).Norm(); nedges++; };	
	};
		printf( " CREATING the cloth4... \n");
	/*
	for (i = 0; i<count ; i ++) 
	{
		vidx  = p.vertices;
		tri   = 0;
		isNew = AddEdgeConstraint( vidx[0] , vidx[1] , &a );
		if (isNew) { a -> L0 =( verts[ vidx[0] ] - verts[ vidx[1] ] ).Norm();  nedges++; };
		for (v = 2; v < p.num_vertices; v++ , tri++  ) //triangulation
		{							
			isNew = AddEdgeConstraint( vidx[v-1] , vidx[v] , &b );
			if (isNew) { b -> L0 = ( verts[ vidx[v-1] ] - verts[ vidx[v] ] ).Norm(); nedges++; };
			isNew = AddEdgeConstraint( vidx[v] , vidx[0] , &c );
			if (isNew) { c -> L0 = ( verts[ vidx[v] ] - verts[ vidx[0] ] ).Norm(); nedges++; };	
			
				       no need to consider 2nd neighbours or dynamic refinement yet
			Triangle2EdgeRef[ index + tri ] = new Constraint*[ 3 ];
			Triangle2EdgeRef[ index + tri ][0] = a;
			Triangle2EdgeRef[ index + tri ][1] = b;
			Triangle2EdgeRef[ index + tri ][2] = c;
			  	
			
			triangles[ index + tri ].a = vidx[0];
			triangles[ index + tri ].b = vidx[v-1];
			triangles[ index + tri ].c = vidx[v];
			a = c;
		};
		index += (p.num_vertices - 2);
	};*/
   
}; // END Cloth::Cloth

Cloth::~Cloth()
{
	if (vertices)        { delete[] vertices; };
	if (triangles)       { delete[] triangles; };
	Constraint* p;
	//p=(Constraint*)Edges->GetFirstItem();
	p=(Constraint*)Edges->Pop();
	do
	{
		//Edges->RemoveItem( (void*) p );
		delete p;
		p=(Constraint*)Edges->Pop();
	} while (p!=NULL);
	delete Edges;
};

