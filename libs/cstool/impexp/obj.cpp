

/*
    Crystal Space 3d format converter 


    Based on IVCON - converts various 3D graphics file
    Author: John Burkardt - used with permission
    CS adaption and conversion to C++ classes  Bruce Williams

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
#include "cstool/impexp.h"

// converter.cpp: implementation of the converter class.
//
//////////////////////////////////////////////////////////////////////

/******************************************************************************/

int converter::obj_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    OBJ_WRITE writes a Wavefront OBJ file.

  Example:

    #  magnolia.obj

    mtllib ./vp.mtl

    g
    v -3.269770 -39.572201 0.876128
    v -3.263720 -39.507999 2.160890
    ...
    v 0.000000 -9.988540 0.000000
    g stem
    s 1
    usemtl brownskn
    f 8 9 11 10
    f 12 13 15 14
    ...
    f 788 806 774

  Modified:

    01 September 1998

  Author:
 
    John Burkardt
*/
  int   i;
  int   iface;
  int   indexvn;
  int   ivert;
  int   k;
  int   converternew;
  int   num_text;
  float w;
/* 
  Initialize. 
*/
  num_text = 0;
  w = 1.0;

  fprintf ( fileout, "# %s created by IVCON.\n", fileout_name );
  fprintf ( fileout, "# Original data in %s.\n", filein_name );
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "g %s\n", object_name );
  fprintf ( fileout, "\n" );

  num_text = num_text + 5;
/* 
  V: vertex coordinates. 
*/
  for ( i = 0; i < num_cor3; i++ ) {
    fprintf ( fileout, "v %f %f %f %f\n", 
      cor3[0][i], cor3[1][i], cor3[2][i], w );
    num_text = num_text + 1;
  }

/* 
  VN: Vertex face normal vectors. 
*/
  if ( num_face > 0 ) {
    fprintf ( fileout, "\n" );
    num_text = num_text + 1;
  }

  for ( iface = 0; iface < num_face; iface++ ) {

    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {

      fprintf ( fileout, "vn %f %f %f\n", vertex_normal[0][ivert][iface],
        vertex_normal[1][ivert][iface], vertex_normal[2][ivert][iface] );
      num_text = num_text + 1;
    }
  }
/* 
  F: faces. 
*/
  if ( num_face > 0 ) {
    fprintf ( fileout, "\n" );
    num_text = num_text + 1;
  }

  indexvn = 0;

  for ( iface = 0; iface < num_face; iface++ ) {

    fprintf ( fileout, "f" );
    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
      indexvn = indexvn + 1;
      fprintf ( fileout, " %d//%d", face[ivert][iface]+1, indexvn );
    }
    fprintf ( fileout, "\n" );
    num_text = num_text + 1;
  }
/* 
  L: lines. 
*/
  if ( num_line > 0 ) {
    fprintf ( fileout, "\n" );
    num_text = num_text + 1;
  }

  converternew = TRUE;

  for ( i = 0; i < num_line; i++ ) {

    k = line_dex[i];

    if ( k == -1 ) {
      fprintf ( fileout, "\n" );
      num_text = num_text + 1;
      converternew = TRUE;
    }
    else {
      if ( converternew == TRUE ) {
        fprintf ( fileout, "l" );
        converternew = FALSE;
      }
      fprintf ( fileout, " %d", k+1 );
    }
    
  }

  fprintf ( fileout, "\n" );
  num_text = num_text + 1;
/*
  Report.
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "OBJ_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
