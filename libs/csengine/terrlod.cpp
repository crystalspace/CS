/*
    Copyright (C) 1999,2000 by Jorrit Tyberghein

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

#include "csengine/terrlod.h"
#include "csengine/pol2d.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "csengine/engine.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csparser/csloader.h"
#include "qsqrt.h"
#include "igraph3d.h"
#include "itxtmgr.h"

IMPLEMENT_CSOBJTYPE (csLODTerrain, csTerrain);

// I don't know if i can use defines or not
#define LOD_EDGE_POINT 0
#define LOD_NODE_POINT 1
#define LOD_UNKNOWN    2

int max_vt;
int max_tr;

csLODTerrain::csLODTerrain() : csTerrain ()
{
    quadTree = NULL;
    heightMap = NULL;
    d2Table = NULL;
    g3dmesh = NULL;
    vertices = NULL;
    texels = NULL;
    triangles = NULL;
    material = NULL;
    dim = 0;
    lod_C = 6;    // this could be changed on run-time for global prformance
    lod_c = 6;
    K = (float)lod_C / 2*((float)lod_C - 1);
    scale = 30;
}

csLODTerrain::~csLODTerrain()
{
  int i;

  if (heightMap != NULL)
  {
    for (i=0; i<dim; i++)
      delete[] heightMap[i];
    delete heightMap;
  }
  if (quadTree != NULL)
  {
    for (i=0; i<dim+1; i++)
      delete[] quadTree[i];
    delete quadTree;
  }
  if (d2Table != NULL)
  {
    for (i=0; i<dim+1; i++)
      delete[] d2Table[i];
    delete d2Table;
  }
	
  delete g3dmesh;
  delete[] vertices;
  delete[] texels;
  delete[] triangles;
  delete material;
}

void csLODTerrain::resetQuadTree (void)
{
  if(quadTree != NULL)
    for(int iy=0; iy<dim; iy++)
      for(int ix=0; ix<dim; ix++)
        quadTree[ix][iy] = LOD_UNKNOWN;
}

float csLODTerrain::setupd2Table (int x, int z, int width)
{
    int width2 = width/2;

    float d2 = 0;
    float dh1 = fabs(((heightMap[x-width][z-width]+
		       heightMap[x+width][z-width])/2)-
		     heightMap[x][z-width2]);
    float dh2 = fabs(((heightMap[x+width][z-width]+
		       heightMap[x+width][z+width])/2)-
		     heightMap[x+width2][z]);
    float dh3 = fabs(((heightMap[x+width][z+width]+
		       heightMap[x-width][z+width])/2)-
		     heightMap[x][z+width2]);
    float dh4 = fabs(((heightMap[x-width][z-width]+
		       heightMap[x-width][z+width])/2)-
		     heightMap[x-width2][z]);
    float dh5 = fabs(((heightMap[x+width][z+width]+
		       heightMap[x-width][z-width])/2)-
		     heightMap[x][z]);
    float dh6 = fabs(((heightMap[x-width][z+width]+
		       heightMap[x+width][z-width])/2)-
		     heightMap[x][z]);
	
    d2 = dh1;
    if(dh2 > d2)
	d2 = dh2;
    if(dh3 > d2)
	d2 = dh3;
    if(dh4 > d2)
	d2 = dh4;
    if(dh5 > d2)
	d2 = dh5;
    if(dh6 > d2)
	d2 = dh6;
    d2 /= width;

    float d21, d22, d23, d24;

    if(width>1) {
	d21 = setupd2Table(x-width2, z-width2, width2);
	d22 = setupd2Table(x+width2, z-width2, width2);
	d23 = setupd2Table(x-width2, z+width2, width2);
	d24 = setupd2Table(x+width2, z+width2, width2);
    } else {
	d2Table[x][z] = d2;
	return d2;
    }

// if you comment this out, you don't take account of the d2 propagation
// this will brake the mesh!!! (holes...., but a lot faster :-)))
// /*
    if(d21*K > d2)
	d2 = d21*K;
    if(d22*K > d2)
	d2 = d22*K;
    if(d23*K > d2)
	d2 = d23*K;
    if(d24*K > d2)
	d2 = d24*K;
// */
    d2Table[x][z] = d2;
    return d2;
}

