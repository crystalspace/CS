// converter.cpp: implementation of the converter class.
//
//////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "sysdef.h"
#include "csutil/converter.h"
#include "csengine/objects/thing.h"
#include "csengine/objects/cssprite.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

converter::converter()
{

}

converter::~converter()
{

}


/*
  Purpose:

   IVCON converts various 3D graphics files.

  Modified:

    27 April 1999

  Author:

    John Burkardt
*/
 




/******************************************************************************/

int converter::ivcon ( char *command ) {

/******************************************************************************/

   int result;

   init_program_data ( );

   comline ( command );
  
   return result;
}

/******************************************************************************/

int converter::ase_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    ASE_READ reads an AutoCAD ASE file.

  Modified:

    19 November 1998

  Author:

    John Burkardt
*/
  float bval;
  int   count;
  float gval;
  int   i;
  int   iface;
  int   ivert;
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
  char  word[MAX_INPUT];
  char  word1[MAX_INPUT];
  char  word2[MAX_INPUT];
  char  wordm1[MAX_INPUT];
  float x;
  float y;
  float z;

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

    if ( fgets ( input, MAX_INPUT, filein ) == NULL ) {
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

          printf ( "\n" );
          printf ( "ASE_READ - Fatal error!\n" );
          printf ( "  Extraneous right bracket on line %d\n", num_text );
          printf ( "Currently processing field:\n" );
          printf ( "%s\n", levnam[level] );
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
          printf ( "Bad data in GEOMOBJECT, line %d\n", num_text );
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
          printf ( "Bad data in MESH, line %d\n", num_text );
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
          printf ( "Bad data in MESH_CFACE, line %d\n", num_text );
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
          printf ( "Bad data in MESH_CVERTLIST, line %d\n", num_text );
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
          printf ( "Bad data in MESH_FACE_LIST, line %d\n", num_text );
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
          printf ( "Bad data in MESH_NORMALS, line %d\n", num_text );
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
          printf ( "Bad data in MESH_TFACE_LIST, line %d\n", num_text );
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
          printf ( "Bad data in MESH_TVERTLIST, line %d\n", num_text );
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
          printf ( "Bad data in MESH_VERTEX_LIST, line %d\n", num_text );
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
          printf ( "Bad data in NODE_TM, line %d\n", num_text );
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
          printf ( "Bad data in SCENE, line %d\n", num_text );
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

  return SUCCESS;
}
/******************************************************************************/

int converter::ase_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:

    ASE_WRITE writes graphics information to an AutoCAD ASE file.

  Modified:

    30 September 1998

  Author:

    John Burkardt

*/
  int i1;
  int i2;
  int i3;
  int i4;
  int iface;
  int ivert;
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
  printf ( "\n" );
  printf ( "ASE_WRITE - Wrote %d text lines;\n", num_text );

  return SUCCESS;
}
/******************************************************************************/

void converter::ave_face_normal ( ) {

/******************************************************************************/

/*
  Purpose:

    AVE_FACE_NORMAL sets face normals as average of face vertex normals.

  Modified:

    09 October 1998

  Author:

    John Burkardt
*/
  int   i;
  int   iface;
  int   ivert;
  int   nfix;
  float norm;
  float x;
  float y;
  float z;

  if ( num_face <= 0 ) {
    return;
  }
  
  nfix = 0;

  for ( iface = 0; iface < num_face; iface++ ) {

/*
  Check the norm of the current normal vector.
*/
    x = face_normal[0][iface];
    y = face_normal[1][iface];
    z = face_normal[2][iface];
    norm = ( float ) sqrt ( x*x + y*y + z*z );

    if ( norm == 0.0 ) {

      nfix = nfix + 1;

      for ( i = 0; i < 3; i++ ) {
        face_normal[i][iface] = 0.0;
      }

      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = face_normal[i][iface] +
            vertex_normal[i][ivert][iface];
        }
      }

      x = face_normal[0][iface];
      y = face_normal[1][iface];
      z = face_normal[2][iface];
      norm = ( float ) sqrt ( x*x + y*y + z*z );

      if ( norm == 0.0 ) {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = ( float ) 1.0 / sqrt ( 3.0 );
        }
      }
      else {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = face_normal[i][iface] / norm;
        }
      }
    }
  }

  if ( nfix > 0 ) {
    printf ( "\n" );
    printf ( "AVE_FACE_NORMAL: Recomputed %d face normals\n", nfix );
    printf ( "  by averaging face vertex normals.\n" );
  }
  return;
}

/******************************************************************************/

int converter::char_index_last ( char* string, char c ) {

/******************************************************************************/

/*
  Purpose:

    CHAR_INDEX_LAST reports the last occurrence of a character in a string.

  Author:
 
    John Burkardt
*/
  int i;
  int j;
  int nchar;

  j = -1;

  nchar = strlen ( string );

  for ( i = 0; i < nchar; i++ ) {
    if ( string[i] == c ) {
      j = i;
    }
  }

  return j;

}
/******************************************************************************/

int converter::char_pad ( int *char_index, int *null_index, char *string, 
  int MAX_STRING ) {

/******************************************************************************/

/*
  Purpose:

    CHAR_PAD "pads" a character in a string with a blank on either side.

  Modified:

    16 October 1998

  Author:

    John Burkardt

  Parameters:

    Input/output, int *CHAR_INDEX, the position of the character to be padded.
    On output, this is increased by 1.

    Input/output, int *NULL_INDEX, the position of the terminating NULL in 
    the string.  On output, this is increased by 2.

    Input/output, char STRING[MAX_STRING], the string to be manipulated.

    Input, int MAX_STRING, the maximum number of characters that can be stored
    in the string.

    Output, int CHAR_PAD, is SUCCESS if the operation worked, and ERROR otherwise.
*/

  int i;

  if ( *char_index < 0 || *char_index >= *null_index || *char_index > MAX_STRING-1 ) {
    return ERROR;
  }

  if ( (*null_index) + 2 > MAX_STRING-1 ) {
    return ERROR;
  }

  for ( i = *null_index + 2; i > *char_index + 2; i-- ) {
    string[i] = string[i-2];
  }
  string[*char_index+2] = ' ';
  string[*char_index+1] = string[*char_index];
  string[*char_index] = ' ';

  *char_index = *char_index + 1;
  *null_index = *null_index + 2;

  return SUCCESS;
}
/******************************************************************************/

int converter::comline ( char *command) {

/******************************************************************************/

/*

  Purpose:

    COMLINE carries out a command-line session of file conversion.

  Modified:

    18 November 1998

  Author:
 
    John Burkardt
*/
  int   i;
  int   iarg;
  int   icor3;
  int   ierror;
  int   iface;
  int   ivert;
  int   revnorm;

/* 
  Initialize the data. 
*/
  iarg = 0;
  ierror = 0;

  data_init ( );

  strcpy ( filein_name, command);

  strcpy(fileout_name,"test.txt");




/*
  Read the input. 
*/
  ierror = data_read ( );

  if ( ierror == ERROR ) {
    printf ( "\n" );
    printf ( "COMLINE - Error!\n" );
    printf ( "  Failure while reading input data.\n" );
    return ERROR;
  }
/*
  Reverse the normal vectors if requested.
*/
  if ( revnorm == TRUE ) {

    for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {
      for ( i = 0; i < 3; i++ ) {
        cor3_normal[i][icor3] = - cor3_normal[i][icor3];
      }
    }

    for ( iface = 0; iface < num_face; iface++ ) {
      for ( i = 0; i < 3; i++ ) {
        face_normal[i][iface] = - face_normal[i][iface];
      }
    }

    for ( iface = 0; iface < num_face; iface++ ) {
      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        for ( i = 0; i < 3; i++ ) {
          vertex_normal[i][ivert][iface] = 
            - vertex_normal[i][ivert][iface];
        }
      }
    }
    printf ( "\n" );
    printf ( "INTERACT - Note:\n" );
    printf ( "  Reversed node, face, and vertex normals.\n" );
  }
/*
  Write the output. 
*/


  ierror = data_write ( );

  if ( ierror == ERROR ) {
    printf ( "\n" );
    printf ( "COMLINE - Error!\n" );
    printf ( "  Failure while writing output data.\n" );
    return ERROR;
  }
  return SUCCESS;
}
/**********************************************************************/

void converter::cor3_2_vertex_rgb ( void ) {

/**********************************************************************/

/*
  Purpose:

    COR3_RGB converts node RGB data to vertex RGB data.

  Modified:

    19 November 1998

  Author:

    John Burkardt
*/
  int icor3;
  int iface;
  int ivert;
  int j;

  for ( iface = 0; iface < num_face; iface ++ ) {
    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
      icor3 = face[ivert][iface];
      for ( j = 0; j < 3; j++ ) {
        vertex_rgb[j][ivert][iface] = cor3_rgb[j][icor3];
      }
    }
  }

  return;
}
/******************************************************************************/

void converter::data_check ( void ) {

/******************************************************************************/

/*
  Purpose:
   
    DATA_CHECK checks the input data.

  Modified:

    09 October 1998

  Author:
 
    John Burkardt
*/
  int iface;
  int nfix;

  if ( num_color > MAX_COLOR ) {
    printf ( "\n" );
    printf ( "DATA_CHECK: Warning!\n" );
    printf ( "  The input data requires %d colors.\n", num_color );
    printf ( "  There was only room for %d\n", MAX_COLOR );
    num_color = MAX_COLOR;
  }
  
  if ( num_cor3 > MAX_COR3 ) {
    printf ( "\n" );
    printf ( "DATA_CHECK: Warning!\n" );
    printf ( "  The input data requires %d points.\n", num_cor3 );
    printf ( "  There was only room for %d\n", MAX_COR3 );
    num_cor3 = MAX_COR3;
  }

  if ( num_face > MAX_FACE ) {
    printf ( "\n" );
    printf ( "DATA_CHECK: Warning!\n" );
    printf ( "  The input data requires %d faces.\n", num_face );
    printf ( "  There was only room for %d\n", MAX_FACE );
    num_face = MAX_FACE;
  }

  if ( num_line > MAX_LINE ) {
    printf ( "\n" );
    printf ( "DATA_CHECK - Warning!\n" );
    printf ( "  The input data requires %d line items.\n", num_line );
    printf ( "  There was only room for %d.\n", MAX_LINE );
    num_line = MAX_LINE;
  }

  nfix = 0;

  for ( iface = 0; iface < num_face; iface++ ) {

    if ( face_order[iface] > MAX_ORDER ) {
      face_order[iface] = MAX_ORDER;
      nfix = nfix + 1;
    }

  }

  if ( nfix > 0 ) {
    printf ( "\n" );
    printf ( "DATA_CHECK - Warning!\n" );
    printf ( "  Corrected %d faces using more than %d vertices per face.\n",
      nfix, MAX_ORDER );
  }
  return;
}
/******************************************************************************/

void converter::data_init ( ) {

/******************************************************************************/

/*
  Purpose:

    DATA_INIT initializes the internal graphics data.

  Modified:

    02 December 1998

  Author:

    John Burkardt
*/

  int i;
  int iface;
  int ivert;
  int j;
  int k;

  for ( i = 0; i < 3; i++ ) {
    for ( j = 0; j < MAX_COR3; j++ ) {
      cor3[i][j] = 0.0;
    }
  }

  for ( i = 0; i < 3; i++ ) {
    for ( j = 0; j < MAX_COR3; j++ ) {
      cor3_normal[i][j] = 0.0;
    }
  }

  for ( j = 0; j < MAX_COR3; j++ ) {
    cor3_rgb[0][j] = 0.299;
    cor3_rgb[1][j] = 0.587;
    cor3_rgb[2][j] = 0.114;
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    for ( ivert = 0; ivert < MAX_ORDER; ivert++ ) {
      face[ivert][iface] = 0;
    }
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    face_flags[iface] = 6;
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    for ( ivert = 0; ivert < MAX_ORDER; ivert++ ) {
      face_mat[ivert][iface] = 0;
    }
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    for ( i = 0; i < 3; i++ ) {
      face_normal[i][iface] = 0;
    }
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    face_object[iface] = -1;
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    face_order[iface] = 0;
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    face_smooth[iface] = 1;
  }

  for ( i = 0; i < MAX_LINE; i++ ) {
    line_dex[i] = -1;
  }

  for ( i = 0; i < MAX_LINE; i++ ) {
    line_mat[i] = 0;
  }

  for ( j = 0; j < MAX_ORDER*MAX_FACE; j++ ) {
    for ( i = 0; i < 3; i++ ) {
      normal_temp[i][j] = 0;
    }
  }

  num_color = 0;
  num_cor3 = 0;
  num_face = 0;
  num_line = 0;
  num_texmap = 0;

  strcpy ( object_name, "IVCON" );

  for ( i = 0; i < 3; i++ ) {
    origin[i] = 0.0;
  }

  for ( i = 0; i < 3; i++ ) {
    pivot[i] = 0.0;
  }

  for ( j = 0; j < MAX_COLOR; j++ ) {
    rgbcolor[0][j] = 0.299;
    rgbcolor[1][j] = 0.587;
    rgbcolor[2][j] = 0.114;
  }

  for ( j = 0; j < MAX_TEXMAP; j++ ) {
    strcpy ( texmap_name[j], "null" );
  }

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 4; j++ ) {
      if ( i == j ) {
        transform_mat[i][j] = 1.0;
      }
      else {
        transform_mat[i][j] = 0.0;
      }
    }
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    for ( ivert = 0; ivert < MAX_ORDER; ivert++ ) {
      for ( i = 0; i < 3; i++ ) {
        vertex_normal[i][ivert][iface] = 0.0;
      }
    }
  }

  for ( j = 0; j < 3; j++ ) {
    for ( k = 0; k < MAX_FACE; k++ ) {
      vertex_rgb[0][j][k] = 0.299;
      vertex_rgb[1][j][k] = 0.587;
      vertex_rgb[2][j][k] = 0.114;
    }
  }

  if ( debug == TRUE ) {
    printf ( "\n" );
    printf ( "DATA_INIT: Graphics data initialized.\n" );
  }

  return;
}
/******************************************************************************/

int converter::data_read ( void ) {

/******************************************************************************/

/*
  Purpose:

    DATA_READ reads a file into internal graphics data.

  Modified:

    19 November 1998

  Author:
 
    John Burkardt
*/
  FILE *filein;
  char *filein_type;
  int   ierror;
  int   ntemp;
/* 
  Retrieve the input file type. 
*/
  filein_type = file_ext ( filein_name );

  if ( filein_type == NULL ) {
    printf ( "\n" );
    printf ( "DATA_READ - Error!\n" );
    printf ( "  Could not determine the type of '%s'.\n", filein_name );
    return ERROR;
  }
  else if ( debug ) {
    printf ( "\n" );
    printf ( "DATA_READ: Input file has type %s.\n", filein_type );  
  }
/* 
  Open the file. 
*/
  if ( leqi ( filein_type, "3DS" ) == TRUE ) {
    filein = fopen ( filein_name, "rb" );
  }
  else {
    filein = fopen ( filein_name, "r" );
  }

  if ( filein == NULL ) {
    printf ( "\n" );
    printf ( "DATA_READ - Error!\n" );
    printf ( "  Could not open the input file '%s'!\n", filein_name );
    return ERROR;
  }
/*
  Initialize some data.
*/
  max_order2 = 0;
  num_bad = 0;
  num_color = 0;
  num_comment = 0;
  num_cor3 = 0;
  num_dup = 0;
  num_face = 0;
  num_group = 0;
  num_line = 0;
  num_object = 0;
  num_text = 0;
/*
  Read the input file. 
*/
  if ( leqi ( filein_type, "3DS" ) == TRUE ) {
    ierror = tds_read ( filein );
  }
  else if ( leqi ( filein_type, "ASE" ) == TRUE ) {
    ierror = ase_read ( filein );
    cor3_2_vertex_rgb ( );
  }
  else if ( leqi ( filein_type, "DXF" ) == TRUE ) {
    ierror = dxf_read ( filein );
  }
  else if ( leqi ( filein_type, "HRC" ) == TRUE ) {
    ierror = hrc_read ( filein );
  }
  else if ( leqi ( filein_type, "IV" ) == TRUE ) {
    ierror = iv_read ( filein );
  }
  else if ( leqi ( filein_type, "OBJ" ) == TRUE ) {
    ierror = obj_read ( filein );
  }
  else if ( leqi ( filein_type, "SMF" ) == TRUE ) {
    ierror = smf_read ( filein );
  }
  else if ( 
    leqi ( filein_type, "STL" ) == TRUE ||
    leqi ( filein_type, "STLA") == TRUE ) {
    ierror = stla_read ( filein );
  }
  else if ( leqi ( filein_type, "VLA" ) == TRUE ) {
    ierror = vla_read ( filein );
  }
  else {
    printf ( "\n" );
    printf ( "DATA_READ - Error!\n" );
    printf ( "  Unacceptable input file type.\n" );
    return ERROR;
  }

  fclose ( filein );
/*
  Report on what we read.
*/
  ntemp = MIN ( num_face, MAX_FACE );
  max_order2 = ivec_max (ntemp, face_order );

  data_report ( );
/*
  Warn about any errors that occurred during reading.
*/
  if ( ierror == ERROR ) {
    printf ( "\n" );
    printf ( "DATA_READ - Error!\n" );
    printf ( "  An error occurred while reading the input file.\n" );
    return ERROR;
  }
/*
  Check the data immediately.
*/
  data_check ( );
/*
  Recompute zero face-vertex normals from vertex positions.
*/
  set_vertex_normal ( );
/*
  Compute the node normals from the vertex normals.
*/
  set_cor3_normal ( );
/*
  Recompute zero face normals by averaging face-vertex normals.
*/
  ave_face_normal ( );
/*
  Report on the data.
*/
  minmax ( );

  return SUCCESS;
}
/**********************************************************************/

void converter::data_report ( void ) {

/**********************************************************************/

/*
  Purpose:

    DATA_REPORT gives a summary of the contents of the data file.

  Modified:

    20 October 1998

  Author:

    John Burkardt
*/

  printf ( "\n" );
  printf ( "DATA_REPORT - The input file contains:\n" );
  printf ( "\n" );
  printf ( "  Bad data items             %d\n", num_bad );
  printf ( "  Text lines                 %d\n", num_text );
  printf ( "  Colors                     %d\n", num_color );
  printf ( "  Comments                   %d\n", num_comment );
  printf ( "  Duplicate points           %d\n", num_dup );
  printf ( "  Faces                      %d\n", num_face );
  printf ( "  Groups                     %d\n", num_group );
  printf ( "  Vertices per face, maximum %d\n", max_order2 );
  printf ( "  Line items                 %d\n", num_line );
  printf ( "  Points                     %d\n", num_cor3 );
  printf ( "  Objects                    %d\n", num_object );

  return;
}
/******************************************************************************/

int converter::data_write ( void ) {

/******************************************************************************/

/*
  Purpose:

    DATA_WRITE writes the graphics data to the output file.

  Modified:

    09 October 1998

  Author:
 
    John Burkardt
*/
  FILE *fileout;
  char *fileout_type;
  int   result;

  result = SUCCESS;
/* 
  Retrieve the output file type. 
*/
  fileout_type = file_ext ( fileout_name );

  if ( fileout_type == NULL ) {
    printf ( "\n" );
    printf ( "DATA_WRITE - Error!\n" );
    printf ( "  Could not determine the output file type.\n" );
    return ERROR;
  }
/* 
  Open the output file. 
*/
  if ( leqi ( fileout_type, "3DS" ) == TRUE ) {
    fileout = fopen ( fileout_name, "wb" );
  }
  else {
    fileout = fopen ( fileout_name, "w" );
  }

  if ( fileout == NULL ) {
    printf ( "\n" );
    printf ( "DATA_WRITE - Error!\n" );
    printf ( "  Could not open the output file!\n" );
    return ERROR;
  }
/* 
  Write the output file. 
*/
  if ( leqi ( fileout_type, "3DS" ) == TRUE ) {
    tds_pre_process();
    result = tds_write ( fileout );
  }
  else if ( leqi ( fileout_type, "ASE" ) == TRUE ) {
    result = ase_write ( fileout );
  }
  else if ( leqi ( fileout_type, "DXF" ) == TRUE ) {
    result = dxf_write ( fileout );
  }
  else if ( leqi ( fileout_type, "HRC" ) == TRUE ) {
    result = hrc_write ( fileout );
  }
  else if ( leqi ( fileout_type, "IV" ) == TRUE ) {
    result = iv_write ( fileout );
  }
  else if ( leqi ( fileout_type, "OBJ" ) == TRUE ) {
    result = obj_write ( fileout );
  }
  else if ( leqi ( fileout_type, "POV" ) == TRUE ) {
    result = pov_write ( fileout );
  }
  else if ( leqi ( fileout_type, "SMF" ) == TRUE ) {
    result = smf_write ( fileout );
  }
  else if ( 
    leqi ( fileout_type, "STL" ) == TRUE ||
    leqi ( fileout_type, "STLA" ) == TRUE ) {
    result = stla_write ( fileout );
  }
  else if ( leqi ( fileout_type, "TXT" ) == TRUE ) {
    result = txt_write ( fileout );
  }
  else if ( leqi ( fileout_type, "VLA" ) == TRUE ) {
    result = vla_write ( fileout );
  }
  else {
    result = ERROR;
    printf ( "\n" );
    printf ( "DATA_WRITE - Error!\n" );
    printf ( "  Unacceptable output file type \"%s\".\n", fileout_type );
  }
/*  
  Close the output file. 
*/
  fclose ( fileout );

  if ( result == ERROR ) {
    return ERROR;
  }
  else {
    return SUCCESS;
  }
}
/******************************************************************************/

