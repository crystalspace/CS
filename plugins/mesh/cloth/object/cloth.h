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

struct csTriangle;
struct iClothFactoryState;	
	
class Constraint
{
	public:
uint v0;
uint v1;	
float L0;
	
Constraint(uint i0, uint i1) 
{
	v0=i0; v1=i1;	
	printf( " CREATING constrain between %u and %u... \n",i0,i1);
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
uint             nverts;

csTriangle*      triangles;
uint             ntris;

csBasicVector*   Edges;
uint             nedges;

//quick access data
uint**            Edge2TriangleRef;  //which triangles share this edge?
Constraint***     Triangle2EdgeRef;  //which edges bound this triangle?
uint**            Vertex2EdgeRef;    //which edges share this vertex?


Cloth ( iClothFactoryState*  mesh,
         csVector3&    Sh,
         csBox3&       box,
         csVector3     grav );
~ Cloth ();

bool AddConstraint( int v0, int v1 , Constraint** edge );

};

//**//**//**//**//**//**//**//**//**//**//**//**//**//**//
 //           Integrator methods                     //
//**//**//**//**//**//**//**//**//**//**//**//**//**//
class Integrator
{
	public:
		
	Cloth*         cloth_object;
	csVector3*     vertices;
	uint           nverts;
	csBasicVector* fields;
	float          structural_k;
	float          density;
	float          friction;
	float          rigidity;
	float          dt;
	float          time;
	csVector3      gravity;
	csVector3*     velocities;
	csVector3*     forces;
			
	Integrator( Cloth* obj )
	{
		cloth_object = obj;
		vertices     = obj->vertices;
		nverts       = obj->nverts;
		fields       = obj->Edges;
		gravity      = obj->gravity;
		structural_k = 35.0;
		density      = 3.0;
		friction     = 0.7;
		rigidity     = 0.9;
		dt           = 0.07;
		time         = 0.0;
		velocities   = new csVector3 [ nverts ];
		forces       = new csVector3 [ nverts ];
	};
	~Integrator()
    {
		if (velocities)   { delete[] velocities; };
		if (forces)       { delete[] forces;     };
	};
	
	inline void ComputeFields()
	{
		Constraint* first;		
		Constraint* p;
		csVector3   temp;
		float       N;
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
	}; 
	
	inline void ApplyProvotConstraint()
	{
	Constraint* first;
	Constraint* p;
	csVector3   temp;
	float       N;
	int size    = fields->Length();  
	for (int i=0; i < size ; i++ )
		{
			p       = (Constraint*) fields -> Get( i ); 
			temp    = vertices [ p->v1 ] - vertices [ p->v0 ];
			N       = temp.Norm();
			if (rigidity*N>p->L0)
				{
					temp *= 0.5*( rigidity - p->L0/N ); 
					vertices[ p->v0 ] += temp;
					vertices[ p->v1 ] -= temp;
				};
		};
    };
	
	inline void ComputeEuler()
	{
		int i;
		time += dt;
		ComputeFields();
		// WARNING:: this 2nd order reference should be profiled!! can be harmful!!
		cloth_object -> object_bbox -> StartBoundingBox ( vertices[0] + *cloth_object->shift );
		for (i=0;i<nverts;i++) 
			{
				vertices[i]  += velocities[i]*dt;
				velocities[i]+= forces[i]*(dt/density); 
				forces[i]     = density*gravity;   
				cloth_object -> object_bbox -> AddBoundingVertexSmart ( vertices[i] + *cloth_object->shift );
			};
		//printf("vertice[0]=%f",(vertices[0] + *cloth_object->shift).x);	
		ApplyProvotConstraint();
    };
	
