
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
#include "csparser/impexp.h"

// converter.cpp: implementation of the converter class.
//
//////////////////////////////////////////////////////////////////////



/******************************************************************************/

int converter::ase_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    ASE_READ reads an AutoCAD ASE file.

  Modified:

    19 November 1998
	15 April 2001 - Added texture mapping (Luca Pancallo)

  Author:

    John Burkardt
*/
  float bval;
  int   count;
  float gval;
  int   i;
  int   iface = 0;
  int   ivert = 0;
  int   itvert = 0;
  int   iword;
  int   j;
  int   khi;
  int   l;
  int   level;
  int   nbase;
  char *next;
  int   nlbrack;
  int   nrbrack;
  float rval;
  float temp;
  int   width;
  char  word[MAX_INCHARS];
  char  word1[MAX_INCHARS];
  char  word2[MAX_INCHARS];
  char  wordm1[MAX_INCHARS];
  float x;
  float y;
  float z;
  float u;
  float v;
  float w;

  level = 0;
  strcpy ( levnam[0], "Top" );
  nbase = 0;
  nlbrack = 0;
  nrbrack = 0;

  strcpy ( word, " " );
  strcpy ( wordm1, " " );
/*
  Read a line of text from the file.
*/
  while ( TRUE ) {

    if ( fgets ( input, MAX_INCHARS, filein ) == NULL ) {
      break;
    }

    num_text = num_text + 1;
    next = input;
    iword = 0;
/*
  Read the next word from the line.
*/
    while ( TRUE ) {

      strcpy ( wordm1, word );
      strcpy ( word, " " );

      count = sscanf ( next, "%s%n", word, &width );
      next = next + width;

      if ( count <= 0 ) {
        break;
      }

      iword = iword + 1;

      if ( iword == 1 ) {
        strcpy ( word1, word );
      }
/*
  In case the converternew word is a bracket, update the bracket count.
*/
      if ( strcmp ( word, "{" ) == 0 ) {

        nlbrack = nlbrack + 1;
        level = nlbrack - nrbrack;
        strcpy ( levnam[level], wordm1 );
      }
      else if ( strcmp ( word, "}" ) == 0 ) {

        nrbrack = nrbrack + 1;

        if ( nlbrack < nrbrack ) {

          fprintf ( logfile,  "\n" );
          fprintf ( logfile,  "ASE_READ - Fatal error!\n" );
          fprintf ( logfile,  "  Extraneous right bracket on line %d\n", num_text );
          fprintf ( logfile,  "Currently processing field:\n" );
          fprintf ( logfile,  "%s\n", levnam[level] );
          return ERROR;
        }

      }
/*
  *3DSMAX_ASCIIEXPORT  200
*/
      if ( strcmp ( word1, "*3DSMAX_ASCIIEXPORT" ) == 0 ) {
        break;
      }
/*
  *COMMENT
*/
      else if ( strcmp ( word1, "*COMMENT" ) == 0 ) {
        break;
      }
/*
  *GEOMOBJECT
*/
      else if ( strcmp ( levnam[level], "*GEOMOBJECT" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
/*
  Why don't you read and save this name?
*/
        else if ( strcmp ( word, "*NODE_NAME" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*NODE_TM" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "*MESH" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "*PROP_CASTSHADOW" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*PROP_MOTIONBLUR" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*PROP_RECVSHADOW" ) == 0 ) {
          break;
        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in GEOMOBJECT, line %d\n", num_text );
          break;
        }
      }
/*
  *MESH
*/
      else if ( strcmp ( levnam[level], "*MESH" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word, "*MESH_CFACELIST" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "*MESH_CVERTLIST" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "*MESH_FACE_LIST" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "*MESH_NORMALS" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "*MESH_NUMCVERTEX" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*MESH_NUMCVFACES" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*MESH_NUMFACES" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*MESH_NUMTVERTEX" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*MESH_NUMTVFACES" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*MESH_NUMVERTEX" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*MESH_TFACELIST" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "*MESH_TVERTLIST" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "*MESH_VERTEX_LIST" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "*TIMEVALUE" ) == 0 ) {
          break;
        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in MESH, line %d\n", num_text );
          break;
        }
      }
/*
  *MESH_CFACELIST
*/
      else if ( strcmp ( levnam[level], "*MESH_CFACELIST" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word, "*MESH_CFACE" ) == 0 ) {
          break;
        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in MESH_CFACE, line %d\n", num_text );
          break;
        }
      }
/*
  *MESH_CVERTLIST

  Mesh vertex indices must be incremented by NBASE+1 before being stored
  in the internal array.
*/
      else if ( strcmp ( levnam[level], "*MESH_CVERTLIST" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word, "*MESH_VERTCOL" ) == 0 ) {

          count = sscanf ( next, "%d%n", &i, &width );
          next = next + width;

          i = i + nbase;

          count = sscanf ( next, "%f%n", &rval, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &gval, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &bval, &width );
          next = next + width;

          if ( i < MAX_COR3 ) {
            cor3_rgb[0][i] = rval;
            cor3_rgb[1][i] = gval;
            cor3_rgb[2][i] = bval;
          }
        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in MESH_CVERTLIST, line %d\n", num_text );
          break;
        }

      }
/*
  *MESH_FACE_LIST
  This coding assumes a face is always triangular or quadrilateral.
*/
      else if ( strcmp ( levnam[level], "*MESH_FACE_LIST" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word, "*MESH_FACE" ) == 0 ) {

          if ( num_face < MAX_FACE ) {

            count = sscanf ( next, "%d%n", &i, &width );
            next = next + width;

            count = sscanf ( next, "%s%n", word2, &width );
            next = next + width;
            count = sscanf ( next, "%s%n", word2, &width );
            next = next + width;

            count = sscanf ( next, "%d%n", &i, &width );
            next = next + width;
            face[0][num_face] = i + nbase;
            face_mat[0][num_face] = 0;
            face_order[num_face] = face_order[num_face] + 1;

            count = sscanf ( next, "%s%n", word2, &width );
            next = next + width;

            count = sscanf ( next, "%d%n", &i, &width );
            next = next + width;
            face[1][num_face] = i + nbase;
            face_mat[1][num_face] = 0;
            face_order[num_face] = face_order[num_face] + 1;

            count = sscanf ( next, "%s%n", word2, &width );
            next = next + width;

            count = sscanf ( next, "%d%n", &i, &width );
            next = next + width;
            face[2][num_face] = i + nbase;
            face_mat[2][num_face] = 0;
            face_order[num_face] = face_order[num_face] + 1;

            count = sscanf ( next, "%s%n", word2, &width );
            next = next + width;

            if ( strcmp ( word2, "D:" ) == 0 ) {
              count = sscanf ( next, "%d%n", &i, &width );
              next = next + width;
              face[3][num_face] = i + nbase;
              face_mat[3][num_face] = 0;
              face_order[num_face] = face_order[num_face] + 1;
            }
          }

          num_face = num_face + 1;

          break;

        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in MESH_FACE_LIST, line %d\n", num_text );
          break;
        }
      }
