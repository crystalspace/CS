/*
    The Crystal Space world file parser definition
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

#include "sysdef.h"
#include "stdldr.h"
#include "csutil/cscolor.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

#include "iworld.h"
#include "itxtmgr.h"

// Unfortunately we can't make yyparse a member function because of the
// dumb prototype definition in bison.simple :-(
//#define yyparse csStandardLoader::yyparse

#define YYPARSE_PARAM ldr
#define THIS	((csStandardLoader *)ldr)
#define yyerror THIS->yyerror
#define yylex   THIS->yylex

/* Define this to debug parser */
//#define YYDEBUG	1

// Macros for accessing yylval as different data types
#define CSCOLOR(x)	(*(csColor *)&x)
#define CSVECTOR2(x)	(*(csVector2 *)&x)
#define CSVECTOR3(x)	(*(csVector3 *)&x)

// More shortcuts
#define STORAGE		THIS->storage
#define TEX		THIS->storage.tex

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
  struct { float x, y; } vect2;
  // A 3D point
  struct { float x, y, z; } vect;
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
%token KW_ACTIVATE
%token KW_ACTIVE
%token KW_ADD
%token KW_ALPHA
%token KW_ATTENUATION
%token KW_BECOMING_ACTIVE
%token KW_BECOMING_INACTIVE
%token KW_BEZIER
%token KW_CEILING
%token KW_CEIL_TEXTURE
%token KW_CENTER
%token KW_CIRCLE
%token KW_CLIP
%token KW_COLLECTION
%token KW_COLOR
%token KW_COLORS
%token KW_CONVEX
%token KW_COPY
%token KW_COSFACT
%token KW_CURVECENTER
%token KW_CURVECONTROL
%token KW_CURVESCALE
%token KW_DETAIL
%token KW_DIM
%token KW_DITHER
%token KW_DYNAMIC
%token KW_F
%token KW_FILE
%token KW_FILTER
%token KW_FIRST
%token KW_FIRST_LEN
%token KW_FLATCOL
%token KW_FLOOR
%token KW_FLOOR_CEIL
%token KW_FLOOR_HEIGHT
%token KW_FLOOR_TEXTURE
%token KW_FOG
%token KW_FOR_2D
%token KW_FOR_3D
%token KW_FRAME
%token KW_GOURAUD
%token KW_HALO
%token KW_HEIGHT
%token KW_HEIGHTMAP
%token KW_IDENTITY
%token KW_KEY
%token KW_KEYCOLOR
%token KW_LEN
%token KW_LIBRARY
%token KW_LIGHT
%token KW_LIGHTING
%token KW_LIGHTX
%token KW_LIMB
%token KW_MATRIX
%token KW_MERGE_NORMALS
%token KW_MERGE_TEXELS
%token KW_MERGE_VERTICES
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
%token KW_PRIMARY_ACTIVE
%token KW_PRIMARY_INACTIVE
%token KW_RADIUS
%token KW_ROOM
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
%token KW_SECONDARY_ACTIVE
%token KW_SECONDARY_INACTIVE
%token KW_SECOND_LEN
%token KW_SECTOR
%token KW_SIXFACE
%token KW_SKELETON
%token KW_SKYDOME
%token KW_SOUND
%token KW_SOUNDS
%token KW_SPLIT
%token KW_SPRITE
%token KW_SPRITE2D
%token KW_START
%token KW_STATBSP
%token KW_STATELESS
%token KW_STATIC
%token KW_TEMPLATE
%token KW_TERRAIN
%token KW_TEX
%token KW_TEXLEN
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
%token KW_TRIGGER
%token KW_UV
%token KW_UVA
%token KW_UVEC
%token KW_UV_SHIFT
%token KW_V
%token KW_VERTEX
%token KW_VERTICES
%token KW_VVEC
%token KW_W
%token KW_WARP
%token KW_WORLD

/* yes/no tokens */
%token KW_yes
%token KW_no

/* light attenuation tokens */
%token KW_none
%token KW_linear
%token KW_inverse
%token KW_realistic

/* Terminal symbols not recognized as keywords by yylex() */
%token <string>	STRING	/* A string ('string' or without quotes) */
%token <fval>	NUMBER	/* A floating-point number */