	inline void Update(uint msec)
	{
		//do
			//{
				ComputeEuler();
		        //printf("compute!");
			//} while (time*1000<msec);				
    };
	
		
};



//TODO: 
//     Integrator should be a global object ?
//       
//


class ClothIntegrator
{
	public:
		ClothIntegrator( Cloth* obj )
{
	cloth_object=obj;	
	
};
		ClothIntegrator(    int XSZ,
				    int YSZ,				    
			     csVector3& Sh,
			        csBox3& box,
			      csVector3 grav  )
	{     cloth_object=NULL;       
		    shift=&Sh;
		   gravity=grav;
                     Xsize=XSZ+1;  //the +1 goes because in Xsize in the plugin is quad axe numbers 
		     Ysize=YSZ+1;
		    nverts= Xsize*Ysize;
                  vertices=new csVector3[nverts];
              tempvertices=new csVector3[nverts];  
	  
         	  //Adams-Moulton predictor-corrector is used on this Integrator,
		  //a lot faster that Runge-Kutta 4, and more precise =), but uses
		  //4 timesteps for velocities and forces!!, no very memory friendly as you could guess
		
    		velocities=new csVector3[nverts];  
 	       velocities1=new csVector3[nverts];
	       velocities2=new csVector3[nverts];
	       velocities3=new csVector3[nverts];
	      
            forces=new csVector3[nverts];	     
		   forces1=new csVector3[nverts];
		   forces2=new csVector3[nverts];
		   forces3=new csVector3[nverts];
	
	int i;
  for (i=0;i<nverts;i++)
  {	
   forces3[i]=gravity;  //set all timestep buffers to default gravity at initialization
   forces2[i]=gravity;   
   forces1[i]=gravity;   
    forces[i]=gravity;   
  };
	

	     // this are separated in two field trying to improve cache coherence,
	      // for this to be effective,  
	         STRfields= 2*Xsize*Ysize - Xsize - Ysize ;
		 SHRfields= 2*(Xsize-1)*(Ysize-1) ;
                EquilbDist= 0.01;
              structural_k= 35.0;
	           shear_k= 35.0;
		   density= 3.1;
                  friction= 0.7;  // 1.0 is no friction at all
         // structural_field=new float[ STRfields ];
          //     shear_field=new float[ SHRfields ];
	    
	       object_bbox=&box;
	                dt=0.07;
		};   //\ this class doesnt know anything
	              //\ about memory allocation, (just on physical-only variables)  =(
	               //\  this class only cares about
	                //\  computation
		
		~ClothIntegrator()
	{
		if (cloth_object)
		{
			delete cloth_object;
		} else
		{
			if (vertices)        { delete[] vertices; };
		};
		if (tempvertices)    { delete[] tempvertices; };
		if (velocities)      { delete[] velocities; };
		if (velocities1)     { delete[] velocities1; };
		if (velocities2)     { delete[] velocities2; };
		if (velocities3)     { delete[] velocities3; };
		if (forces)          { delete[] forces; };
		if (forces1)         { delete[] forces1; };
		if (forces2)         { delete[] forces2; };
		if (forces3)         { delete[] forces3; };
			
	};
		

	        Cloth* cloth_object;
		csVector3* vertices;
		csVector3* tempvertices;
		
		csVector3* velocities;
		csVector3* forces;
		
		csVector3* velocities1;
		csVector3* forces1;
		csVector3* velocities2;
		csVector3* forces2;
		csVector3* velocities3;
		csVector3* forces3;
		 
		    float* structural_field;
		    float* shear_field;
		     float EquilbDist;
		     float structural_k;
		     float shear_k;
		     float density;
		     float friction;
                 csVector3 gravity;     
		csVector3* shift;
		   csBox3* object_bbox;
		csVector3* aux;
	      int nverts;
	      int STRfields;
	      int SHRfields;
	      int Xsize;
	      int Ysize;

	             float dt;  //time interval

		     /*
		      * Numerical Methods implemented in this integrator:
		      *
		      *     Euler integrator 
		      *     (recommended only for startup timesteps and switch then to Adams-Moulton)
		      *
		      *     Related methods:
		      *                   ComputeInitial();
		      *        
		      *                       
		      *     4th order Adams-Moulton predictor-corrector
		      *     Related methods:
		      *                          Compute();
		      *
		      *     Verlet-Constrain
		      *     Related methods:
		      *                       VerletSwap();
		      *                    ComputeVerlet();
		      *         ComputeConstrainedForces();         
		      *         
		      *      Common methods:
		      *         
		      *            Swap();   timesteps swapping
		      *   ComputeForces();   evaluate force field for a structural + shear spring system        
		      *     
		      *      */

