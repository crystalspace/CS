

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

int converter::obj_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    OBJ_READ reads a Wavefront OBJ file.

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

    20 October 1998

  Author:
 
    John Burkardt
*/
  int   count;
  int   i, j;
  int   ivert;
  char *next;
  char *next2;
  char *next3;
  int   node;
  int   num_vertex_normal;
  float r1;
  float r2;
  float r3;
  char  token[MAX_INCHARS];
  char  token2[MAX_INCHARS];
  int   width;
/* 
  Initialize. 
*/
  num_vertex_normal = 0;
/* 
  Read the next line of the file into INPUT. 
*/
  while ( fgets ( input, MAX_INCHARS, filein ) != NULL ) {

    num_text = num_text + 1;
/* 
  Advance to the first nonspace character in INPUT. 
*/
    for ( next = input; *next != '\0' && isspace(*next); next++ ) {
    }
/* 
  Skip blank lines and comments. 
*/

    if ( *next == '\0' ) {
      continue;
    }

    if ( *next == '#' || *next == '$' ) {
      num_comment = num_comment + 1;
      continue;
    }
/* 
  Extract the first word in this line. 
*/
    sscanf ( next, "%s%n", token, &width );
/* 
  Set NEXT to point to just after this token. 
*/

    next = next + width;
/*
  BEVEL
  Bevel interpolation.
*/
    if ( leqi ( token, "BEVEL" ) == TRUE ) {
      continue;
    }
/*
  BMAT
  Basis matrix.
*/
    else if ( leqi ( token, "BMAT" ) == TRUE ) {
      continue;
    }
/*
  C_INTERP
  Color interpolation.
*/
    else if ( leqi ( token, "C_INTERP" ) == TRUE ) {
      continue;
    }
/*
  CON
  Connectivity between free form surfaces.
*/
    else if ( leqi ( token, "CON" ) == TRUE ) {
      continue;
    }
/*
  CSTYPE
  Curve or surface type.
*/
    else if ( leqi ( token, "CSTYPE" ) == TRUE ) {
      continue;
    }
/*
  CTECH
  Curve approximation technique.
*/
    else if ( leqi ( token, "CTECH" ) == TRUE ) {
      continue;
    }
/*
  CURV
  Curve.
*/
    else if ( leqi ( token, "CURV" ) == TRUE ) {
      continue;
    }
/*
  CURV2
  2D curve.
*/
    else if ( leqi ( token, "CURV2" ) == TRUE ) {
      continue;
    }
/*
  D_INTERP
  Dissolve interpolation.
*/
    else if ( leqi ( token, "D_INTERP" ) == TRUE ) {
      continue;
    }
/*
  DEG
  Degree.
*/
    else if ( leqi ( token, "DEG" ) == TRUE ) {
      continue;
    }
/*
  END
  End statement.
*/
    else if ( leqi ( token, "END" ) == TRUE ) {
      continue;
    }
/*  
  F V1 V2 V3
    or
  F V1/VT1/VN1 V2/VT2/VN2 ...
    or
  F V1//VN1 V2//VN2 ...

  Face.
  A face is defined by the vertices.
  Optionally, slashes may be used to include the texture vertex
  and vertex normal indices.

  OBJ line node indices are 1 based rather than 0 based.
  So we have to decrement them before loading them into FACE.
*/

    else if ( leqi ( token, "F" ) == TRUE ) {

      ivert = 0;
      face_order[num_face] = 0;
/*
  Read each item in the F definition as a token, and then
  take it apart.
*/
      for ( ;; ) {

        count = sscanf ( next, "%s%n", token2, &width );
        next = next + width;
 
        if ( count != 1 ) {
          break;
        }
 
        count = sscanf ( token2, "%d%n", &node, &width );
        next2 = token2 + width;

        if ( count != 1 ) {
          break;
        }

        if ( ivert < MAX_ORDER && num_face < MAX_FACE ) {
          face[ivert][num_face] = node-1;
          face_mat[ivert][num_face] = 0;
          face_order[num_face] = face_order[num_face] + 1;
        } 
/*
  If there's a slash, skip to the next slash, and extract the
  index of the normal vector.
*/
	//DEBUG_BREAK;
        if ( *next2 == '/' ) {
	  next2++;
	  if (*next2 != '/')
	  {
	    // read texture index
	    count = sscanf ( next2, "%d%n", &node, &width );
	    node = node - 1;
	    if ( 0 <= node && node < temp_num_cor3_uv )
	      face_texnode[ivert][num_face] = node;
	    next2 = next2 + width;
	  }
	  next3 = next2;
	  if ( *next3 == '/' )
	  {
	    next3 = next3 + 1;
	    count = sscanf ( next3, "%d%n", &node, &width );

	    node = node - 1;
	    if ( 0 <= node && node < num_vertex_normal )
	      for ( i = 0; i < 3; i++ )
		vertex_normal[i][ivert][num_face] = normal_temp[i][node];
	  }
        }
        ivert = ivert + 1;
      } 
      num_face = num_face + 1;
    }

/*  
  G  
  Group name.
*/

    else if ( leqi ( token, "G" ) == TRUE ) {
      continue;
    }
/*
  HOLE
  Inconverternew trimming hole.
*/
    else if ( leqi ( token, "HOLE" ) == TRUE ) {
      continue;
    }
/*  
  L  
  I believe OBJ line node indices are 1 based rather than 0 based.
  So we have to decrement them before loading them into LINE_DEX.
*/

    else if ( leqi ( token, "L" ) == TRUE ) {

      for ( ;; ) {

        count = sscanf ( next, "%d%n", &node, &width );
        next = next + width;

        if ( count != 1 ) {
          break;
        }

        if ( num_line < MAX_LINE  ) {
          line_dex[num_line] = node-1;
          line_mat[num_line] = 0;
        } 
        num_line = num_line + 1;

      } 

      if ( num_line < MAX_LINE ) {
        line_dex[num_line] = -1;
        line_mat[num_line] = -1;
      }
      num_line = num_line + 1;

    }

/*
  LOD
  Level of detail.
*/
    else if ( leqi ( token, "LOD" ) == TRUE ) {
      continue;
    }
/*
  MG
  Merging group.
*/
    else if ( leqi ( token, "MG" ) == TRUE ) {
      continue;
    }
/*
  MTLLIB
  Material library.
*/

    else if ( leqi ( token, "MTLLIB" ) == TRUE ) {
      continue;
    }
/*
  O
  Object name.
*/
    else if ( leqi ( token, "O" ) == TRUE ) {
      continue;
    }
/*
  P
  Point.
*/
    else if ( leqi ( token, "P" ) == TRUE ) {
      continue;
    }
/*
  PARM
  Parameter values.
*/
    else if ( leqi ( token, "PARM" ) == TRUE ) {
      continue;
    }
/*
  S  
  Smoothing group
*/
    else if ( leqi ( token, "S" ) == TRUE ) {
      continue;
    }
/*
  SCRV
  Special curve.
*/
    else if ( leqi ( token, "SCRV" ) == TRUE ) {
      continue;
    }
/*
  SHADOW_OBJ
  Shadow casting.
*/
    else if ( leqi ( token, "SHADOW_OBJ" ) == TRUE ) {
      continue;
    }
/*
  SP
  Special point.
*/
    else if ( leqi ( token, "SP" ) == TRUE ) {
      continue;
    }
/*
  STECH
  Surface approximation technique.
*/
    else if ( leqi ( token, "STECH" ) == TRUE ) {
      continue;
    }
/*
  STEP
  Stepsize.
*/
    else if ( leqi ( token, "CURV" ) == TRUE ) {
      continue;
    }
/*
  SURF
  Surface.
*/
    else if ( leqi ( token, "SURF" ) == TRUE ) {
      continue;
    }
/*
  TRACE_OBJ
  Ray tracing.
*/
    else if ( leqi ( token, "TRACE_OBJ" ) == TRUE ) {
      continue;
    }
/*
  TRIM
  Outer trimming loop.
*/
    else if ( leqi ( token, "TRIM" ) == TRUE ) {
      continue;
    }
/*
  USEMTL  
  Material name.
*/
    else if ( leqi ( token, "USEMTL" ) == TRUE ) {
      continue;
    }

/*
  V X Y Z W
  Geometric vertex.
  W is optional, a weight for rational curves and surfaces.
  The default for W is 1.
*/

    else if ( leqi ( token, "V" ) == TRUE ) {

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

      if ( temp_num_cor3 < MAX_COR3 ) {
        temp_cor3[0][temp_num_cor3] = r1;
        temp_cor3[1][temp_num_cor3] = r2;
        temp_cor3[2][temp_num_cor3] = r3;
      }

      temp_num_cor3 = temp_num_cor3 + 1;

    }
/*
  VN
  Vertex normals.
*/

    else if ( leqi ( token, "VN" ) == TRUE ) {

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

      if ( num_vertex_normal < MAX_ORDER * MAX_FACE ) {
        normal_temp[0][num_vertex_normal] = r1;
        normal_temp[1][num_vertex_normal] = r2;
        normal_temp[2][num_vertex_normal] = r3;
      }

      num_vertex_normal = num_vertex_normal + 1;

    }
/*
  VT
  Vertex texture.
*/
    else if ( leqi ( token, "VT" ) == TRUE ) {

      sscanf ( next, "%e %e", &r1, &r2 );

      if ( temp_num_cor3_uv < MAX_COR3 ) {
        temp_cor3_uv[0][temp_num_cor3_uv] = r1;
        temp_cor3_uv[1][temp_num_cor3_uv] = r2;
	temp_num_cor3_uv = temp_num_cor3_uv + 1;
      }

    }
/*
  VP
  Parameter space vertices.
*/
    else if ( leqi ( token, "VP" ) == TRUE ) {
      continue;
    }
/*
  Unrecognized  
*/
    else {
      num_bad = num_bad + 1;
    }

  }

/*
  create new vertices if 2 faces share a vertex but have different texturecoo assigned
  to the vertex.
*/
  for (i=0; i < num_face; i++)
  {
    for (j=0; j < face_order[i]; j++)
    {
      int vidx = face[j][i];
      int vtidx = face_texnode[j][i];
      face[j][i] = makeunique (vidx, vtidx);
    }
  }
  return SUCCESS;
}

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
