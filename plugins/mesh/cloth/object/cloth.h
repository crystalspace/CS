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

#ifndef __CLOTH_H__
#define __CLOTH_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/box.h"
#include "csgeom/tesselat.h"
#include "csutil/csvector.h"
#include "csutil/bitarray.h"

struct csTriangle;
struct iClothFactoryState;	

//#define      EULER_PROVOT
#define      AMPC_PROVOT  
	
//#define      STATIC_CONSTRAINT
#define      DYNAMIC_CONSTRAINT

class Constraint
{
	public:
uint v0;
uint v1;	
float L0;
	
Constraint()
{
};
Constraint(uint i0, uint i1) 
{
	v0=i0; v1=i1;	
	//printf( " CREATING constrain between %u and %u... \n",i0,i1);
};

~Constraint() 
{};
	
};
 
// things allocated under the responsability of Cloth:
//  Cloth::vertices
//  Cloth::triangles
//  Cloth::(all Constraint elements)

class Cloth
{
public:
csVector3*       shift;
csBox3*          object_bbox;
csVector3        gravity;

csVector3*       vertices;	
int              nverts;

csTriangle*      triangles;
int              ntris;

csBasicVector*   Edges;
csBasicVector*   Shear_Neighbours;
int              nedges;

//quick access data
int**            Edge2TriangleRef;  //which triangles share this edge?
int**            Triangle2EdgeRef;  //which edges bound this triangle?
//uint**            Vertex2EdgeRef;    //which edges share this vertex?

#if defined(STATIC_CONSTRAINT)
Constraint*      Fields;
Constraint*      ShearFields;
#endif

Cloth ( iClothFactoryState*  mesh,
         csVector3&    Sh,
         csBox3&       box,
         csVector3     grav );
~ Cloth ();

bool AddConstraint( int v0, int v1 , Constraint** edge , int* pos );
bool AddShearConstraint( int v0, int v1 , Constraint** edge , int* pos );

void ReallocFields(Constraint** Struct_F, int* StrSize, Constraint** Shear_F , int* ShrSize );
};

//**//**//**//**//**//**//**//**//**//**//**//**//**//**//
 //           Integrator methods                     //
//**//**//**//**//**//**//**//**//**//**//**//**//**//
class Integrator
{
	public:
		
	Cloth*         cloth_object;
	csVector3*     vertices;
	csBitArray*    ConstrainedVertices;
	uint           nverts;
#if defined (DYNAMIC_CONSTRAINT)	
	csBasicVector* fields;
	csBasicVector* shear_fields;
#elif defined (STATIC_CONSTRAINT)
	Constraint*    fields;
	Constraint*    shear_fields;
	int            field_size;
	int            shearfield_size;
#endif
	float          structural_k;
	float          shear_k;
	float          density;
	float          friction;
	float          structural_rigidity;
	float          shear_rigidity;
	float          dt;
	float          time;
	csVector3      gravity;

	csVector3*     velocities;
	csVector3*     forces;
	
#if defined(AMPC_PROVOT)
	csVector3*     aux;	
	csVector3*     tempvertices;
	csVector3*     velocities1;	
	csVector3*     velocities2;
	csVector3*     velocities3;
	csVector3*     forces1;
	csVector3*     forces2;
	csVector3*     forces3;	
#endif	
			