      void Swap() 
      {
	           aux = forces3;
               forces3 = forces2;
	       forces2 = forces1;
	       forces1 = forces;
	        forces = aux;

	           aux = velocities3;
           velocities3 = velocities2;
	   velocities2 = velocities1;
	   velocities1 = velocities;
            velocities = aux;
      };

     void VerletSwap()
     {
	           aux = vertices;
              vertices = tempvertices;
          tempvertices = aux;
	          
     };      
  
     csVector3* GetVerticeBuffer()
     {
	return vertices;
     };      

/*     
      void  ComputeInitial()
   {	
	//float xindex=0.0;
	//float yindex=0.0;
	int i;
	int j;
	int k;
	csVector3 temp;
	float N;
		
 object_bbox->StartBoundingBox ( *shift );
    
   //here we loop over structural fields
   for (j=0;j<Xsize-1;j++)
   { //the first row of Horizontal structural fields
      //structural_field[ j ]=( vertices[ j + 1 ]-vertices[ j ] ).Norm();
      temp=vertices[j+1]-vertices[j];
      N=temp.Norm();
      N=structural_k*((N-EquilbDist)/N);
      temp*=N;
      forces[j]+=temp;   //action -
      forces[j+1]-=temp;  //reaction -  At Work     
	   
   };
      
   for (i=1;i<Ysize;i++)
   {
      k = ( 2*Xsize -1 )*i - Xsize;   
      //ahh this are array offsets related to the way of storing fields
      for (j=0;j<Xsize;j++) 
      { //Vertical structural fields
       // structural_field[ j + k ]=( vertices[ j + Xsize*(i-1) ] - vertices[ j + Xsize*i ] ).Norm();
	  temp=vertices[j + Xsize*(i-1) ]-vertices[ j + Xsize*i ];
	  N=temp.Norm();
	  N=structural_k*((N-EquilbDist)/N);
	  temp*=N;
	  forces[ j + Xsize*i ]+=temp;
	  forces[ j + Xsize*(i-1) ]-=temp;
	  //printf(" %f \n",N);
       };
      k += Xsize;	 
      for (j=0;j<Xsize-1;j++)
      { 
        //structural_field[ j + k ]=( vertices[ j + Xsize*i ] - vertices[ j + Xsize*i + 1] ).Norm();
	temp=vertices[ j + Xsize*i ]-vertices[ j + Xsize*i + 1 ];
	N=temp.Norm();
	N=structural_k*((N-EquilbDist)/N);
	temp*=N;
	forces[ j + Xsize*i + 1 ]+=temp;
	forces[ j + Xsize*i ]-=temp;
	
        //shear_field[ 2*((i-1)*(Xsize-1) + j) ]=( vertices[ j + Xsize*i + 1 ] - vertices[ j + Xsize*(i-1) ] ).Norm();
	temp= vertices[ j + Xsize*i + 1 ] - vertices[ j + Xsize*(i-1) ];
	N=temp.Norm();
	N=shear_k*((N-EquilbDist)/N);
	temp*=N;
	forces[ j + Xsize*(i-1) ]+=temp;
	forces[ j + Xsize*i + 1 ]-=temp;

	//shear_field[ 2*((i-1)*(Xsize-1) + j) + 1 ]=( vertices[ j + Xsize*i ] - vertices[ j + Xsize*(i-1) + 1 ] ).Norm();
        temp= vertices[ j + Xsize*(i-1) + 1 ] - vertices[ j + Xsize*i ];
	N=temp.Norm();
	N=shear_k*((N-EquilbDist)/N);
	temp*=N;
	forces[ j + Xsize*i ]+=temp;
	forces[ j + Xsize*(i-1) + 1 ]-=temp;
      };
   }; 
};  
*/

void ComputeInitial() {

	ComputeForces();
        int i;     
   for (i=0;i<nverts;i++)
   {
	   //let apply here blunt integration (this is for first-timesteps only)
	   velocities[i] = velocities1[i] + 0.5f * forces[i] * (dt/density); 
	     vertices[i]+= velocities1[i]*dt;
	  
	      forces3[i] = density*gravity;              //clean what is going to be the new timestep buffer 
	                                      //after the Swap() call and set to default gravity 
	
	//	 vertices[i].z=cos(1./(cos(pow( xindex-3 , 2 )+pow( yindex-3 ,2)-time*time)+2.1) );
	//	 vertices[i].x=xindex;
	//	 vertices[i].y=yindex;
       object_bbox->AddBoundingVertexSmart ( vertices[i] + *shift );
		 //texels[i].Set(xindex/5.1f , yindex/5.1f );
		 //colors[i].Set (1.0f, 1.0f, 1.0f);
	//	 xindex+=5.0/Xsize;
	//	 if ( ((i+1)%(Xsize+1))==0 ) { xindex=0.0; yindex+=5.0/Ysize;  };
		 	 };
       Swap();                        //swap timebuffer   

       // some boundary condition. Here for now
   for (i=0;i<Xsize-1;i++) {
  vertices[i].Set(2.0*i/(Xsize-1),0,0);
   };
  // vertices[Xsize-1].Set(0,0,0);

    };

