/*
    The Crystal Space map file parser definition
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

%{ /* Startup C++ code */

#include "cssysdef.h"
#include "stdldr.h"
#include "csutil/cscolor.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

#include "iengine/iengine.h"
#include "ivideo/itxtmgr.h"
#include "iengine/isector.h"
#include "ipolyset.h"
#include "iengine/ipolygon.h"
#include "iengine/ithing.h"
#include "iengine/iportal.h"

/* Define this to debug parser */
//#define YYDEBUG	1

/* yyparse is a member function */
#define yyparse csStandardLoader::yyparse

/* Provide detailed info about parse errors */
#define YYERROR_VERBOSE	1
/* Avoid some "signed vs unsigned comparison" warnings */
#define sizeof	(int)sizeof

// More shortcuts
#define TEX		storage.tex
#define CAMERA		storage.camera
#define PLANE		storage.plane
#define SECTOR		storage.sector

#define ABORTMSG							\
  { yyerror ("loading error, aborting"); YYABORT; }

#define YYERROR_EXTENDED(msg)						\
  if (yychar == STRING)							\
  {									\
    msg = (char *) realloc(msg, size += 14 + strlen (yylval.string));	\
    sprintf (strchr (msg, 0), " (value = `%s')", yylval.string);	\
  }									\
  else if (yychar == NUMBER)						\
  {									\
    msg = (char *) realloc(msg, size += 14 + 20);			\
    sprintf (strchr (msg, 0), " (value = `%g')", yylval.fval);		\
  }

%} /* Definition for all tokens */

/* Tell Bison to build a table with all token names */
%token_table

/* We want a re-enterant parser */
%pure_parser

/* Define the top-level non-terminal symbol */
%start input

/* Possible values for tokens returned by yylex()
 * NOTE: We can't use csColor or csVector3 inside the union since classes
 * with constructors are not allowed there. However, if we carefully place
 * structure members, we can typecast our union to either of the above
 * types (see macros above)
 */
%union
{
  // Just a number
  float fval;
  // A integer number
  int ival;
  // A boolean value
  bool bval;
  // A string value
  char *string;
  // A color
  csPColor color;
  // A 2D point
  csPVector2 vect2;
  // A 3D point
  csPVector3 vect;
  // A transformation matrix
  csMatrix3 *matrix;
  // A transformation matrix/vector
  csStandardLoader::yystorage *transform;
}