	Integrator( Cloth* obj )
	{
		cloth_object = obj;
		vertices     = obj->vertices;
		nverts       = obj->nverts;
#if  defined(DYNAMIC_CONSTRAINT)		
		fields       = obj->Edges;
		shear_fields = obj->Shear_Neighbours;
#elif defined(STATIC_CONSTRAINT)
		obj -> ReallocFields( &fields, &field_size ,&shear_fields , &shearfield_size );
#endif		
		gravity      = obj->gravity;
		structural_k        = 7.0;
		shear_k             = 7.0;
		density             = 1.0;
		friction            = 0.8;
		structural_rigidity = 0.97;
		shear_rigidity      = 0.8;
		dt           = 0.05;
		time         = 0.0;
		uint  i;
		velocities   = new csVector3 [ nverts ];
		forces       = new csVector3 [ nverts ];
#if defined(AMPC_PROVOT)
		tempvertices=new csVector3[nverts];  
	    //Adams-Moulton predictor-corrector is used on this Integrator,
		//a lot faster that Runge-Kutta 4, and as precise =), but uses
		//4 timesteps for velocities and forces!!, no very memory friendly as you could guess
	     velocities1 = new csVector3[nverts];
	     velocities2 = new csVector3[nverts];
	     velocities3 = new csVector3[nverts];
	     forces1     = new csVector3[nverts];
		 forces2     = new csVector3[nverts];
		 forces3     = new csVector3[nverts];
		 
		 for (i=0;i<nverts;i++)
  {	
   forces3[i]=gravity;  //set all timestep buffers to default gravity at initialization
   forces2[i]=gravity;   
   forces1[i]=gravity;   
    forces[i]=gravity;   
  };
#endif		
		
	ConstrainedVertices = new csBitArray( nverts );
	ConstrainedVertices->Clear(); 
		for (i=9;i<15;i++) {ConstrainedVertices->SetBit(i); };
	
	};
	~Integrator()
    {
		if (velocities)   { delete[] velocities; };
		if (forces)       { delete[] forces;     };
#if defined(AMPC_PROVOT)
		if (tempvertices)    { delete[] tempvertices; };
		if (velocities1)     { delete[] velocities1; };
		if (velocities2)     { delete[] velocities2; };
		if (velocities3)     { delete[] velocities3; };
		if (forces1)         { delete[] forces1; };
		if (forces2)         { delete[] forces2; };
		if (forces3)         { delete[] forces3; };
#endif		
	};
	
    inline void Swap() 
      {
#if defined(AMPC_PROVOT)		  
		  aux     = forces3;
		  forces3 = forces2;
		  forces2 = forces1;
		  forces1 = forces;
		  forces  = aux;
		  
		  aux         = velocities3;
		  velocities3 = velocities2;
		  velocities2 = velocities1;
		  velocities1 = velocities;
		  velocities  = aux;
#endif		  
      };
	  
	inline void ComputeInitial()
	  {
#if defined(AMPC_PROVOT)
	ComputeFields();
	ComputeShearFields();	  
        uint i;     
		  cloth_object->object_bbox->StartBoundingBox ( *(cloth_object->shift) +vertices[0] );
   for (i=0;i<nverts;i++)
   {
	   //let apply here blunt integration (this is for first-timesteps only)
	   velocities[i] = velocities1[i] + 0.5f * forces[i] * (dt/density); 
	     vertices[i]+= velocities1[i]*dt;
	      forces3[i] = density*gravity;              //clean what is going to be the new timestep buffer 
	                                     //after the Swap() call and set to default gravity 
       cloth_object->object_bbox->AddBoundingVertexSmart ( vertices[i] + *(cloth_object->shift) );
		 	 };
       Swap();                        //swap timebuffer   
       // some boundary condition. Here for now
#endif			 
      }; 
	  
	inline void ComputeFields()
	{
		Constraint* p;
		csVector3   temp;
		float       N;
#if  defined(DYNAMIC_CONSTRAINT)		
		int size    = fields->Length();  
		for (int i=0; i < size ; i++ )
			{
				p       = (Constraint*) fields -> Get( i ); 
				temp    = vertices [ p->v1 ] - vertices [ p->v0 ];
				N       = temp.Norm();
				
				N       = structural_k*( ( N - p->L0 ) / N );
				temp   *= N;
				forces[ p->v0 ] += temp;
				forces[ p->v1 ] -= temp;
			};
#elif defined(STATIC_CONSTRAINT)
		for (int i=0; i < field_size ; i++ )
		    {
				p       = &fields[ i ];
				temp    = vertices [ p->v1 ] - vertices [ p->v0 ];
				N       = temp.Norm();
				
				N       = structural_k*( ( N - p->L0 ) / N );
				temp   *= N;
				forces[ p->v0 ] += temp;
				forces[ p->v1 ] -= temp;
			};
#endif			
	}; 
	