/*
  *MESH_TVERTLIST
*/
      else if ( strcmp ( levnam[level], "*MESH_TVERTLIST" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word, "*MESH_TVERT" ) == 0 ) {

          count = sscanf ( next, "%d%n", &itvert, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &u, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &v, &width );
          next = next + width;

		  // this value is always 0.0 - ignored
          count = sscanf ( next, "%f%n", &w, &width );
          next = next + width;

          itvert = itvert + nbase;
          ivert = 0;

          vertex_uv[0][0][itvert] = u;
          vertex_uv[1][0][itvert] = v;
          //vertex_uv[2][0][itvert] = w; - ignored

          break;

        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in MESH_TVERTLIST, line %d\n", num_text );
          break;
        }
      }
/*
  *MESH_NORMALS
*/
      else if ( strcmp ( levnam[level], "*MESH_NORMALS" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word, "*MESH_FACENORMAL" ) == 0 ) {

          count = sscanf ( next, "%d%n", &iface, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &x, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &y, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &z, &width );
          next = next + width;

          iface = iface + nbase;
          ivert = 0;

          face_normal[0][iface] = x;
          face_normal[1][iface] = y;
          face_normal[2][iface] = z;

          break;

        }
        else if ( strcmp ( word, "*MESH_VERTEXNORMAL" ) == 0 ) {

          count = sscanf ( next, "%d%n", &i, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &x, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &y, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &z, &width );
          next = next + width;

          vertex_normal[0][ivert][iface] = x;
          vertex_normal[1][ivert][iface] = y;
          vertex_normal[2][ivert][iface] = z;
          ivert = ivert + 1;

          break;
        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in MESH_NORMALS, line %d\n", num_text );
          break;
        }
      }