/*
    Terminal symbols -- keywords
    The alphabetical order of these is not crucial, but doing so will
    cause less initialization work on the parser (during startup the
    list of keywords is QuickSort'ed). Thus it is still advisable to
    store them in alphabetical order here.

    WARNING: Max 256-3 tokens are allowed. More tokens would require
    slight changes to tokenizer. Currently only 130 are used.
*/
%token KW_ACTION
%token KW_ADD
%token KW_ALPHA
%token KW_ATTENUATION
%token KW_BEZIER
%token KW_CAMERA
%token KW_CENTER
%token KW_CIRCLE
%token KW_CLIP
%token KW_COLLECTION
%token KW_COLOR
%token KW_COLORS
%token KW_CONVEX
%token KW_COPY
%token KW_CURVECENTER
%token KW_CURVECONTROL
%token KW_CURVESCALE
%token KW_DETAIL
%token KW_DITHER
%token KW_DYNAMIC
%token KW_F
%token KW_FILE
%token KW_FIRST
%token KW_FIRST_LEN
%token KW_FLATCOL
%token KW_FOG
%token KW_FOR_2D
%token KW_FOR_3D
%token KW_FORWARD
%token KW_FRAME
%token KW_GOURAUD
%token KW_HALO
%token KW_HEIGHTMAP
%token KW_IDENTITY
%token KW_KEY
%token KW_KEYCOLOR
%token KW_LEN
%token KW_LIBRARY
%token KW_LIGHT
%token KW_LIGHTING
%token KW_LIMB
%token KW_MATERIAL
%token KW_MATRIX
%token KW_MIPMAP
%token KW_MIRROR
%token KW_MIXMODE
%token KW_MOVE
%token KW_MOVEABLE
%token KW_MULTIPLY
%token KW_MULTIPLY2
%token KW_NODE
%token KW_ORIG
%token KW_PLANE
%token KW_POLYGON
%token KW_PORTAL
%token KW_POSITION
%token KW_RADIUS
%token KW_ROT
%token KW_ROT_X
%token KW_ROT_Y
%token KW_ROT_Z
%token KW_SCALE
%token KW_SCALE_X
%token KW_SCALE_Y
%token KW_SCALE_Z
%token KW_SCRIPT
%token KW_SECOND
%token KW_SECOND_LEN
%token KW_SECTOR
%token KW_SKELETON
%token KW_SKYDOME
%token KW_SOUND
%token KW_SOUNDS
%token KW_SPRITE
%token KW_SPRITE2D
%token KW_START
%token KW_STATBSP
%token KW_STATIC
%token KW_TEMPLATE
%token KW_TERRAIN
%token KW_TEX
%token KW_TEXLEN
%token KW_TEXMAP
%token KW_TEXNR
%token KW_TEXTURE
%token KW_TEXTURES
%token KW_TEXTURE_LIGHTING
%token KW_TEXTURE_MIPMAP
%token KW_TEXTURE_SCALE
%token KW_TEX_SET
%token KW_TEX_SET_SELECT
%token KW_THING
%token KW_TRANSFORM
%token KW_TRANSPARENT
%token KW_TRIANGLE
%token KW_UPWARD
%token KW_UV
%token KW_UVA
%token KW_UVEC
%token KW_UV_SHIFT
%token KW_V
%token KW_VERTEX
%token KW_VERTICES
%token KW_VVEC
%token KW_W
%token KW_WORLD
%token KW_ZFILL

/* yes/no tokens */
%token KW_yes
%token KW_no

/* light attenuation tokens */
%token KW_none
%token KW_linear
%token KW_inverse
%token KW_realistic

/* This should be the last keyword.
 * It is used to check the version of tokenized binary files
 */
%token PARSER_VERSION

/* Terminal symbols not recognized as keywords by yylex() */
%token <string>	STRING	/* A string ('string' or without quotes) */
%token <fval>	NUMBER	/* A floating-point number */

/* Non-terminal symbol types */
%type <string>	name
%type <ival>	yesno
%type <ival>	yesno_onearg
%type <color>	color
%type <vect>	vect_idx
%type <vect>	vector
%type <vect2>	vector2
%type <matrix>	matrix
%type <transform> move

%% /* Grammar */

/*-- Input stream ------------------------------------------------------------*/

input:
  KW_WORLD name '('
  { engine->SelectLibrary (storage.cur_library = $2); }
  world_ops ')'
| KW_LIBRARY name '('
  { engine->SelectLibrary (storage.cur_library = $2); }
  world_ops ')'
;

/*-- Grammar -----------------------------------------------------------------*/

world_ops:
  /* empty world */
| world_ops world_op
;

world_op:
  KW_TEXTURES '(' textures ')'
| KW_TEX_SET name '('
  { storage.tex_prefix = $2; }
  textures ')'
  { storage.tex_prefix = NULL; }
| KW_LIBRARY '(' STRING ')'
  { if (!RecursiveLoad ($3)) YYABORT; }
| KW_SOUNDS '(' sounds ')'
  { printf ("SOUNDS\n"); }
| KW_START '(' STRING ',' vector ')'
  {
    if (!engine->CreateCamera ("Start", $3,
      (csVector3 &)$5, csVector3 (0, 0, 1), csVector3 (0, 1, 0)))
      YYABORT;
  }
| KW_CAMERA name '('
  { InitCamera ($2); }
  camera_ops ')'
  { if (!CreateCamera ()) YYABORT; }
| KW_PLANE name '('
  {
    PLANE.mode = pmNONE;
    polygon.first_len = polygon.second_len = 0.0;
  }
  plane_ops ')'
  { if (!CreatePlane ($2)) YYABORT; }