	inline void ComputeShearFields()
	{
		Constraint* p;
		csVector3   temp;
		float       N;
#if  defined(DYNAMIC_CONSTRAINT)		
		int size    = shear_fields->Length();  
		for (int i=0; i < size ; i++ )
			{
				p       = (Constraint*) shear_fields -> Get( i ); 
				temp    = vertices [ p->v1 ] - vertices [ p->v0 ];
				N       = temp.Norm();
				
				N       = shear_k*( ( N - p->L0 ) / N );
				temp   *= N;
				forces[ p->v0 ] += temp;
				forces[ p->v1 ] -= temp;
			};
#elif defined(STATIC_CONSTRAINT)
		for (int i=0; i < shearfield_size ; i++)
		    {
				p       = &shear_fields[ i ];
				temp    = vertices [ p->v1 ] - vertices [ p->v0 ];
				N       = temp.Norm();
				
				N       = shear_k*( ( N - p->L0 ) / N );
				temp   *= N;
				forces[ p->v0 ] += temp;
				forces[ p->v1 ] -= temp;
			};
#endif			
	}; 
	
	inline void ApplyProvotConstraint()
	{
	Constraint* p;
	csVector3   temp;
	float       N;
#if   defined(DYNAMIC_CONSTRAINT)		
	int size    = fields->Length();  
	for (int i=0; i < size ; i++ )
#elif defined(STATIC_CONSTRAINT)
	for (int i=0; i < field_size ; i++ )
#endif		
		{
#if   defined(DYNAMIC_CONSTRAINT)			
			p       = (Constraint*) fields -> Get( i ); 
#elif defined(STATIC_CONSTRAINT)
			p       = &fields[ i ];
#endif			
			if (!( ConstrainedVertices->IsBitSet(p->v0) || ConstrainedVertices->IsBitSet(p->v1) ))
				{
					temp    = (vertices [ p->v1 ] - vertices [ p->v0 ]);
					N       = temp.Norm();
					if (structural_rigidity*N>p->L0)
					{
						temp *= 0.5*( structural_rigidity - p->L0/N ); 
					    vertices[ p->v0 ] += temp;
						vertices[ p->v1 ] -= temp;
					};
				}
			else if ( ConstrainedVertices->IsBitSet(p->v0) )
				{
					temp    = (vertices [ p->v1 ] - vertices [ p->v0 ]);
					N       = temp.Norm();
					if (structural_rigidity*N>p->L0)
					{
						temp *= ( structural_rigidity - p->L0/N );
						vertices[ p->v1 ] -= temp;
					};
				}
			else if ( ConstrainedVertices->IsBitSet(p->v1) )
				{
					temp    = (vertices [ p->v1 ] - vertices [ p->v0 ]);
					N       = temp.Norm();
					if (structural_rigidity*N>p->L0)
					{
						temp *= ( structural_rigidity - p->L0/N );
						vertices[ p->v0 ] += temp;
					};
				};					
		};
    };
	

	
		inline void ApplyShearProvotConstraint()
	{
	Constraint* p;
	csVector3   temp;
	float       N;
#if   defined(DYNAMIC_CONSTRAINT)		
	int size    = shear_fields->Length();  
	for (int i=0; i < size ; i++ )
#elif defined(STATIC_CONSTRAINT)
	for (int i=0; i < shearfield_size ; i++ )
#endif		
		{
#if   defined(DYNAMIC_CONSTRAINT)			
			p       = (Constraint*) shear_fields -> Get( i ); 
#elif defined(STATIC_CONSTRAINT)
			p       = &shear_fields[ i ];
#endif			
			if (!( ConstrainedVertices->IsBitSet(p->v0) || ConstrainedVertices->IsBitSet(p->v1) ))
				{
					temp    = (vertices [ p->v1 ] - vertices [ p->v0 ]);
					N       = temp.Norm();
					if (shear_rigidity*N>p->L0)
					{
						temp *= 0.5*( shear_rigidity - p->L0/N ); 
					    vertices[ p->v0 ] += temp;
						vertices[ p->v1 ] -= temp;
					};
				}
			else if ( ConstrainedVertices->IsBitSet(p->v0) )
				{
					temp    = (vertices [ p->v1 ] - vertices [ p->v0 ]);
					N       = temp.Norm();
					if (shear_rigidity*N>p->L0)
					{
						temp *= ( shear_rigidity - p->L0/N );
						vertices[ p->v1 ] -= temp;
					};
				}
			else if ( ConstrainedVertices->IsBitSet(p->v1) )
				{
					temp    = (vertices [ p->v1 ] - vertices [ p->v0 ]);
					N       = temp.Norm();
					if (shear_rigidity*N>p->L0)
					{
						temp *= ( shear_rigidity - p->L0/N );
						vertices[ p->v0 ] += temp;
					};
				};					
		};
    };
	

	
	
	
	
