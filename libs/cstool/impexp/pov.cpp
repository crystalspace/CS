

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

int converter::pov_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:

    POV_WRITE writes graphics information to a POV file.

  Example:

    // cone.pov created by IVCON.
    // Original data in cone.iv

    #version 3.0
    #include "colors.inc"
    #include "shapes.inc"
    global_settings { assumed_gamma 2.2 }

    camera {
     right < 4/3, 0, 0>
     up < 0, 1, 0 >
     sky < 0, 1, 0 >
     angle 20
     location < 0, 0, -300 >
     look_at < 0, 0, 0>
    }

    light_source { < 20, 50, -100 > color White }

    background { color SkyBlue }

    #declare RedText = texture {
      pigment { color rgb < 0.8, 0.2, 0.2> }
      finish { ambient 0.2 diffuse 0.5 }
    }

    #declare BlueText = texture {
      pigment { color rgb < 0.2, 0.2, 0.8> }
      finish { ambient 0.2 diffuse 0.5 }
    }
    mesh {
      smooth_triangle {
        < 0.29, -0.29, 0.0>, < 0.0, 0.0, -1.0 >,
        < 38.85, 10.03, 0.0>, < 0.0, 0.0, -1.0 >,
        < 40.21, -0.29, 0.0>, <  0.0, 0.0, -1.0 >
        texture { RedText } }
        ...
      smooth_triangle {
        <  0.29, -0.29, 70.4142 >, < 0.0,  0.0, 1.0 >,
        <  8.56,  -2.51, 70.4142 >, < 0.0,  0.0, 1.0 >,
        <  8.85, -0.29, 70.4142 >, < 0.0,  0.0, 1.0 >
        texture { BlueText } }
    }

  Modified:

    08 October 1998

  Author:

    John Burkardt
*/
  int i;
  int j;
  int jj;
  int jlo;
  int k;
  int num_text;

  num_text = 0;
  fprintf ( fileout,  "// %s created by IVCON.\n", fileout_name );
  fprintf ( fileout,  "// Original data in %s.\n", filein_name );
  num_text = num_text + 2;
/*
  Initial declarations.
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "#version 3.0\n" );
  fprintf ( fileout, "#include \"colors.inc\"\n" );
  fprintf ( fileout, "#include \"shapes.inc\"\n" );
  fprintf ( fileout, "global_settings { assumed_gamma 2.2 }\n" );
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "camera {\n" );
  fprintf ( fileout, " right < 4/3, 0, 0>\n" );
  fprintf ( fileout, " up < 0, 1, 0 >\n" );
  fprintf ( fileout, " sky < 0, 1, 0 >\n" );
  fprintf ( fileout, " angle 20\n" );
  fprintf ( fileout, " location < 0, 0, -300 >\n" );
  fprintf ( fileout, " look_at < 0, 0, 0>\n" );
  fprintf ( fileout, "}\n" );
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "light_source { < 20, 50, -100 > color White }\n" );
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "background { color SkyBlue }\n" );

  num_text = num_text + 15;
/*
  Declare RGB textures.
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "#declare RedText = texture {\n" );
  fprintf ( fileout, "  pigment { color rgb < 0.8, 0.2, 0.2> }\n" );
  fprintf ( fileout, "  finish { ambient 0.2 diffuse 0.5 }\n" );
  fprintf ( fileout, "}\n" );
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "#declare GreenText = texture {\n" );
  fprintf ( fileout, "  pigment { color rgb < 0.2, 0.8, 0.2> }\n" );
  fprintf ( fileout, "  finish { ambient 0.2 diffuse 0.5 }\n" );
  fprintf ( fileout, "}\n" );
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "#declare BlueText = texture {\n" );
  fprintf ( fileout, "  pigment { color rgb < 0.2, 0.2, 0.8> }\n" );
  fprintf ( fileout, "  finish { ambient 0.2 diffuse 0.5 }\n" );
  fprintf ( fileout, "}\n" );
/*
  Write one big object.
*/
  fprintf ( fileout,  "mesh {\n" );
  num_text = num_text + 1;
/*
  Do the next face.
*/
  for ( i = 0; i < num_face; i++ ) {
/*
  Break the face up into triangles, anchored at node 1.
*/
    for ( jlo = 0; jlo < face_order[i] - 2; jlo++ ) {
      fprintf ( fileout, "  smooth_triangle {\n" );
      num_text = num_text + 1;

      for ( j = jlo; j < jlo + 3; j++ ) {

        if ( j == jlo ) {
          jj = 0;
        }
        else {
          jj = j;
        }

        k = face[jj][i];

        fprintf ( fileout, "<%f, %f, %f>, <%f, %f, %f>",
          cor3[0][k], cor3[1][k], cor3[2][k], 
          vertex_normal[0][jj][i], 
          vertex_normal[1][jj][i],
          vertex_normal[2][jj][i] );

        if ( j < jlo + 2 ) {
          fprintf ( fileout, ",\n" );
        }
        else {
          fprintf ( fileout, "\n" );
        }
        num_text = num_text + 1;

      }

      if (i%6 == 1 ) {
        fprintf ( fileout,  "texture { RedText } }\n" );
      }
      else if ( i%2 == 0 ) {
        fprintf ( fileout,  "texture { BlueText } }\n" );
      }
      else {
        fprintf ( fileout,  "texture { GreenText } }\n" );
      }
      num_text = num_text + 1;

    }

  }

  fprintf ( fileout,  "}\n" );
  num_text = num_text + 1;
/*
  Report.
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "POV_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