void csLODTerrain::setupQuadTree (int x, int z, int width)
{	
    int width2 = width/2;

    float dis = qsqrt((x - position[0]) * (x - position[0]) +
			(z - position[2]) * (z - position[2]));

    float f = lod_c*d2Table[x][z];
    if(f<1) f = 1;

    f = dis / (width * lod_C * f);

// if you change those to lines, you can change from simple distance LOD
// to full distance and roughness LOD
// the first line is a lot faster!!! (but has a lot of popping)

//	if((width>1) && (dis < width * lod_C)) {
    if((width>1) && (f<1)) {
	quadTree[x][z] = LOD_NODE_POINT;
	quadTree[x-width2][z-width2] = LOD_EDGE_POINT;
	quadTree[x+width2][z-width2] = LOD_EDGE_POINT;
	quadTree[x-width2][z+width2] = LOD_EDGE_POINT;
	quadTree[x+width2][z+width2] = LOD_EDGE_POINT;

	setupQuadTree(x - width2, z - width2, width2);
	setupQuadTree(x + width2, z - width2, width2);
	setupQuadTree(x - width2, z + width2, width2);
	setupQuadTree(x + width2, z + width2, width2);
	
    } else {
	quadTree[x][z] = LOD_EDGE_POINT;
    }
}

void csLODTerrain::draw (int x, int z, int width)
{
    int width2;

// this function goes top to bottom throu the QuadTree and draw the
// nessecary triangles

    if(width > 1) {
	width2 = width/2;
	if(quadTree[x][z] == LOD_NODE_POINT) {
	    //NORTH
	    if(quadTree[x-width2][z-width2] == LOD_EDGE_POINT &&
	       quadTree[x+width2][z-width2] == LOD_EDGE_POINT) {
		if((quadTree[x][z-width*2] == LOD_NODE_POINT)){
                    //NORTH_L
		    drawTriangle(x, z, x, z-width, x-width, z-width);
                    //NORTH_R
		    drawTriangle(x, z, x+width, z-width, x, z-width);
		} else {
                    //NORTH
		    drawTriangle(x, z, x+width, z-width, x-width, z-width);
		}
	    } else if(quadTree[x-width2][z-width2] == LOD_EDGE_POINT) {
		drawTriangle(x, z, x, z-width, x-width, z-width);
	    } else if(quadTree[x+width2][z-width2] == LOD_EDGE_POINT) {
		drawTriangle(x, z, x+width, z-width, x, z-width);
	    }

	    //SOUTH
	    if(quadTree[x-width2][z+width2] == LOD_EDGE_POINT &&
	       quadTree[x+width2][z+width2] == LOD_EDGE_POINT) {
		if((quadTree[x][z+width*2] == LOD_NODE_POINT)){
                    //SOUTH_L
		    drawTriangle(x, z, x-width, z+width, x, z+width);
                    //SOUTH_R
		    drawTriangle(x, z, x, z+width, x+width, z+width);
		} else {
                    //SOUTH
		    drawTriangle(x, z, x-width, z+width, x+width, z+width);
		}
	    } else if(quadTree[x-width2][z+width2] == LOD_EDGE_POINT) {
		drawTriangle(x, z, x-width, z+width, x, z+width);
	    } else if(quadTree[x+width2][z+width2] == LOD_EDGE_POINT) {
		drawTriangle(x, z, x, z+width, x+width, z+width);
	    }

	    //EAST
	    if(quadTree[x+width2][z-width2] == LOD_EDGE_POINT &&
	       quadTree[x+width2][z+width2] == LOD_EDGE_POINT) {
		if((x+width*2)<=dim) {
		    if((quadTree[x+width*2][z] == LOD_NODE_POINT)){
			//EAST_T
			drawTriangle(x, z, x+width, z, x+width, z-width);
			//EAST_B
			drawTriangle(x, z, x+width, z+width, x+width, z);
		    } else {
			//EAST
			drawTriangle(x, z, x+width, z+width, x+width, z-width);
		    }
		} else {
		    //EAST
		    drawTriangle(x, z, x+width, z+width, x+width, z-width);
		}
	    } else if(quadTree[x+width2][z-width2] == LOD_EDGE_POINT) {
		drawTriangle(x, z, x+width, z, x+width, z-width);
	    } else if(quadTree[x+width2][z+width2] == LOD_EDGE_POINT) {
		drawTriangle(x, z, x+width, z+width, x+width, z);
	    }

	    //WEST
	    if(quadTree[x-width2][z-width2] == LOD_EDGE_POINT &&
	       quadTree[x-width2][z+width2] == LOD_EDGE_POINT) {
		if((x-width) != 0) {
		    if((quadTree[x-width*2][z] == LOD_NODE_POINT)) {
			//WEST_T
			drawTriangle(x, z, x-width, z-width, x-width, z);
			//WEST_B
			drawTriangle(x, z, x-width, z, x-width, z+width);
		    } else {
			//WEST
			drawTriangle(x, z, x-width, z-width, x-width, z+width);
		    }
		} else {
		    //WEST
		    drawTriangle(x, z, x-width, z-width, x-width, z+width);
		}
	    } else if(quadTree[x-width2][z-width2] == LOD_EDGE_POINT) {
		drawTriangle(x, z, x-width, z-width, x-width, z);
	    } else if(quadTree[x-width2][z+width2] == LOD_EDGE_POINT) {
		drawTriangle(x, z, x-width, z, x-width, z+width);
	    }
	} else {
	    return;
	}
	draw(x-width2, z-width2, width2);
	draw(x+width2, z-width2, width2);
	draw(x-width2, z+width2, width2);
	draw(x+width2, z+width2, width2);
    }
}

