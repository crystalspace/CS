

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
  int   ivert = 0;
  int   iword;
  int   jval;
  int   level;
  char *next;
  int   nlbrack;
  int   nrbrack;
  float temp[3];
  int   width;
  char  word[MAX_INCHARS];
  char  word1[MAX_INCHARS];
  char  word2[MAX_INCHARS];
  char  wordm1[MAX_INCHARS];
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

    if ( fgets ( input, MAX_INCHARS, filein ) == NULL ) {
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
          fprintf ( logfile,  "\n" );
          fprintf ( logfile,  "HRC_READ - Fatal error!\n" );
          fprintf ( logfile,  "  The input file has a bad header.\n" );
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
          fprintf ( logfile,  "converternew level: %s\n", levnam[level] );
        }
      }
      else if ( strcmp ( word, "}" ) == 0 ) {
        nrbrack = nrbrack + 1;

        if ( nlbrack < nrbrack ) {
          fprintf ( logfile,  "\n" );
          fprintf ( logfile,  "HRC_READ - Fatal error!\n" );
          fprintf ( logfile,  "  Extraneous right bracket on line %d.\n", num_text );
          fprintf ( logfile,  "  Currently processing field %s\n.", levnam[level] );
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
          fprintf ( logfile,  "CONTROLPOINTS: Bad data %s\n", word );
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
          fprintf ( logfile,  "EDGES: Bad data %s\n", word );
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
          fprintf ( logfile,  "MESH: Bad data %s\n", word );
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
          fprintf ( logfile,  "MODEL: Bad data %s\n", word );
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
          fprintf ( logfile,  "NODES: Bad data %s\n", word );
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
          fprintf ( logfile,  "POLYGONS: Bad data %s\n", word );
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
          fprintf ( logfile,  "SPLINE: Bad data %s\n", word );
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
          fprintf ( logfile,  "VERTICES: Bad data %s\n", word );
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
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "HRC_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
