typedef union
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
  struct { float red, green, blue; } color;
  // A 2D point
  struct { float x, y; } vect2;
  // A 3D point
  struct { float x, y, z; } vect;
  // A transformation matrix
  csMatrix3 *matrix;
  // A transformation matrix/vector
  csTransform *transform;
} YYSTYPE;
#define	KW_ACTION	258
#define	KW_ACTIVATE	259
#define	KW_ACTIVE	260
#define	KW_ADD	261
#define	KW_ALPHA	262
#define	KW_ATTENUATION	263
#define	KW_BECOMING_ACTIVE	264
#define	KW_BECOMING_INACTIVE	265
#define	KW_BEZIER	266
#define	KW_CEILING	267
#define	KW_CEIL_TEXTURE	268
#define	KW_CENTER	269
#define	KW_CIRCLE	270
#define	KW_CLIP	271
#define	KW_COLLECTION	272
#define	KW_COLOR	273
#define	KW_COLORS	274
#define	KW_CONVEX	275
#define	KW_COPY	276
#define	KW_COSFACT	277
#define	KW_CURVECENTER	278
#define	KW_CURVECONTROL	279
#define	KW_CURVESCALE	280
#define	KW_DETAIL	281
#define	KW_DIM	282
#define	KW_DITHER	283
#define	KW_DYNAMIC	284
#define	KW_F	285
#define	KW_FILE	286
#define	KW_FILTER	287
#define	KW_FIRST	288
#define	KW_FIRST_LEN	289
#define	KW_FLATCOL	290
#define	KW_FLOOR	291
#define	KW_FLOOR_CEIL	292
#define	KW_FLOOR_HEIGHT	293
#define	KW_FLOOR_TEXTURE	294
#define	KW_FOG	295
#define	KW_FRAME	296
#define	KW_GOURAUD	297
#define	KW_HALO	298
#define	KW_HEIGHT	299
#define	KW_HEIGHTMAP	300
#define	KW_IDENTITY	301
#define	KW_KEY	302
#define	KW_KEYCOLOR	303
#define	KW_LEN	304
#define	KW_LIBRARY	305
#define	KW_LIGHT	306
#define	KW_LIGHTING	307
#define	KW_LIGHTX	308
#define	KW_LIMB	309
#define	KW_MATRIX	310
#define	KW_MERGE_NORMALS	311
#define	KW_MERGE_TEXELS	312
#define	KW_MERGE_VERTICES	313
#define	KW_MIPMAP	314
#define	KW_MIRROR	315
#define	KW_MIXMODE	316
#define	KW_MOVE	317
#define	KW_MOVEABLE	318
#define	KW_MULTIPLY	319
#define	KW_MULTIPLY2	320
#define	KW_NODE	321
#define	KW_ORIG	322
#define	KW_PLANE	323
#define	KW_POLYGON	324
#define	KW_PORTAL	325
#define	KW_POSITION	326
#define	KW_PRIMARY_ACTIVE	327
#define	KW_PRIMARY_INACTIVE	328
#define	KW_RADIUS	329
#define	KW_ROOM	330
#define	KW_ROT	331
#define	KW_ROT_X	332
#define	KW_ROT_Y	333
#define	KW_ROT_Z	334
#define	KW_SCALE	335
#define	KW_SCALE_X	336
#define	KW_SCALE_Y	337
#define	KW_SCALE_Z	338
#define	KW_SCRIPT	339
#define	KW_SECOND	340
#define	KW_SECONDARY_ACTIVE	341
#define	KW_SECONDARY_INACTIVE	342
#define	KW_SECOND_LEN	343
#define	KW_SECTOR	344
#define	KW_SIXFACE	345
#define	KW_SKELETON	346
#define	KW_SKYDOME	347
#define	KW_SOUND	348
#define	KW_SOUNDS	349
#define	KW_SPLIT	350
#define	KW_SPRITE	351
#define	KW_SPRITE2D	352
#define	KW_START	353
#define	KW_STATBSP	354
#define	KW_STATELESS	355
#define	KW_STATIC	356
#define	KW_TEMPLATE	357
#define	KW_TERRAIN	358
#define	KW_TEX	359
#define	KW_TEXLEN	360
#define	KW_TEXNR	361
#define	KW_TEXTURE	362
#define	KW_TEXTURES	363
#define	KW_TEXTURE_LIGHTING	364
#define	KW_TEXTURE_MIPMAP	365
#define	KW_TEXTURE_SCALE	366
#define	KW_TEX_SET	367
#define	KW_TEX_SET_SELECT	368
#define	KW_THING	369
#define	KW_TRANSFORM	370
#define	KW_TRANSPARENT	371
#define	KW_TRIANGLE	372
#define	KW_TRIGGER	373
#define	KW_UV	374
#define	KW_UVA	375
#define	KW_UVEC	376
#define	KW_UV_SHIFT	377
#define	KW_V	378
#define	KW_VERTEX	379
#define	KW_VERTICES	380
#define	KW_VVEC	381
#define	KW_W	382
#define	KW_WARP	383
#define	KW_WORLD	384
#define	KW_yes	385
#define	KW_no	386
#define	KW_none	387
#define	KW_linear	388
#define	KW_inverse	389
#define	KW_realistic	390
#define	STRING	391
#define	NUMBER	392


extern YYSTYPE yylval;