/*
  *MESH_TFACELIST
*/
      else if ( strcmp ( levnam[level], "*MESH_TFACELIST" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word1, "*MESH_TFACE" ) == 0 ) {
          break;
        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in MESH_TFACE_LIST, line %d\n", num_text );
          break;
        }
      }
/*
  *MESH_TVERTLIST
*/
      else if ( strcmp ( levnam[level], "*MESH_TVERTLIST" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word1, "*MESH_TVERT" ) == 0  ) {
          break;
        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in MESH_TVERTLIST, line %d\n", num_text );
          break;
        }
      }
/*
  *MESH_VERTEX_LIST
*/
      else if ( strcmp ( levnam[level], "*MESH_VERTEX_LIST" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          nbase = num_cor3;
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word1, "*MESH_VERTEX" ) == 0 ) {

          count = sscanf ( next, "%d%n", &i, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &x, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &y, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &z, &width );
          next = next + width;

          i = i + nbase;
          num_cor3 = MAX ( num_cor3, i+1 );

          if ( i < MAX_COR3 ) {

            cor3[0][i] =
              transform_mat[0][0] * x 
            + transform_mat[0][1] * y 
            + transform_mat[0][2] * z 
            + transform_mat[0][3];

            cor3[1][i] =
              transform_mat[1][0] * x 
            + transform_mat[1][1] * y 
            + transform_mat[1][2] * z 
            + transform_mat[1][3];

            cor3[2][i] =
              transform_mat[2][0] * x 
            + transform_mat[2][1] * y 
            + transform_mat[2][2] * z 
            + transform_mat[2][3];
          }

          break;
        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in MESH_VERTEX_LIST, line %d\n", num_text );
          break;
        }
      }
/*
  *NODE_TM

  Each node should start out with a default transformation matrix.
*/
      else if ( strcmp ( levnam[level], "*NODE_TM" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {

          for ( i = 0; i < 4; i++ ) {
            for ( j = 0; j < 4; j++ ) {
              if ( i == j ) {
                transform_mat[j][i] = 1.0;
              }
              else {
                transform_mat[j][i] = 0.0;
              }
            }
          }

          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word, "*INHERIT_POS" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*INHERIT_ROT" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*INHERIT_SCL" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*NODE_NAME" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*TM_POS" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*TM_ROTANGLE" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*TM_ROTAXIS" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*TM_ROW0" ) == 0 ) {

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[0][0] = temp;

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[1][0] = temp;

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[2][0] = temp;

          break;
        }
        else if ( strcmp ( word, "*TM_ROW1" ) == 0 ) {

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[0][1] = temp;

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[1][1] = temp;

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[2][1] = temp;

          break;
        }
        else if ( strcmp ( word, "*TM_ROW2" ) == 0 ) {

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[0][2] = temp;

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[1][2] = temp;

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[2][2] = temp;

          break;
        }
        else if ( strcmp ( word, "*TM_ROW3" ) == 0 ) {

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[0][3] = temp;

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[1][3] = temp;

          count = sscanf ( next, "%f%n", &temp, &width );
          next = next + width;
          transform_mat[2][3] = temp;

          break;
        }
        else if ( strcmp ( word, "*TM_SCALE" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*TM_SCALEAXIS" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*TM_SCALEAXISANG" ) == 0 ) {
          break;
        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in NODE_TM, line %d\n", num_text );
          break;
        }
      }
/*
  *SCENE
*/
      else if ( strcmp ( levnam[level], "*SCENE" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else if ( strcmp ( word, "*SCENE_AMBIENT_STATIC" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*SCENE_BACKGROUND_STATIC" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*SCENE_FILENAME" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*SCENE_FIRSTFRAME" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*SCENE_FRAMESPEED" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*SCENE_LASTFRAME" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "*SCENE_TICKSPERFRAME" ) == 0 ) {
          break;
        }
        else {
          num_bad = num_bad + 1;
          fprintf ( logfile,  "Bad data in SCENE, line %d\n", num_text );
          break;
        }

      }

    }
/*
  End of loop reading words from the line.
*/
  }
/*
  End of loop reading lines from input file.
*/

/*
  Assign face vertex colors based on node colors.
*/
  khi = MIN ( num_face, MAX_FACE );

  for ( iface = 0; iface < khi; iface++ ) {
    for ( ivert = 0; ivert < 3; ivert++ ) {
      l = face[ivert][iface];
      for ( i = 0; i < 3; i++ ) {
        vertex_rgb[i][ivert][iface] = cor3_rgb[i][l];
      }
    }
  }

  return true;
}
/******************************************************************************/