   void  ComputeForces()
   {	
	//float xindex=0.0;
	//float yindex=0.0;
	int i;
	int j;
	int k;
	csVector3 temp;
	float N;
		
 object_bbox->StartBoundingBox ( *shift );
    
   //here we loop over structural fields
   for (j=0;j<Xsize-1;j++)
   { //the first row of Horizontal structural fields
      //structural_field[ j ]=( vertices[ j + 1 ]-vertices[ j ] ).Norm();
      temp=vertices[j+1]-vertices[j];
      N=temp.Norm();
      //N*=N*N;
      N=structural_k*((N-EquilbDist)/N);
      temp*=N;
      forces[j]+=temp;   //action -
      forces[j+1]-=temp;  //reaction -  At Work     
     
	   
   };
     
 
   for (i=1;i<Ysize;i++)
   {
      k = ( 2*Xsize -1 )*i - Xsize;   
      //ahh this are array offsets related to the way of storing fields
      for (j=0;j<Xsize;j++) 
      { //Vertical structural fields
       // structural_field[ j + k ]=( vertices[ j + Xsize*(i-1) ] - vertices[ j + Xsize*i ] ).Norm();
	  temp=vertices[j + Xsize*(i-1) ]-vertices[ j + Xsize*i ];
	  N=temp.Norm();
          //N*=N*N;
	  N=structural_k*((N-EquilbDist)/N);
	  temp*=N;
	  forces[ j + Xsize*i ]+=temp;
	  forces[ j + Xsize*(i-1) ]-=temp;
	 	  //printf(" %f \n",N);
       };
      k += Xsize;	 
      for (j=0;j<Xsize-1;j++)
      { 
        //structural_field[ j + k ]=( vertices[ j + Xsize*i ] - vertices[ j + Xsize*i + 1] ).Norm();
	temp=vertices[ j + Xsize*i ]-vertices[ j + Xsize*i + 1 ];
	N=temp.Norm();
        //N*=N*N;
	N=structural_k*((N-EquilbDist)/N);
	temp*=N;
	forces[ j + Xsize*i + 1 ]+=temp;
	forces[ j + Xsize*i ]-=temp;

        //shear_field[ 2*((i-1)*(Xsize-1) + j) ]=( vertices[ j + Xsize*i + 1 ] - vertices[ j + Xsize*(i-1) ] ).Norm();
	temp= vertices[ j + Xsize*i + 1 ] - vertices[ j + Xsize*(i-1) ];
	N=temp.Norm();
	//N*=N*N;
	N=shear_k*((N-EquilbDist)/N);
	temp*=N;
	forces[ j + Xsize*(i-1) ]+=temp;
	forces[ j + Xsize*i + 1 ]-=temp;
	
	//shear_field[ 2*((i-1)*(Xsize-1) + j) + 1 ]=( vertices[ j + Xsize*i ] - vertices[ j + Xsize*(i-1) + 1 ] ).Norm();
        temp= vertices[ j + Xsize*(i-1) + 1 ] - vertices[ j + Xsize*i ];
	N=temp.Norm();
	//N*=N*N;
	N=shear_k*((N-EquilbDist)/N);
	temp*=N;
	forces[ j + Xsize*i ]+=temp;
	forces[ j + Xsize*(i-1) + 1 ]-=temp;

      };
   };
        for (i=0;i<Xsize-8;i++) {
  forces[i]= 0.0;
  //vertices[1].Set(0.1,0,0);
   };
    for (i=nverts-Ysize+1;i<nverts;i++) {
  forces[i]= 0.0;   
 // vertices[i].Set(2.0*(i-nverts+Xsize)/Xsize,2.0,0);
  //vertices[1].Set(0.1,0,0);
   };

 //  ComputeConstrainConditionsForces();
 };

void ComputeConstrainConditionsForces()
{
	int i; 
	
      for (i=0;i<Xsize-8;i++) {
  forces[i]= 0.0;
  //vertices[1].Set(0.1,0,0);
   };
    for (i=nverts-Ysize+1;i<nverts;i++) {
  forces[i]= 0.0;   
 // vertices[i].Set(2.0*(i-nverts+Xsize)/Xsize,2.0,0);
  //vertices[1].Set(0.1,0,0);
   };

};
void Compute() {   
int i;   
ComputeForces();

csVector3* aux2;  //swapping variables
csVector3* aux3;
         
for (i=0;i<nverts;i++)
{
	//predicted vertice positions
	//IMPORTANT: note that velocities holds at this moment the value holded from velocities3 (t - 3h) which was swapped   
tempvertices[i] = vertices[i]+(dt/24)*( 55*velocities1[i] - 59*velocities2[i] + 37*velocities3[i] - 9*velocities[i] ); 
      //predicted velocities 
velocities[i]   = friction*velocities1[i]+(dt/(24*density))*( 55*forces[i] - 59*forces1[i] + 37*forces2[i] - 9*forces3[i] );
          
forces3[i]      = density*gravity;

// Now we need to recompute forces for the new vertices to apply corrector, ugh
};
aux3            = vertices;
vertices        = tempvertices; 
Swap            ();
ComputeForces   ();   // this evaluation is done on the predicted vertice positions
aux2            = vertices;
vertices        = aux3;
tempvertices    = aux2;
             
for (i=0;i<nverts;i++)
   {      
       // IMPORTANT: now, velocities is the timestep t - 2h, 
      vertices   [i] += (dt/24)*( 9*velocities1[i] + 19*velocities2[i] - 5*velocities3[i] + velocities[i]); 
	  velocities1[i]  = velocities2[i] + (dt/(24*density))*( 9*forces[i] + 19*forces1[i] - 5*forces2[i] + forces3[i]);
	  forces     [i]  = density*gravity;   //clean temporal predicted force buffer and set to default gravity for
	  object_bbox ->    AddBoundingVertexSmart ( vertices[i] + *shift );
	};
};