void csLODTerrain::drawTriangle (int x1, int z1, int x2, int z2, int x3, int z3)
{
    int vnum = g3dmesh->num_vertices;
    int tnum = g3dmesh->num_triangles;

    if (vnum >= max_vt)
      CsPrintf (MSG_FATAL_ERROR, "ERROR: Vertex buffer overflow!\n");
    if (tnum >= max_tr)
      CsPrintf (MSG_FATAL_ERROR, "ERROR: Triangle buffer overflow!\n");

// !!! HELP !!! the texels are bad....
// never understood this texel thing....
// but anyway the texture thing isn't finished

    vertices[vnum].Set(x1,heightMap[x1][z1]*scale, z1);
    texels[vnum].Set((float)x1/257.0, (float)z1/257.0);

    vertices[vnum+1].Set(x2,heightMap[x2][z2]*scale, z2);
    texels[vnum+1].Set((float)x2/257.0, (float)z2/257.0);

    vertices[vnum+2].Set(x3,heightMap[x3][z3]*scale, z3);
    texels[vnum+2].Set((float)x3/257.0, (float)z3/257.0);
    triangles[tnum].a = vnum;
    triangles[tnum].b = vnum+1;
    triangles[tnum].c = vnum+2;

    g3dmesh->num_vertices += 3;
    g3dmesh->num_triangles += 1;
}

bool csLODTerrain::Initialize (const void *vheightMapFile, unsigned long /*size*/)
{
    const char* heightMapFile = (const char*)vheightMapFile;
    unsigned char red, green, blue;
    unsigned int  xsize, ysize;
    int           i, ix, iy;

// TGA header;

// length of Identifier String
    unsigned char 	IDLength;
// 0 = no map
    unsigned char 	CoMapType;
// image type (see below for values)
    unsigned char 	ImgType;
// index of first color map entry
    unsigned char 	Index_lo, Index_hi;
// number of entries in color map
    unsigned char	Length_lo, Length_hi;
// size colormap entry (15,16,24,32)
    unsigned char 	CoSize;
// x origin of image
    unsigned char 	X_org_lo, X_org_hi;
// y origin of image
    unsigned char 	Y_org_lo, Y_org_hi;
// width of image
    unsigned char 	Width_lo, Width_hi;
// height of image
    unsigned char 	Height_lo, Height_hi;
// pixel size (8,16,24,32)
    unsigned char 	PixelSize;
// pixel size (8,16,24,32)
    unsigned char 	Dbyte;

    CsPrintf (MSG_INITIALIZATION, "init HeightMap\n");

// 90% of this isn't needed.
    IDLength  = heightMapFile[0];
    CoMapType = heightMapFile[1];
    ImgType   = heightMapFile[2];
    Index_lo  = heightMapFile[3];
    Index_hi  = heightMapFile[4];
    Length_lo = heightMapFile[5];
    Length_hi = heightMapFile[6];
    CoSize    = heightMapFile[7];
    X_org_lo  = heightMapFile[8];
    X_org_hi  = heightMapFile[9];
    Y_org_lo  = heightMapFile[10];
    Y_org_hi  = heightMapFile[11];
    Width_lo  = heightMapFile[12];
    Width_hi  = heightMapFile[13];  // file in lo-byte, hi-byte order b12,b13
    Height_lo = heightMapFile[14];
    Height_hi = heightMapFile[15];  // ysize b14, b15
    PixelSize = heightMapFile[16];
    Dbyte     = heightMapFile[17];  // descriptor byte b17

    xsize = (unsigned int)Width_lo + 256 * Width_hi;
    ysize = (unsigned int)Height_lo + 256 * Height_hi;

    if(xsize != ysize) {
	CsPrintf(MSG_FATAL_ERROR, "Terrain sizes do not match\n");
	fatal_exit(0, false);
    }

    dim = xsize;
    CsPrintf (MSG_INITIALIZATION, "HeightMap size: %d\n", dim);

    heightMap = new float*[dim];
    for(i=0; i<dim; i++)
	heightMap[i] = new float[dim];

    quadTree = new char*[dim+1];
    for(i=0; i<dim+1; i++)
	quadTree[i] = new char[dim+1];

    d2Table = new float*[dim+1];
    for(i=0; i<dim+1; i++)
	d2Table[i] = new float[dim+1];

// BUG BUG: this has to be changed dynamically!!!!!
// this could crash the hole system........

    max_vt = dim*dim*3*2;
    max_tr = (dim-1)*(dim-1)*2;
    vertices = new csVector3[max_vt];           //maximum ??
    texels = new csVector2[max_vt];             //maximum ??
    triangles = new csTriangle[max_tr]; //maximum ??

// HELP: I don't understand a word here
// (how can I clip, do fog, and color ???????)
// some docs please :-(

    g3dmesh = new G3DTriangleMesh;
    g3dmesh->vertex_colors[0] = NULL;
    g3dmesh->morph_factor = 0;
    g3dmesh->num_vertices_pool = 1;
    g3dmesh->num_materials = 1;
    g3dmesh->use_vertex_color = false;
    g3dmesh->do_mirror = false;
    g3dmesh->do_morph_texels = false;
    g3dmesh->do_morph_colors = false;
    g3dmesh->do_fog = false;
    g3dmesh->do_clip = true;
    g3dmesh->use_vertex_color = false;
    g3dmesh->vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
    g3dmesh->fxmode = 0;//CS_FX_GOURAUD;
    g3dmesh->num_vertices = 0;
    g3dmesh->num_triangles = 0;
    g3dmesh->vertices[0] = vertices;
    g3dmesh->texels[0][0] = texels;
    g3dmesh->triangles = triangles;

    for(iy=0; iy<dim; iy++) {
	for(ix=0; ix<dim; ix++) {
	    blue  = heightMapFile[(3*(ix+dim*iy))+18];
	    green = heightMapFile[(3*(ix+dim*iy))+19];
	    red   = heightMapFile[(3*(ix+dim*iy))+20];
	    heightMap[ix][iy] =
		((((float)red*256.0+(float)green)/65535.0));
	}
    }

    setupd2Table(dim/2, dim/2, dim/2);

    CsPrintf (MSG_INITIALIZATION, "HeightMap initialized\n");

    return true;
}