| KW_SECTOR name '('
  {
    SECTOR.object = engine->CreateSector ($2);
    SECTOR.polyset = QUERY_INTERFACE (SECTOR.object, iPolygonSet);
    SECTOR.texname = NULL;
    SECTOR.texlen = 1.0;
    SECTOR.statbsp = false;
  }
  sector_ops ')'
  {
    SECTOR.polyset->CompressVertices ();
    if (SECTOR.statbsp) SECTOR.object->CreateBSP ();
    SECTOR.polyset->DecRef ();
  }
| KW_KEY STRING '(' STRING ')'
  { if (!engine->CreateKey ($2, $4)) ABORTMSG; }
| KW_COLLECTION name '(' collection_ops ')'
  { printf ("COLLECTION [%s]\n", $2); }
| KW_SCRIPT name '(' STRING ':' STRING ')'
  { printf ("SCRIPT '%s' (%s: %s)\n", $2, $4, $6); }
| KW_THING name '(' thing_tpl_ops ')'
  { printf ("THING_tpl [%s]\n", $2); }
| KW_SPRITE name '(' sprite_tpl_ops ')'
  { printf ("SPRITE [%s]\n", $2); }
;

/*--------*/

textures:
  /* empty */
| textures texture
;

texture:
  KW_TEXTURE name '('
  { InitTexture ($2); }
  texture_ops ')'
  { if (!CreateTexture ()) ABORTMSG; }
;

texture_ops:
  /* empty */
| texture_ops texture_op
;

texture_op:
  KW_MIPMAP yesno_onearg
  {
    if ($2)
      TEX.flags = (TEX.flags & ~CS_TEXTURE_NOMIPMAPS);
    else
      TEX.flags |= CS_TEXTURE_NOMIPMAPS;
  }
| KW_DITHER yesno_onearg
  {
    if ($2)
      TEX.flags |= CS_TEXTURE_DITHER;
    else
      TEX.flags = (TEX.flags & ~CS_TEXTURE_DITHER);
  }
| KW_FILE '(' STRING ')'
  { TEX.filename = $3; }
| KW_TRANSPARENT '(' color ')'
  { TEX.transp = $3; TEX.do_transp = true; }
| KW_FOR_3D yesno_onearg
  {
    if ($2)
      TEX.flags |= CS_TEXTURE_3D;
    else
      TEX.flags = (TEX.flags & ~CS_TEXTURE_3D);
  }
| KW_FOR_2D yesno_onearg
  {
    if ($2)
      TEX.flags |= CS_TEXTURE_2D;
    else
      TEX.flags = (TEX.flags & ~CS_TEXTURE_2D);
  }
;

/*--------*/

sounds:
  /* empty */
| sounds sound
;

sound:
  KW_SOUND name '(' sound_ops ')'
;

sound_ops:
  /* empty */
| sound_ops sound_op
;

sound_op:
  KW_FILE '(' STRING ')'		{ printf ("FILE (%s)\n", $3); }
;

/*--------*/

sector_ops:
  sector_op
| sector_ops sector_op
;

sector_op:
  KW_VERTEX '(' vector ')'
  { SECTOR.polyset->CreateVertex (CSVECTOR3 ($3)); }
| KW_POLYGON name '('
  {
    polygon.object = SECTOR.polyset->CreatePolygon ($2);
    polygon.texname = SECTOR.texname;
    polygon.texlen = SECTOR.texlen;
  }
  polygon_ops ')'
| KW_TEXNR '(' STRING ')'
  { SECTOR.texname = $3; }
| KW_TEXLEN '(' NUMBER ')'
  { SECTOR.texlen = $3; }
| KW_STATBSP yesno_onearg
  { SECTOR.statbsp = $2; }
| KW_THING name '('
  {
    thing.object = engine->CreateThing ($2, SECTOR.object);
    thing.polyset = QUERY_INTERFACE (thing.object, iPolygonSet);
    thing.texname = NULL;
    thing.texlen = 1.0;
  }
  thing_ops ')'
  {
    thing.polyset->DecRef ();
  }
| KW_LIGHT name '(' light_ops ')'
  { printf ("LIGHT '%s' (...)\n", $2); }