int converter::ase_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:

    ASE_WRITE writes graphics information to an AutoCAD ASE file.

  Modified:

    30 September 1998
	15 April 2001 - Added texture mapping (Luca Pancallo)

  Author:

    John Burkardt

*/
  int i1;
  int i2;
  int i3;
  int i4;
  int iface;
  int ivert;
  int itvert;
  int j;
  int num_text;

  num_text = 0;
/*
  Write the header.
*/
  fprintf ( fileout, "*3DSMAX_ASCIIEXPORT 200\n" );
  fprintf ( fileout, "*COMMENT \"%s, created by IVCON.\"\n", fileout_name );
  fprintf ( fileout, "*COMMENT \"Original data in %s\"\n", filein_name );

  num_text = num_text + 3;
/*
  Write the scene block.
*/
  fprintf ( fileout, "*SCENE {\n" );
  fprintf ( fileout, "  *SCENE_FILENAME \"\"\n" );
  fprintf ( fileout, "  *SCENE_FIRSTFRAME 0\n" );
  fprintf ( fileout, "  *SCENE_LASTFRAME 100\n" );
  fprintf ( fileout, "  *SCENE_FRAMESPEED 30\n" );
  fprintf ( fileout, "  *SCENE_TICKSPERFRAME 160\n" );
  fprintf ( fileout, "  *SCENE_BACKGROUND_STATIC 0.0000 0.0000 0.0000\n" );
  fprintf ( fileout, "  *SCENE_AMBIENT_STATIC 0.0431 0.0431 0.0431\n" );
  fprintf ( fileout, "}\n" );

  num_text = num_text + 9;
/*
  Begin the big geometry block.
*/
  fprintf ( fileout, "*GEOMOBJECT {\n" );
  fprintf ( fileout, "  *NODE_NAME \"%s\"\n", object_name );

  num_text = num_text + 2;
/*
  Sub block NODE_TM:
*/
  fprintf ( fileout, "  *NODE_TM {\n" );
  fprintf ( fileout, "    *NODE_NAME \"Object01\"\n" );
  fprintf ( fileout, "    *INHERIT_POS 0 0 0\n" );
  fprintf ( fileout, "    *INHERIT_ROT 0 0 0\n" );
  fprintf ( fileout, "    *INHERIT_SCL 0 0 0\n" );
  fprintf ( fileout, "    *TM_ROW0 1.0000 0.0000 0.0000\n" );
  fprintf ( fileout, "    *TM_ROW1 0.0000 1.0000 0.0000\n" );
  fprintf ( fileout, "    *TM_ROW2 0.0000 0.0000 1.0000\n" );
  fprintf ( fileout, "    *TM_ROW3 0.0000 0.0000 0.0000\n" );
  fprintf ( fileout, "    *TM_POS 0.0000 0.0000 0.0000\n" );
  fprintf ( fileout, "    *TM_ROTAXIS 0.0000 0.0000 0.0000\n" );
  fprintf ( fileout, "    *TM_ROTANGLE 0.0000\n" );
  fprintf ( fileout, "    *TM_SCALE 1.0000 1.0000 1.0000\n" );
  fprintf ( fileout, "    *TM_SCALEAXIS 0.0000 0.0000 0.0000\n" );
  fprintf ( fileout, "    *TM_SCALEAXISANG 0.0000\n" );
  fprintf ( fileout, "  }\n" );

  num_text = num_text + 16;
/*
  Sub block MESH:
    Items
*/
  fprintf ( fileout, "  *MESH {\n" );
  fprintf ( fileout, "    *TIMEVALUE 0\n" );
  fprintf ( fileout, "    *MESH_NUMVERTEX %d\n", num_cor3 );
  fprintf ( fileout, "    *MESH_NUMFACES %d\n", num_face );

  num_text = num_text + 4;