 void ComputeVerlet() 
 {
	 
	//ComputeConstrainedForces( 1.0 );
        int i;     
   for (i=0;i<nverts;i++)
   {
	   //let apply here Verlet integration
           velocities[i] = 2*vertices[i] - tempvertices[i] + forces[i]*(dt/density); 
         tempvertices[i] = vertices[i];
	     vertices[i] = velocities[i];
	  
	      forces[i] = density*gravity;              //clean what is going to be the new timestep buffer 
	                                      //after the Swap() call and set to default gravity 
	
	//	 vertices[i].z=cos(1./(cos(pow( xindex-3 , 2 )+pow( yindex-3 ,2)-time*time)+2.1) );
	//	 vertices[i].x=xindex;
	//	 vertices[i].y=yindex;
       object_bbox->AddBoundingVertexSmart ( vertices[i] + *shift );
		 //texels[i].Set(xindex/5.1f , yindex/5.1f );
		 //colors[i].Set (1.0f, 1.0f, 1.0f);
	//	 xindex+=5.0/Xsize;
	//	 if ( ((i+1)%(Xsize+1))==0 ) { xindex=0.0; yindex+=5.0/Ysize;  };
		 	 };
      // VerletSwap();                        //swap timebuffer   

       // some boundary condition. Here for now
    for (i=0;i<Xsize-8;i++) {
  vertices[i].Set(2.0*i/Xsize,0,0);
  //vertices[1].Set(0.1,0,0);
   };
    for (i=nverts-Ysize+1;i<nverts;i++) {
  vertices[i].Set(2.0*(i-nverts+Xsize)/Xsize,2.0,0);
  //vertices[1].Set(0.1,0,0);
   };

  // vertices[Xsize-1].Set(0,0,0);

    };