| KW_SPRITE name '(' sprite_ops ')'
  { printf ("SPRITE '%s' (...)\n", $2); }
| KW_FOG '(' color NUMBER ')'
  { printf ("FOG (%g,%g,%g : %g)\n", $3.red, $3.green, $3.blue, $4); }
| KW_CIRCLE '(' vector ':' vector NUMBER ')'
  /* <coordinate> ':' <radius> <num-verts> */
  { printf ("CIRCLE (...)\n"); }
| KW_SKYDOME '(' skydome_ops ')'
  { printf ("SKYDOME (...)\n"); }
| KW_KEY STRING '(' STRING ')'
  { if (!SECTOR.polyset->CreateKey ($2, $4)) ABORTMSG; }
| KW_NODE '(' node_ops ')'
  { printf ("NODE (...)\n"); }
;

skydome_ops:
  skydome_op
| skydome_ops skydome_op
;

skydome_op:
  KW_RADIUS '(' NUMBER ')'
  { printf ("RADIUS (%g)\n", $3); }
| KW_VERTICES '(' vertex_indices ')'
  { printf ("VERTICES (...)\n"); }
| KW_LIGHTING yesno_onearg
  { printf ("LIGHTING (%d)\n", $2); }
;

vertex_indices:
  /* none */
| vertex_indices NUMBER
  { }
;

node_ops:
  node_op
| node_ops node_op
;

node_op:
  KW_POSITION '(' vector ')'
  { printf ("POSITION (...)\n"); }
| KW_KEY '(' STRING ',' STRING ')'
  { printf ("KEY ('%s', '%s')\n", $3, $5); }
;

/*--------*/

plane_ops:
  plane_op
| plane_ops plane_op
;

plane_op:
  KW_ORIG '(' vect_idx ')'
  {
    PLANE.mode |= pmORIGIN;
    PLANE.origin.Set ($3);
  }
| KW_FIRST '(' vect_idx ')'
  {
    PLANE.mode |= pmFIRSTSECOND;
    PLANE.first.Set ($3);
  }
| KW_SECOND '(' vect_idx ')'
  {
    PLANE.mode |= pmFIRSTSECOND;
    PLANE.second.Set ($3);
  }
| KW_FIRST_LEN '(' NUMBER ')'
  {
    PLANE.mode |= pmFIRSTSECOND;
    PLANE.first_len = $3;
  }
| KW_SECOND_LEN '(' NUMBER ')'
  {
    PLANE.mode |= pmFIRSTSECOND;
    PLANE.second_len = $3;
  }
| KW_UVEC '(' vector ')'
  {
    PLANE.mode |= pmVECTORS;
    PLANE.first = $3;
    PLANE.first_len = 1.0;
  }
| KW_VVEC '(' vector ')'
  {
    PLANE.mode |= pmVECTORS;
    PLANE.second = $3;
    PLANE.second_len = 1.0;
  }
| KW_MATRIX '(' matrix ')'
  {
    PLANE.mode |= pmMATRIX;
    PLANE.matrix.Set (*$3);
  }
| KW_V '(' vector ')'
  {
    PLANE.mode |= pmMATRIX;
    PLANE.origin.Set ($3);
  }
;

/*--------*/

light_ops:
  light_op
| light_ops light_op
;

light_op:
  vector ':' NUMBER color NUMBER
  /* <pos> <radius> <color> <dynamic-flag> */
  { printf ("<pos> <radius> <color> <dynamic-flag>\n"); }
| KW_CENTER '(' vector ')'
  { printf ("CENTER (...)\n"); }
| KW_RADIUS '(' NUMBER ')'
  { printf ("RADIUS (%g)\n", $3); }
| KW_DYNAMIC yesno_onearg
  { printf ("DYNAMIC (%d)\n", $2); }
| KW_COLOR '(' color ')'
  { printf ("COLOR ( ... )\n"); }
| KW_HALO '(' NUMBER NUMBER ')'
  { printf ("HALO (%g,%g)\n", $3, $4); }
| KW_ATTENUATION '(' attenuation_op ')'
;

attenuation_op:
  KW_none