/* Non-terminal symbol types */
%type <string>	name
%type <ival>	yesno
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
  { THIS->world->SelectLibrary (STORAGE.cur_library = $2); }
  world_ops ')'
| KW_LIBRARY name '('
  { THIS->world->SelectLibrary (STORAGE.cur_library = $2); }
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
  { STORAGE.tex_prefix = $2; }
  textures ')'
  { STORAGE.tex_prefix = NULL; }
| KW_LIBRARY '(' STRING ')'
  { if (!THIS->RecursiveLoad ($3)) YYABORT; }
| KW_SOUNDS '(' sounds ')'
  { printf ("SOUNDS\n"); }
| KW_START '(' start ')'
  { printf ("START\n"); }
| KW_SECTOR name '(' sector_ops ')'
  { printf ("SECTOR [%s]\n", $2); }
| KW_PLANE name '(' plane_ops ')'
  { printf ("PLANE [%s]\n", $2); }
| KW_KEY name '(' key ')'
  { printf ("KEY [%s]\n", $2); }
| KW_COLLECTION name '(' collection_ops ')'
  { printf ("COLLECTION [%s]\n", $2); }
| KW_SCRIPT name '(' STRING ':' STRING ')'
  { printf ("SCRIPT '%s' (%s: %s)\n", $2, $4, $6); }
| KW_LIGHTX name '(' lightx ')'
  { printf ("LIGHTX [%s]\n", $2); }
| KW_THING name '(' thing_tpl_ops ')'
  { printf ("THING_tpl [%s]\n", $2); }
| KW_SPRITE name '(' sprite_tpl_ops ')'
  { printf ("SPRITE [%s]\n", $2); }
| KW_ROOM name '(' room_ops ')'
  { printf ("ROOM [%s]\n", $2); }
| KW_SIXFACE name '(' sixface_tpl_ops ')'
  { printf ("SIXFACE [%s]\n", $2); }
;

/*--------*/

textures:
  /* empty */
| textures texture
;

texture:
  KW_TEXTURE name
  { THIS->InitTexture ($2); }
  '(' texture_ops ')'
  { if (!THIS->CreateTexture ()) YYABORT; }
;

texture_ops:
  /* empty */
| texture_ops texture_op
;

texture_op:
  KW_MIPMAP '(' yesno ')'		{ printf ("MIPMAP (%d)\n", $3); }
  {
    if ($3)
      TEX.flags = (TEX.flags & ~CS_TEXTURE_NOMIPMAPS);
    else
      TEX.flags |= CS_TEXTURE_NOMIPMAPS;
  }
| KW_DITHER '(' yesno ')'
  {
    if ($3)
      TEX.flags |= CS_TEXTURE_DITHER;
    else
      TEX.flags = (TEX.flags & ~CS_TEXTURE_DITHER);
  }
| KW_FILE '(' STRING ')'
  { TEX.filename = $3; }
| KW_TRANSPARENT '(' color ')'
  { TEX.transp = $3; TEX.do_transp = true; }
| KW_FOR_3D '(' yesno ')'
  {
    if ($3)
      TEX.flags |= CS_TEXTURE_3D;
    else
      TEX.flags = (TEX.flags & ~CS_TEXTURE_3D);
  }