	/*
 void  ComputeConstrainedForces(float maxlengthfactor)
	
   {
   
   //the purpose of this routine is evaluate how off are satisfied distance constrains
   //for rigid edges and apply relaxation  ( i wonder if there is a way to induce critical relaxation)
   //
   //
	//float xindex=0.0;
	//float yindex=0.0;
	int i;
	int j;
	int k;
	csVector3 temp;
	float N;
		
 object_bbox->StartBoundingBox ( *shift );
    
   //here we loop over structural fields
   for (j=0;j<Xsize-1;j++)
   { //the first row of Horizontal structural fields
      //structural_field[ j ]=( vertices[ j + 1 ]-vertices[ j ] ).Norm();
      temp=vertices[j+1]-vertices[j];
      N=temp.Norm();
      N=((N-EquilbDist)/N);
      temp*=N;
      //forces[j]+=softness*temp;   //action -
      vertices[j]+=0.5*temp;
      //forces[j+1]-=softness*temp;  //reaction -  At Work     
      vertices[j+1]-=0.5*temp;
	   
   };
      
   for (i=1;i<Ysize;i++)
   {
      k = ( 2*Xsize -1 )*i - Xsize;   
      //ahh this are array offsets related to the way of storing fields
      for (j=0;j<Xsize;j++) 
      { //Vertical structural fields
       // structural_field[ j + k ]=( vertices[ j + Xsize*(i-1) ] - vertices[ j + Xsize*i ] ).Norm();
	  temp=vertices[j + Xsize*(i-1) ]-vertices[ j + Xsize*i ];
	  N=temp.Norm();
	  N=structural_k*((N-EquilbDist)/N);
	  temp*=N;
	  forces[ j + Xsize*i ]+=softness*temp;
	  vertices[ j + Xsize*i ]+=0.5*temp;
	  forces[ j + Xsize*(i-1) ]-=softness*temp;
	  vertices[ j + Xsize*(i-1) ]-=0.5*temp;
	  //printf(" %f \n",N);
       };
      k += Xsize;	 
      for (j=0;j<Xsize-1;j++)
      { 
        //structural_field[ j + k ]=( vertices[ j + Xsize*i ] - vertices[ j + Xsize*i + 1] ).Norm();
	temp=vertices[ j + Xsize*i ]-vertices[ j + Xsize*i + 1 ];
	N=temp.Norm();
	N=structural_k*((N-EquilbDist)/N);
	temp*=N;
	forces[ j + Xsize*i + 1 ]+=softness*temp;
	vertices[ j + Xsize*i + 1 ]+=.5*temp;
	forces[ j + Xsize*i ]-=softness*temp;
	vertices[ j + Xsize*i ]-=.5*temp;

        //shear_field[ 2*((i-1)*(Xsize-1) + j) ]=( vertices[ j + Xsize*i + 1 ] - vertices[ j + Xsize*(i-1) ] ).Norm();
	temp= vertices[ j + Xsize*i + 1 ] - vertices[ j + Xsize*(i-1) ];
	N=temp.Norm();
	N=shear_k*((N-EquilbDist)/N);
	temp*=N;
	forces[ j + Xsize*(i-1) ]+=softness*temp;
	forces[ j + Xsize*i + 1 ]-=softness*temp;
	vertices[ j + Xsize*(i-1) ]+=.5*temp;
	vertices[ j + Xsize*i + 1 ]-=.5*temp;

	//shear_field[ 2*((i-1)*(Xsize-1) + j) + 1 ]=( vertices[ j + Xsize*i ] - vertices[ j + Xsize*(i-1) + 1 ] ).Norm();
        temp= vertices[ j + Xsize*(i-1) + 1 ] - vertices[ j + Xsize*i ];
	N=temp.Norm();
	N=shear_k*((N-EquilbDist)/N);
	temp*=N;
	forces[ j + Xsize*i ]+=softness*temp;
	forces[ j + Xsize*(i-1) + 1 ]-=softness*temp;
	vertices[ j + Xsize*i ]+=.5*temp;
	vertices[ j + Xsize*(i-1) + 1 ]-=.5*temp;
	
      };
   }; 
   
 };


*/
 };

#endif // __CLOTH_H__