| KW_linear
| KW_inverse
| KW_realistic
;

/*--------*/

collection_ops:
  collection_op
| collection_ops collection_op
;

collection_op:
  KW_THING '(' STRING ')'
  { printf ("THING ('%s')\n", $3); }
| KW_COLLECTION '(' STRING ')'
  { printf ("COLLECTION ('%s')\n", $3); }
| KW_LIGHT '(' STRING ',' NUMBER ')'
  { printf ("LIGHT ('%s':%g)\n", $3, $5); }
| KW_SECTOR '(' STRING ')'
  { printf ("SECTOR ('%s')\n", $3); }
;

/*--------*/

thing_tpl_ops:
  thing_tpl_op
| thing_tpl_ops thing_tpl_op
;

thing_tpl_op:
  KW_POLYGON name '('
  {
    polygon.object = thing.polyset->CreatePolygon ($2);
    polygon.texname = thing.texname;
    polygon.texlen = thing.texlen;
  }
  polygon_ops ')'
| KW_VERTEX '(' vector ')'
  { thing.polyset->CreateVertex (CSVECTOR3 ($3)); }
| KW_TEXNR '(' STRING ')'
  { thing.texname = $3; }
| KW_TEXLEN '(' NUMBER ')'
  { thing.texlen = $3; }
| KW_MOVE '(' move ')'
  {
    if ($3->matrix_valid)
      thing.object->SetTransform (CSMATRIX3 ($3->matrix));
    if ($3->vector_valid)
      thing.object->SetPosition (CSVECTOR3 ($3->vector));
  }
| KW_FOG '(' color NUMBER ')'
  { printf ("FOG (%g,%g,%g : %g)\n", $3.red, $3.green, $3.blue, $4); }
| KW_CONVEX yesno_onearg
  { printf ("CONVEX (%d)\n", $2); }
| KW_CIRCLE '(' vector ':' vector NUMBER ')'
  /* <coordinate> ':' <radius> <num-verts> */
  { printf ("CIRCLE (...)\n"); }
| KW_BEZIER name '(' bezier_ops ')'
  { printf ("BEZIER '%s' (...)\n", $2); }
| KW_CURVECENTER '(' vector ')'
  { printf ("CURVECENTER (...)\n"); }
| KW_CURVESCALE '(' NUMBER ')'
  { printf ("CURVESCALE (%g)\n", $3); }
| KW_CURVECONTROL '(' vector ':' vector2 ')'
  { printf ("CURVECONTROL (...)\n"); }
;

thing_ops:
  thing_op
| thing_ops thing_op
;

thing_op:
  thing_tpl_op
| KW_KEY '(' STRING ',' STRING ')'
  { printf ("KEY ('%s', '%s')\n", $3, $5); }
| KW_TEMPLATE '(' STRING ')'
  { printf ("TEMPLATE ('%s')\n", $3); }
| KW_MOVEABLE yesno_onearg
  { printf ("MOVEABLE (%d)\n", $2); }
| KW_TEX_SET_SELECT '(' STRING ')'
  { printf ("TEX_SET_SELECT ('%s')\n", $3); }
| KW_FILE '(' STRING ')'
  { printf ("FILE ('%s')\n", $3); }
;

/*--------*/

bezier_ops:
  bezier_op
| bezier_ops bezier_op
;

bezier_op:
  KW_TEXNR '(' STRING ')'
  { printf ("TEXNR (%s)\n", $3); }
| KW_TEXTURE '(' bezier_texture_ops ')'
  { printf ("TEXTURE (...)\n"); }
| KW_VERTICES '(' vertex_indices ')'
  { printf ("VERTICES (...)\n"); }
;

bezier_texture_ops:
  bezier_texture_op
| bezier_texture_ops bezier_texture_op
;

bezier_texture_op:
  /* unused op for now */
  "unused"
;

/*--------*/

sprite_tpl_ops:
  sprite_tpl_op
| sprite_tpl_ops sprite_tpl_op
;

sprite_tpl_op:
  KW_TEXNR '(' STRING ')'
  { printf ("TEXNR ('%s')\n", $3); }