int converter::dxf_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    DXF_READ reads an AutoCAD DXF file.

  Examples:

      0
    SECTION
      2
    ENTITIES
      0
    LINE
      8
    0
     10
      (X coordinate of beginning of first line.)
     20
      (Y coordinate of beginning of first line.)
     30
      (Z coordinate of beginning of first line.)
     11
      (X coordinate of end of first line.)
     21
      (Y coordinate of end of first line.)
     31
      (Z coordinate of end of first line.)
      0
    LINE
      8
    0
     10
      (X coordinate of beginning of second line.)
     20
      ...etc...
     31
     (Z coordinate of end of last line.)
      0
    ENDSEC
      0
    EOF

  Modified:

    27 April 1999

  Author:
 
    John Burkardt
*/
  int   code;
  int   count;
  float cvec[3];
  int   icor3;
  char  input1[MAX_INPUT];
  char  input2[MAX_INPUT];
  float rval;
  int   width;
/* 
  Read the next two lines of the file into INPUT1 and INPUT2. 
*/

  while ( TRUE ) {

/* 
  INPUT1 should contain a single integer, which tells what INPUT2
  will contain.
*/
    if ( fgets ( input1, MAX_INPUT, filein ) == NULL ) {
      break;
    }

    num_text = num_text + 1;

    count = sscanf ( input1, "%d%n", &code, &width );
    if ( count <= 0 ) {
      break;
    }
/*
  Read the second line, and interpret it according to the code.
*/
    if ( fgets ( input2, MAX_INPUT, filein ) == NULL ) {
      break;
    }

    num_text = num_text + 1;

    if ( code == 10 || code == 20 || code == 30 ||
         code == 11 || code == 21 || code == 31 ) {

      count = sscanf ( input2, "%e%n", &rval, &width );

      if ( code == 10 && num_line > 0 ) {
        line_dex[num_line] = - 1;
        line_mat[num_line] = - 1;
        num_line = num_line + 1;
      }

      if ( code == 10 || code == 11 ) {
        cvec[0] = rval;
      }
      else if ( code == 20 || code == 21 ) {
        cvec[1] = rval;
      }
      else if ( code == 30 || code == 31 ) {

        cvec[2] = rval;

        if ( num_cor3 < 1000 ) {
          icor3 = rcol_find ( cor3, 3, num_cor3, cvec );
        }
        else {
          icor3 = -1;
        }

        if ( icor3 == -1 ) {
          icor3 = num_cor3;
          cor3[0][num_cor3] = cvec[0];
          cor3[1][num_cor3] = cvec[1];
          cor3[2][num_cor3] = cvec[2];
          num_cor3 = num_cor3 + 1;
        }
        else {
          num_dup = num_dup + 1;
        }

        line_dex[num_line] = icor3;
        line_mat[num_line] = 0;
        num_line = num_line + 1;
      }

    }

  }

  if ( num_line > 0 ) {
    line_dex[num_line] = - 1;
    line_mat[num_line] = - 1;
    num_line = num_line + 1;
  }
  return SUCCESS;
}
/******************************************************************************/

int converter::dxf_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    DXF_WRITE writes graphics information to an AutoCAD DXF file.

  Modified:

    18 August 1998

  Author:
 
    John Burkardt
*/
  int   iline;
  int   k;
  int   kold;
  int   converternewline;
  int   num_text;

/* 
  Initialize. 
*/
  num_text = 0;

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "HEADER\n" );
  fprintf ( fileout, "999\n" );
  fprintf ( fileout, "%s created by IVCON.\n", fileout_name );
  fprintf ( fileout, "999\n" );
  fprintf ( fileout, "Original data in %s.\n", filein_name );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );
  num_text = num_text + 8;

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "TABLES\n" );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );
  num_text = num_text + 4;

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "BLOCKS\n" );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );
  num_text = num_text + 8;

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "ENTITIES\n" );
  num_text = num_text + 8;

  kold = 0;
  converternewline = TRUE;

  for ( iline = 0; iline < num_line; iline++ ) {
    
    k = line_dex[iline];

    if ( k == -1 ) {

      converternewline = TRUE;
    }
    else {

      if ( converternewline == FALSE ) {

        fprintf ( fileout, "  0\n" );
        fprintf ( fileout, "LINE\n" );
        fprintf ( fileout, "  8\n" );
        fprintf ( fileout, "  0\n" );
        fprintf ( fileout, " 10\n" );
        fprintf ( fileout, "%f\n", cor3[0][kold] );
        fprintf ( fileout, " 20\n" );
        fprintf ( fileout, "%f\n", cor3[1][kold] );
        fprintf ( fileout, " 30\n" );
        fprintf ( fileout, "%f\n", cor3[2][kold] );
        fprintf ( fileout, " 11\n" );
        fprintf ( fileout, "%f\n", cor3[0][k] );
        fprintf ( fileout, " 21\n" );
        fprintf ( fileout, "%f\n", cor3[1][k] );
        fprintf ( fileout, " 31\n" );
        fprintf ( fileout, "%f\n", cor3[2][k] );

        num_text = num_text + 16;

      }

      kold = k;
      converternewline = FALSE;

    }
  }

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "EOF\n" );
  num_text = num_text + 4;
/*
  Report.
*/
  printf ( "\n" );
  printf ( "DXF_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
/******************************************************************************/

int converter::face_print ( int iface ) {

/******************************************************************************/

/*
  Purpose:

    FACE_PRINT prints out information about a face.

  Modified:

    31 August 1998

  Author:

    John Burkardt
*/
  int ivert;
  int j;
  int k;

  if ( iface < 0 || iface > num_face-1 ) {
    printf ( "\n" );
    printf ( "FACE_PRINT - Error!\n" );
    printf ( "  Face indices must be between 1 and %d\n", num_face );
    printf ( "  But your requested value was %d\n", iface );
    return ERROR;
  }

  printf ( "\n" );
  printf ( "FACE_PRINT\n" );
  printf ( "  Information about face %d\n", iface );
  printf ( "\n" );
  printf ( "  Number of vertices is %d\n", face_order[iface] );
  printf ( "\n" );
  printf ( "  Vertex list:\n" );
  printf ( "    Vertex #, Node #, Material #, X, Y, Z:\n" );
  printf ( "\n" );
  for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
    j = face[ivert][iface];
    k = face_mat[ivert][iface];
    printf ( " %d %d %d %f %f %f\n", ivert, j, k, cor3[0][j], cor3[1][j], 
     cor3[2][j] );
  }

  printf ( "\n" );
  printf ( "  Face normal vector:\n" );
  printf ( "\n" );
  printf ( " %f %f %f\n", face_normal[0][iface], face_normal[1][iface],
    face_normal[2][iface] );

  printf ( "\n" );
  printf ( "  Vertex face normals:\n" );
  printf ( "\n" );
  for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
    printf ( " %d %f %f %f\n", ivert, vertex_normal[0][ivert][iface],
      vertex_normal[1][ivert][iface], vertex_normal[2][ivert][iface] );
  }

  return SUCCESS;

}
/******************************************************************************/

int converter::face_subset ( void ) {

/******************************************************************************/


/*
  Purpose:

    FACE_SUBSET selects a subset of the current faces as the converternew object.

  Warning:

    The original graphic object is overwritten by the converternew one.

  Modified:

    12 October 1998

  Author:

    John Burkardt

*/
  int i;
  int iface;
  int iface1;
  int iface2;
  int inc;
  int ivert;
  int j;
  int k;
  int num_cor32;

  num_line = 0;
/*
  Get the first and last faces to save, IFACE1 and IFACE2.
*/
  printf ( "\n" );
  printf ( "Enter lowest face number to save between 0 and %d:\n", num_face-1 );
  scanf ( "%d", &iface1 );
  if ( iface1 < 0 || iface1 > num_face - 1 ) {
    printf ( "Illegal choice!\n" );
    return ERROR;
  }

  printf ( "\n" );
  printf ( "Enter highest face number to save between %d and %d:\n", 
    iface1, num_face-1 );
  scanf ( "%d", &iface2 );
  if ( iface2 < iface1 || iface2 > num_face - 1 ) {
    printf ( "Illegal choice!\n" );
    return ERROR;
  }

  inc = iface1;
/*
  "Slide" the data for the saved faces down the face arrays.
*/
  for ( iface = 0; iface < iface2 + 1 - iface1; iface++ ) {
    face_order[iface] = face_order[iface+inc];
    for ( ivert = 0; ivert < MAX_ORDER; ivert++ ) {
      face[ivert][iface] = face[ivert][iface+inc];
      face_mat[ivert][iface] = face_mat[ivert][iface+inc];
      for ( i = 0; i < 3; i++ ) {
        vertex_normal[i][ivert][iface] =
          vertex_normal[i][ivert][iface+inc];
        vertex_rgb[i][ivert][iface] = vertex_rgb[i][ivert][iface+inc];
      }
    }
    for ( i = 0; i < 3; i++ ) {
      face_normal[i][iface] = face_normal[i][iface+inc];
    }
  }
/*
  Now reset the number of faces.
*/
  num_face = iface2 + 1 - iface1;
/*
  Now, for each point I, set LIST(I) = J if point I is the J-th
  point we are going to save, and 0 otherwise.  Then J will be
  the converternew label of point I.
*/
  for ( i = 0; i < num_cor3; i++ ) {
    list[i] = -1;
  }

  num_cor32 = 0;

  for ( iface = 0; iface < num_face; iface++ ){
    for ( ivert = 0; ivert < face_order[iface]; ivert++ ){
      j = face[ivert][iface];
      if ( list[j] == -1 ) {
        num_cor32 = num_cor32 + 1;
        list[j] = num_cor32;
      }
    }
  }
/*
  Now make the nonzero list entries rise in order, so that
  we can compress the COR3 data in a minute.
*/
  num_cor32 = 0;

  for ( i = 0; i < num_cor3; i++ ) {
    if ( list[i] != -1 ) {
      list[i] = num_cor32;
      num_cor32 = num_cor32 + 1;
    }
  }
/*
  Relabel the FACE array with the converternew node indices.
*/
  for ( iface = 0; iface < num_face; iface++ ){
    for ( ivert = 0; ivert < face_order[iface]; ivert++ ){
      j = face[ivert][iface];
      face[ivert][iface] = list[j];
    }
  }
/*
  Rebuild the COR3 array by sliding data down.
*/
  for ( i = 0; i < num_cor3; i++ ){
    k = list[i];
    if ( k != -1 ) {
      for ( j = 0; j < 3; j++ ) {
        cor3[j][k] = cor3[j][i];
      }
    }
  }

  num_cor3 = num_cor32;

  return SUCCESS;
}

/******************************************************************************/

char * converter::file_ext ( char *file_name ) {

/******************************************************************************/

/*
  Purpose:
   
    FILE_EXT picks out the extension in a file name.

  Modified:

    21 July 1998

  Author:
 
    John Burkardt
*/
  int i;

  i = char_index_last ( file_name, '.' );

  if ( i == -1 ) {
    return NULL;
  }
  else {
    return file_name+i+1;
  }
}
/******************************************************************************/

void converter::hello ( void ) {

/******************************************************************************/
/*
  Purpose:

    HELLO prints an explanatory header message.

  Modified:

    19 October 1998

  Author:
 
    John Burkardt
*/
  printf ( "\n" );
  printf ( "Hello:  This is IVCON,\n" );
  printf ( "  for 3D graphics file conversion.\n" );
  printf ( "\n" );
  printf ( "    \".3ds\"   3D Studio Max binary;\n" );
  printf ( "    \".ase\"   3D Studio Max ASCII export;\n" );
  printf ( "    \".dxf\"   DXF;\n" );
  printf ( "    \".hrc\"   SoftImage hierarchy;\n" );
  printf ( "    \".iv\"    SGI Open Inventor;\n" );
  printf ( "    \".obj\"   WaveFront Advanced Visualizer;\n" );
  printf ( "    \".pov\"   Persistence of Vision (output only);\n" );
  printf ( "    \".smf\"   Michael Garland's format;\n" );
  printf ( "    \".stl\"   ASCII StereoLithography;\n" );
  printf ( "    \".stla\"  ASCII StereoLithography;\n" );
  printf ( "    \".txt\"   Text (output only);\n" );
  printf ( "    \".vla\"   VLA.\n" );
  printf ( "\n" );
  printf ( "  Current limits include:\n" );
  printf ( "    %d faces;\n", MAX_FACE );
  printf ( "    %d line items;\n", MAX_LINE );
  printf ( "    %d points.\n", MAX_COR3 );
  printf ( "\n" );
  printf ( "  Last modification: 18 November 1998.\n" );
  printf ( "\n" );
  printf ( "  Send problem reports to burkardt@psc.edu.\n" );
 
  return;
}
/******************************************************************************/

void converter::help ( void ) {

/******************************************************************************/

/*
  Purpose:

    HELP prints a list of the interactive commands.

  Modified:

    17 November 1998

  Author:
 
    John Burkardt
*/
  printf ( "\n" );
  printf ( "Commands:\n" );
  printf ( "\n" );
  printf ( "< file  Read input file;\n" );
  printf ( "> file  Write output file;\n" );
  printf ( "B       Switch the binary file byte-swapping mode;\n" );
  printf ( "D       Switch the debugging mode;\n" );
  printf ( "F       Print information about one face;\n" );
  printf ( "H       Print this help list;\n" );
  printf ( "I       Info, print out recent changes;\n" );
  printf ( "N       Recompute normal vectors;\n" );
  printf ( "Q       Quit;\n" );
  printf ( "R       Reverse the normal vectors.\n" );
  printf ( "S       Select face subset (NOT WORKING).\n" );
  printf ( "T       Transform the data.\n" );
  printf ( "W       Reverse the face node ordering.\n" );

  return;
}
/******************************************************************************/

int converter::hrc_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:

    HRC_READ reads graphics information from a SoftImage HRC file.

  Modified:

    27 April 1999

  Author:

    John Burkardt
*/
  int   count;
  int   i;
  int   icor3;
  int   ivert;
  int   iword;
  int   jval;
  int   level;
  char *next;
  int   nlbrack;
  int   nrbrack;
  float temp[3];
  int   width;
  char  word[MAX_INPUT];
  char  word1[MAX_INPUT];
  char  word2[MAX_INPUT];
  char  wordm1[MAX_INPUT];
  float x;
  float y;
  float z;

  level = 0;
  strcpy ( levnam[0], "Top" );
  nlbrack = 0;
  nrbrack = 0;
  strcpy ( word, " " );
  strcpy ( wordm1, " " );
