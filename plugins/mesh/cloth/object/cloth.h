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
#include "csgeom/tesselat.h"


class ClothIntegrator
{
	public:
	        ClothIntegrator(    int XSZ,
				    int YSZ,				    
			     csVector3& Sh,
			        csBox3& box,
			      csVector3 grav  )
	{            shift=&Sh;
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
                EquilbDist= 0.05;
              structural_k= 45.0;
	           shear_k= 45.0;
		   density= 5.1;
         // structural_field=new float[ STRfields ];
          //     shear_field=new float[ SHRfields ];
	    
	       object_bbox=&box;
	                dt=0.02;
		};   //\ this class doesnt know anything
	              //\ about memory allocation, (just on physical-only variables)  =(
	               //\  this class only cares about
	                //\  computation
		
		~ClothIntegrator();

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
                 csVector3 gravity;     
		csVector3* shift;
		   csBox3* object_bbox;
		csVector3* aux;
	      unsigned int nverts;
	      unsigned int STRfields;
	      unsigned int SHRfields;
	      unsigned int Xsize;
	      unsigned int Ysize;

	             float dt;  //time interval

      void Swap() 
      {
	           aux=forces3;
               forces3=forces2;
	       forces2=forces1;
	       forces1=forces;
	        forces=aux;

	           aux=velocities3;
           velocities3=velocities2;
	   velocities2=velocities1;
	   velocities1=velocities;
            velocities=aux;

	          /*   aux=vertices;
	          vertices=old_vertices;
	      old_vertices=aux;
	          */
	      
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
	   velocities[i] = velocities1[i] + 0.5*forces[i]*(dt/density); 
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

void Compute() {   
   
	ComputeForces();
	
    
     csVector3* aux2;  //swapping variables
     csVector3* aux3;
            int i;   
   for (i=0;i<nverts;i++)
   {
	   //let apply here Adams-Moulton predictor

	   /*
    tempVelocity = velocities[i]+(dt/24)*( 55*forces[i] - 59*forces1[i] + 37*forces2[i] - 9*forces3[i] ); 
    tempVertices = vertices[i]+(dt/24)*( 55*velocities[i] - 59*velocities1[i] + 37*velocities2[i] - 9*velocities3[i] );  */
	//predicted vertice positions
	//IMPORTANT: note that velocities holds at this moment the value holded from velocities3 (t - 3h) which was swapped   
    tempvertices[i] = vertices[i]+(dt/24)*( 55*velocities1[i] - 59*velocities2[i] + 37*velocities3[i] - 9*velocities[i] ); 
      //predicted velocities 
      velocities[i] = velocities1[i]+(dt/(24*density))*( 55*forces[i] - 59*forces1[i] + 37*forces2[i] - 9*forces3[i] );
          
         forces3[i] = density*gravity;

      // Now we need to recompute forces for the new vertices to apply corrector, ugh
   };
                 aux3 = vertices;
             vertices = tempvertices; 
                Swap();
       ComputeForces();   // this evaluation is done on the predicted vertice positions
                 aux2 = vertices;
             vertices = aux3;
         tempvertices = aux2;
             
     for (i=0;i<nverts;i++)
   {      
            	   //  and here lets apply the corrector
	   //  
                   // IMPORTANT: now, if im not wrong, in velocities is the timestep t - 2h, 
	    vertices[i]+= (dt/24)*( 9*velocities1[i] + 19*velocities2[i] - 5*velocities3[i] + velocities[i]); 
         velocities1[i] = velocities2[i] + (dt/(24*density))*( 9*forces[i] + 19*forces1[i] - 5*forces2[i] + forces3[i]);
             forces[i] = density*gravity;   //clean temporal predicted force buffer and set to default gravity for
	                            //calculation on next timestep
	
	//	 vertices[i].z=cos(1./(cos(pow( xindex-3 , 2 )+pow( yindex-3 ,2)-time*time)+2.1) );
	//	 vertices[i].x=xindex;
	//	 vertices[i].y=yindex;
       object_bbox->AddBoundingVertexSmart ( vertices[i] + *shift );
		 //texels[i].Set(xindex/5.1f , yindex/5.1f );
		 //colors[i].Set (1.0f, 1.0f, 1.0f);
	//	 xindex+=5.0/Xsize;
	//	 if ( ((i+1)%(Xsize+1))==0 ) { xindex=0.0; yindex+=5.0/Ysize;  };
		 	 };
  // Swap();   //swap timebuffers             
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



};

#endif // __CLOTH_H__