| KW_FRAME name '(' sprite_verts ')'
  { printf ("FRAME '%s' (...)\n", $2); }
| KW_ACTION name '(' sprite_actions ')'
  { printf ("ACTION '%s' ( ... )\n", $2); }
| KW_TRIANGLE '(' NUMBER NUMBER NUMBER ')'
  { printf ("TRIANGLE (%g,%g,%g)\n", $3, $4, $5); }
| KW_FILE '(' STRING ')'
  { printf ("FILE ('%s')\n", $3); }
/*
@@todo:
| <sprite-skeleton>
*/
;

sprite_verts:
  /* empty */
| sprite_verts sprite_vert
;

sprite_vert:
  KW_V '(' vector ':' vector2 ')'
  { printf ("V (%g,%g,%g:%g,%g)\n", $3.x, $3.y, $3.z, $5.x, $5.y); }
;

sprite_actions:
  /* empty */
| sprite_actions sprite_action
;

sprite_action:
  KW_F '(' STRING ',' NUMBER ')'
  { printf ("F ('%s', %g)\n", $3, $5); }
;

sprite_ops:
  sprite_op
| sprite_ops sprite_op
;

sprite_op:
  KW_MOVE '(' move ')'
  { printf ("MOVE ()\n"); }
| KW_TEMPLATE '(' STRING ',' STRING ')'
  { printf ("TEMPLATE ('%s', '%s')\n", $3, $5); }
| KW_TEXNR '(' STRING ')'
  { printf ("TEXNR ('%s')\n", $3); }
| KW_MIXMODE '(' mixmode_ops ')'
  { printf ("MIXMODE (...)\n"); }
;

mixmode_ops:
  mixmode_op
| mixmode_ops mixmode_op
;

mixmode_op:
  KW_COPY noargs
| KW_MULTIPLY noargs
| KW_MULTIPLY2 noargs
| KW_ADD noargs
| KW_ALPHA '(' NUMBER ')'
  { printf ("ALPHA (%g)\n", $3); }
| KW_TRANSPARENT noargs
| KW_KEYCOLOR noargs
;

/*--------*/

camera_ops:
  /* empty */
| camera_ops camera_op
;

camera_op:
  KW_POSITION '(' vector ')'
  { CAMERA.pos.Set ($3); }
| KW_FORWARD '(' vector ')'
  { CAMERA.forward.Set ($3); }
| KW_UPWARD '(' vector ')'
  { CAMERA.upward.Set ($3); }
| KW_SECTOR '(' STRING ')'
  { CAMERA.sector = $3;  }
;

/*-- General-use non-terminals -----------------------------------------------*/

/*
    GENERAL NOTE: To minimize space occupied by pre-tokenized data,
    the ',' characters between numbers are removed. Thus the lists of
    numbers will look like "NUMBER NUMBER NUMBER" instead of (how it
    should be in the most obvious case) "NUMBER ',' NUMBER ',' NUMBER".
*/

/* A name - either empty or defined */
name:
  /* empty == no name */
  { $$ = NULL; }
| STRING
;

/* Yes or no - the result is a boolean */
yesno:
  KW_yes
  { $$ = true; }
| KW_no
  { $$ = false; }
;

/* Yes or no, one argument, possibly no arguments at all */
yesno_onearg:
  /* empty - true by default */
  { $$ = true; }
| '(' ')'
  { $$ = true; }
| '(' yesno ')'
  { $$ = $2; }
;

/* The definition of a color - red, green, blue from 0 to 1 */
color:
  NUMBER NUMBER NUMBER
  { CSCOLOR ($$).Set ($1, $2, $3); }
;

/* Simply a vector */
vector:
  NUMBER NUMBER NUMBER
  { $$.Set ($1, $2, $3); }
;

/* A vector defined either directly or using a vertex index */
vect_idx:
  NUMBER
  /*@@todo*/
  { $$.x = $$.y = $$.z = 0; }
| vector
;

/* A 2D vector */
vector2:
  NUMBER NUMBER
  { $$.Set ($1, $2); }
;