/*
  Sub sub block MESH_VERTEX_LIST
*/
  fprintf ( fileout, "    *MESH_VERTEX_LIST {\n" );
  num_text = num_text + 1;

  for ( j = 0; j < num_cor3; j++ ) {
    fprintf ( fileout, "      *MESH_VERTEX %d %f %f %f\n", j, cor3[0][j],
      cor3[1][j], cor3[2][j] );
    num_text = num_text + 1;
  }

  fprintf ( fileout, "    }\n" );
  num_text = num_text + 1;
/*
  Sub sub block MESH_FACE_LIST
    Items MESH_FACE
*/
  fprintf ( fileout, "    *MESH_FACE_LIST {\n" );
  num_text = num_text + 1;

  for ( iface = 0; iface < num_face; iface++ ) {

    i1 = face[0][iface];
    i2 = face[1][iface];
    i3 = face[2][iface];

    if ( face_order[iface] == 3 ) {
      fprintf ( fileout, "      *MESH_FACE %d: A: %d B: %d C: %d", iface, i1, i2, i3 ); 
      fprintf ( fileout, " AB: 1 BC: 1 CA: 1 *MESH_SMOOTHING *MESH_MTLID 1\n" );
      num_text = num_text + 1;
    }
    else if ( face_order[iface] == 4 ) {
      i4 = face[3][iface];
      fprintf ( fileout, "      *MESH_FACE %d: A: %d B: %d C: %d D: %d", iface, i1, i2, i3, i4 ); 
      fprintf ( fileout, " AB: 1 BC: 1 CD: 1 DA: 1 *MESH_SMOOTHING *MESH_MTLID 1\n" );
      num_text = num_text + 1;
    }
  }

  fprintf ( fileout, "    }\n" );
  num_text = num_text + 1;
/*
  Item MESH_NUMTVERTEX.
*/
  fprintf ( fileout, "    *MESH_NUMTVERTEX 0\n" );
  num_text = num_text + 1;
/*
  Item NUMCVERTEX.
*/
  fprintf ( fileout, "    *MESH_NUMCVERTEX 0\n" );
  num_text = num_text + 1;
/*
  Sub block MESH_NORMALS
    Items MESH_FACENORMAL, MESH_VERTEXNORMAL (repeated)
*/
  fprintf ( fileout, "    *MESH_NORMALS {\n" );
  num_text = num_text + 1;

  for ( iface = 0; iface < num_face; iface++ ) {

    fprintf ( fileout, "      *MESH_FACENORMAL %d %f %f %f\n", 
      iface, face_normal[0][iface], face_normal[1][iface], face_normal[2][iface] );
    num_text = num_text + 1;

    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
      fprintf ( fileout, "      *MESH_VERTEXNORMAL %d %f %f %f\n", 
        face[ivert][iface], vertex_normal[0][ivert][iface], 
        vertex_normal[1][ivert][iface], vertex_normal[2][ivert][iface] );
      num_text = num_text + 1;
    }
  }

  fprintf ( fileout, "    }\n" );
  num_text = num_text + 1;

/*
  Sub block MESH_TVERTLIST
    Items MESH_TVERT (repeated)
*/
  fprintf ( fileout, "    *MESH_TVERTLIST {\n" );
  num_text = num_text + 1;

  for ( itvert = 0; itvert < num_cor3; itvert++ ) {

    fprintf ( fileout, "      *MESH_TVERT %d %f %f 0.0\n", 
      itvert, vertex_uv[0][0][itvert], vertex_uv[1][0][itvert]);
    num_text = num_text + 1;
  }

/*
  Close the MESH object.
*/
  fprintf ( fileout, "  }\n" );
/*
  A few closing parameters.
*/
  fprintf ( fileout, "  *PROP_MOTIONBLUR 0\n" );
  fprintf ( fileout, "  *PROP_CASTSHADOW 1\n" );
  fprintf ( fileout, "  *PROP_RECVSHADOW 1\n" );
/*
  Close the GEOM object.
*/
  fprintf ( fileout, "}\n" );

  num_text = num_text + 5;
/*
  Report.
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "ASE_WRITE - Wrote %d text lines;\n", num_text );

  return SUCCESS;
}