| KW_FOR_2D '(' yesno ')'
  {
    if ($3)
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
  { printf ("VERTEX (%g,%g,%g)\n", $3.x, $3.y, $3.z); }
| KW_POLYGON '(' polygon_ops ')'
  { printf ("POLYGON (...)\n"); }
| KW_TEXNR '(' STRING ')'
  { printf ("TEXNR ('%s')\n", $3); }
| KW_TEXLEN '(' NUMBER ')'
  { printf ("TEXLEN (%g)\n", $3); }
| KW_LIGHTX '(' STRING ')'
  { printf ("LIGHTX ('%s')\n", $3); }
| KW_ACTIVATE '(' STRING ')'
  { printf ("ACTIVATE (%s)\n", $3); }
| KW_TRIGGER '(' STRING ',' STRING ')'
  { printf ("TRIGGER (%s, %s)\n", $3, $5); }
| KW_STATBSP noargs
  { printf ("STATBSP ()\n"); }
| KW_THING name '(' thing_ops ')'
  { printf ("THING '%s' (...)\n", $2); }
| KW_SIXFACE name '(' sixface_ops ')'
  { printf ("SIXFACE '%s' (...)\n", $2); }
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
| KW_KEY '(' STRING ',' STRING ')'
  { printf ("KEY ('%s', '%s')\n", $3, $5); }
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
| KW_LIGHTING '(' yesno ')'
  { printf ("LIGHTING (%d)\n", $3); }
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
  KW_ORIG '(' vect_idx ')'		{ printf ("ORIG (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_FIRST '(' vect_idx ')'		{ printf ("FIRST (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_SECOND '(' vect_idx ')'		{ printf ("SECOND (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_FIRST_LEN '(' NUMBER ')'		{ printf ("FIRST_LEN (%g)\n", $3); }
| KW_SECOND_LEN '(' NUMBER ')'		{ printf ("SECOND_LEN (%g)\n", $3); }
| KW_UVEC '(' vector ')'		{ printf ("UVEC (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_VVEC '(' vector ')'		{ printf ("VVEC (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_MATRIX '(' matrix ')'
  {
    printf ("MATRIX (\n  %g, %g, %g\n  %g, %g, %g\n  %g, %g, %g\n)\n",
    $3->m11, $3->m12, $3->m13, $3->m21, $3->m22, $3->m21, $3->m31, $3->m32, $3->m33);
  }
| KW_V '(' vector ')'			{ printf ("V (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
;

/*--------*/

start:
;

/*--------*/

key:
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
| KW_TRIGGER '(' STRING ',' STRING '-' '>' STRING ')'
  { printf ("TRIGGER ('%s', '%s' -> '%s')\n", $3, $5, $8); }
| KW_SECTOR '(' STRING ')'
  { printf ("SECTOR ('%s')\n", $3); }
;

/*--------*/

lightx:
  /* empty */
| lightx lightx_desc
;

lightx_desc:
  KW_ACTIVE '(' NUMBER ')'		{ printf ("ACTIVE (%g)\n", $3); }
| KW_STATELESS '(' NUMBER ')'		{ printf ("STATELESS (%g)\n", $3); }
| KW_PRIMARY_ACTIVE '(' NUMBER NUMBER NUMBER NUMBER NUMBER ')'
  { printf ("PRIMARY_ACTIVE (%g,%g,%g,%g,%g)\n", $3, $4, $5, $6, $7); }
| KW_SECONDARY_ACTIVE '(' NUMBER NUMBER NUMBER NUMBER NUMBER ')'
  { printf ("SECONDARY_ACTIVE (%g,%g,%g,%g,%g)\n", $3, $4, $5, $6, $7); }
| KW_BECOMING_ACTIVE '(' NUMBER NUMBER NUMBER NUMBER NUMBER ')'
  { printf ("BECOMING_ACTIVE (%g,%g,%g,%g,%g)\n", $3, $4, $5, $6, $7); }
| KW_PRIMARY_INACTIVE '(' NUMBER NUMBER NUMBER NUMBER NUMBER ')'
  { printf ("PRIMARY_INACTIVE (%g,%g,%g,%g,%g)\n", $3, $4, $5, $6, $7); }
| KW_SECONDARY_INACTIVE '(' NUMBER NUMBER NUMBER NUMBER NUMBER ')'
  { printf ("SECONDARY_INACTIVE (%g,%g,%g,%g,%g)\n", $3, $4, $5, $6, $7); }
| KW_BECOMING_INACTIVE '(' NUMBER NUMBER NUMBER NUMBER NUMBER ')'
  { printf ("BECOMING_INACTIVE (%g,%g,%g,%g,%g)\n", $3, $4, $5, $6, $7); }
;

/*--------*/

thing_tpl_ops:
  thing_tpl_op
| thing_tpl_ops thing_tpl_op
;

thing_tpl_op:
  KW_POLYGON name '(' polygon_ops ')'
  { printf ("POLYGON_tpl '%s' ()\n", $2); }
| KW_VERTEX '(' vector ')'
  { printf ("VERTEX (%g,%g,%g)\n", $3.x, $3.y, $3.z); }
| KW_TEXNR '(' STRING ')'
  { printf ("TEXNR (%s)\n", $3); }
| KW_TEXLEN '(' NUMBER ')'
  { printf ("TEXLEN (%g)\n", $3); }
| KW_LIGHTX '(' STRING ')'
  { printf ("LIGHTX ('%s')\n", $3); }
| KW_MOVE '(' move ')'
  { printf ("MOVE ()\n"); }
| KW_FOG '(' color NUMBER ')'
  { printf ("FOG (%g,%g,%g : %g)\n", $3.red, $3.green, $3.blue, $4); }
| KW_CONVEX noargs
  { printf ("CONVEX ()\n"); }
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
| KW_ACTIVATE '(' STRING ')'
  { printf ("ACTIVATE (%s)\n", $3); }
| KW_TRIGGER '(' STRING ',' STRING ')'
  { printf ("TRIGGER (%s, %s)\n", $3, $5); }
| KW_TEMPLATE '(' STRING ')'
  { printf ("TEMPLATE ('%s')\n", $3); }
| KW_MOVEABLE noargs
  { printf ("MOVEABLE ()\n"); }
| KW_TEX_SET_SELECT '(' STRING ')'
  { printf ("TEX_SET_SELECT ('%s')\n", $3); }
| KW_FILE '(' STRING ')'
  { printf ("FILE ('%s')\n", $3); }
;

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
| KW_MERGE_TEXELS '(' yesno ')'
  { printf ("MERGE_TEXELS (%d)\n", $3); }
/*
@@todo: can't really understand how it all works
| KW_MERGE_NORMALS '(' <sprite-merge> ')'
| KW_MERGE_VERTICES '(' <sprite-merge> ')'
*/
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

/*--------*/

room_ops:
  room_op
| room_ops room_op
;

room_op:
  KW_TEXTURE_LIGHTING '(' yesno ')'
  { printf ("TEXTURE_LIGHTING (%d)\n", $3); }
| KW_TEXTURE_SCALE '(' NUMBER ')'
  { printf ("TEXTURE_SCALE (%g)\n", $3); }
| KW_TEXTURE '(' STRING ')'
  { printf ("TEXTURE ('%s')\n", $3); }
| KW_TEX name '(' room_tex_ops ')'
  { printf ("TEX '%s' (...)\n", $2); }
| KW_CEIL_TEXTURE '(' STRING ')'
  { printf ("CEIL_TEXTURE (%s)\n", $3); }
| KW_FLOOR_TEXTURE '(' STRING ')'
  { printf ("FLOOR_TEXTURE (%s)\n", $3); }
| KW_LIGHTX '(' STRING ',' STRING ')'
  { printf ("LIGHTX ('%s', '%s')\n", $3, $5); }
| KW_LIGHT '(' light_ops ')'
  { printf ("LIGHT (...)\n"); }
| KW_DIM '(' NUMBER NUMBER NUMBER ')'
  { printf ("DIM (%g, %g, %g)\n", $3, $4, $5); }
| KW_HEIGHT '(' NUMBER ')'
  { printf ("HEIGHT (%g)\n", $3); }
| KW_FLOOR_HEIGHT '(' NUMBER ')'
  { printf ("FLOOR_HEIGHT (%g)\n", $3); }
| KW_FLOOR_CEIL '(' '(' vector2 ')' '(' vector2 ')' '(' vector2 ')' '(' vector2 ')' ')'
  {
    printf ("FLOOR_CEILING ((%g,%g) (%g,%g) (%g,%g) (%g,%g))\n",
      $4.x, $4.y, $7.x, $7.y, $10.x, $10.y, $13.x, $13.y);
  }
| KW_FLOOR '(' '(' vector ')' '(' vector ')' '(' vector ')' '(' vector ')' ')'
  {
    printf ("FLOOR ( (%g,%g,%g) (%g,%g,%g) (%g,%g,%g) (%g,%g,%g) )\n",
      $4.x, $4.y, $4.z, $7.x, $7.y, $7.z,
      $10.x, $10.y, $10.z, $13.x, $13.y, $13.z);
  }
| KW_CEILING '(' '(' vector ')' '(' vector ')' '(' vector ')' '(' vector ')' ')'
  { printf ("CEILING ()\n"); }
| KW_TRIGGER '(' STRING ',' STRING ')'
  { printf ("TRIGGER (%s, %s)\n", $3, $5); }
| KW_ACTIVATE '(' STRING ')'
  { printf ("ACTIVATE (%s)\n", $3); }
| KW_STATBSP noargs
  { printf ("STATBSP ()\n"); }
| KW_MOVE '(' move ')'
  { printf ("MOVE ()\n"); }
| KW_SIXFACE name '(' sixface_ops ')'
  { printf ("SIXFACE [%s]\n", $2); }
| KW_THING name '(' thing_ops ')'
  { printf ("THING '%s' (...)\n", $2); }
| KW_PORTAL '(' room_portal_ops ')'
  { printf ("PORTAL (...)\n"); }
| KW_SPLIT '(' STRING ',' STRING '(' split_list ')' ')'
  { printf ("SPLIT (%s, %s, ...)\n", $3, $5); }
| KW_SPRITE name '(' sprite_ops ')'
  { printf ("SPRITE '%s' (...)\n", $2); }
| KW_FOG '(' color NUMBER ')'
  { printf ("FOG (%g,%g,%g : %g)\n", $3.red, $3.green, $3.blue, $4); }
;

room_tex_ops:
  room_tex_op
| room_tex_ops room_tex_op
;

room_tex_op:
  KW_TEXTURE '(' STRING ')'
  { printf ("TEXTURE ('%s')\n", $3); }
| KW_PLANE '(' STRING ')'
  { printf ("PLANE ('%s')\n", $3); }
| KW_LEN '(' NUMBER ')'
  { printf ("LEN ('%g')\n", $3); }
;

room_portal_ops:
  room_portal_op
| room_portal_ops room_portal_op
;

room_portal_op:
  KW_POLYGON '(' STRING ')'
  { printf ("POLYGON ('%s')\n", $3); }
| KW_SECTOR '(' STRING ')'
  { printf ("SECTOR ('%s')\n", $3); }
| KW_ALPHA '(' NUMBER ')'
  { printf ("ALPHA (%g)\n", $3); }
| KW_WARP '(' warp_ops ')'
  { printf ("WARP (...)\n"); }
;

split_list:
  /* empty */
| split_list NUMBER
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

sixface_tpl_ops:
  sixface_tpl_op
| sixface_tpl_ops sixface_tpl_op
;

sixface_tpl_op:
  KW_MOVE '(' move ')'
  { printf ("MOVE ()\n"); }
| KW_TEXTURE_SCALE '(' NUMBER ')'
  { printf ("TEXTURE_SCALE (%g)\n", $3); }
| KW_TEXTURE '(' STRING ')'
  { printf ("TEXTURE (%s)\n", $3); }
| KW_CEIL_TEXTURE '(' STRING ')'
  { printf ("CEIL_TEXTURE (%s)\n", $3); }
| KW_DIM '(' NUMBER NUMBER NUMBER ')'
  { printf ("DIM (%g, %g, %g)\n", $3, $4, $5); }
| KW_HEIGHT '(' NUMBER ')'
  { printf ("HEIGHT (%g)\n", $3); }
| KW_FLOOR_HEIGHT '(' NUMBER ')'
  { printf ("FLOOR_HEIGHT (%g)\n", $3); }
| KW_FLOOR_CEIL '(' '(' vector2 ')' '(' vector2 ')' '(' vector2 ')' '(' vector2 ')' ')'
  {
    printf ("FLOOR_CEILING ((%g,%g) (%g,%g) (%g,%g) (%g,%g))\n",
      $4.x, $4.y, $7.x, $7.y, $10.x, $10.y, $13.x, $13.y);
  }
| KW_FLOOR_TEXTURE '(' STRING ')'
  { printf ("FLOOR_TEXTURE (%s)\n", $3); }
| KW_FLOOR '(' '(' vector ')' '(' vector ')' '(' vector ')' '(' vector ')' ')'
  {
    printf ("FLOOR ( (%g,%g,%g) (%g,%g,%g) (%g,%g,%g) (%g,%g,%g) )\n",
      $4.x, $4.y, $4.z, $7.x, $7.y, $7.z,
      $10.x, $10.y, $10.z, $13.x, $13.y, $13.z);
  }
| KW_CEILING '(' '(' vector ')' '(' vector ')' '(' vector ')' '(' vector ')' ')'
  { printf ("CEILING ()\n"); }
| KW_FOG '(' color NUMBER ')'
  { printf ("FOG (%g,%g,%g : %g)\n", $3.red, $3.green, $3.blue, $4); }
| KW_CONVEX noargs
  { printf ("CONVEX ()\n"); }
;

sixface_ops:
  sixface_op
| sixface_ops sixface_op
;

sixface_op:
  sixface_tpl_op
| KW_TRIGGER '(' STRING ',' STRING ')'
  { printf ("TRIGGER (%s, %s)\n", $3, $5); }
| KW_ACTIVATE '(' STRING ')'
  { printf ("ACTIVATE ('%s')\n", $3); }
| KW_MOVEABLE noargs
  { printf ("MOVEABLE ()\n"); }
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
  KW_yes				{ $$ = true; }
| KW_no					{ $$ = false; }
;

/* The definition of a color - red, green, blue from 0 to 1 */
color:
  NUMBER NUMBER NUMBER			{ CSCOLOR ($$).Set ($1, $2, $3); }
;

/* Simply a vector */
vector:
  NUMBER NUMBER NUMBER			{ CSVECTOR3 ($$).Set ($1, $2, $3); }
;

/* A vector defined either directly or using a vertex index */
vect_idx:
  NUMBER				{ $$.x = $$.y = $$.z = 0; }
| vector
;

/* A 2D vector */
vector2:
  NUMBER NUMBER				{ CSVECTOR2 ($$).Set ($1, $2); }
;

/* A matrix: there are lots of ways to define a matrix */
matrix:
  NUMBER NUMBER NUMBER
  NUMBER NUMBER NUMBER
  NUMBER NUMBER NUMBER			/* Matrix defined by nine numbers */
  {
    $$ = &STORAGE.matrix2;
    $$->Set ($1, $2, $3, $4, $5, $6, $7, $8, $9);
  }
| NUMBER				/* Uniform matrix scaler */
  {
    $$ = &STORAGE.matrix2;
    $$->Set ($1, 0, 0, 0, $1, 0, 0, 0, $1);
  }
| /* Initialize matrix to identity before starting any complex defs */
  {
    $$ = &STORAGE.matrix2;
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
  { STORAGE.matrix2.Identity (); }
| KW_ROT_X '(' NUMBER ')'		/* Rotate around OX */
  { STORAGE.matrix2 *= csXRotMatrix3 ($3); }
| KW_ROT_Y '(' NUMBER ')'		/* Rotate around OY */
  { STORAGE.matrix2 *= csYRotMatrix3 ($3); }
| KW_ROT_Z '(' NUMBER ')'		/* Rotate around OZ */
  { STORAGE.matrix2 *= csZRotMatrix3 ($3); }
| KW_SCALE '(' NUMBER ')'		/* Uniform matrix scaler */
  { STORAGE.matrix2 *= $3; }
| KW_SCALE '(' NUMBER NUMBER NUMBER ')'	/* Scalers for X/Y/Z individually */
  { STORAGE.matrix2 *= csMatrix3 ($3, 0, 0, 0, $4, 0, 0, 0, $5); }
| KW_SCALE_X '(' NUMBER ')'		/* Scale related to YOZ */
  { STORAGE.matrix2 *= csXScaleMatrix3 ($3); }
| KW_SCALE_Y '(' NUMBER ')'		/* Scale related to XOZ */
  { STORAGE.matrix2 *= csYScaleMatrix3 ($3); }
| KW_SCALE_Z '(' NUMBER ')'		/* Scale related to XOY */
  { STORAGE.matrix2 *= csYScaleMatrix3 ($3); }
;

noargs:
  /* No arguments */
| '(' ')'
;

/* MOVE ( ... ) operator (used in things/sprites/sixfaces/etc) */
move:
  {
    $$ = &STORAGE;
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
    STORAGE.matrix = *$3;
    STORAGE.matrix_valid = true;
  }
| KW_V '(' vector ')'
  {
    STORAGE.vector = CSVECTOR3 ($3);
    STORAGE.vector_valid = true;
  }
;

/* POLYGON 'name' ( ... ) (used in things and sectors) */
polygon_ops:
  polygon_op
| polygon_ops polygon_op
;

polygon_op:
  KW_TEXNR '(' STRING ')'
  { printf ("TEXNR ('%s')\n", $3); }
| KW_LIGHTING '(' yesno ')'
  { printf ("LIGHTING (%d)\n", $3); }
| KW_TEXTURE '(' polygon_texture_ops ')'
  { printf ("TEXTURE (...)\n"); }
| KW_VERTICES '(' vertex_indices ')'
  { printf ("VERTICES (...)\n"); }
| KW_GOURAUD noargs
  { printf ("GOURAUD ()\n"); }
| KW_FLATCOL '(' color ')'
  { printf ("FLATCOL (%g,%g,%g)\n", $3.red, $3.green, $3.blue); }
| KW_ALPHA '(' NUMBER ')'
  { printf ("ALPHA (%g)\n", $3); }
| KW_UV '(' tex_coordinates ')'
  { printf ("UV (...)\n"); }
| KW_UVA '(' uva_coordinates ')'
  { printf ("UVA (...)\n"); }
| KW_COLORS '(' colors ')'
  { printf ("COLORS (...)\n"); }
| KW_COSFACT '(' NUMBER ')'
  { printf ("COSFACT (%g)\n", $3); }
| KW_CLIP noargs
  { printf ("CLIP ()\n"); }
| KW_PORTAL '(' STRING ')'
  { printf ("PORTAL (%s)\n", $3); }
| KW_WARP '(' warp_ops ')'
  { printf ("WARP (...)\n"); }
| KW_LIGHTX '(' STRING ')'
  { printf ("LIGHTX ('%s')\n", $3); }
;

colors:
  /* none */
| colors color
  { }
;

vertex_indices:
  /* none */
| vertex_indices NUMBER
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

/* WARP ( ... ) */
warp_ops:
  warp_op
| warp_ops warp_op
;

warp_op:
  KW_MATRIX '(' matrix ')'
  { printf ("MATRIX (...)\n"); }
| KW_V '(' vector ')'
  { printf ("V (...)\n"); }
| KW_W '(' vector ')'
  { printf ("W (...)\n"); }
| KW_MIRROR noargs
  { printf ("MIRROR ()\n"); }
| KW_STATIC noargs
  { printf ("STATIC ()\n"); }
;

/* TEXTURE (...) */
polygon_texture_ops:
  polygon_texture_op
| polygon_texture_ops polygon_texture_op
;

polygon_texture_op:
  KW_ORIG '(' vect_idx ')'		{ printf ("ORIG (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_FIRST '(' vect_idx ')'		{ printf ("FIRST (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_SECOND '(' vect_idx ')'		{ printf ("SECOND (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_FIRST_LEN '(' NUMBER ')'		{ printf ("FIRST_LEN (%g)\n", $3); }
| KW_SECOND_LEN '(' NUMBER ')'		{ printf ("SECOND_LEN (%g)\n", $3); }
| KW_UVEC '(' vector ')'		{ printf ("UVEC (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_VVEC '(' vector ')'		{ printf ("VVEC (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_MATRIX '(' matrix ')'
  {
    printf ("MATRIX (\n  %g, %g, %g\n  %g, %g, %g\n  %g, %g, %g\n)\n",
    $3->m11, $3->m12, $3->m13, $3->m21, $3->m22, $3->m21, $3->m31, $3->m32, $3->m33);
  }
| KW_V '(' vector ')'			{ printf ("V (%g, %g, %g)\n", $3.x, $3.y, $3.z); }
| KW_TEXLEN '(' NUMBER ')'
  { printf ("TEXLEN (%g)\n", $3); }
| KW_PLANE '(' STRING ')'
  { printf ("PLANE ('%s')\n", $3); }
| KW_UV_SHIFT '(' vector2 ')'
  { printf ("UV_SHIFT (%g, %g)\n", $3.x, $3.y); }
;

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
| KW_DYNAMIC noargs
  { printf ("DYNAMIC ()\n"); }
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
