

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
  int   icolor = 0;
  int   icface;
  int   ihi;
  int   inormface;
  int   inum_face;
  int   ivert = 0;
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
  char  word[MAX_INCHARS];
  char  word1[MAX_INCHARS];
  char  wordm1[MAX_INCHARS];

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

    if ( fgets ( input, MAX_INCHARS, filein ) == NULL ) {
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
    while ( input[i] != '\0' && i < MAX_INCHARS ) {

      if ( input[i] == '[' || input[i] == ']' || 
           input[i] == '{' || input[i] == '}' ) {

        result = char_pad ( &i, &null_index, input, MAX_INCHARS );
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
          fprintf ( logfile,  "\n" );
          fprintf ( logfile,  "IV_READ - Fatal error!\n" );
          fprintf ( logfile,  "  The input file has a bad header.\n" );
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
          fprintf ( logfile,  "Level: %s\n", wordm1 );
        }
      }
      else if ( strcmp ( word, "}" ) == 0 || strcmp ( word, "]" ) == 0 ) {
        nrbrack = nrbrack + 1;

        if ( nlbrack < nrbrack ) {
          fprintf ( logfile,  "\n" );
          fprintf ( logfile,  "IV_READ - Fatal error!\n" );
          fprintf ( logfile,  "  Extraneous right bracket on line %d.\n", num_text );
          fprintf ( logfile,  "  Currently processing field %s\n.", levnam[level] );
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
          fprintf ( logfile,  "Bad data %s\n", word );
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
          fprintf ( logfile,  "COORDINATE3: Bad data %s\n", word );
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
          fprintf ( logfile,  "COORDINATE4: Bad data %s\n", word );
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
          fprintf ( logfile,  "Bad data %s\n", word );
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
          fprintf ( logfile,  "Bad data %s\n", word );
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
            fprintf ( logfile,  "Bad data %s\n", word );
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
          fprintf ( logfile,  "Bad data %s\n", word );
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
          fprintf ( logfile,  "Bad data %s\n", word );
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
        else if ( strcmp ( word, "\"" ) == 0) {
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
            fprintf ( logfile,  "Bad data %s\n", word );
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
            fprintf ( logfile,  "Bad data %s\n", word );
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
            fprintf ( logfile,  "Bad data %s\n", word );
          }
        }
        else {
          count = sscanf ( word, "%d%n", &jval, &width );

          if ( count > 0 ) {
          }
          else {
            num_bad = num_bad + 1;
            fprintf ( logfile,  "Bad data %s\n", word );
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
              fprintf ( logfile,  "Bad data %s\n", word );
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
            fprintf ( logfile,  "Bad data %s\n", word );
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
              fprintf ( logfile,  "Bad data %s\n", word );
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
              fprintf ( logfile,  "Bad data %s\n", word );
            }
          }
        }
/*
  (anythingelse RGB)
*/
        else {
          fprintf ( logfile,  "HALSBAND DES TODES!\n" );
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
              fprintf ( logfile,  "Bad data %s\n", word );
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
            fprintf ( logfile,  "Bad data %s\n", word );
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
          fprintf ( logfile,  "Bad data %s\n", word );
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
            fprintf ( logfile,  "Bad data %s\n", word );
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
            fprintf ( logfile,  "NORMAL VECTOR: bad data %s\n", word );
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
              fprintf ( logfile,  "Bad data %s\n", word );
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
          fprintf ( logfile,  "Bad data %s\n", word );
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
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "IV_WRITE - Wrote %d text lines;\n", num_text );

  return SUCCESS;
}