/* A matrix: there are lots of ways to define a matrix */
matrix:
  NUMBER NUMBER NUMBER
  NUMBER NUMBER NUMBER
  NUMBER NUMBER NUMBER			/* Matrix defined by nine numbers */
  {
    $$ = &storage.matrix2;
    $$->Set ($1, $2, $3, $4, $5, $6, $7, $8, $9);
  }
| NUMBER				/* Uniform matrix scaler */
  {
    $$ = &storage.matrix2;
    $$->Set ($1, 0, 0, 0, $1, 0, 0, 0, $1);
  }
| /* Initialize matrix to identity before starting any complex defs */
  {
    $$ = &storage.matrix2;
    $$->Identity ();
  }
  matrix_ops
;

matrix_ops:
  matrix_op
| matrix_ops matrix_op
;

matrix_op:
  KW_IDENTITY noargs			/* Load identity matrix */
  { storage.matrix2.Identity (); }
| KW_ROT_X '(' NUMBER ')'		/* Rotate around OX */
  { storage.matrix2 *= csXRotMatrix3 ($3); }
| KW_ROT_Y '(' NUMBER ')'		/* Rotate around OY */
  { storage.matrix2 *= csYRotMatrix3 ($3); }
| KW_ROT_Z '(' NUMBER ')'		/* Rotate around OZ */
  { storage.matrix2 *= csZRotMatrix3 ($3); }
| KW_SCALE '(' NUMBER ')'		/* Uniform matrix scaler */
  { storage.matrix2 *= $3; }
| KW_SCALE '(' NUMBER NUMBER NUMBER ')'	/* Scalers for X/Y/Z individually */
  { storage.matrix2 *= csMatrix3 ($3, 0, 0, 0, $4, 0, 0, 0, $5); }
| KW_SCALE_X '(' NUMBER ')'		/* Scale related to YOZ */
  { storage.matrix2 *= csXScaleMatrix3 ($3); }
| KW_SCALE_Y '(' NUMBER ')'		/* Scale related to XOZ */
  { storage.matrix2 *= csYScaleMatrix3 ($3); }
| KW_SCALE_Z '(' NUMBER ')'		/* Scale related to XOY */
  { storage.matrix2 *= csYScaleMatrix3 ($3); }
;

noargs:
  /* No arguments */
| '(' ')'
;

/* MOVE ( ... ) operator (used in things/sprites/etc) */
move:
  {
    $$ = &storage;
    $$->matrix.Identity ();
    $$->matrix_valid = false;
    $$->vector_valid = false;
  }
  move_ops
;

move_ops:
  move_op
| move_ops move_op
;

move_op:
  KW_MATRIX '(' matrix ')'
  {
    storage.matrix = *$3;
    storage.matrix_valid = true;
  }
| KW_V '(' vector ')'
  {
    storage.vector = CSVECTOR3 ($3);
    storage.vector_valid = true;
  }
;

/* POLYGON 'name' ( ... ) (used in things and sectors) */
polygon_ops:
  polygon_op
| polygon_ops polygon_op
;

polygon_op:
  KW_TEXNR '(' STRING ')'
  { polygon.texname = $3; }
| KW_LIGHTING yesno_onearg
  { polygon.object->SetFlags (CS_POLY_LIGHTING, $2 ? CS_POLY_LIGHTING : 0); }
| KW_TEXMAP '('
  {
    polygon.mode = pmNONE;
    polygon.first_len = polygon.second_len = polygon.texlen;
  }
  polygon_texture_ops ')'
  {
    if (!CreateTexturePlane (polygon.object))
      YYABORT;
  }
| KW_VERTICES '(' polygon_vertex_indices ')'
| KW_GOURAUD yesno_onearg
  { polygon.object->SetLightingMode ($2); }
| KW_FLATCOL '(' color ')'
  { polygon.object->SetFlatColor (CSCOLOR ($3)); }
| KW_ALPHA '(' NUMBER ')'
  { polygon.object->SetAlpha ($3); }
| KW_UV '(' tex_coordinates ')'
  { printf ("UV (...)\n"); }
| KW_UVA '(' uva_coordinates ')'
  { printf ("UVA (...)\n"); }