/*
  Read a line of text from the file.
*/
  while ( TRUE ) {

    if ( fgets ( input, MAX_INPUT, filein ) == NULL ) {
      break;
    }

    num_text = num_text + 1;
    next = input;
    iword = 0;
/*
  Read a word from the line.
*/
    while ( TRUE ) {

      strcpy ( wordm1, word );

      count = sscanf ( next, "%s%n", word2, &width );
      next = next + width;

      if ( count <= 0 ) {
        break;
      }

      strcpy ( word, word2 );

      iword = iword + 1;

      if ( iword == 1 ) {
        strcpy ( word1, word );
      }
/*
  The first line of the file must be the header.
*/
      if ( num_text == 1 ) {

        if ( strcmp ( word1, "HRCH:" ) != 0 ) {
          printf ( "\n" );
          printf ( "HRC_READ - Fatal error!\n" );
          printf ( "  The input file has a bad header.\n" );
          return ERROR;
        }
        else {
          num_comment = num_comment + 1;
        }
        break;
      }
/*
  If the word is a curly bracket, count it.
*/
      if ( strcmp ( word, "{" ) == 0 ) {
        nlbrack = nlbrack + 1;
        level = nlbrack - nrbrack;
        strcpy ( levnam[level], wordm1 );
        if ( debug ) {
          printf ( "converternew level: %s\n", levnam[level] );
        }
      }
      else if ( strcmp ( word, "}" ) == 0 ) {
        nrbrack = nrbrack + 1;

        if ( nlbrack < nrbrack ) {
          printf ( "\n" );
          printf ( "HRC_READ - Fatal error!\n" );
          printf ( "  Extraneous right bracket on line %d.\n", num_text );
          printf ( "  Currently processing field %s\n.", levnam[level] );
          return ERROR;
        }
      }
/*
  CONTROLPOINTS
*/
      if ( strcmp ( levnam[level], "controlpoints" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {

          if ( num_line < MAX_LINE ) {
            line_dex[num_line] = -1;
            line_mat[num_line] = 0;
          }
          num_line = num_line + 1;
          level = nlbrack - nrbrack;
        }
        else if ( word[0] == '[' ) {
        }
        else if ( strcmp ( word, "position" ) == 0 ) {

          count = sscanf ( next, "%f%n", &x, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &y, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &z, &width );
          next = next + width;

          temp[0] = x;
          temp[1] = y;
          temp[2] = z;

          if ( num_cor3 < 1000 ) {
            icor3 = rcol_find ( cor3, 3, num_cor3, temp );
          }
          else {
            icor3 = -1;
          }

          if ( icor3 == -1 ) {

            icor3 = num_cor3;
            if ( num_cor3 < MAX_COR3 ) {
              cor3[0][num_cor3] = x;
              cor3[1][num_cor3] = y;
              cor3[2][num_cor3] = z;
            }
            num_cor3 = num_cor3 + 1;

          }
          else {
            num_dup = num_dup + 1;
          }

          if ( num_line < MAX_LINE ) {
            line_dex[num_line] = icor3;
            line_mat[num_line] = 0;
          }
          num_line = num_line + 1;
        }
        else {
          num_bad = num_bad + 1;
          printf ( "CONTROLPOINTS: Bad data %s\n", word );
          return ERROR;
        }

      }
/*
  EDGES
*/
      else if ( strcmp ( levnam[level], "edges" ) == 0 ) {

        if ( strcmp( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( word[0] == '[' ) {
        }
        else if ( strcmp ( word, "vertices" ) == 0 ) {

          count = sscanf ( next, "%d%n", &jval, &width );
          next = next + width;

          if ( num_line < MAX_LINE ) {
            line_dex[num_line] = jval;
            line_mat[num_line] = 0;
          }
          num_line = num_line + 1;

          count = sscanf ( next, "%d%n", &jval, &width );
          next = next + width;

          if ( num_line < MAX_LINE ) {
            line_dex[num_line] = jval;
            line_mat[num_line] = 0;
          }
          num_line = num_line + 1;

          if ( num_line < MAX_LINE ) {
            line_dex[num_line] = -1;
            line_mat[num_line] = -1;
          }
          num_line = num_line + 1;

        }
        else {
          num_bad = num_bad + 1;
          printf ( "EDGES: Bad data %s\n", word );
          return ERROR;
        }

      }
/*
  MESH
*/
      else if ( strcmp ( levnam[level], "mesh" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( strcmp ( word, "discontinuity" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "edges" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "flag" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "polygons" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "vertices" ) == 0 ) {
          break;
        }
        else {
          num_bad = num_bad + 1;
          printf ( "MESH: Bad data %s\n", word );
          return ERROR;
        }

      }
/*
  MODEL
*/
      else if ( strcmp ( levnam[level], "model" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( strcmp ( word, "mesh" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "name" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "rotation" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "scaling" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "spline" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "translation" ) == 0 ) {
          break;
        }
        else {
          num_bad = num_bad + 1;
          printf ( "MODEL: Bad data %s\n", word );
          return ERROR;
        }

      }
/*
  NODES
*/
      else if ( strcmp ( levnam[level], "nodes" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
          ivert = 0;
          face_order[num_face] = 0;
          num_face = num_face + 1;
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( word[0] == '[' ) {
        }
        else if ( strcmp ( word, "normal" ) == 0 ) {

          count = sscanf ( next, "%f%n", &x, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &y, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &z, &width );
          next = next + width;

          if ( ivert < MAX_ORDER && num_face < MAX_FACE ) {
            vertex_normal[0][ivert-1][num_face-1] = x;
            vertex_normal[1][ivert-1][num_face-1] = y;
            vertex_normal[2][ivert-1][num_face-1] = z;
          }

        }
        else if ( strcmp ( word, "uvTexture" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "vertex" ) == 0 ) {

          count = sscanf ( next, "%d%n", &jval, &width );
          next = next + width;

          if ( ivert < MAX_ORDER && num_face < MAX_FACE ) {
            face_order[num_face-1] = face_order[num_face-1] + 1;
            face[ivert][num_face-1] = jval;
          }
          ivert = ivert + 1;

        }
        else {
          num_bad = num_bad + 1;
          printf ( "NODES: Bad data %s\n", word );
          return ERROR;
        }

      }
/*
  POLYGONS
*/
      else if ( strcmp ( levnam[level], "polygons" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( word[0] == '[' ) {
        }
        else if ( strcmp ( word, "material" ) == 0 ) {

          count = sscanf ( next, "%d%n", &jval, &width );
          next = next + width;

          for ( ivert = 0; ivert < MAX_ORDER; ivert++ ) {
            face_mat[ivert][num_face-1] = jval;
          }

        }
        else if ( strcmp ( word, "nodes" ) == 0 ) {
          count = sscanf ( next, "%s%n", word2, &width );
          next = next + width;
        }
        else {
          num_bad = num_bad + 1;
          printf ( "POLYGONS: Bad data %s\n", word );
          return ERROR;
        }

      }
/*
  SPLINE
*/
      else if ( strcmp ( levnam[level], "spline" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( strcmp ( word, "controlpoints" ) == 0 ) {
          break;
        }
/*
  WHY DON'T YOU READ IN THE OBJECT NAME HERE?
*/
        else if ( strcmp ( word, "name" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "nbKeys" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "step" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "tension" ) == 0 ) {
          break;
        }
        else if ( strcmp ( word, "type" ) == 0 ) {
          break;
        }
        else {
          num_bad = num_bad + 1;
          printf ( "SPLINE: Bad data %s\n", word );
          return ERROR;
        }

      }
/*
  VERTICES
*/
      else if ( strcmp ( levnam[level], "vertices" ) == 0 ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( word[0] == '[' ) {
        }
        else if ( strcmp ( word, "position" ) == 0 ) {

          count = sscanf ( next, "%f%n", &x, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &y, &width );
          next = next + width;

          count = sscanf ( next, "%f%n", &z, &width );
          next = next + width;

          if ( num_cor3 < MAX_COR3 ) {
            cor3[0][num_cor3] = x;
            cor3[1][num_cor3] = y;
            cor3[2][num_cor3] = z;
          }
          num_cor3 = num_cor3 + 1;
        }
        else {
          num_bad = num_bad + 1;
          printf ( "VERTICES: Bad data %s\n", word );
          return ERROR;
        }
      }
/*
  Any other word:
*/
      else {

      }
    }
  }

/*
  End of information in file.

  Check the "materials" defining a line.

  If COORDINDEX is -1, so should be the MATERIALINDEX.
  If COORDINDEX is not -1, then the MATERIALINDEX shouldn"t be either.
*/
  for ( i = 0; i < num_line; i++ ) {

    if ( line_dex[i] == -1 ) {
      line_mat[i] = -1;
    }
    else if ( line_mat[i] == -1 ) {
      line_mat[i] = 0;
    }

  }
  return SUCCESS;
}
/******************************************************************************/

int converter::hrc_write ( FILE* fileout ) {

/******************************************************************************/

/*
  Purpose:

    HRC_WRITE writes graphics data to an HRC SoftImage file.

  Deficiencies:

    SoftImage expects each face to be made of a single material,
    so I'm just using the material associated with the first vertex
    of a given face, to apply to the whole face.

  Examples:

    HRCH: Softimage 4D Creative Environment v3.00


    model
    {
      name         "cube_10x10"
      scaling      1.000 1.000 1.000
      rotation     0.000 0.000 0.000
      translation  0.000 0.000 0.000

      mesh
      {
        flag    ( PROCESS )
        discontinuity  60.000

        vertices   8
        {
          [0] position  -5.000  -5.000  -5.000
          [1] position  -5.000  -5.000  5.000
          [2] position  -5.000  5.000  -5.000
          [3] position  -5.000  5.000  5.000
          [4] position  5.000  -5.000  -5.000
          [5] position  5.000  -5.000  5.000
          [6] position  5.000  5.000  -5.000
          [7] position  5.000  5.000  5.000
        }
 
        polygons   6
        {
          [0] nodes  4
          {
            [0] vertex  0
                normal  -1.000  0.000  0.000
                uvTexture  0.000  0.000
            [1] vertex  1
                normal  -1.000  0.000  0.000
                uvTexture  0.000  0.000
            [2] vertex  3
                normal  -1.000  0.000  0.000
                uvTexture  0.000  0.000
            [3] vertex  2
                normal  -1.000  0.000  0.000
                uvTexture  0.000  0.000
          }
          material  0
          [1] nodes  4
          {
            [0] vertex  1
                normal  0.000  0.000  1.000
                uvTexture  0.000  0.000
            [1] vertex  5

    ...etc.....

          [5] nodes  4
          {
            [0] vertex  2
                normal  0.000  1.000  0.000
                uvTexture  0.000  0.000
            [1] vertex  3
                normal  0.000  1.000  0.000
                uvTexture  0.000  0.000
            [2] vertex  7
                normal  0.000  1.000  0.000
                uvTexture  0.000  0.000
            [3] vertex  6
                normal  0.000  1.000  0.000
                uvTexture  0.000  0.000
          }
          material  0
        }

        edges   12
        {
          [1] vertices  3  2
          [2] vertices  2  0
          [3] vertices  0  1
          [4] vertices  1  3
          [5] vertices  7  3
          [6] vertices  1  5
          [7] vertices  5  7
          [8] vertices  6  7
          [9] vertices  5  4
          [10] vertices  4  6
          [11] vertices  2  6
          [12] vertices  4  0
        }
      }
    }

  Modified:

    30 September 1998

  Author:

    John Burkardt

*/
  int iface;
  int ivert;
  int j;
  int jhi;
  int jlo;
  int jrel;
  int k;
  int npts;
  int nseg;
  int num_text;

  nseg = 0;
  num_text = 0;

  fprintf ( fileout, "HRCH: Softimage 4D Creative Environment v3.00\n" );
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "\n" );
  num_text = num_text + 3;

  fprintf ( fileout, "model\n" );
  fprintf ( fileout, "{\n" );
  fprintf ( fileout, "  name         \"%s\"\n", object_name );
  fprintf ( fileout, "  scaling      1.000 1.000 1.000\n" );
  fprintf ( fileout, "  rotation     0.000 0.000 0.000\n" );
  fprintf ( fileout, "  translation  0.000 0.000 0.000\n" );
  num_text = num_text + 6;

  if ( num_face > 0 ) {

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  mesh\n" );
    fprintf ( fileout, "  {\n" );
    fprintf ( fileout, "    flag    ( PROCESS )\n" );
    fprintf ( fileout, "    discontinuity  60.000\n" );
    num_text = num_text + 5;
/*
  Point coordinates.
*/
    if ( num_cor3 > 0 ) {

      fprintf ( fileout, "\n" );
      fprintf ( fileout, "    vertices %d\n", num_cor3 );
      fprintf ( fileout, "    {\n" );
      num_text = num_text + 3;

      for ( j = 0; j < num_cor3; j++ ) {

        fprintf ( fileout, "      [%d] position %f %f %f\n", j, cor3[0][j], 
          cor3[1][j], cor3[2][j] );
        num_text = num_text + 1;
      }
      fprintf ( fileout, "    }\n" );
      num_text = num_text + 1;
    }
/*
  Faces.
*/
    fprintf ( fileout, "\n" );
    fprintf ( fileout, "    polygons %d\n", num_face );
    fprintf ( fileout, "    {\n" );
    num_text = num_text + 3;

    for ( iface = 0; iface < num_face; iface++ ) {

      fprintf ( fileout, "      [%d] nodes %d\n", iface, face_order[iface] );
      fprintf ( fileout, "      {\n" );
      num_text = num_text + 2;

      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {

        fprintf ( fileout, "        [%d] vertex %d\n", ivert, face[ivert][iface] );
        fprintf ( fileout, "            normal %f %f %f\n", 
          vertex_normal[0][ivert][iface], 
          vertex_normal[1][ivert][iface], vertex_normal[2][ivert][iface] );
        fprintf ( fileout, "            uvTexture  0.000  0.000\n" );
        num_text = num_text + 3;
      }
      fprintf ( fileout, "      }\n" );
      fprintf ( fileout, "      material %d\n", face_mat[0][iface] );
      num_text = num_text + 2;
    }
    fprintf ( fileout, "    }\n" );
    fprintf ( fileout, "  }\n" );
    num_text = num_text + 2;
  }
/*
  IndexedLineSet.
*/


  if ( num_line > 0 ) {

    nseg = 0;

    jhi = -1;

    while ( TRUE ) {

      jlo = jhi + 1;
/*
  Look for the next index JLO that is not -1.
*/
      while ( jlo < num_line ) {
        if ( line_dex[jlo] != -1 ) {
          break;
        }
        jlo = jlo + 1;
      }

      if ( jlo >= num_line ) {
        break;
      }
/*
  Look for the highest following index JHI that is not -1.
*/
      jhi = jlo + 1;

      while ( jhi < num_line ) {
        if ( line_dex[jhi] == -1 ) {
          break;
        }
        jhi = jhi + 1;
      }

      jhi = jhi - 1;
/*
  Our next line segment involves LINE_DEX indices JLO through JHI.
*/     
      nseg = nseg + 1;
      npts = jhi + 1 - jlo;

      fprintf ( fileout, "\n" );
      fprintf ( fileout, "  spline\n" );
      fprintf ( fileout, "  {\n" );
      fprintf ( fileout, "    name     \"spl%d\"\n", nseg );
      fprintf ( fileout, "    type     LINEAR\n" );
      fprintf ( fileout, "    nbKeys   %d\n", npts );
      fprintf ( fileout, "    tension  0.000\n" );
      fprintf ( fileout, "    step     1\n" );
      fprintf ( fileout, "\n" );
      num_text = num_text + 9;

      fprintf ( fileout, "    controlpoints\n" );
      fprintf ( fileout, "    {\n" );
      num_text = num_text + 2;

      for ( j = jlo; j <= jhi; j++ ) {
        jrel = j - jlo;
        k = line_dex[j];
        fprintf ( fileout, "      [%d] position %f %f %f\n", jrel,
          cor3[0][k], cor3[1][k], cor3[2][k] );
        num_text = num_text + 1;
      }

      fprintf ( fileout, "    }\n" );
      fprintf ( fileout, "  }\n" );
      num_text = num_text + 2;
    }
  }

  fprintf ( fileout, "}\n" );
  num_text = num_text + 1;
/*
  Report.
*/
  printf ( "\n" );
  printf ( "HRC_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
/******************************************************************************/

void converter::init_program_data ( ) {

/******************************************************************************/

/*
  Purpose:

    INIT_PROGRAM_DATA initializes the internal program data.

  Modified:

    18 November 1998

  Author:

    John Burkardt
*/
  byte_swap = FALSE;
  debug = FALSE;
  num_color = 0;
  num_cor3 = 0;
  num_face = 0;
  num_line = 0;

  if ( debug == TRUE ) {
    printf ( "\n" );
    printf ( "INIT_PROGRAM_DATA: Program data initialized.\n" );
  }

  return;

}
/******************************************************************************/

int converter::interact ( ) {

/******************************************************************************/

/*
  Purpose:
   
    INTERACT carries on an interactive session with the user.

  Modified:

    18 November 1998

  Author:
 
    John Burkardt
*/
  int    i;
  int    icor3;
  int    ierror;
  int    iface;
  int    itemp;
  int    ivert;
  int    j;
  int    jvert;
  int    m;
  char  *next;
  float  temp;
  float  x;
  float  y;
  float  z;

  strcpy ( filein_name, "NO_IN_NAME" );
  strcpy ( fileout_name, "NO_OUT_NAME" );

/*  
  Say hello. 
*/
  hello ( );
/*  
  Get the next user command. 
*/
  printf ( "\n" );
  printf ( "Enter command (H for help)\n" );

  while ( fgets ( input, MAX_INPUT, stdin ) != NULL ) {
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
/*  
  Command: < FILENAME 
*/
    if ( *next == '<' ) {

      next = next + 1;
      sscanf ( next, "%s", filein_name );

      data_init ( );

      ierror = data_read ( );

      if ( ierror == ERROR ) {
        printf ( "\n" );
        printf ( "INTERACT - Error!\n" );
        printf ( "  DATA_READ failed to read input data.\n" );
      }
    }
/*  
  Command: > FILENAME 
*/
    else if ( *next == '>' ) {

      next = next + 1;
      sscanf ( next, "%s", fileout_name ); 

      ierror = data_write ( );
 
      if ( ierror == ERROR ) {
        printf ( "\n" );
        printf ( "INTERACT - Error!\n" );
        printf ( "  OUTPUT_DATA failed to write output data.\n" );
      }

    }
/*
  B: Switch byte swapping option.
*/
    else if ( *next == 'B' || *next == 'b' ) {

      if ( byte_swap == TRUE ) {
        byte_swap = FALSE;
        printf ( "Byte_swapping reset to FALSE.\n" );
      }
      else {
        byte_swap = TRUE;
        printf ( "Byte_swapping reset to TRUE.\n" );
      }

    }
/*
  D: Switch debug option.
*/
    else if ( *next == 'D' || *next == 'd' ) {
      if ( debug == TRUE ) {
        debug = FALSE;
        printf ( "Debug reset to FALSE.\n" );
      }
      else {
        debug = TRUE;
        printf ( "Debug reset to TRUE.\n" );
      }
    }
/*  
  F: Check a face. 
*/
    else if ( *next == 'f' || *next == 'F' ) {
      printf ( "\n" );
      printf ( "  Enter a face index between 0 and %d:", num_face-1 );
      scanf ( "%d", &iface );
      face_print ( iface );
    }
/*  
  H: Help
*/
    else if ( *next == 'h' || *next == 'H' ) {
      help ( );
    }
/*
  I: Print change information. 
*/
    else if ( *next == 'i' || *next == 'I') {
      news ( );
    }
/*
  N: Recompute normal vectors.
*/
    else if ( *next == 'n' || *next == 'N') {

      for ( iface = 0; iface < num_face; iface++ ) {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = 0.0;
        }
      }

      for ( iface = 0; iface < num_face; iface++ ) {
         for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
          for ( i = 0; i < 3; i++ ) {
            vertex_normal[i][ivert][iface] = 0.0;
          }
        }
      }

      set_vertex_normal ( );

      set_cor3_normal ( );

      ave_face_normal ( );
    }
/*
  Q: Quit
*/
    else if ( *next == 'q' || *next == 'Q' ) {
      printf ( "\n" );
      printf ( "INTERACT - Normal end of execution.\n" );
      return SUCCESS;
    }
/*
  R: Reverse normal vectors. 
*/
    else if ( *next == 'r' || *next == 'R' ) {

      for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {
        for ( i = 0; i < 3; i++ ) {
          cor3_normal[i][icor3] = - cor3_normal[i][icor3];
        }
      }

      for ( iface = 0; iface < num_face; iface++ ) {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = - face_normal[i][iface];
        }
      }

      for ( iface = 0; iface < num_face; iface++ ) {
        for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
          for ( i = 0; i < 3; i++ ) {
            vertex_normal[i][ivert][iface] = 
              - vertex_normal[i][ivert][iface];
          }
        }
      }
      printf ( "\n" );
      printf ( "INTERACT - Note:\n" );
      printf ( "  Reversed node, face and vertex normals.\n" );
    }
/*  
  S: Select a few faces, discard the rest.
*/
    else if ( *next == 's' || *next == 'S' ) {
      face_subset ( );
    }
/*  
  T: Transform the data.
*/
    else if ( *next == 't' || *next == 'T' ) {

      printf ( "\n" );
      printf ( "For now, we only offer point scaling.\n" );
      printf ( "Enter X, Y, Z scale factors:\n" );

      scanf ( "%f %f %f", &x, &y, &z );

      for ( j = 0; j < num_cor3; j++ ) {
        cor3[0][j] = x * cor3[0][j];
        cor3[1][j] = y * cor3[1][j];
        cor3[2][j] = z * cor3[2][j];
      }

      for ( iface = 0; iface < num_face; iface++ ) {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = 0.0;
        }
      }

      for ( iface = 0; iface < num_face; iface++ ) {
         for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
          for ( i = 0; i < 3; i++ ) {
            vertex_normal[i][ivert][iface] = 0.0;
          }
        }
      }

      set_vertex_normal ( );

      set_cor3_normal ( );

      ave_face_normal ( );
    }
/*
  U: Renumber faces, count objects:
*/
    else if ( *next == 'u' || *next == 'U' ) {
    }
/*
  V: Convert polygons to triangles:
*/
    else if ( *next == 'v' || *next == 'V' ) {
    }
/*
  W: Reverse the face node ordering. 
*/
    else if ( *next == 'w' || *next == 'W' ) {

      if ( num_face > 0 ) {

        for ( iface = 0; iface < num_face; iface++ ) {

          m = face_order[iface];

          for ( ivert = 0; ivert < m/2; ivert++ ) {

            jvert = m - ivert - 1;

            itemp = face[ivert][iface];
            face[ivert][iface] = face[jvert][iface];
            face[jvert][iface] = itemp;

            itemp = face_mat[ivert][iface];
            face_mat[ivert][iface] = face_mat[jvert][iface];
            face_mat[jvert][iface] = itemp;

            for ( i = 0; i < 3; i++ ) {
              temp = vertex_normal[i][ivert][iface];
              vertex_normal[i][ivert][iface] = 
                vertex_normal[i][jvert][iface];
              vertex_normal[i][jvert][iface] = temp;
            }
          }
        }
        printf ( "\n" );
        printf ( "INTERACT - Note:\n" );
        printf ( "  Reversed face node ordering.\n" );
      }
    }
/*
  Command: ???  
*/
    else {
      printf ( "\n" );
      printf ( "INTERACT: Warning!\n" );
      printf ( "  Your command was not recognized.\n" );
    }

    printf ( "\n" );
    printf ( "Enter command (H for help)\n" );

  }
  return SUCCESS;
}
/******************************************************************************/

int converter::iv_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:

    IV_READ reads graphics information from an Inventor file.

  Modified:

    20 October 1998

  Author:

    John Burkardt
*/
  int   count;
  int   i;
  int   icolor;
  int   icface;
  int   ihi;
  int   inormface;
  int   inum_face;
  int   ivert;
  int   iword;
  int   ix;
  int   ixyz;
  int   iy;
  int   iz;
  int   jval;
  int   level;
  char  material_binding[80];
  char  normal_binding[80];
  int   nbase;
  char *next;
  int   nlbrack;
  int   nrbrack;
  int   nu;
  int   null_index;
  int   num_line2;
  int   num_face2;
  int   num_normal_temp;
  int   nv;
  int   result;
  float rval;
  int   width;
  char  word[MAX_INPUT];
  char  word1[MAX_INPUT];
  char  wordm1[MAX_INPUT];

  icface = 0;
  inormface = 0;
  inum_face = 0;
  ix = 0;
  ixyz = 0;
  iy = 0;
  iz = 0;
  jval = 0;
  level = 0;
  strcpy ( levnam[0], "Top" );
  nbase = 0;
  nlbrack = 0;
  nrbrack = 0;
  num_face2 = 0;
  num_line2 = 0;
  num_normal_temp = 0;
  rval = 0.0;
  strcpy ( word, " " );
  strcpy ( wordm1, " " );
/*
  Read the next line of text from the input file.
*/
  while ( TRUE ) {

    if ( fgets ( input, MAX_INPUT, filein ) == NULL ) {
      break;
    }

    num_text = num_text + 1;
    next = input;
    iword = 0;
/*
  Remove all commas from the line, so we can use SSCANF to read
  numeric items.
*/
    i = 0;
    while ( input[i] != '\0' ) {
      if ( input[i] == ',' ) {
        input[i] = ' ';
      }
      i++;
    }
/*
  Force brackets and braces to be buffered by spaces.
*/
    i = 0;
    while ( input[i] != '\0' ) {
      i++;
    }
    null_index = i;

    i = 0;
    while ( input[i] != '\0' && i < MAX_INPUT ) {

      if ( input[i] == '[' || input[i] == ']' || 
           input[i] == '{' || input[i] == '}' ) {

        result = char_pad ( &i, &null_index, input, MAX_INPUT );
        if ( result == ERROR ) {
          break;
        }
      } 
      else {
        i++;
      }
    }
/*
  Read a word from the line.
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
  The first line of the file must be the header.
*/
      if ( num_text == 1 ) {

        if ( leqi ( word1, "#Inventor" ) != TRUE ) { 
          printf ( "\n" );
          printf ( "IV_READ - Fatal error!\n" );
          printf ( "  The input file has a bad header.\n" );
          return ERROR;
        }
        else {
          num_comment = num_comment + 1;
        }
        break;
      }
/*
  A comment begins anywhere with '#'.
  Skip the rest of the line.
*/
      if ( word[1] == '#' ) {
        num_comment = num_comment + 1;
        break;
      }
/*
  If the word is a curly or square bracket, count it.
  If the word is a left bracket, the previous word is the name of a node.
*/
      if ( strcmp ( word, "{" ) == 0 || strcmp ( word, "[" ) == 0 ) {
        nlbrack = nlbrack + 1;
        level = nlbrack - nrbrack;
        strcpy ( levnam[level], wordm1 );
        if ( debug == TRUE ) {
          printf ( "Level: %s\n", wordm1 );
        }
      }
      else if ( strcmp ( word, "}" ) == 0 || strcmp ( word, "]" ) == 0 ) {
        nrbrack = nrbrack + 1;

        if ( nlbrack < nrbrack ) {
          printf ( "\n" );
          printf ( "IV_READ - Fatal error!\n" );
          printf ( "  Extraneous right bracket on line %d.\n", num_text );
          printf ( "  Currently processing field %s\n.", levnam[level] );
          return ERROR;
        }
      }
/*
  BASECOLOR
*/
      if ( leqi ( levnam[level], "BASECOLOR" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "RGB" ) == TRUE ) {
        }
        else {
          num_bad = num_bad + 1;
          printf ( "Bad data %s\n", word );
        }
      }
/*
  COORDINATE3
*/
      else if ( leqi ( levnam[level], "COORDINATE3" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "POINT" ) == TRUE ) {
        }
        else {
          num_bad = num_bad + 1;
          printf ( "COORDINATE3: Bad data %s\n", word );
        }
      }
/*
  COORDINATE4
*/
      else if ( leqi ( levnam[level], "COORDINATE4" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "POINT" ) == TRUE ) {
        }
        else {
          num_bad = num_bad + 1;
          printf ( "COORDINATE4: Bad data %s\n", word );
        }
      }