void csLODTerrain::Draw (csRenderView& rview, bool use_z_buf)
{

// HELP again ....
    rview.g3d->SetObjectToCamera (&rview);
    rview.g3d->SetClipper (rview.view->GetClipPoly (), rview.view->GetNumVertices ());
    rview.g3d->SetPerspectiveAspect (rview.GetFOV ());
    rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE,
			       use_z_buf ? CS_ZBUF_USE : CS_ZBUF_FILL);

    position = rview.GetOrigin();

    resetQuadTree();

    setupQuadTree(dim/2,dim/2, dim/2);

// this is only for debugging
// if you have a terrain 129x129, it prints a nice QuadTree of the first
// run

/*    static bool first = true;
    int ix, iy;
    if(first) {
	for(iy=0; iy<dim; iy++) {
	    for(ix=0; ix<dim; ix++) {
		if(quadTree[ix][iy] == LOD_UNKNOWN)
		    CsPrintf (MSG_INITIALIZATION, " ");
		if(quadTree[ix][iy] == LOD_NODE_POINT)
		    CsPrintf (MSG_INITIALIZATION, "*");
		if(quadTree[ix][iy] == LOD_EDGE_POINT)
		    CsPrintf (MSG_INITIALIZATION, "-");
	    }
	    CsPrintf (MSG_INITIALIZATION, "\n");
	}
	first = false;
    }
*/

    g3dmesh->num_vertices = 0;
    g3dmesh->num_triangles = 0;

    draw (dim/2, dim/2, dim/2);

// ???? has this to be done all the time ????

    material->Visit ();
    g3dmesh->mat_handle[0] = material->GetMaterialHandle();

    if (rview.callback)
        rview.callback (&rview, CALLBACK_MESH, g3dmesh);
    else
	rview.g3d->DrawTriangleMesh (*g3dmesh);

}

int csLODTerrain::CollisionDetect (csTransform* transform)
{
  // @@@ To be implemented
  (void) transform;
  return false;
#if 0
    float height1;
    float height2;
    float height3;
    float relx;
    float rely;

    int x1, x2;
    int y1, y2;

// very simple collision detect
// only for debugging

    if(pos[0]<0 || pos[2]<0 || pos[0] > dim || pos[2] > dim)
	return 0;
    x1 = (int)floor(pos[0]);
    x2 = (int)ceil(pos[0]);
    y1 = (int)floor(pos[2]);
    y2 = (int)ceil(pos[2]);

    relx = pos[0] - x1;
    rely = pos[2] - y1;

    if(relx < rely) {
	height1 = heightMap[x1][y1]*scale;
	height2 = heightMap[x2][y2]*scale;
	height3 = heightMap[x1][y2]*scale;
	pos[1] = (height2-height3)*relx+(height3-height1)*rely+height1;
    }else{
	height1 = heightMap[x1][y1]*scale;
	height2 = heightMap[x2][y1]*scale;
	height3 = heightMap[x2][y2]*scale;
	pos[1] = (height2-height1)*relx+(height3-height2)*rely+height1;
    }
    pos[1] += 10;

    return 1;
#endif
}