| KW_COLORS '(' colors ')'
  { printf ("COLORS (...)\n"); }
| KW_PORTAL '('
  { portals->Push (polygon.portal = new csPPortal (polygon.object)); }
  portal_ops ')'
  {
    if (!polygon.portal->Check ())
    {
      yyerror ("invalid portal definition");
      YYABORT;
    }
  }
;

colors:
  /* none */
| colors color
  { }
;

polygon_vertex_indices:
  /* none */
| polygon_vertex_indices NUMBER
  { polygon.object->CreateVertex ($2); }
;

tex_coordinates:
  /* none */
| tex_coordinates NUMBER NUMBER
  /* <u> <v> */
;

uva_coordinates:
  /* none */
| uva_coordinates NUMBER NUMBER NUMBER
  /* <angle> <uva-scale> <uva-offset> */
;

/* PORTAL ( ... ) */
portal_ops:
  portal_op
| portal_ops portal_op
;

portal_op:
  KW_SECTOR '(' STRING ')'
  { polygon.portal->destsec = $3; }
| KW_MATRIX '(' matrix ')'
  { polygon.portal->mode = csPPortal::pmWarp; }
| KW_V '(' vector ')'
  { polygon.portal->mode = csPPortal::pmWarp; }
| KW_W '(' vector ')'
  { polygon.portal->mode = csPPortal::pmWarp; }
| KW_MIRROR yesno_onearg
  { polygon.portal->mode = csPPortal::pmMirror; }
| KW_STATIC yesno_onearg
  { polygon.portal->SetFlags (CS_PORTAL_STATICDEST, $2); }
| KW_CLIP yesno_onearg
  { polygon.portal->SetFlags (CS_PORTAL_CLIPDEST, $2); }
| KW_ZFILL yesno_onearg
  { polygon.portal->SetFlags (CS_PORTAL_ZFILL, $2); }
;

/* TEXTURE (...) */
polygon_texture_ops:
  polygon_texture_op
| polygon_texture_ops polygon_texture_op
;

polygon_texture_op:
  KW_ORIG '(' vect_idx ')'
  {
    polygon.mode |= pmORIGIN;
    polygon.origin.Set ($3);
  }
| KW_FIRST '(' vect_idx ')'
  {
    polygon.mode |= pmFIRSTSECOND;
    polygon.first.Set ($3);
  }
| KW_SECOND '(' vect_idx ')'
  {
    polygon.mode |= pmFIRSTSECOND;
    polygon.second.Set ($3);
  }
| KW_FIRST_LEN '(' NUMBER ')'
  {
    polygon.mode |= pmFIRSTSECOND;
    polygon.first_len = $3;
  }
| KW_SECOND_LEN '(' NUMBER ')'
  {
    polygon.mode |= pmFIRSTSECOND;
    polygon.second_len = $3;
  }
| KW_UVEC '(' vector ')'
  {
    polygon.mode |= pmVECTORS;
    polygon.first = $3;
    polygon.first_len = 1.0;
  }
| KW_VVEC '(' vector ')'
  {
    polygon.mode |= pmVECTORS;
    polygon.second = $3;
    polygon.second_len = 1.0;
  }
| KW_MATRIX '(' matrix ')'
  {
    polygon.mode |= pmMATRIX;
    polygon.matrix.Set (*$3);
  }
| KW_V '(' vector ')'
  {
    polygon.mode |= pmMATRIX;
    polygon.origin.Set ($3);
  }
| KW_PLANE '(' STRING ')'
  {
    polygon.mode |= pmPLANEREF;
    polygon.planetpl = $3;
  }
| KW_UV_SHIFT '(' vector2 ')'
  { printf ("UV_SHIFT (%g, %g)\n", $3.x, $3.y); }
;

%% /* End of grammar */

/* On initialization, register keyword list with the C++ parser */
extern int init_token_table (const char * const *yytname);
struct __parser_init
{
  __parser_init ()
  {
    init_token_table (yytname);
#if YYDEBUG
    yydebug = 1;
#endif
  }
} __parser_init_dummy;