/*
  COORDINDEX
*/
      else if ( leqi ( levnam[level], "COORDINDEX" ) == TRUE ) {

        if ( strcmp ( word, "[" ) == 0 ) {
          ivert = 0;
        }
        else if ( strcmp ( word, "]" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
/*
  (indexedlineset) COORDINDEX
*/
        else if ( leqi ( levnam[level-1], "INDEXEDLINESET" ) == TRUE ) {

          count = sscanf ( word, "%d%n", &jval, &width );

          if ( count > 0 ) {

            if ( jval < -1 ) {
              num_bad = num_bad + 1;
            }
            else {
              if ( num_line < MAX_LINE ) {
                if ( jval != -1 ) {
                  jval = jval + nbase;
                }
                line_dex[num_line] = jval;
              }
              num_line = num_line + 1;
            }
          }
          else {
            num_bad = num_bad + 1;
          }
        }
/*
  (indexedfaceset) COORDINDEX
  Warning: If the list of indices is not terminated with a final -1, then
  the last face won't get counted.
*/
        else if ( leqi ( levnam[level-1], "INDEXEDFACESET" ) == TRUE ) {

          count = sscanf ( word, "%d%n", &jval, &width );

          if ( count > 0 ) {
            if ( jval == -1 ) {
              ivert = 0;
              num_face = num_face + 1;
            }
            else {
              if ( ivert == 0 ) {
                if ( num_face < MAX_FACE ) {
                  face_order[num_face] = 0;
                }
              }
              if ( num_face < MAX_FACE ) {
                face_order[num_face] = face_order[num_face] + 1;
                face[ivert][num_face] = jval + nbase;
                ivert = ivert + 1;
              }
            }
          }
        }
/*
  (indexednurbssurface) COORDINDEX
*/
        else if ( leqi ( levnam[level-1], "INDEXEDNURBSSURFACE" ) == TRUE ) {
        }
/*
  (indexedtrianglestripset) COORDINDEX

  First three coordinate indices I1, I2, I3 define a triangle.
  Next triangle is defined by I2, I3, I4 (actually, I4, I3, I2
  to stay with same counterclockwise sense).
  Next triangle is defined by I3, I4, I5 ( do not need to reverse
  odd numbered triangles) and so on.
  List is terminated with -1.
*/
        else if ( leqi ( levnam[level-1], "INDEXEDTRIANGLESTRIPSET" ) == TRUE ) {

          count = sscanf ( word, "%d%n", &jval, &width );

          if ( count > 0 ) {

            if ( jval == -1 ) {
              ivert = 0;
            }
            else {

              ix = iy;
              iy = iz;
              iz = jval + nbase;

              if ( ivert == 0 ) {
                if ( num_face < MAX_FACE ) {
                  face[ivert][num_face] = jval + nbase;
                  face_order[num_face] = 3;
                }
              }
              else if ( ivert == 1 ) {
                if ( num_face < MAX_FACE ) {
                  face[ivert][num_face] = jval + nbase;
                }
              }
              else if ( ivert == 2 ) {
                if ( num_face < MAX_FACE ) {
                  face[ivert][num_face] = jval + nbase;
                }
                num_face = num_face + 1;
              }
              else {

                if ( num_face < MAX_FACE ) {
                  face_order[num_face] = 3;
                  if ( ( ivert % 2 ) == 0 ) {
                    face[0][num_face] = ix;
                    face[1][num_face] = iy;
                    face[2][num_face] = iz;
                  }
                  else {
                    face[0][num_face] = iz;
                    face[1][num_face] = iy;
                    face[2][num_face] = ix;
                  }
                }
                num_face = num_face + 1;
              }
              ivert = ivert + 1;
/*
  Very very tentative guess as to how indices into the normal
  vector array are set up...
*/
              if ( num_face < MAX_FACE && ivert > 2 ) {
                for ( i = 0; i < 3; i++ ) {
                  face_normal[i][num_face] = normal_temp[i][ix];
                }
              }
            }
          }
        }
      }
/*
  INDEXEDFACESET
*/
      else if ( leqi ( levnam[level], "INDEXEDFACESET" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "COORDINDEX" ) == TRUE ) {
          ivert = 0;
        }
        else if ( leqi ( word, "MATERIALINDEX" ) == TRUE ) {
        }
        else if ( leqi ( word, "NORMALINDEX" ) == TRUE ) {
        }
        else if ( leqi ( word, "TEXTURECOORDINDEX" ) == TRUE ) {
        }
        else {
          num_bad = num_bad + 1;
          printf ( "Bad data %s\n", word );
        }
      }
/*
  INDEXEDLINESET
*/
      else if ( leqi ( levnam[level], "INDEXEDLINESET" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "COORDINDEX" ) == TRUE ) {
        }
        else if ( leqi ( word, "MATERIALINDEX" ) == TRUE ) {
        }
        else {
          num_bad = num_bad + 1;
          printf ( "Bad data %s\n", word );
        }
      }
/*
  INDEXEDNURBSSURFACE
*/
      else if ( leqi ( levnam[level], "INDEXEDNURBSSURFACE" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "NUMUCONTROLPOINTS") == TRUE ) {

          count = sscanf ( word, "%d%n", &jval, &width );

          if ( count > 0 ) {
            nu = jval;
          }
          else {
            nu = 0;
            num_bad = num_bad + 1;
            printf ( "Bad data %s\n", word );
          }
        }
        else if ( leqi ( word, "NUMVCONTROLPOINTS" ) == TRUE ) {

          count = sscanf ( word, "%d%n", &jval, &width );

          if ( count > 0 ) {
            nv = jval;
          }
          else {
            nv = 0;
            num_bad = num_bad + 1;
          }
        }
        else if ( leqi ( word, "COORDINDEX" ) == TRUE ) {
        }
        else if ( leqi ( word, "UKNOTVECTOR" ) == TRUE ) {
        }
        else if ( leqi ( word, "VKNOTVECTOR" ) == TRUE ) {
        }
        else {
          num_bad = num_bad + 1;
          printf ( "Bad data %s\n", word );
        }
      }
/*
  INDEXEDTRIANGLESTRIPSET
*/
      else if ( leqi ( levnam[level], "INDEXEDTRIANGLESTRIPSET" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "VERTEXPROPERTY" ) == TRUE ) {
          count = sscanf ( next, "%s%n", word, &width );
          next = next + width;
        }
        else if ( leqi ( word, "COORDINDEX" ) == TRUE ) {
          ivert = 0;
        }
        else if ( leqi ( word, "NORMALINDEX" ) == TRUE ) {
          count = sscanf ( next, "%s%n", word, &width );
          next = next + width;
        }
        else {
          num_bad = num_bad + 1;
          printf ( "Bad data %s\n", word );
        }
      }
/*
  INFO
*/
      else if ( leqi ( levnam[level], "INFO" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "STRING" ) == TRUE ) {
        }
        else if ( word == "\"" ) {
        }
        else {
        }
      }
/*
  LIGHTMODEL
  Read, but ignore.
*/
      else if ( leqi ( levnam[level], "LIGHTMODEL" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "model" ) == TRUE ) {
        }
        else {
        }
      }
/*
  MATERIAL
  Read, but ignore.
*/
      else if ( leqi ( levnam[level],"MATERIAL" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "AMBIENTCOLOR" ) == TRUE ) {
        }
        else if ( leqi ( word, "EMISSIVECOLOR" ) == TRUE ) {
        }
        else if ( leqi ( word, "DIFFUSECOLOR" ) == TRUE ) {
        }
        else if ( leqi ( word, "SHININESS" ) == TRUE ) {
        }
        else if ( leqi ( word, "SPECULARCOLOR" ) == TRUE ) {
        }
        else if ( leqi ( word, "TRANSPARENCY" ) == TRUE ) {
        }
        else {
        }
      }
/*
  MATERIALBINDING
  Read, but ignore
*/
      else if ( leqi ( levnam[level], "MATERIALBINDING" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "VALUE" ) == TRUE ) {
          count = sscanf ( next, "%s%n", material_binding, &width );
          next = next + width;
        }
        else {
          count = sscanf ( next, "%f%n", &rval, &width );
          next = next + width;

          if ( count > 0 ) {
          }
          else {
            num_bad = num_bad + 1;
            printf ( "Bad data %s\n", word );
          }
        }
      }