	inline void Compute()
	{
		uint i;
#if defined(EULER_PROVOT)		
		time += dt;
		ComputeFields();
		ComputeShearFields();
		// WARNING:: this 2nd order reference should be profiled!! can be harmful!!
		cloth_object -> object_bbox -> StartBoundingBox ( vertices[0] + *(cloth_object->shift) );
		for (i=0;i<nverts;i++) 
			{				
				vertices[i]  += velocities[i]*dt;
				if ( !ConstrainedVertices->IsBitSet(i) ) 
					{
						velocities[i]+= forces[i]*(dt/density); 
					};
				forces[i]     = density*gravity;   
				cloth_object -> object_bbox -> AddBoundingVertexSmart ( vertices[i] + *(cloth_object->shift) );
			};
		//printf("vertice[0]=%f",(vertices[0] + *cloth_object->shift).x);	
		//ApplyShearProvotConstraint();	
		ApplyProvotConstraint();
		ApplyShearProvotConstraint();	
		ApplyProvotConstraint();	
			
// <<<<<<<<<<<<<<<-----Predictor Corrector-compute---------->>>>>>>>>>>>>>>			
#elif defined(AMPC_PROVOT)      // Adams Moulton predictor corrector
//  <<<<<<<<<<<<<<<<<<<<----------------------------->>>>>>>>>>>>>>>>>
			
		time+=dt;	
	ComputeFields();
	ComputeShearFields();		
csVector3* aux2;  //swapping variables
csVector3* aux3;
         
for (i=0;i<nverts;i++)
{
	//predicted vertice positions
	//IMPORTANT: note that velocities holds at this moment the value holded from velocities3 (t - 3h) which was swapped   
      //predicted velocities 
	if ( !ConstrainedVertices->IsBitSet(i) ) 
		{
			tempvertices[i] = vertices[i]+(dt/24)*( 55*velocities1[i] - 59*velocities2[i] + 37*velocities3[i] - 9*velocities[i] ); 
			velocities[i]   = friction*velocities1[i]+(dt/(24*density))*( 55*forces[i] - 59*forces1[i] + 37*forces2[i] - 9*forces3[i] );
			forces3[i]      = density*gravity;
		}
	else 
	{
		velocities[i]   = 0.0;
		tempvertices[i] = vertices[i];
		forces3[i]      = 0.0;
	};

// Now we need to recompute forces for the new vertices to apply corrector, ugh
};
aux3              = vertices;
vertices          = tempvertices; 
Swap              ();
ComputeFields     ();   // this evaluation is done on the predicted vertice positions
ComputeShearFields();
aux2              = vertices;
vertices          = aux3;
tempvertices      = aux2;
             cloth_object->object_bbox->StartBoundingBox ( *(cloth_object->shift) + vertices[0] );
for (i=0;i<nverts;i++)
   {      
       // IMPORTANT: now, velocities is the timestep t - 2h, 
   if ( !ConstrainedVertices->IsBitSet(i) ) 
		{   
	vertices   [i] += (dt/24)*( 9*velocities1[i] + 19*velocities2[i] - 5*velocities3[i] + velocities[i]); 
	velocities1[i]  = velocities2[i] + (dt/(24*density))*( 9*forces[i] + 19*forces1[i] - 5*forces2[i] + forces3[i]);
	forces     [i]  = density*gravity;   //clean temporal predicted force buffer and set to default gravity for
		}
		else 
			{
				velocities1[i] = 0.0;
				forces[i]      = 0.0;
			};
	  cloth_object->object_bbox->AddBoundingVertexSmart ( vertices[i] + *(cloth_object->shift) );
	};
	ApplyShearProvotConstraint();
	ApplyProvotConstraint();
	//ApplyProvotConstraint();
	//ApplyProvotConstraint();
	
#endif 			
	};
	
	inline void Update(uint msec)
	{
		//do
			//{
				Compute();
		        //printf("compute!");
			//} while (time*1000<msec);				
    };
	
		
};








//TODO: 
//     Integrator should be a global object ?
//       
//

#endif // __CLOTH_H__