/*
  MATERIALINDEX
*/
      else if ( leqi ( levnam[level], "MATERIALINDEX" ) == TRUE ) {

        if ( strcmp ( word, "[" ) == 0 ) {
          ivert = 0;
        }
        else if ( strcmp ( word, "]" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
/*
  (indexedfaceset) MATERIALINDEX
*/
        else if ( leqi ( levnam[level-1], "INDEXEDFACESET" ) == TRUE ) {

          count = sscanf ( word, "%d%n", &jval, &width );

          if ( count > 0 ) {

            if ( jval == -1 ) {
              ivert = 0;
              num_face2 = num_face2 + 1;
            }
            else {

              if ( num_face2 < MAX_FACE ) {
                if ( jval != -1 ) {
                  jval = jval + nbase;
                }
                face_mat[ivert][num_face2] = jval;
                ivert = ivert + 1;
              }
            }
          }
          else {
            num_bad = num_bad + 1;
            printf ( "Bad data %s\n", word );
          }
        } 
/*
  (indexedlineset) MATERIALINDEX
*/
        else if ( leqi ( levnam[level-1], "INDEXEDLINESET" ) == TRUE ) {

          count = sscanf ( word, "%d%n", &jval, &width );

          if ( count > 0 ) {

            if ( num_line2 < MAX_LINE ) {
              if ( jval != -1 ) {
                jval = jval + nbase;
              }
              line_mat[num_line2] = jval;
              num_line2 = num_line2 + 1;
            }
          }
          else {
            num_bad = num_bad + 1;
            printf ( "Bad data %s\n", word );
          }
        }
        else {
          count = sscanf ( word, "%d%n", &jval, &width );

          if ( count > 0 ) {
          }
          else {
            num_bad = num_bad + 1;
            printf ( "Bad data %s\n", word );
          }
        }
      }
/*
  NORMAL
  The field "VECTOR" may be followed by three numbers,
  (handled here),  or by a square bracket, and sets of three numbers.
*/
      else if ( leqi ( levnam[level], "NORMAL" ) == TRUE ) {
/*
  (vertexproperty) NORMAL
*/
        if ( leqi ( levnam[level-1], "VERTEXPROPERTY" ) == TRUE ) {

          if ( strcmp ( word, "[" ) == 0 ) {
            ixyz = 0;
          }
          else if ( strcmp ( word, "]" ) == 0 ) {
            level = nlbrack - nrbrack;
          }
          else {
  
            count = sscanf ( word, "%f%n", &rval, &width );

            if ( count > 0 ) {

              if ( inormface < MAX_FACE ) {
                face_normal[ixyz][inormface] = rval;
              }

              ixyz = ixyz + 1;
              if ( ixyz > 2 ) {
                ixyz = 0;
                inormface = inormface + 1;
              }
            }
          }
        }
/*
  (anythingelse) NORMAL
*/
        else {

          if ( strcmp ( word, "{" ) == 0 ) {
            ixyz = 0;
          }
          else if ( strcmp ( word, "}" ) == 0 ) {
            level = nlbrack - nrbrack;
          }
          else if ( leqi ( word, "VECTOR" ) == TRUE ) {
          }
          else {

            count = sscanf ( word, "%f%n", &rval, &width );

            if ( count > 0 ) {

/*  COMMENTED OUT

              if ( nfnorm < MAX_FACE ) {
                normal[ixyz][nfnorm] = rval;
              }

*/
              ixyz = ixyz + 1;
              if ( ixyz > 2 ) {
                ixyz = 0;
              }
            }
            else {
              num_bad = num_bad + 1;
              printf ( "Bad data %s\n", word );
            }
          }
        }
      }
/*
  NORMALBINDING
  Read, but ignore
*/
      else if ( leqi ( levnam[level], "NORMALBINDING" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "VALUE" ) == TRUE ) {
          count = sscanf ( next, "%s%n", normal_binding, &width );
          next = next + width;
        }
        else {
          count = sscanf ( word, "%f%n", &rval, &width );

          if ( count > 0 ) {
          }
          else {
            num_bad = num_bad + 1;
            printf ( "Bad data %s\n", word );
          }
        }
      }
/*
  NORMALINDEX
*/
      else if ( leqi ( levnam[level], "NORMALINDEX" ) == TRUE ) {
/*
  (indexedtrianglestripset) NORMALINDEX
*/
        if ( leqi ( levnam[level-1], "INDEXEDTRIANGLESTRIPSET" ) == TRUE ) {
          count = sscanf ( word, "%d%n", &jval, &width );

          if ( count > 0 ) {
          }
          else if ( strcmp ( word, "[" ) == 0 ) {
          }
          else if ( strcmp ( word, "]" ) == 0 ) {
          }
        }
/*
  (anythingelse) NORMALINDEX
*/
        else {

          if ( strcmp ( word, "[" ) == 0 ) {
            ivert = 0;
          }
          else if ( strcmp ( word, "]" ) == 0 ) {
            level = nlbrack - nrbrack;
          }
          else {

            count = sscanf ( word, "%d%n", &jval, &width );

            if ( count > 0 ) {
              if ( jval == -1 ) {
                ivert = 0;
                inum_face = inum_face + 1;
              }
              else {
                if ( inum_face < MAX_FACE ) {
                  for ( i = 0; i < 3; i++ ){
                    vertex_normal[i][ivert][inum_face] = normal_temp[i][jval];
                  }
                  ivert = ivert + 1;
                }
              }
            }
            else {
              num_bad = num_bad + 1;
              printf ( "Bad data %s\n", word );
            }
          }
        }
      }
/*
  POINT
*/
      else if ( leqi ( levnam[level], "POINT" ) == TRUE ) {

        if ( strcmp ( word, "[" ) == 0 ) {
          ixyz = 0;
        }
        else if ( strcmp ( word, "]" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else {

          count = sscanf ( word, "%f%n", &rval, &width );

          if ( count > 0 ) {
            if ( num_cor3 < MAX_COR3 ) {
              cor3[ixyz][num_cor3] = rval;
            }
            ixyz = ixyz + 1;
            if ( ixyz == 3 ) {
              ixyz = 0;
              num_cor3 = num_cor3 + 1;
              continue;
            }
          }
          else {
            num_bad = num_bad + 1;
            break;
          }
        }
      }
/*
  RGB
*/
      else if ( leqi ( levnam[level],"RGB" ) == TRUE ) {
/*
  (basecolor) RGB
*/
        if ( leqi ( levnam[level-1], "BASECOLOR" ) == TRUE ) {

          if ( strcmp ( word, "[" ) == 0 ) {
            icolor = 0;
          }
          else if ( strcmp ( word, "]" ) == 0 ) {
            level = nlbrack - nrbrack;
          }
          else {

            count = sscanf ( word, "%f%n", &rval, &width );
  
            if ( count > 0 ) {

              rgbcolor[icolor][num_color] = rval;
              icolor = icolor + 1;

              if ( icolor == 3 ) {
                icolor = 0;
                num_color = num_color + 1;
              }
            }
            else {
              num_bad = num_bad + 1;
              printf ( "Bad data %s\n", word );
            }
          }
        }
/*
  (anythingelse RGB)
*/
        else {
          printf ( "HALSBAND DES TODES!\n" );
          if ( strcmp ( word, "[" ) == 0 ) {
            icolor = 0;
            ivert = 0;
          }
          else if ( strcmp ( word, "]" ) == 0 ) {
            level = nlbrack - nrbrack;
          }
          else {

            count = sscanf ( word, "%f%n", &rval, &width );
  
            if ( count > 0 ) {
 
              if ( icface < MAX_FACE ) {

                vertex_rgb[icolor][ivert][icface] = rval;

                icolor = icolor + 1;
                if ( icolor == 3 ) {
                  icolor = 0;
                  num_color = num_color + 1;
                  ivert = ivert + 1;
                  if ( ivert == face_order[icface] ) {
                    ivert = 0;
                    icface = icface + 1;
                  }
                }
              }
            }
            else {
              num_bad = num_bad + 1;
              printf ( "Bad data %s\n", word );
            }
          }
        }

      }
/*
  SEPARATOR
*/
      else if ( leqi ( levnam[level], "SEPARATOR" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else {
        }
      }
/*
  SHAPEHINTS
  Read, but ignore.
*/
      else if ( leqi ( levnam[level], "SHAPEHINTS" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "CREASEANGLE" ) == TRUE ) {

          count = sscanf ( next, "%f%n", &rval, &width );
          next = next + width;

          if ( count <= 0 ) {
            num_bad = num_bad + 1;
            printf ( "Bad data %s\n", word );
          }
        }
        else if ( leqi ( word, "FACETYPE" ) == TRUE ) {
          count = sscanf ( next, "%s%n", word, &width );
          next = next + width;
        }
        else if ( leqi ( word, "SHAPETYPE" ) == TRUE ) {
          count = sscanf ( next, "%s%n", word, &width );
          next = next + width;
        }
        else if ( leqi ( word, "VERTEXORDERING" ) == TRUE ) {
          count = sscanf ( next, "%s%n", word, &width );
          next = next + width;
        }
        else {
          num_bad = num_bad + 1;
          printf ( "Bad data %s\n", word );
        }
      }
/*
  TEXTURECOORDINDEX
  Read but ignore.
*/
      else if ( leqi ( levnam[level], "TEXTURECOORDINDEX" ) == TRUE ) {

        if ( strcmp ( word, "[" ) == 0 ) {
        }
        else if ( strcmp ( word, "]" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else {
          count = sscanf ( word, "%d%n", &jval, &width );
          if ( count > 0 ) {
          }
          else {
            num_bad = num_bad + 1;
            printf ( "Bad data %s\n", word );
          }
        }
      }
/*
  UKNOTVECTOR
*/
      else if ( leqi ( levnam[level], "UKNOTVECTOR" ) == TRUE ) {

        if ( strcmp ( word, "[" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "]" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else {
          count = sscanf ( word, "%d%n", &jval, &width );
        }
      }
/*
  VECTOR
*/
      else if ( leqi ( levnam[level], "VECTOR" ) == TRUE ) {
        if ( strcmp ( word, "[" ) == 0 ) {
        }
        else if ( strcmp ( word, "]" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
/*
  (normal) VECTOR
*/
        else if ( leqi ( levnam[level-1], "NORMAL" ) == TRUE ) {

          count = sscanf ( word, "%f%n", &rval, &width );

          if ( count > 0 ) {

            if ( num_normal_temp < MAX_ORDER * MAX_FACE ) {
              normal_temp[ixyz][num_normal_temp] = rval;
              ixyz = ixyz + 1;
              if ( ixyz == 3 ) {
                ixyz = 0;
                num_normal_temp = num_normal_temp + 1;
              }
            }
          }
          else {
            num_bad = num_bad + 1;
            printf ( "NORMAL VECTOR: bad data %s\n", word );
          }
        }
      }
/*
  (vertexproperty) VERTEX
*/
      else if ( leqi ( levnam[level], "VERTEX" ) == TRUE ) {

        if ( leqi ( levnam[level-1], "VERTEXPROPERTY" ) == TRUE ) {

          if ( strcmp ( word, "[" ) == 0 ) {
            ixyz = 0;
            nbase = num_cor3;
          }
          else if ( strcmp ( word, "]" ) == 0 ) {
            level = nlbrack - nrbrack;
          }
          else {
            count = sscanf ( word, "%f%n", &rval, &width );

            if ( count > 0 ) {
 
              if ( num_cor3 < MAX_COR3 ) {
                cor3[ixyz][num_cor3] = rval;
              }
              ixyz = ixyz + 1;
              if ( ixyz == 3 ) {
                ixyz = 0;
                num_cor3 = num_cor3 + 1;
              }

            }
            else {
              num_bad = num_bad + 1;
              printf ( "Bad data %s\n", word );
            }
          }
        }
      }
/*
  (indexedtrianglestripset) VERTEXPROPERTY
*/
      else if ( leqi ( levnam[level], "VERTEXPROPERTY" ) == TRUE ) {

        if ( strcmp ( word, "{" ) == 0 ) {
        }
        else if ( strcmp ( word, "}" ) == 0 ) {
          level = nlbrack - nrbrack;
        }
        else if ( leqi ( word, "VERTEX" ) == TRUE ) {
        }
        else if ( leqi ( word, "NORMAL" ) == TRUE ) {
          ixyz = 0;
        }
        else if ( leqi ( word, "MATERIALBINDING" ) == TRUE ) {
          count = sscanf ( next, "%s%n", word, &width );
          next = next + width;
        }
        else if ( leqi ( word, "NORMALBINDING" ) == TRUE ) {
          count = sscanf ( next, "%s%n", word, &width );
          next = next + width;
        }
        else {
          num_bad = num_bad + 1;
          printf ( "Bad data %s\n", word );
        }
      }
/*
  VKNOTVECTOR
*/
      else if ( leqi ( levnam[level], "VKNOTVECTOR" ) == TRUE ) {

        if ( strcmp ( word, "[" ) == 0 ) {
          continue;
        }
        else if ( strcmp ( word, "]" ) == 0 ) {
          level = nlbrack - nrbrack;
          continue;
        }
        else {
          count = sscanf ( word, "%d%n", &jval, &width );
        }
      }
/*
  Any other word:
*/
      else {
      }
    }
/*
  End of information on current line.
*/
  }
/*
  Check the "materials" defining a line.

  If COORDINDEX is -1, so should be the MATERIALINDEX.
  If COORDINDEX is not -1, then the MATERIALINDEX shouldn"t be either.
*/
  ihi = MIN ( num_line, MAX_LINE );

  for ( i = 0; i < ihi; i++ ) {

    if ( line_dex[i] == -1 ) {
      line_mat[i] = -1;
    }
    else if ( line_mat[i] == -1 ) {
      line_mat[i] = 0;
    }

  }
  return SUCCESS;
}

/******************************************************************************/

int converter::iv_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    IV_WRITE writes graphics information to an Inventor file.

  Modified:

    19 November 1998

  Author:
 
    John Burkardt
*/
  int icor3;
  int iface;
  int ivert;
  int j;
  int length;
  int num_text;

  num_text = 0;

  fprintf ( fileout, "#Inventor V2.0 ascii\n" );
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "Separator {\n" );
  fprintf ( fileout, "  Info {\n" );
  fprintf ( fileout, "    string \"%s geconverternewated by IVCON.\"\n", fileout_name );
  fprintf ( fileout, "    string \"Original data in file %s.\"\n", filein_name );
  fprintf ( fileout, "  }\n" );
  fprintf ( fileout, "  Separator {\n" );
  num_text = num_text + 8;
/*
  LightModel:

    BASE_COLOR ignores light sources, and uses only diffuse color
      and transparency.  Even without normal vector information,
      the object will show up.  However, you won't get shadow
      and lighting effects.

    PHONG uses the Phong lighting model, accounting for light sources
      and surface orientation.  This is the default.  I believe
      you need accurate normal vector information in order for this
      option to produce nice pictures.

    DEPTH ignores light sources, and calculates lighting based on
      the location of the object within the near and far planes
      of the current camera's view volume.
*/
  fprintf ( fileout, "    LightModel {\n" );
  fprintf ( fileout, "      model PHONG\n" );
  fprintf ( fileout, "    }\n" );
  num_text = num_text + 3;
/*
  Material
*/
  fprintf ( fileout, "    Material {\n" );
  fprintf ( fileout, "      ambientColor  0.2 0.2 0.2\n" );
  fprintf ( fileout, "      diffuseColor  0.8 0.8 0.8\n" );
  fprintf ( fileout, "      emissiveColor 0.0 0.0 0.0\n" );
  fprintf ( fileout, "      specularColor 0.0 0.0 0.0\n" );
  fprintf ( fileout, "      shininess     0.2\n" );
  fprintf ( fileout, "      transparency  0.0\n" );
  fprintf ( fileout, "    }\n" );
  num_text = num_text + 8;
/*
  MaterialBinding
*/
  fprintf ( fileout, "    MaterialBinding {\n" );
  fprintf ( fileout, "      value PER_VERTEX_INDEXED\n" );
  fprintf ( fileout, "    }\n" );
  num_text = num_text + 3;
/*
  NormalBinding

    PER_VERTEX promises that we will write a list of normal vectors
    in a particular order, namely, the normal vectors for the vertices
    of the first face, then the second face, and so on.

    PER_VERTEX_INDEXED promises that we will write a list of normal vectors,
    and then, as part of the IndexedFaceSet, we will give a list of
    indices referencing this normal vector list.

*/
  fprintf ( fileout, "    NormalBinding {\n" );
  fprintf ( fileout, "      value PER_VERTEX_INDEXED\n" );
  fprintf ( fileout, "    }\n" );
  num_text = num_text + 3;
/*
  ShapeHints
*/
  fprintf ( fileout, "    ShapeHints {\n" );
  fprintf ( fileout, "      vertexOrdering COUNTERCLOCKWISE\n" );
  fprintf ( fileout, "      shapeType UNKNOWN_SHAPE_TYPE\n" );
  fprintf ( fileout, "      faceType CONVEX\n" );
  fprintf ( fileout, "      creaseAngle 6.28319\n" );
  fprintf ( fileout, "    }\n" );
  num_text = num_text + 6;
/*
  Point coordinates.
*/
  fprintf ( fileout, "    Coordinate3 {\n" );
  fprintf ( fileout, "      point [\n" );
  num_text = num_text + 2;

  for ( j = 0; j < num_cor3; j++ ) {
    fprintf ( fileout, "        %f %f %f,\n", cor3[0][j], cor3[1][j], cor3[2][j] );
    num_text = num_text + 1;
  }
  fprintf ( fileout, "      ]\n" );
  fprintf ( fileout, "    }\n" );
  num_text = num_text + 2;
/*
  BaseColor.
*/
  if ( num_color > 0 ) {

    fprintf ( fileout, "    BaseColor {\n" );
    fprintf ( fileout, "      rgb [\n" );
    num_text = num_text + 2;

    for ( j = 0; j < num_color; j++ ) {
      fprintf ( fileout, "        %f %f %f,\n", rgbcolor[0][j], rgbcolor[1][j], 
        rgbcolor[2][j] );
      num_text = num_text + 1;
    }

    fprintf ( fileout, "      ]\n" );
    fprintf ( fileout, "    }\n" );
    num_text = num_text + 2;
  }
/*
  Normal vectors.
    Use the normal vectors associated with nodes.
*/
  if ( num_face > 0 ) {

    fprintf ( fileout, "    Normal { \n" );
    fprintf ( fileout, "      vector [\n" );
    num_text = num_text + 2;

    for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {
      fprintf ( fileout, "        %f %f %f,\n", 
        cor3_normal[0][icor3], 
        cor3_normal[1][icor3], 
        cor3_normal[2][icor3] );
      num_text = num_text + 1;
    }

    fprintf ( fileout, "      ]\n" );
    fprintf ( fileout, "    }\n" );
    num_text = num_text + 2;
  }
/*
  IndexedLineSet
*/
  if ( num_line > 0 ) {

    fprintf ( fileout, "    IndexedLineSet {\n" );
/*
  IndexedLineSet coordIndex
*/
    fprintf ( fileout, "      coordIndex [\n" );
    num_text = num_text + 2;

    length = 0;

    for ( j = 0; j < num_line; j++ ) {

      if ( length == 0 ) {
        fprintf ( fileout, "       " );
      }

      fprintf ( fileout, " %d,", line_dex[j] );
      length = length + 1;

      if ( line_dex[j] == -1 || length >= 10 || j == num_line-1 ) {
        fprintf ( fileout, "\n" );
        num_text = num_text + 1;
        length = 0;
      }
    }

    fprintf ( fileout, "      ]\n" );
    num_text = num_text + 1;
/*
  IndexedLineSet materialIndex.
*/
    fprintf ( fileout, "      materialIndex [\n" );
    num_text = num_text + 1;

    length = 0;

    for ( j = 0; j < num_line; j++ ) {

      if ( length == 0 ) {
        fprintf ( fileout, "       " );
      }

      fprintf ( fileout, " %d,", line_mat[j] );
      length = length + 1;

      if ( line_mat[j] == -1 || length >= 10 || j == num_line-1 ) {
        fprintf ( fileout, "\n" );
        num_text = num_text + 1;
        length = 0;
      }
    }

    fprintf ( fileout, "      ]\n" );
    fprintf ( fileout, "    }\n" );
    num_text = num_text + 2;
  }
/*
  IndexedFaceSet.
*/
  if ( num_face > 0 ) {

    fprintf ( fileout, "    IndexedFaceSet {\n" );
    fprintf ( fileout, "      coordIndex [\n" );
    num_text = num_text + 2;

    for ( iface = 0; iface < num_face; iface++ ) {

      fprintf ( fileout, "       " );

      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        fprintf ( fileout, " %d,", face[ivert][iface] );
      }
      fprintf ( fileout, " -1,\n" );
      num_text = num_text + 1;
    }

    fprintf ( fileout, "      ]\n" );
    num_text = num_text + 1;
/*
  IndexedFaceSet materialIndex
*/
    fprintf ( fileout, "      materialIndex [\n" );
    num_text = num_text + 1;

    for ( iface = 0; iface < num_face; iface++ ) {

      fprintf ( fileout, "       " );

      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        fprintf ( fileout, " %d,", face_mat[ivert][iface] );
      }
      fprintf ( fileout, " -1,\n" );
      num_text = num_text + 1;
    }

    fprintf ( fileout, "      ]\n" );
    num_text = num_text + 1;
/*
  IndexedFaceSet normalIndex
*/
    fprintf ( fileout, "      normalIndex [\n" );
    num_text = num_text + 1;

    for ( iface = 0; iface < num_face; iface++ ) {

      fprintf ( fileout, "       " );

      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        fprintf ( fileout, " %d,", face[ivert][iface] );
      }
      fprintf ( fileout, " -1,\n" );
      num_text = num_text + 1;
    }
    fprintf ( fileout, "      ]\n" );
    fprintf ( fileout, "    }\n" );
    num_text = num_text + 2;
  }
/*
  Close up the Separator nodes.
*/
  fprintf ( fileout, "  }\n" );
  fprintf ( fileout, "}\n" );
  num_text = num_text + 2;
/*
  Report.
*/
  printf ( "\n" );
  printf ( "IV_WRITE - Wrote %d text lines;\n", num_text );

  return SUCCESS;
}
/******************************************************************************/

int converter::ivec_max ( int n, int *a ) {

/******************************************************************************/

/*
  Purpose:

    IVEC_MAX returns the maximum element in an integer array.

  Modified:

    09 October 1998

  Author:

    John Burkardt
*/
  int  i;
  int *ia;
  int  imax;

  if ( n <= 0 ) {
    imax = 0;
  }
  else {
    ia = a;
    imax = *ia;
    for ( i = 1; i < n; i++ ) {
      ia = ia + 1;
      if ( imax < *ia ) {
        imax = *ia;
      }
    }
  }
  return imax;
}
/******************************************************************************/

int eqi ( char* string1, char* string2 ) {

/******************************************************************************/

/*
  Purpose:

    LEQI compares two strings for equality, disregarding case.

  Modified:

    15 September 1998

  Author:
 
    John Burkardt
*/
  int i;
  int nchar;
  int nchar1;
  int nchar2;

  nchar1 = strlen ( string1 );
  nchar2 = strlen ( string2 );
  nchar = MIN ( nchar1, nchar2 );

/*
  The strings are not equal if they differ over their common length.
*/
  for ( i = 0; i < nchar; i++ ) {

    if ( toupper ( string1[i] ) != toupper ( string2[i] ) ) {
      return FALSE;
    }
  }
/*
  The strings are not equal if the longer one includes nonblanks
  in the tail.
*/
  if ( nchar1 > nchar ) {
    for ( i = nchar; i < nchar1; i++ ) {
      if ( string1[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  else if ( nchar2 > nchar ) {
    for ( i = nchar; i < nchar2; i++ ) {
      if ( string2[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  return TRUE;
}

/******************************************************************************/

void converter::minmax ( void ) {

/******************************************************************************/

/*
  Purpose:

    MINMAX computes the coordinate minima and maxima.

  Modified:

    31 August 1998

  Author:

    John Burkardt
*/
  int   i;
  float xave;
  float xmax;
  float xmin;
  float yave;
  float ymax;
  float ymin;
  float zave;
  float zmax;
  float zmin;

  xave = cor3[0][0];
  xmax = cor3[0][0];
  xmin = cor3[0][0];

  yave = cor3[1][0];
  ymax = cor3[1][0];
  ymin = cor3[1][0];

  zave = cor3[2][0];
  zmax = cor3[2][0];
  zmin = cor3[2][0];

  for ( i = 1; i < num_cor3; i++ ) {

    xave = xave + cor3[0][i];
    xmin = MIN ( xmin, cor3[0][i] );
    xmax = MAX ( xmax, cor3[0][i] );

    yave = yave + cor3[1][i];
    ymin = MIN ( ymin, cor3[1][i] );
    ymax = MAX ( ymax, cor3[1][i] );

    zave = zave + cor3[2][i];
    zmin = MIN ( zmin, cor3[2][i] );
    zmax = MAX ( zmax, cor3[2][i] );
  }

  xave = xave / num_cor3;
  yave = yave / num_cor3;
  zave = zave / num_cor3;

  printf ( "\n" );
  printf ( "MINMAX - Data range:\n" );
  printf ( "\n" );
  printf ( "   Minimum   Average   Maximum  Range\n" );
  printf ( "\n" );
  printf ( "X  %f %f %f %f\n", xmin, xave, xmax, xmax-xmin );
  printf ( "Y  %f %f %f %f\n", ymin, yave, ymax, ymax-ymin );
  printf ( "Z  %f %f %f %f\n", zmin, zave, zmax, zmax-zmin );

}
/******************************************************************************/

void converter::news ( void ) {

/******************************************************************************/

/*
  Purpose:

    converternewS reports the program change history.

  Modified:
 
    03 December 1998

  Author:
 
    John Burkardt
*/
  printf ( "\n" );
  printf ( "Recent changes:\n" );
  printf ( "\n" );
  printf ( "  03 December 1998\n" );
  printf ( "    Set up simple hooks in TDS_READ_MATERIAL_SECTION.\n" );
  printf ( "  02 December 1998\n" );
  printf ( "    Set up simple hooks for texture map names.\n" );
  printf ( "  19 November 1998\n" );
  printf ( "    IV_WRITE uses PER_VERTEX normal binding.\n" );
  printf ( "  18 November 1998\n" );
  printf ( "    Added node normals.\n" );
  printf ( "    Finally added the -RN option.\n" );
  printf ( "  17 November 1998\n" );
  printf ( "    Added face node ordering reversal option.\n" );
  printf ( "  20 October 1998\n" );
  printf ( "    Added DATA_REPORT.\n" );
  printf ( "  19 October 1998\n" );
  printf ( "    SMF_READ and SMF_WRITE added.\n" );
  printf ( "  16 October 1998\n" );
  printf ( "    Fixing a bug in IV_READ that chokes on ]} and other\n" );
  printf ( "    cases where brackets aren't properly spaced.\n" );
  printf ( "  11 October 1998\n" );
  printf ( "    Added face subset selection option S.\n" );
  printf ( "  09 October 1998\n" );
  printf ( "    Reworking normal vector treatments.\n" );
  printf ( "    Synchronizing IVREAD and IVCON.\n" );
  printf ( "    POV_WRITE added.\n" );
  printf ( "  02 October 1998\n" );
  printf ( "    IVCON reproduces BOX.3DS and CONE.3DS exactly.\n" );
  printf ( "  30 September 1998\n" );
  printf ( "    IVCON compiled on the PC.\n" );
  printf ( "    Interactive BYTE_SWAP option added for binary files.\n" );
  printf ( "  25 September 1998\n" );
  printf ( "    OBJECT_NAME made available to store object name.\n" );
  printf ( "  23 September 1998\n" );
  printf ( "    3DS binary files can be written.\n" );
  printf ( "  15 September 1998\n" );
  printf ( "    3DS binary files can be read.\n" );
  printf ( "  01 September 1998\n" );
  printf ( "    MINMAX, AVE_FACE_NORMAL added.\n" );
  printf ( "    Major modifications to normal vectors.\n" );
  printf ( "  24 August 1998\n" );
  printf ( "    HRC_READ added.\n" );
  printf ( "  21 August 1998\n" );
  printf ( "    TXT_WRITE improved.\n" );
  printf ( "  20 August 1998\n" );
  printf ( "    HRC_WRITE can output lines as linear splines.\n" );
  printf ( "  19 August 1998\n" );
  printf ( "    Automatic normal computation for OBJ files.\n" );
  printf ( "    Added normal vector computation.\n" );
  printf ( "    HRC_WRITE is working.\n" );
  printf ( "  18 August 1998\n" );
  printf ( "    IV read/write handles BASECOLOR RGB properly now.\n" );
  printf ( "    Improved treatment of face materials and normals.\n" );
  printf ( "  17 August 1998\n" );
  printf ( "    MAX_ORDER increased to 35.\n" );
  printf ( "    FACE_PRINT routine added.\n" );
  printf ( "    INIT_DATA routine added.\n" );
  printf ( "  14 August 1998\n" );
  printf ( "    IV_Read is working.\n" );
  printf ( "  13 August 1998\n" );
  printf ( "    ASE_Write is working.\n" );
  printf ( "    IV_Write is working.\n" );
  printf ( "  12 August 1998\n" );
  printf ( "    ASE_Read is working.\n" );
  printf ( "  10 August 1998\n" );
  printf ( "    DXF_Write is working.\n" );
  printf ( "    DXF_Read is working.\n" );
  printf ( "  27 July 1998\n" );
  printf ( "    Interactive mode is working.\n" );
  printf ( "    OBJ_Read is working.\n" );
  printf ( "  25 July 1998\n" );
  printf ( "    OBJ_Write is working.\n" );
  printf ( "  24 July 1998\n" );
  printf ( "    InCheck checks the input data.\n" );
  printf ( "    VLA_Read is working.\n" );
  printf ( "    VLA_Write is working.\n" );
  printf ( "  23 July 1998\n" );
  printf ( "    STL_Write is working.\n" );
  printf ( "  22 July 1998\n" );
  printf ( "    STL_Read is working.\n" );
  printf ( "    TXT_Write is working.\n" );
}
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
  int   i;
  int   ivert;
  char *next;
  char *next2;
  char *next3;
  int   node;
  int   num_vertex_normal;
  float r1;
  float r2;
  float r3;
  char  token[MAX_INPUT];
  char  token2[MAX_INPUT];
  int   width;
/* 
  Initialize. 
*/
  num_vertex_normal = 0;
/* 
  Read the next line of the file into INPUT. 
*/
  while ( fgets ( input, MAX_INPUT, filein ) != NULL ) {

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
        if ( *next2 == '/' ) {

          for ( next3 = next2 + 1; next3 < token2 + MAX_INPUT; next3++ ) {

            if ( *next3 == '/' ) {
              next3 = next3 + 1;
              count = sscanf ( next3, "%d%n", &node, &width );

              node = node - 1;
              if ( 0 <= node && node < num_vertex_normal ) {
                for ( i = 0; i < 3; i++ ) {
                  vertex_normal[i][ivert][num_face] = normal_temp[i][node];
                }
              }
              break;
            }
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

      if ( num_cor3 < MAX_COR3 ) {
        cor3[0][num_cor3] = r1;
        cor3[1][num_cor3] = r2;
        cor3[2][num_cor3] = r3;
      }

      num_cor3 = num_cor3 + 1;

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
      continue;
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
  printf ( "\n" );
  printf ( "OBJ_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
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
  printf ( "\n" );
  printf ( "POV_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
/******************************************************************************/

void converter::print_sizes ( ) {

/******************************************************************************/

/*
  Purpose:

    PRINT_SIZES reports the size in bytes of various data types.

  Modified:

    14 October 1998

  Author:

    John Burkardt
*/
  printf ( "\n" );
  printf ( "PRINT_SIZES: Report data type sizes.\n" );
  printf ( "\n" );
  printf ( "  Type                Size     Min     Max\n" );
  printf ( "\n" );
  printf ( "  char                %d       %d      %d\n", 
    sizeof ( char ), CHAR_MIN, CHAR_MAX );
  printf ( "  unsigned char       %d       0       %d\n", 
    sizeof ( char ),           UCHAR_MAX );
  printf ( "  short int           %d       %d      %d\n", 
    sizeof ( short int ), SHRT_MIN, SHRT_MAX );
  printf ( "  unsigned short int  %d       0       %u\n", 
    sizeof ( unsigned short int ),           USHRT_MAX );
  printf ( "  int                 %d       %d      %d\n", 
    sizeof ( int ), INT_MIN, INT_MAX );
  printf ( "  unsigned int        %d       0       %u\n", 
    sizeof ( unsigned int ), UINT_MAX );
  printf ( "  long int            %d       %d      %d\n", 
    sizeof ( long int ), LONG_MIN, LONG_MAX );
  printf ( "  unsigned long int   %d       0       %u\n", 
    sizeof ( unsigned long int ), ULONG_MAX );
/*
  FLT_MIN, FLT_MAX, DBL_MIN, DBL_MAX not defined on Microsoft C.
*/

/*
  printf ( "  float               %d       %e      %e\n", 
    sizeof ( float ), FLT_MIN, FLT_MAX );
  printf ( "  double              %d       %e      %e\n", 
    sizeof ( double ), DBL_MIN, DBL_MAX );
*/

  return;
}

/******************************************************************************/

int converter::rcol_find ( float a[][MAX_COR3], int m, int n, float r[] ) { 

/******************************************************************************/

/*
  Purpose:
   
    RCOL_FIND finds if a vector occurs in a table.

  Comment:

    Explicitly forcing the second dimension to be MAX_COR3 is a kludge.
    I have to figure out how to do this as pointer references.

    Also, since the array is not sorted, this routine should not be carelessly
    called repeatedly for really big values of N, because you'll waste a
    lot of time.

  Modified:

    27 April 1999

  Author:
 
    John Burkardt
*/
  int i;
  int icol;
  int j;

  icol = -1;

  for ( j = 0; j < n; j++ ) {
    for ( i = 0; i < m; i++ ) {
      if ( a[i][j] != r[i] ) {
        break;
      }
      if ( i == m-1 ) {
        return j;
      }
    }
  }

  return icol;
}
/******************************************************************************/

float converter::reverse_bytes_float ( float x ) {

/******************************************************************************/

/*
  Purpose:

    REVERSE_BYTES_FLOAT reverses the four bytes in a float.

  Modified:

    11 September 1998

  Author:

    John Burkardt

  Parameters:

    X, a float whose bytes are to be reversed.

    REVERSE_BYTES_FLOAT, a float with bytes in reverse order from those in X.

*/
  char c;
  union {
    float yfloat;
    char ychar[4];
  } y;

  y.yfloat = x;
  
  c = y.ychar[0];
  y.ychar[0] = y.ychar[3];
  y.ychar[3] = c;

  c = y.ychar[1];
  y.ychar[1] = y.ychar[2];
  y.ychar[2] = c;

  return ( y.yfloat );
}
/******************************************************************************/

void converter::set_cor3_normal (  ) {

/******************************************************************************/

/*
  Purpose:

    SET_COR3_NORMAL computes node normal vectors.

  Modified:

    18 November 1998

  Author:

    John Burkardt
*/
  int   icor3;
  int   iface;
  int   ivert;
  int   j;
  float norm;
  float temp;

  for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {
    for ( j = 0; j < 3; j++ ) {
      cor3_normal[j][icor3] = 0.0;
    }
  }
/*
  Add up the normals at all the faces to which the node belongs.
*/
  for ( iface = 0; iface < num_face; iface++ ) {

    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {

      icor3 = face[ivert][iface];

      for ( j = 0; j < 3; j++ ) {
        cor3_normal[j][icor3] = cor3_normal[j][icor3]
          + vertex_normal[j][ivert][iface];
      }
    }
  }
/*
  Renormalize.
*/
  for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {

    norm = 0.0;
    for ( j = 0; j < 3; j++ ) {
      temp = cor3_normal[j][icor3];
      norm = norm + temp * temp;
    }

    if ( norm == 0.0 ) {
      norm = 3.0;
      for ( j = 0; j < 3; j++ ) {
        cor3_normal[j][icor3] = 1.0;
      }
    }

    norm = ( float ) sqrt ( norm );

    for ( j = 0; j < 3; j++ ) {
      cor3_normal[j][icor3] = cor3_normal[j][icor3] / norm;
    }
  }

  return;
}
/******************************************************************************/

void converter::set_vertex_normal (  ) {

/******************************************************************************/

/*
  Purpose:

    SET_VERTEX_NORMAL recomputes the face vertex normal vectors.

  Modified:

    12 October 1998

  Author:

    John Burkardt
*/
  int   i;
  int   i0;
  int   i1;
  int   i2;
  int   iface;
  int   ivert;
  int   jp1;
  int   jp2;
  int   nfix;
  float norm;
  float temp;
  float x0;
  float x1;
  float x2;
  float xc;
  float y0;
  float y1;
  float y2;
  float yc;
  float z0;
  float z1;
  float z2;
  float zc;

  if ( num_face <= 0 ) {
    return;
  }

  nfix = 0;
/*
  Consider each face.
*/
  for ( iface = 0; iface < num_face; iface++ ) {

    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {

      norm = 0.0;
      for ( i = 0; i < 3; i++ ) {
        temp = vertex_normal[i][ivert][iface];
        norm = norm + temp * temp;
      }
      norm = ( float ) sqrt ( norm );

      if ( norm == 0.0 ) {

        nfix = nfix + 1;

        i0 = face[ivert][iface];
        x0 = cor3[0][i0];
        y0 = cor3[1][i0];
        z0 = cor3[2][i0];

        jp1 = ivert + 1;
        if ( jp1 >= face_order[iface] ) {
          jp1 = jp1 - face_order[iface];
        }
        i1 = face[jp1][iface];
        x1 = cor3[0][i1];
        y1 = cor3[1][i1];
        z1 = cor3[2][i1];

        jp2 = ivert + 2;
        if ( jp2 >= face_order[iface] ) {
          jp2 = jp2 - face_order[iface];
        }
        i2 = face[jp2][iface];
        x2 = cor3[0][i2];
        y2 = cor3[1][i2];
        z2 = cor3[2][i2];

        xc = ( y1 - y0 ) * ( z2 - z0 ) - ( z1 - z0 ) * ( y2 - y0 );
        yc = ( z1 - z0 ) * ( x2 - x0 ) - ( x1 - x0 ) * ( z2 - z0 );
        zc = ( x1 - x0 ) * ( y2 - y0 ) - ( y1 - y0 ) * ( x2 - x0 );

        norm = ( float ) sqrt ( xc * xc + yc * yc + zc * zc );

        if ( norm == 0.0 ) {
          xc = ( float ) 1.0 / sqrt ( 3.0 );
          yc = ( float ) 1.0 / sqrt ( 3.0 );
          zc = ( float ) 1.0 / sqrt ( 3.0 );
        }
        else {
          xc = xc / norm;
          yc = yc / norm;
          zc = zc / norm;
        }

        vertex_normal[0][ivert][iface] = xc;
        vertex_normal[1][ivert][iface] = yc;
        vertex_normal[2][ivert][iface] = zc;

      }
    }
  }

  if ( nfix > 0 ) {
    printf ( "\n" );
    printf ( "SET_VERTEX_NORMAL: Recomputed %d face vertex normals.\n", nfix );
  }

  return;
}
/******************************************************************************/

int converter::smf_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    SMF_READ reads an SMF file.

  Example:

    #  magnolia.obj

    v -3.269770 -39.572201 0.876128
    v -3.263720 -39.507999 2.160890
    ...
    v 0.000000 -9.988540 0.000000
    f 8 9 11 10
    f 12 13 15 14
    ...
    f 788 806 774

  Modified:

    22 October 1998

  Author:
 
    John Burkardt
*/
  int   count;
  int   ivert;
  char *next;
  int   node;
  float r1;
  float r2;
  float r3;
  char  token[MAX_INPUT];
  char  token2[MAX_INPUT];
  int   vert_base;
  int   width;

  vert_base = 0;
/* 
  Read the next line of the file into INPUT. 
*/
  while ( fgets ( input, MAX_INPUT, filein ) != NULL ) {

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
  BEGIN
*/
    if ( leqi ( token, "BEGIN" ) == TRUE ) {

      vert_base = num_cor3;
      num_group = num_group + 1;

    }
/*
  END
*/
    else if ( leqi ( token, "END" ) == TRUE ) {
    }
/*  
  F V1 V2 V3

  Face.
  A face is defined by the vertices.
  Node indices are 1 based rather than 0 based.
  So we have to decrement them before loading them into FACE.
  Note that vertex indices start back at 0 each time a BEGIN is entered.
  The strategy here won't handle nested BEGIN's, just one at a time.
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

        if ( count != 1 ) {
          break;
        }

        if ( ivert < MAX_ORDER && num_face < MAX_FACE ) {
          face[ivert][num_face] = node - 1 + vert_base;
          face_mat[ivert][num_face] = 0;
          face_order[num_face] = face_order[num_face] + 1;
        } 
        ivert = ivert + 1;
      } 
      num_face = num_face + 1;
    }
/*
  ROT
*/
    else if ( leqi ( token, "ROT" ) == TRUE ) {
    }
/*
  TRANS
*/
    else if ( leqi ( token, "TRANS" ) == TRUE ) {
    }
/*
  V X Y Z
  Geometric vertex.
*/

    else if ( leqi ( token, "V" ) == TRUE ) {

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

      if ( num_cor3 < MAX_COR3 ) {
        cor3[0][num_cor3] = r1;
        cor3[1][num_cor3] = r2;
        cor3[2][num_cor3] = r3;
      }

      num_cor3 = num_cor3 + 1;

    }
/*
  Unrecognized  
*/
    else {
      num_bad = num_bad + 1;
    }

  }
  return SUCCESS;
}
/******************************************************************************/

int converter::smf_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    SMF_WRITE writes an SMF file.

  Example:

    #  magnolia.obj

    v -3.269770 -39.572201 0.876128
    v -3.263720 -39.507999 2.160890
    ...
    v 0.000000 -9.988540 0.000000
    f 8 9 11 10
    f 12 13 15 14
    ...
    f 788 806 774

  Modified:

    19 October 1998

  Author:
 
    John Burkardt
*/
  int   i;
  int   iface;
  int   ivert;
  int   num_text;
/* 
  Initialize. 
*/
  num_text = 0;

  fprintf ( fileout, "#$SMF 1.0\n" );
  fprintf ( fileout, "#$vertices %d\n", num_cor3 );
  fprintf ( fileout, "#$faces %d\n", num_face );
  fprintf ( fileout, "#\n" );
  fprintf ( fileout, "# %s created by IVCON.\n", fileout_name );
  fprintf ( fileout, "# Original data in %s.\n", filein_name );
  fprintf ( fileout, "#\n" );

  num_text = num_text + 7;
/* 
  V: vertex coordinates. 
*/
  for ( i = 0; i < num_cor3; i++ ) {
    fprintf ( fileout, "v %f %f %f\n", 
      cor3[0][i], cor3[1][i], cor3[2][i] );
    num_text = num_text + 1;
  }
/* 
  F: faces. 
*/
  if ( num_face > 0 ) {
    fprintf ( fileout, "\n" );
    num_text = num_text + 1;
  }

  for ( iface = 0; iface < num_face; iface++ ) {

    fprintf ( fileout, "f" );
    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
      fprintf ( fileout, " %d", face[ivert][iface]+1 );
    }
    fprintf ( fileout, "\n" );
    num_text = num_text + 1;
  }
/*
  Report.
*/
  printf ( "\n" );
  printf ( "SMF_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
/******************************************************************************/

int converter::stla_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    STLA_READ reads an ASCII STL (stereolithography) file.

  Modified:

    20 October 1998

  Author:
 
    John Burkardt
*/
  int   count;
  int   i;
  int   icor3;
  int   ivert;
  char *next;
  float r1;
  float r2;
  float r3;
  float r4;
  float temp[3];
  char  token[MAX_INPUT];
  int   width;
/*
  Read the next line of the file into INPUT. 
*/
  while ( fgets ( input, MAX_INPUT, filein ) != NULL ) {

    num_text = num_text + 1;
/*
  Advance to the first nonspace character in INPUT. 
*/
    for ( next = input; *next != '\0' && isspace(*next); next++ ) {
    }
/*
  Skip blank lines and comments. 
*/
    if ( *next == '\0' || *next == '#' || *next == '!' || *next == '$' ) {
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
  FACET
*/
    if ( leqi ( token, "facet" ) == TRUE ) {
/* 
  Get the XYZ coordinates of the normal vector to the face. 
*/
      sscanf ( next, "%*s %e %e %e", &r1, &r2, &r3 );  

      if ( num_face < MAX_FACE ) {
        face_normal[0][num_face] = r1;
        face_normal[1][num_face] = r2;
        face_normal[2][num_face] = r3;
      }

      fgets ( input, MAX_INPUT, filein );
      num_text = num_text + 1;

      ivert = 0;

      for ( ;; ) {

        fgets ( input, MAX_INPUT, filein );
        num_text = num_text + 1;

        count = sscanf ( input, "%*s %e %e %e", &r1, &r2, &r3 );

        if ( count != 3 ) {
          break;
        }

        temp[0] = r1;
        temp[1] = r2;
        temp[2] = r3;

        if ( num_cor3 < 1000 ) {
          icor3 = rcol_find ( cor3, 3, num_cor3, temp );
        }
        else {
          icor3 = -1;
        }

        if ( icor3 == -1 ) {

          icor3 = num_cor3;

          if ( num_cor3 < MAX_COR3 ) {
            for ( i = 0; i < 3; i++ ) {
              cor3[i][num_cor3] = temp[i];
            }
          }
          num_cor3 = num_cor3 + 1;
        }
        else {
          num_dup = num_dup + 1;
        }

        if ( ivert < MAX_ORDER && num_face < MAX_FACE ) {
          face[ivert][num_face] = icor3;
          face_mat[ivert][num_face] = 0;
          for ( i = 0; i < 3; i++ ) {
            vertex_normal[i][ivert][num_face] = face_normal[i][num_face];
          }
        }

        ivert = ivert + 1;
      } 

      fgets ( input, MAX_INPUT, filein );
      num_text = num_text + 1;

      if ( num_face < MAX_FACE ) {
        face_order[num_face] = ivert;
      } 

      num_face = num_face + 1;

    }
/*
  COLOR 
*/

    else if ( leqi ( token, "color" ) == TRUE ) {
      sscanf ( next, "%*s %f %f %f %f", &r1, &r2, &r3, &r4 );
    }
/* 
 SOLID 
*/
    else if ( leqi ( token, "solid" ) == TRUE ) {
      num_object = num_object + 1;
    }
/* 
 ENDSOLID 
*/
    else if ( leqi ( token, "endsolid" ) == TRUE ) {
    }
/* 
  Unexpected or unrecognized. 
*/
    else {
      printf ( "\n" );
      printf ( "STLA_READ - Error!\n" );
      printf ( "  Unrecognized first word on line.\n" );
      return ERROR;
    }

  }
  return SUCCESS;
}
/******************************************************************************/

int converter::stla_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    STLA_WRITE writes an ASCII STL (stereolithography) file.

  Modified:

    01 September 1998

  Author:
 
    John Burkardt
*/
  int   iface;
  int   ivert;
  int   k;
  int   num_text;
/*
  Initialize.
*/
  num_text = 0;
/* 
  SOLID 
*/
  fprintf ( fileout, "solid MYSOLID created by IVCON, original data in %s\n", 
    filein_name );

  num_text = num_text + 1;
/* 
  FACET NORMAL x y z
     OUTER LOOP
       VERTEX x y z
       ...
       VERTEX x y z
     ENDLOOP
   ENDFACET
*/
  for ( iface = 0; iface < num_face; iface++ ) {

    fprintf ( fileout, "  facet normal %f %f %f\n", 
      face_normal[0][iface], face_normal[1][iface], face_normal[2][iface] );

    fprintf ( fileout, "    outer loop\n" );
    num_text = num_text + 2;

    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {

      k = face[ivert][iface];

      fprintf ( fileout, "      vertex %f %f %f\n", 
        cor3[0][k], cor3[1][k], cor3[2][k] );
      num_text = num_text + 1;
    }

    fprintf ( fileout, "    endloop\n" );
    fprintf ( fileout, "  endfacet\n" );
    num_text = num_text + 2;
  }
/* 
  ENDSOLID 
*/
  fprintf ( fileout, "endsolid MYSOLID\n" );
  num_text = num_text + 1;
/*
  Report.
*/
  printf ( "\n" );
  printf ( "STLA_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
/******************************************************************************/

void converter::tds_pre_process ( void ) {

/******************************************************************************/

/*
  Purpose:

    TDS_PRE_PROCESS divides the monolithic object into acceptably small pieces.

  Note:

    The 3DS binary format allows an unsigned short int for the number of
    points, and number of faces in an object.  This limits such quantities
    to 65535.  We have at least one interesting object with more faces
    than that.  So we need to tag faces and nodes somehow.

  Modified:

    14 October 1998

  Author:

    John Burkardt
*/
  
  static unsigned short int BIG = 60000;

  return;
}
/******************************************************************************/

int converter::tds_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    TDS_READ reads a 3D Studio MAX binary 3DS file.

  Modified:

    20 October 1998

  Author:
 
    John Burkardt
*/
  unsigned long int   chunk_begin;
  unsigned long int   chunk_end;
  unsigned long int   chunk_length;
  unsigned long int   chunk_length2;
  unsigned long int   position;
  unsigned short int  temp_int;
  int                 version;
  int                 views_read;
/* 
  Initialize.
*/
  views_read = 0;

  temp_int = tds_read_u_short_int ( filein );

  if ( temp_int == 0x4d4d ) {

    if ( debug == TRUE ) {
      printf ( "TDS_READ: DEBUG: Read magic number.\n" );
    }
/* 
  Move to 28 bytes from the beginning of the file. 
*/
    position = 28;
    fseek ( filein, position, SEEK_SET );
    version = fgetc ( filein );

    if ( version < 3 ) {
      printf ( "\n" );
      printf ( "TDS_READ - Fatal error!\n" );
      printf ( "  This routine can only read 3DS version 3 or later.\n" );
      printf ( "  The input file is version %d.\n" ,version );
      return ERROR;
    }

    if ( debug == TRUE ) {
      printf ( "TDS_READ: DEBUG: Read version number.\n" );
    }
/* 
  Move to 2 bytes from the beginning of the file. 
  Set CURRENT_POINTER to the first byte of the chunk.
  Set CHUNK_LENGTH to the number of bytes in the chunk.
*/
    chunk_begin = 0;
    position = 2;
    fseek ( filein, position, SEEK_SET );

    chunk_length = tds_read_u_long_int ( filein );
    position = 6;

    chunk_end = chunk_begin + chunk_length;

    if ( debug == TRUE ) {
      printf ( "TDS_READ: Chunk length = %lu.\n", chunk_length );
    }

    while ( position + 2 < chunk_end ) {

      temp_int = tds_read_u_short_int ( filein );
      position = position + 2;

      if ( debug == TRUE ) {
        printf ( "TDS_READ: Short int = %hu, position = %lu.\n", temp_int, position );
      }

      if ( temp_int == 0x0002 ) {
        chunk_length2 = tds_read_u_long_int ( filein );
        position = position + 4;
        position = position - 6 + chunk_length2;
        fseek ( filein, position, SEEK_SET );
      }
      else if ( temp_int == 0x3d3d ) {
        position = position - 2;
        position = position + tds_read_edit_section ( filein, &views_read );
      }
      else if ( temp_int == 0xb000 ) {
        position = position - 2;
        position = position + tds_read_keyframe_section ( filein, &views_read );
      }
      else {
        printf ( "\n" );
        printf ( "TDS_READ: Error!\n" );
        printf ( "  Unexpected input, position = %lu.\n", position );
        printf ( "  TEMP_INT = %hux\n", temp_int );
        return ERROR;
      }
    }
    position = chunk_begin + chunk_length;
    fseek ( filein, position, SEEK_SET );
  }
  else {
    printf ( "\n" );
    printf ( "TDS_READ - Fatal error!\n" );
    printf ( "  Could not find the main section tag.\n" );
    return ERROR;
  }

  return SUCCESS;
}
/******************************************************************************/

unsigned long converter::tds_read_ambient_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  int                 i;
  float               rgb_val[3];
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller; 
  unsigned char       true_c_val[3];
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;
 
    switch ( temp_int ) {
      case 0x0010:
        if ( debug == TRUE ) {
          printf ( "     COLOR_F color definition section tag of %0X\n", 
            temp_int );
        }
        for ( i = 0; i < 3; i++ ) {
          rgb_val[i] = tds_read_float ( filein );
        }
        teller = teller + 3 * sizeof ( float );
        break;
      case 0x0011:
        if ( debug == TRUE ) {
          printf ( "     COLOR_24 24 bit color definition section tag of %0X\n",
            temp_int );
        }

        for ( i = 0; i < 3; i++ ) {
          true_c_val[i] = fgetc ( filein );
        }
        teller = teller + 3;
        break;
      default:
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_background_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  int                 i;
  float               rgb_val[3];
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller; 
  unsigned char       true_c_val[3];
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;

  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;
 
    switch ( temp_int ) {
      case 0x0010:
        if ( debug == TRUE ) {
          printf ( "   COLOR_F RGB color definition section tag of %0X\n", 
            temp_int );
        }
        for ( i = 0; i < 3; i++ ) {
          rgb_val[i] = tds_read_float ( filein );
        }
        teller = teller + 3 * sizeof ( float );
        break;
      case 0x0011:
        if ( debug == TRUE ) {
          printf ( "   COLOR_24 24 bit color definition section tag of %0X\n", 
            temp_int );
        }
 
        for ( i = 0; i < 3; i++ ) {
          true_c_val[i] = fgetc ( filein );
        }
        teller = teller + 3;
        break;
      default:
        break;
    }

    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_boolean ( unsigned char *boolean, FILE *filein ) {

/******************************************************************************/

  unsigned long current_pointer;
  unsigned long temp_pointer;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  *boolean = fgetc ( filein );
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_camera_section ( FILE *filein ) {

/******************************************************************************/

  float               camera_eye[3];
  float               camera_focus[3];
  unsigned long int   current_pointer;
  float               lens;
  float               rotation;
  unsigned long int   temp_pointer;
  unsigned short int  u_short_int_val;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  camera_eye[0] = tds_read_float ( filein );
  camera_eye[1] = tds_read_float ( filein );
  camera_eye[2] = tds_read_float ( filein );
 
  camera_focus[0] = tds_read_float ( filein );
  camera_focus[1] = tds_read_float ( filein );
  camera_focus[2] = tds_read_float ( filein );

  rotation = tds_read_float ( filein );
  lens = tds_read_float ( filein );

  if ( debug == TRUE ) {
    printf ( " Found camera viewpoint at XYZ = %f %f %f.\n",
      camera_eye[0], camera_eye[1], camera_eye[2] );
    printf ( "     Found camera focus coordinates at XYZ = %f %f %f.\n",
      camera_focus[0], camera_focus[1], camera_focus[2] );
    printf ( "     Rotation of camera is:  %f.\n", rotation );
    printf ( "     Lens in used camera is: %f mm.\n", lens );
  }
 
  if ( ( temp_pointer-38 ) > 0 ) {

    if ( debug == TRUE ) {
      printf ( "          Found extra camera sections.\n" );
    }

    u_short_int_val = tds_read_u_short_int ( filein );

    if ( u_short_int_val == 0x4710 ) {
      if ( debug == TRUE ) {
        printf ( "          CAM_SEE_CONE.\n" );
      }
      tds_read_unknown_section ( filein );
    }

    u_short_int_val = tds_read_u_short_int ( filein );

    if ( u_short_int_val == 0x4720 ) {
      if ( debug == TRUE ) {
        printf ( "          CAM_RANGES.\n" );
      }
      tds_read_unknown_section ( filein );
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_edit_section ( FILE *filein, int *views_read ) {

/******************************************************************************/

/*
  Modified:

    18 September 1998
*/
  unsigned long int   chunk_length;
  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  unsigned long int   teller;
  unsigned short int  temp_int;

  current_pointer = ftell ( filein ) - 2;
  chunk_length = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;
 
    switch ( temp_int ) {
      case 0x1100:
        if ( debug == TRUE ) {
          printf ( "    BIT_MAP section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1201:
        if ( debug == TRUE ) {
          printf ( "    USE_SOLID_BGND section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1300:
        if ( debug == TRUE ) {
          printf ( "    V_GRADIENT section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1400:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1420:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1450:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1500:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2200:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2201:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2210:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2300:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2302:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x3000:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2100:
        if ( debug == TRUE ) {
          printf ( "    AMBIENT_LIGHT section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_ambient_section ( filein );
        break;
      case 0x1200:
        if ( debug == TRUE ) {
          printf ( "    SOLID_BGND section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_background_section ( filein );
        break;
      case 0x0100:
        if ( debug == TRUE ) {
          printf ( "    MASTER_SCALE section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x3d3e:
        if ( debug == TRUE ) {
          printf ( "    MESH_VERSION section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xafff:
        if ( debug == TRUE ) {
          printf ( "    MAT_ENTRY section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_material_section ( filein );
        break;
      case 0x4000:
        if ( debug == TRUE ) {
          printf ( "    NAMED_OBJECT section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_object_section ( filein );
        break;
      case 0x7001:
        if ( debug == TRUE ) {
          printf ( "    VIEWPORT_LAYOUT section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_view_section ( filein, views_read );
        break;
      case 0x7012:
        if ( debug == TRUE ) {
          printf ( "    VIEWPORT_DATA_3 section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x7011:
        if ( debug == TRUE ) {
          printf ( "    VIEWPORT_DATA section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x7020:
        if ( debug == TRUE ) {
          printf ( "    VIEWPORT_SIZE section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default:
        if ( debug == TRUE ) {
          printf ( "    Junk.\n" );
        }
        break;
    }
 
    if ( teller >= chunk_length ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+chunk_length, SEEK_SET );

  return ( chunk_length );
}
/******************************************************************************/

float converter::tds_read_float ( FILE *filein ) {

/******************************************************************************/

/*
  Note that we may need to reverse the bytes on floating point values!
*/
  float rval;
  float temp;

  fread ( &temp, sizeof ( float ), 1, filein );

  if ( byte_swap == TRUE ) {
    rval = reverse_bytes_float ( temp );
  }
  else {
    rval = temp;
  }

  return rval;
}
/******************************************************************************/

unsigned long converter::tds_read_keyframe_section ( FILE *filein, int *views_read ) {

/******************************************************************************/

  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;

    switch ( temp_int ) {
      case 0x7001:
        if ( debug == TRUE ) {
          printf ( "    VIEWPORT_LAYOUT main definition section tag of %0X\n",
            temp_int );
        }
        teller = teller + tds_read_view_section ( filein, views_read );
        break;
      case 0xb008:
        if ( debug == TRUE ) {
          printf ( "    KFSEG frames section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb002:
        if ( debug == TRUE ) {
          printf ( "    OBJECT_NODE_TAG object description section tag of %0X\n",
            temp_int);
        }
        teller = teller + tds_read_keyframe_objdes_section ( filein );
        break;
      case 0xb009:
        if ( debug == TRUE ) {
          printf ( "    KFCURTIME section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb00a:
        if ( debug == TRUE ) {
          printf ( "    KFHDR section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default: 
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_keyframe_objdes_section ( FILE *filein ) {

/******************************************************************************/

/*
  Modified:

    21 September 1998
*/
  unsigned long int   chunk_size;
  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller;
  unsigned long int   u_long_int_val;
  unsigned short int  u_short_int_val;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;

    switch ( temp_int ) {
      case 0xb011:
        if ( debug == TRUE ) {
          printf ( "      INSTANCE_NAME section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb010:
        if ( debug == TRUE ) {
          printf ( "      NODE_HDR section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb020:
        if ( debug == TRUE ) {
          printf ( "      POS_TRACK_TAG section tag of %0X\n", temp_int );
        }
        chunk_size = tds_read_u_long_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_long_int_val = tds_read_u_long_int ( filein );
        origin[0] = tds_read_float ( filein );
        origin[1] = tds_read_float ( filein );
        origin[2] = tds_read_float ( filein );
        teller = teller + 32;
        break;
      case 0xb013:
        if ( debug == TRUE ) {
          printf ( "      PIVOT section tag of %0X\n", temp_int );
        }
        chunk_size = tds_read_u_long_int ( filein );
        pivot[0] = tds_read_float ( filein );
        pivot[1] = tds_read_float ( filein );
        pivot[2] = tds_read_float ( filein );
        teller = teller + 12;
        break;
      case 0xb014:
        if ( debug == TRUE ) {
          printf ( "      BOUNDBOX section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb015:
        if ( debug == TRUE ) {
          printf ( "      MORPH_SMOOTH section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb021:
        if ( debug == TRUE ) {
          printf ( "      ROT_TRACK_TAG section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb022:
        if ( debug == TRUE ) {
          printf ( "      SCL_TRACK_TAG section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb030:
        if ( debug == TRUE ) {
          printf ( "      NODE_ID section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default: 
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_light_section ( FILE *filein ) {

/******************************************************************************/

  unsigned char       boolean;
  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  int                 i;
  float               light_coors[3];
  float               rgb_val[3];
  unsigned long int   teller;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned char       true_c_val[3];
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;

  light_coors[0] = tds_read_float ( filein );
  light_coors[1] = tds_read_float ( filein );
  light_coors[2] = tds_read_float ( filein );

  teller = teller + 3 * 4;
 
  if ( debug == TRUE ) {
    printf ( "     Found light at coordinates XYZ = %f %f %f.\n",
      light_coors[0], light_coors[1], light_coors[2] );
  }
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;
 
    switch ( temp_int ) {
      case 0x0010:
        if ( debug == TRUE ) {
          printf ( "      COLOR_F RGB color definition section tag of %0X\n", 
            temp_int );
        }
        for ( i = 0; i < 3; i++ ) {
          rgb_val[i] = tds_read_float ( filein );
        }
        teller = teller + 3 * sizeof ( float );
        break;
      case 0x0011:
        if ( debug == TRUE ) {
          printf ( "      COLOR_24 24 bit color definition section tag of %0X\n",
            temp_int );
        }

        for ( i = 0; i < 3; i++ ) {
          true_c_val[i] = fgetc ( filein );
        }
        teller = teller + 3;
        break;
      case 0x4620:
        if ( debug == TRUE ) {
          printf ( "      DL_OFF section: %0X\n", temp_int );
        }
        teller = teller + tds_read_boolean ( &boolean, filein );
        if ( debug == TRUE ) {
          if ( boolean == TRUE ) {
            printf ( "      Light is on\n" );
          }
          else {
            printf ( "      Light is off\n" );
          }
        }
        break;
      case 0x4610:
        if ( debug == TRUE  ) {
          printf ( "      DL_SPOTLIGHT section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_spot_section ( filein );
        break;
      case 0x465a:
        if ( debug == TRUE  ) {
          printf ( "      DL_OUTER_RANGE section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default:
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );
 
  return ( temp_pointer );
}
/******************************************************************************/

unsigned long int converter::tds_read_u_long_int ( FILE *filein ) {

/******************************************************************************/

/*
  Modified:
 
    01 October 1998

  Author:

    John Burkardt
*/

  union {
    unsigned long int yint;
    char ychar[4];
  } y;

  if ( byte_swap == TRUE ) {
    y.ychar[3] = fgetc ( filein );
    y.ychar[2] = fgetc ( filein );
    y.ychar[1] = fgetc ( filein );
    y.ychar[0] = fgetc ( filein );
  }
  else {
    y.ychar[0] = fgetc ( filein );
    y.ychar[1] = fgetc ( filein );
    y.ychar[2] = fgetc ( filein );
    y.ychar[3] = fgetc ( filein );
  }

  return y.yint;
}
/******************************************************************************/

int converter::tds_read_long_name ( FILE *filein ) {

/******************************************************************************/

  unsigned char  letter;
  unsigned int   teller;

  teller = 0;
  letter = fgetc ( filein );
/*
  Could be a dummy object. 
*/
  if ( letter == 0 ) {
    strcpy ( temp_name, "Default_name" );
    return -1; 
  }

  temp_name[teller] = letter;
  teller = teller + 1;
 
  do {
    letter = fgetc ( filein );
    temp_name[teller] = letter;
    teller = teller + 1;
  } while ( letter != 0 );
 
  temp_name[teller-1] = 0;
 
  if ( debug == TRUE ) {
    printf ( "      tds_read_long_name found name: %s.\n", temp_name );
  }

  return teller;
}
/******************************************************************************/

unsigned long converter::tds_read_matdef_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int  current_pointer;
  unsigned int       teller;
  unsigned long int  temp_pointer;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  teller = tds_read_long_name ( filein );

  if ( teller == -1 ) {
    if ( debug == TRUE ) {
      printf ( "      No material name found.\n" );
    }
  }
  else {
    strcpy ( mat_name, temp_name );
    if ( debug == TRUE ) {
      printf ( "      Material name %s.\n", mat_name );
    }
  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_material_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller;
 
  current_pointer = ftell ( filein ) - 2;

  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;
 
    switch ( temp_int ) {

      case 0xa000:
        if ( debug == TRUE ) {
          printf ( "     MAT_NAME definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_matdef_section ( filein );
        break;
      case 0xa010:
        if ( debug == TRUE ) {
          printf ( "     MAT_AMBIENT definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa020:
        if ( debug == TRUE ) {
          printf ( "     MAT_DIFFUSE definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa030:
        if ( debug == TRUE ) {
          printf ( "     MAT_SPECULAR definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa040:
        if ( debug == TRUE ) {
          printf ( "     MAT_SHININESS definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa041:
        if ( debug == TRUE ) {
          printf ( "     MAT_SHIN2PCT definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa042:
        if ( debug == TRUE ) {
          printf ( "     MAT_SHIN3PCT definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa050:
        if ( debug == TRUE ) {
          printf ( "     MAT_TRANSPARENCY definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa052:
        if ( debug == TRUE ) {
          printf ( "     MAT_XPFALL definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa053:
        if ( debug == TRUE ) {
          printf ( "     MAT_REFBLUR definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa080:
        if ( debug == TRUE ) {
          printf ( "     MAT_SELF_ILLUM definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa081:
        if ( debug == TRUE ) {
          printf ( "     MAT_TWO_SIDE definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa082:
        if ( debug == TRUE ) {
          printf ( "     MAT_DECAL definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa083:
        if ( debug == TRUE ) {
          printf ( "     MAT_ADDITIVE definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa084:
        if ( debug == TRUE ) {
          printf ( "     MAT_SELF_ILPCT definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa085:
        if ( debug == TRUE ) {
          printf ( "     MAT_WIRE definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa086:
        if ( debug == TRUE ) {
          printf ( "     MAT_SUPERSMP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa087:
        if ( debug == TRUE ) {
          printf ( "     MAT_WIRESIZE definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa088:
        if ( debug == TRUE ) {
          printf ( "     MAT_FACEMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa08a:
        if ( debug == TRUE ) {
          printf ( "     MAT_XPFALLIN definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa08c:
        if ( debug == TRUE ) {
          printf ( "     MAT_PHONGSOFT definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa08e:
        if ( debug == TRUE ) {
          printf ( "     MAT_WIREABS definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa100:
        if ( debug == TRUE ) {
          printf ( "     MAT_SHADING definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa200:
        if ( debug == TRUE ) {
          printf ( "     MAT_TEXMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa204:
        if ( debug == TRUE ) {
          printf ( "     MAT_SPECMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa210:
        if ( debug == TRUE ) {
          printf ( "     MAT_OPACMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa220:
        if ( debug == TRUE ) {
          printf ( "     MAT_REFLMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa230:
        if ( debug == TRUE ) {
          printf ( "     MAT_BUMPMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa353:
        if ( debug == TRUE ) {
          printf ( "     MAT_MAP_TEXBLUR definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default:
        if ( debug == TRUE ) {
          printf ( "  Junk section tag of %0X\n", temp_int );
        }
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

int converter::tds_read_name ( FILE *filein ) {

/******************************************************************************/

  unsigned char  letter;
  unsigned int   teller;

  teller = 0;
  letter = fgetc ( filein );
/*
  Could be a dummy object.  
*/

  if ( letter == 0 ) {
    strcpy ( temp_name, "Default name" );
    return (-1); 
  }

  temp_name[teller] = letter;
  teller = teller + 1;
 
  do {
    letter = fgetc ( filein );
    temp_name[teller] = letter;
    teller = teller + 1;
  } while ( ( letter != 0 ) && ( teller < 12 ) );
 
  temp_name[teller-1] = 0;
 
  if ( debug == TRUE ) {
    printf ( "      tds_read_name found name: %s.\n", temp_name );
  }

  return 0;
}
/******************************************************************************/

unsigned long converter::tds_read_obj_section ( FILE *filein ) {

/******************************************************************************/

/*
  Comments:

    Thanks to John F Flanagan for some suggested corrections.

  Modified:

    27 April 1999
*/
  unsigned long int   chunk_size;
  unsigned short int  color_index;
  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  int                 i;
  int                 j;
  int                 num_cor3_base;
  int                 num_cor3_inc;
  int                 num_face_inc;
  unsigned long int   u_long_int_val;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   temp_pointer2;
  unsigned long int   teller; 
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
  num_cor3_base = num_cor3;

  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;

    switch ( temp_int ) {

      case 0x4000:
        if ( debug == TRUE ) {
          printf ( "        NAMED_OBJECT section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4100:
        if ( debug == TRUE ) {
          printf ( "        N_TRI_OBJECT section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4110:

        if ( debug == TRUE ) {
          printf ( "        POINT_ARRAY section tag of %0X\n", temp_int );
        }

        current_pointer = ftell ( filein ) - 2;
        temp_pointer2 = tds_read_u_long_int ( filein );
        num_cor3_inc =  ( int ) tds_read_u_short_int ( filein );
 
        for ( i = num_cor3; i < num_cor3 + num_cor3_inc; i++ ) {
          cor3[0][i] = tds_read_float ( filein );
          cor3[1][i] = tds_read_float ( filein );
          cor3[2][i] = tds_read_float ( filein );
        }
 
        num_cor3 = num_cor3 + num_cor3_inc;
        teller = teller + temp_pointer2;
        break;

      case 0x4111:
        if ( debug == TRUE ) {
          printf ( "        POINT_FLAG_ARRAY faces (2) section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4120:

        if ( debug == TRUE ) {
          printf ( "        FACE_ARRAY section tag of %0X\n", 
            temp_int );
        }

        temp_pointer2 = tds_read_u_long_int ( filein );
        num_face_inc = ( int ) tds_read_u_short_int ( filein );
 
        for ( i = num_face; i < num_face + num_face_inc; i++ ) {
          face[0][i] = tds_read_u_short_int ( filein ) + num_cor3_base;
          face[1][i] = tds_read_u_short_int ( filein ) + num_cor3_base;
          face[2][i] = tds_read_u_short_int ( filein ) + num_cor3_base;
          face_order[i] = 3;
          face_flags[i] = tds_read_u_short_int ( filein );
        }

        temp_int = tds_read_u_short_int ( filein );
        if ( temp_int == 0x4150 ) {
          for ( i = num_face; i < num_face + num_face_inc; i++ ) {
            face_smooth[i] = tds_read_u_long_int ( filein ) + num_cor3_base;
          }
        }
        num_face = num_face + num_face_inc;
        teller = ftell ( filein );
        break;

      case 0x4130:
        if ( debug == TRUE ) {
          printf ( "        MSH_MAT_GROUP section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4140:
        if ( debug == TRUE ) {
          printf ( "        TEX_VERTS section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4150:
        if ( debug == TRUE ) {
          printf ( "        SMOOTH_GROUP section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4160:

        if ( debug == TRUE ) {
          printf ( "        MESH_MATRIX section tag of %0X\n", 
            temp_int );
        }

        u_long_int_val = tds_read_u_long_int ( filein );

        for ( j = 0; j < 4; j++ ) {
          for ( i = 0; i < 3; i++ ) {
            transform_mat[j][i] = tds_read_float ( filein );
          }
        }
        transform_mat[0][3] = 0.0;
        transform_mat[1][3] = 0.0;
        transform_mat[2][3] = 0.0;
        transform_mat[3][3] = 0.0;

        teller = teller + 12 * sizeof ( float );
        break;

      case 0x4165:

        if ( debug == TRUE ) {
          printf ( "        MESH_COLOR section tag of %0X\n", temp_int );
        }

        chunk_size = tds_read_u_long_int ( filein );

        if ( chunk_size == 7 ) {
          color_index = fgetc ( filein );
          teller = teller + 5;
        }
        else {
          color_index = tds_read_u_short_int ( filein );
          teller = teller + 6;
        } 

        break;

      case 0x4170:
        if ( debug == TRUE ) {
          printf ( "        MESH_TEXTURE_INFO section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      default:
        if ( debug == TRUE ) {
          printf ( "        JUNK section tag of %0X\n", temp_int );
        }
        break;
    }
 
    if (  teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_object_section ( FILE *filein ) {

/******************************************************************************/

  unsigned char       end_found = FALSE;
  unsigned long int   current_pointer;
  int                 int_val;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
/*
  Why don't you read and save the name here?
*/
  int_val = tds_read_name ( filein );

  if ( int_val == -1 ) {
    if ( debug == TRUE ) {
      printf ( "      Dummy Object found\n" );
    }
  }
  else {
    strcpy ( object_name, temp_name );
  }
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;

    switch ( temp_int ) {
      case 0x4700:
        if ( debug == TRUE ) {
          printf ( "      N_CAMERA section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_camera_section ( filein );
        break;
      case 0x4600:
        if ( debug == TRUE ) {
          printf ( "      N_DIRECT_LIGHT section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_light_section ( filein );
        break;
      case 0x4100:
        if ( debug == TRUE ) {
          printf ( "      OBJ_TRIMESH section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_obj_section ( filein );
        break;
      case 0x4010: 
        if ( debug == TRUE ) {
          printf ( "      OBJ_HIDDEN section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x4012: 
        if ( debug == TRUE ) {
          printf ( "      OBJ_DOESNT_CAST section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default:
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned short int converter::tds_read_u_short_int ( FILE *filein ) {

/******************************************************************************/

  unsigned char  c1;
  unsigned char  c2;
  short int      ival;

  c1 = fgetc ( filein );
  c2 = fgetc ( filein );

  ival = c1 | ( c2 << 8 );

  return ival;
}
/******************************************************************************/

unsigned long converter::tds_read_spot_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int  current_pointer;
  float              falloff;
  float              hotspot;
  float              target[4];
  unsigned long int  temp_pointer;

  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  target[0] = tds_read_float ( filein );
  target[1] = tds_read_float ( filein );
  target[2] = tds_read_float ( filein );
  hotspot = tds_read_float ( filein );
  falloff = tds_read_float ( filein );
 
  if ( debug == TRUE ) {
    printf ( "      The target of the spot is XYZ = %f %f %f.\n",
      target[0], target[1], target[2] );
    printf ( "      The hotspot of this light is %f.\n", hotspot );
    printf ( "      The falloff of this light is %f.\n", falloff );
  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_unknown_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int  current_pointer;
  unsigned long int  temp_pointer;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_view_section ( FILE *filein, int *views_read ) {

/******************************************************************************/

  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;

    switch ( temp_int ) {
      case 0x7012:
        if ( debug == TRUE ) {
          printf ( "     VIEWPORT_DATA_3 section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_vp_section ( filein, views_read );
        break;
      case 0x7011:
        if ( debug == TRUE ) {
          printf ( "     VIEWPORT_DATA section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x7020:
        if ( debug == TRUE ) {
          printf ( "     VIEWPORT_SIZE section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_vp_section ( filein, views_read );
        break;
      default:
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }
 
    if ( *views_read > 3 ) {
      end_found = TRUE;
    }
  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_vp_section ( FILE *filein, int *views_read ) {

/******************************************************************************/

  unsigned int       attribs;
  unsigned long int  current_pointer;
  int                i;
  int                int_val;
  unsigned int       port;
  unsigned long int  temp_pointer;
  char              *viewports[11] = {
                      "Bogus",
                      "Top",
                      "Bottom",
                      "Left",
                      "Right",
                      "Front",
                      "Back",
                      "User",
                      "Camera",
                      "Light",
                      "Disabled"
                     };

  *views_read = *views_read + 1;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  attribs = tds_read_u_short_int ( filein );

  if ( attribs == 3 ) {
    if ( debug == TRUE ) {
      printf ( "<Snap> active in viewport.\n" );
    }
  }

  if ( attribs == 5 ) {
    if ( debug == TRUE ) {
      printf ( "<Grid> active in viewport.\n" );
    }
  }
/* 
  Read 5 INTS to get to the viewport information. 
*/
  for ( i = 1; i < 6; i++ ) {
    tds_read_u_short_int ( filein ); 
  }

  port = tds_read_u_short_int ( filein );
/*
  Find camera section. 
*/
  if ( ( port == 0xffff ) || ( port == 0 ) ) {

    for ( i = 0; i < 12; i++ ) {
      tds_read_u_short_int ( filein );
    }
 
    int_val = tds_read_name (filein );
 
    if ( int_val == -1 ) {
      if ( debug == TRUE ) {
        printf ( "      No Camera name found\n" );
      }
    }

    port = 0x0008;
  }
 
  if ( debug == TRUE ) {
    printf ( "Reading [%s] information with tag:%d\n", viewports[port], port );
  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET ); 

  return ( temp_pointer );
}
/******************************************************************************/

int converter::tds_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:

    TDS_WRITE writes graphics information to a 3D Studio Max 3DS file.

  Modified:

    14 October 1998

  Author:

    John Burkardt

*/
  float               float_val;
  int                 i;
  int                 icor3;
  int                 iface;
  int                 j;
  long int            l0002;
  long int            l0100;
  long int            l3d3d;
  long int            l3d3e;
  long int            l4000;
  long int            l4100;
  long int            l4110;
  long int            l4120;
  long int            l4150;
  long int            l4160;
  long int            l4d4d;
  long int            lb000;
  long int            lb002;
  long int            lb00a;
  long int            lb008;
  long int            lb009;
  long int            lb010;
  long int            lb013;
  long int            lb020;
  long int            lb021;
  long int            lb022;
  long int            lb030;
  long int            long_int_val;
  int                 name_length;
  int                 num_bytes;
  short int           short_int_val;
  unsigned short int  u_short_int_val;

  num_bytes = 0;
  name_length = strlen ( object_name );

  l0002 = 10;

  l4150 = 2 + 4 + num_face * 4;
  l4120 = 2 + 4 + 2 + 4 * num_face * 2 + l4150;
  l4160 = 2 + 4 + 4 * 12;
  l4110 = 2 + 4 + 2 + num_cor3 * 3 * 4;
  l4100 = 2 + 4 + l4110 + l4160 + l4120;
  l4000 = 2 + 4 + ( name_length + 1 ) + l4100;
  l0100 = 2 + 4 + 4;
  l3d3e = 2 + 4 + 4;
  l3d3d = 2 + 4 + l3d3e + l0100 + l4000;

  lb022 = 2 + 4 + 32;
  lb021 = 2 + 4 + 9 * 4;
  lb020 = 2 + 4 + 8 * 4;
  lb013 = 2 + 4 + 6 * 2;
  lb010 = 2 + 4 + ( name_length + 1 ) + 3 * 2;
  lb030 = 2 + 4 + 2;
  lb002 = 2 + 4 + lb030 + lb010 + lb013 + lb020 + lb021 + lb022;
  lb009 = 2 + 4 + 4;
  lb008 = 2 + 4 + 2 * 4;
  lb00a = 2 + 4 + 2 + 9 + 2 * 2;
  lb000 = 2 + 4 + lb00a + lb008 + lb009 + lb002;

  l4d4d = 2 + 4 + l0002 + l3d3d + lb000;
/*  
  M3DMAGIC begin.
    tag, size.
*/
  short_int_val = ( short ) 0x4d4d;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4d4d );
/*
  M3D_VERSION begin.
    tag, size, version.
*/
  short_int_val = ( short ) 0x0002;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l0002 );
  long_int_val = 3;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
/*  
  M3D_VERSION end.
  MDATA begin.
    tag, size.
*/
  short_int_val = ( short ) 0x3d3d;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l3d3d );
/*  
  MESH_VERSION begin.
    tag, size, version.
*/
  short_int_val = ( short ) 0x3d3e;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l3d3e );
  long_int_val = 3;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
/*  
  MESH_VERSION end.  
  MASTER_SCALE begin.  
    tag, size, scale.
*/
  short_int_val = ( short ) 0x0100;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l0100 );
  float_val = 1.0;
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
/*
  MASTER_SCALE end. 
  NAMED_OBJECT begin. 
    tag, size, name. 
*/
  short_int_val = ( short ) 0x4000;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4000 );
  num_bytes = num_bytes + tds_write_string ( fileout, object_name );
/*  
  N_TRI_OBJECT begin.  
    tag, size.
*/
  short_int_val = ( short ) 0x4100;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4100 );
/*
  POINT_ARRAY begin.  
    tag, size, number of points, coordinates of points.
  Warning! number of points could exceed a short!
*/
  short_int_val = ( short ) 0x4110;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4110 );

  u_short_int_val = ( unsigned short ) num_cor3;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );

  for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {
    for ( j = 0; j < 3; j++ ) {
      num_bytes = num_bytes + tds_write_float ( fileout, cor3[j][icor3] );
    }
  }
/*
  POINT_ARRAY end.
  MESH_MATRIX begin.  
    tag, size, 4 by 3 matrix.
*/
  short_int_val = ( short ) 0x4160;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4160 );

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 3; j++ ) {
      float_val = transform_mat[i][j];
      num_bytes = num_bytes + tds_write_float ( fileout, float_val );
    }
  }
/*
  MESH_MATRIX end.  
  FACE_ARRAY begin. 
    tag, size, number of faces, nodes per face. 
  Warning: number of faces could exceed a short!
*/
  short_int_val = ( short ) 0x4120;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4120 );

  u_short_int_val = ( unsigned short ) num_face;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );

  for ( iface = 0; iface < num_face; iface++ ) {
    for ( j = 0; j < 3; j++ ) {
      short_int_val = face[j][iface];
      num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
    }
    short_int_val = face_flags[iface];
    num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  }
/*
  SMOOTH_GROUP begin.
    tag, size, group for each face.
*/
  short_int_val = ( short ) 0x4150;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4150 );

  for ( iface = 0; iface < num_face; iface++ ) {
    long_int_val = face_smooth[iface];
    num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
  }
/*
  SMOOTH_GROUP end.
  FACE_ARRAY end.
  N_TRI_OBJECT end.
  NAMED_OBJECT end.
  MDATA end. 
  KFDATA begin.
*/
  short_int_val = ( short ) 0xb000;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb000 );
/*
  KFHDR begin.  
    tag, size, revision, filename, animlen.
*/
  short_int_val = ( short ) 0xb00a;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb00a );
  short_int_val = 5;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_string ( fileout, "MAXSCENE" );
  short_int_val = 100;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
/*
  KFHDR end.  
  KFSEG begin.  
    tag, size, start, end.
*/
  short_int_val = ( short ) 0xb008;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb008 );
  long_int_val = 0;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
  long_int_val = 100;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
/*
  KFSEG end.  
  KFCURTIME begin.
    tag, size, current_frame.
*/
  short_int_val = ( short ) 0xb009;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb009 );
  long_int_val = 0;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
/*
  KFCURTIME end.
  OBJECT_NODE_TAG begin.
    tag, size.  
*/
  short_int_val = ( short ) 0xb002;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb002 );
/*
  NODE_ID begin.
    tag, size, id.
*/
  short_int_val = ( short ) 0xb030;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb030 );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
/*
  NODE_ID end.  
  NODE_HDR begin. 
    tag, size, object_name, flag1, flag2, hierarchy.
*/
  short_int_val = ( short ) 0xb010;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb010 );
  num_bytes = num_bytes + tds_write_string ( fileout, object_name );
  short_int_val = 16384;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = -1;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
/*
  NODE_HDR end. 
  PIVOT begin. 
    tag, size, pivot_x, pivot_y, pivot_z.
*/
  short_int_val = ( short ) 0xb013;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb013 );
  for ( i = 0; i < 3; i++ ) {
    float_val = pivot[i];
    num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  }
/*
  PIVOT end. 
  POS_TRACK_TAG begin.  
    tag, size, flag, i1, i2, i3, i4, i5, i6, frame, l1, pos_x, pos_y, pos_z.
*/
  short_int_val = ( short ) 0xb020;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb020 );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 1;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  long_int_val = 0;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
  for ( i = 0; i < 3; i++ ) {
    float_val = origin[i];
    num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  }
/*
  POS_TRACK_TAG end. 
  ROT_TRACK_TAG begin. 
    tag, size, i1, i2, i3, i4, i5, i6, i7, i8, l1, rad, axis_x, axis_y, axis_z. 
*/
  short_int_val = ( short ) 0xb021;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb021 );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 1;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  long_int_val = 0;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
  float_val = 0.0;
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
/*
  ROT_TRACK_TAG end. 
  SCL_TRACK_TAG begin.  
    tag, size, i1, i2, i3, i4, i5, i6, i7, i8, l1, scale_x, scale_y, scale_z.
*/
  short_int_val = ( short ) 0xb022;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb022 );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 1;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  long_int_val = 0;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
  float_val = 1.0;
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
/*  
  SCL_TRACK_TAG end.
  OBJECT_NODE_TAG end.
  KFDATA end.
  M3DMAGIC end. 
*/

/*
  Report.
*/
  printf ( "TDS_WRITE wrote %d bytes.\n", num_bytes );

  return SUCCESS;
}
/******************************************************************************/

int converter::tds_write_float ( FILE *fileout, float float_val ) {

/******************************************************************************/

/*
  Modified:

    23 September 1998
*/
  float temp;

  if ( byte_swap == TRUE ) {
    temp = reverse_bytes_float ( float_val );
  }
  else {
    temp = float_val;
  }

  fwrite ( &temp, sizeof ( float ), 1, fileout );

  return 4;
}
/******************************************************************************/

int converter::tds_write_long_int ( FILE *fileout, long int int_val ) {

/******************************************************************************/

/*
  Modified:
 
    14 October 1998

  Author:

    John Burkardt
*/
  union {
    long int yint;
    char ychar[4];
  } y;

  y.yint = int_val;

  if ( byte_swap == TRUE ) {
    fputc ( y.ychar[3], fileout );
    fputc ( y.ychar[2], fileout );
    fputc ( y.ychar[1], fileout );
    fputc ( y.ychar[0], fileout );
  }
  else {
    fputc ( y.ychar[0], fileout );
    fputc ( y.ychar[1], fileout );
    fputc ( y.ychar[2], fileout );
    fputc ( y.ychar[3], fileout );
  }

  return 4;
}
/******************************************************************************/

int converter::tds_write_string ( FILE *fileout, char *string ) {

/******************************************************************************/

/*
  Modified:

    23 September 1998

  Author:

    John Burkardt
*/
  char *c;
  int   nchar;

  nchar = 0;

  for ( c = string; nchar < 12; c++ ) {

    fputc ( *c, fileout );
    nchar = nchar + 1;

    if  ( *c == 0 ) {
      return nchar;
    }

  }

  return nchar;
}
/******************************************************************************/

int converter::tds_write_short_int ( FILE *fileout, short int short_int_val ) {

/******************************************************************************/

/*
  Modified:

    14 October 1998

  Author:

    John Burkardt
*/
  union {
    short int yint;
    char ychar[2];
  } y;

  y.yint = short_int_val;

  if ( byte_swap == TRUE ) {
    fputc ( y.ychar[1], fileout );
    fputc ( y.ychar[0], fileout );
  }
  else {
    fputc ( y.ychar[0], fileout );
    fputc ( y.ychar[1], fileout );
  }

  return 2;
}
/******************************************************************************/

int converter::tds_write_u_short_int ( FILE *fileout, unsigned short int short_int_val ) {

/******************************************************************************/

/*
  Modified:

    14 October 1998

  Author:

    John Burkardt
*/
  union {
    unsigned short int yint;
    char ychar[2];
  } y;

  y.yint = short_int_val;

  if ( byte_swap == TRUE ) {
    fputc ( y.ychar[1], fileout );
    fputc ( y.ychar[0], fileout );
  }
  else {
    fputc ( y.ychar[0], fileout );
    fputc ( y.ychar[1], fileout );
  }

  return 2;
}
/******************************************************************************/

int converter::txt_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    TXT_WRITE writes the graphics data to a text file.

  Modified:

    02 December 1998

  Author:
 
    John Burkardt
*/
  int i;
  int iface;
  int iline;
  int ivert;
  int nitem;
  int num_text;

  num_text = 0;

  fprintf ( fileout, "%s created by IVCON.\n", fileout_name );
  fprintf ( fileout, "Original data in %s.\n", filein_name );
  fprintf ( fileout, "Object name is %s.\n", object_name );
  fprintf ( fileout, "Object origin at %f %f %f.\n", origin[0], origin[1],
    origin[2] );
  fprintf ( fileout, "Object pivot at %f %f %f.\n", pivot[0], pivot[1],
    pivot[2] );
  num_text = num_text + 5;
/*
  TRANSFORMATION MATRIX.
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "Transformation matrix:\n" );
  fprintf ( fileout, "\n" );
  for ( i = 0; i < 4; i++ ) {
    fprintf ( fileout, "  %f %f %f %f\n", transform_mat[i][0],
      transform_mat[i][1], transform_mat[i][2], transform_mat[i][3] );
  }
  num_text = num_text + 7;
/*
  POINT COORDINATES.
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "  %d points.\n", num_cor3 );
  num_text = num_text + 2;

  if ( num_cor3 > 0 ) {

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Point coordinate data:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    for ( i = 0; i < num_cor3; i++ ) {
      fprintf ( fileout, " %d %f %f %f\n ", i, cor3[0][i], cor3[1][i], 
        cor3[2][i] );
      num_text = num_text + 1;
    }

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Point normal vectors:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    for ( i = 0; i < num_cor3; i++ ) {
      fprintf ( fileout, " %d %f %f %f\n ", i, cor3_normal[0][i], 
        cor3_normal[1][i], cor3_normal[2][i] );
      num_text = num_text + 1;
    }

  }
/*
  LINE INDICES.
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "  %d line data items.\n", num_line );
  num_text = num_text + 2;

  if ( num_line > 0 ) {
    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Line index data:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    nitem = 0;

    for ( iline = 0; iline < num_line; iline++ ) {

      fprintf ( fileout, " %d", line_dex[iline] );
      nitem = nitem + 1;

      if ( iline == num_line - 1 || line_dex[iline] == -1 || nitem >= 10 ) {
        nitem = 0;
        fprintf ( fileout, "\n" );
        num_text = num_text + 1;
      }

    }
/*
  LINE MATERIALS.
*/
    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Line material data:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    nitem = 0;

    for ( iline = 0; iline < num_line; iline++ ) {

      fprintf ( fileout, " %d", line_mat[iline] );
      nitem = nitem + 1;

      if ( iline == num_line - 1 || line_mat[iline] == -1 || nitem >= 10 ) {
        nitem = 0;
        fprintf ( fileout, "\n" );
        num_text = num_text + 1;
      }
    }

  }
/*
  COLOR DATA
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "  %d colors.\n", num_color );
  num_text = num_text + 2;
/*
  TEXTURE MAPS
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "  %d texture maps.\n", num_texmap );
  num_text = num_text + 2;
/*
  FACE DATA.
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "  %d faces.\n", num_face );
  num_text = num_text + 2;

  if ( num_face > 0 ) {

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Face, Number of vertices, Smoothing, Flags:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    for ( iface = 0; iface < num_face; iface++ ) {
      fprintf ( fileout, " %d %d %d %d\n", iface, face_order[iface], 
        face_smooth[iface], face_flags[iface] );
      num_text = num_text + 1;
    }

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Face, Vertices\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    for ( iface = 0; iface < num_face; iface++ ) {

      fprintf ( fileout, "%d   ", iface );
      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        fprintf ( fileout, " %d", face[ivert][iface] );
      }

      fprintf ( fileout, "\n" );
      num_text = num_text + 1;
    }

  }
/*
  FACE NORMAL VECTORS.
*/
  if ( num_face > 0 ) {

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Face normal vectors:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    for ( iface = 0; iface < num_face; iface++ ) {
      fprintf ( fileout, " %d %f %f %f\n", iface, face_normal[0][iface],
        face_normal[1][iface], face_normal[2][iface] );
      num_text = num_text + 1;
    }

  }
/*
  FACE VERTEX NORMAL INDICES.
*/
  if ( num_face > 0 ) {

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "Face vertex-to-normal indices:\n" );
    num_text = num_text + 2;

    for ( iface = 0; iface < num_face; iface++ ) {
      fprintf ( fileout, "\n" );
      num_text = num_text + 1;
      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        fprintf ( fileout, " %d %d %f %f %f\n", iface, ivert, 
          vertex_normal[0][ivert][iface], vertex_normal[1][ivert][iface],
          vertex_normal[2][ivert][iface] );
        num_text = num_text + 1;
      }
    }
  }
/*
  Report.
*/
  printf ( "\n" );
  printf ( "TXT_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
/******************************************************************************/

int converter::vla_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    VLA_READ reads a VLA file.

  Modified:

    24 July 1998

  Author:
 
    John Burkardt
*/
  int   i;
  int   icor3;
  int   num_dup;
  char *next;
  int   num_text;
  float r1;
  float r2;
  float r3;
  float temp[3];
  char  token[MAX_INPUT];
  int   width;
/*
  Initialize. 
*/
  num_cor3 = 0;
  num_dup = 0;
  num_face = 0;
  num_line = 0;
  num_text = 0;
/* 
  Read the next line of the file into INPUT. 
*/
  while ( fgets ( input, MAX_INPUT, filein ) != NULL ) {

    num_text = num_text + 1;
/* 
  Advance to the first nonspace character in INPUT.
*/
    for ( next = input; *next != '\0' && isspace(*next); next++ ) {
    }
/* 
  Skip blank lines and comments. 
*/
    if ( *next == '\0' || *next == ';' ) {
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
  SET (ignore) 
*/
    if ( leqi ( token, "set" ) == TRUE ) {
    }
/* 
  P (begin a line)
  L (continue a line) 
*/
    else if ( leqi ( token, "P" ) == TRUE || leqi ( token, "L") == TRUE ) {

      if ( leqi ( token, "P" ) == TRUE ) {
        if ( num_line > 0 ) {
          if ( num_line < MAX_LINE ) {
            line_dex[num_line] = -1;
            line_mat[num_line] = -1;
            num_line = num_line + 1;
          }
        }
      }

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

      temp[0] = r1;
      temp[1] = r2;
      temp[2] = r3;

      if ( num_cor3 < 1000 ) {
        icor3 = rcol_find ( cor3, 3, num_cor3, temp );
      }
      else {
        icor3 = -1;
      }

      if ( icor3 == -1 ) {

        icor3 = num_cor3;

        if ( num_cor3 < MAX_COR3 ) {
          for ( i = 0; i < 3; i++ ) {
            cor3[i][num_cor3] = temp[i];
          }
        }
        num_cor3 = num_cor3 + 1;
      }
      else {
        num_dup = num_dup + 1;
      }

      if ( num_line < MAX_LINE ) {
        line_dex[num_line] = icor3;
        line_mat[num_line] = 0;
        num_line = num_line + 1;
      }
    }
/* 
  Unexpected or unrecognized. 
*/
    else {
      printf ( "\n" );
      printf ( "VLA_READ - Error!\n" );
      printf ( "  Unrecognized first word on line.\n" );
      return ERROR;
    }

  }

  if ( num_line > 0 ) {
    if ( num_line < MAX_LINE ) {
      line_dex[num_line] = -1;
      line_mat[num_line] = -1;
      num_line = num_line + 1;
    }
  }
/*
  Report.
*/
  printf ( "\n" );
  printf ( "VLA_READ - Input file statistics:\n" );
  printf ( "  %d lines of text;\n", num_text );
  printf ( "  %d faces;\n", num_face );
  printf ( "  %d lines;\n", num_line );
  printf ( "  %d points;\n", num_cor3 );
  printf ( "  %d duplicate points.\n", num_dup );

  return SUCCESS;
}
/******************************************************************************/

int converter::vla_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    VLA_WRITE writes a VLA file.

  Modified:

    24 July 1998

  Author:
 
    John Burkardt
*/
  char  c;
  int   iline;
  int   k;
  int   num_text;
/* 
  Initialize. 
*/
  num_text = 0;

  fprintf ( fileout, "set comment %s created by IVCON.\n", fileout_name );
  fprintf ( fileout, "set comment Original data in %s.\n", filein_name );
  fprintf ( fileout, "set comment\n" );
  fprintf ( fileout, "set intensity EXPLICIT\n" );
  fprintf ( fileout, "set parametric NON_PARAMETRIC\n" );
  fprintf ( fileout, "set filecontent LINES\n" );
  fprintf ( fileout, "set filetype converternew\n" );
  fprintf ( fileout, "set depthcue 0\n" );
  fprintf ( fileout, "set defaultdraw stellar\n" );
  fprintf ( fileout, "set coordsys RIGHT\n" );
  fprintf ( fileout, "set author IVCON\n" );
  fprintf ( fileout, "set site Buhl Planetarium\n" );
  fprintf ( fileout, "set library_id UNKNOWN\n" );

  num_text = num_text + 13;

  c = 'P';

  for ( iline = 0; iline < num_line; iline++ ) {
    
    k = line_dex[iline];

    if ( k == -1 ) {

      c = 'P';
    }
    else {

      fprintf ( fileout, "%c %f %f %f 1.00\n", 
        c, cor3[0][k], cor3[1][k], cor3[2][k] );

      num_text = num_text + 1;

      c = 'L';
    }
  }
/*
  Report.
*/
  printf ( "\n" );
  printf ( "VLA_WRITE - Wrote %d text lines.\n", num_text );


  return SUCCESS;
}
/******************************************************************************/

int converter::leqi ( char* string1, char* string2 ) {

/******************************************************************************/

/*
  Purpose:

    LEQI compares two strings for equality, disregarding case.

  Modified:

    15 September 1998

  Author:
 
    John Burkardt
*/
  int i;
  int nchar;
  int nchar1;
  int nchar2;

  nchar1 = strlen ( string1 );
  nchar2 = strlen ( string2 );
  nchar = MIN ( nchar1, nchar2 );

/*
  The strings are not equal if they differ over their common length.
*/
  for ( i = 0; i < nchar; i++ ) {

    if ( toupper ( string1[i] ) != toupper ( string2[i] ) ) {
      return FALSE;
    }
  }
/*
  The strings are not equal if the longer one includes nonblanks
  in the tail.
*/
  if ( nchar1 > nchar ) {
    for ( i = nchar; i < nchar1; i++ ) {
      if ( string1[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  else if ( nchar2 > nchar ) {
    for ( i = nchar; i < nchar2; i++ ) {
      if ( string2[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  return TRUE;
}




