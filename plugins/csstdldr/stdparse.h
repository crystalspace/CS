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
  csPColor color;
  // A 2D point
  struct { float x, y; } vect2;
  // A 3D point
  struct { float x, y, z; } vect;
  // A transformation matrix
  csMatrix3 *matrix;
  // A transformation matrix/vector
  csStandardLoader::yystorage *transform;
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
#define	KW_FOR_2D	296
#define	KW_FOR_3D	297
#define	KW_FRAME	298
#define	KW_GOURAUD	299
#define	KW_HALO	300
#define	KW_HEIGHT	301
#define	KW_HEIGHTMAP	302
#define	KW_IDENTITY	303
#define	KW_KEY	304
#define	KW_KEYCOLOR	305
#define	KW_LEN	306
#define	KW_LIBRARY	307
#define	KW_LIGHT	308
#define	KW_LIGHTING	309
#define	KW_LIGHTX	310
#define	KW_LIMB	311
#define	KW_MATRIX	312
#define	KW_MERGE_NORMALS	313
#define	KW_MERGE_TEXELS	314
#define	KW_MERGE_VERTICES	315
#define	KW_MIPMAP	316
#define	KW_MIRROR	317
#define	KW_MIXMODE	318
#define	KW_MOVE	319
#define	KW_MOVEABLE	320
#define	KW_MULTIPLY	321
#define	KW_MULTIPLY2	322
#define	KW_NODE	323
#define	KW_ORIG	324
#define	KW_PLANE	325
#define	KW_POLYGON	326
#define	KW_PORTAL	327
#define	KW_POSITION	328
#define	KW_PRIMARY_ACTIVE	329
#define	KW_PRIMARY_INACTIVE	330
#define	KW_RADIUS	331
#define	KW_ROOM	332
#define	KW_ROT	333
#define	KW_ROT_X	334
#define	KW_ROT_Y	335
#define	KW_ROT_Z	336
#define	KW_SCALE	337
#define	KW_SCALE_X	338
#define	KW_SCALE_Y	339
#define	KW_SCALE_Z	340
#define	KW_SCRIPT	341
#define	KW_SECOND	342
#define	KW_SECONDARY_ACTIVE	343
#define	KW_SECONDARY_INACTIVE	344
#define	KW_SECOND_LEN	345
#define	KW_SECTOR	346
#define	KW_SIXFACE	347
#define	KW_SKELETON	348
#define	KW_SKYDOME	349
#define	KW_SOUND	350
#define	KW_SOUNDS	351
#define	KW_SPLIT	352
#define	KW_SPRITE	353
#define	KW_SPRITE2D	354
#define	KW_START	355
#define	KW_STATBSP	356
#define	KW_STATELESS	357
#define	KW_STATIC	358
#define	KW_TEMPLATE	359
#define	KW_TERRAIN	360
#define	KW_TEX	361
#define	KW_TEXLEN	362
#define	KW_TEXNR	363
#define	KW_TEXTURE	364
#define	KW_TEXTURES	365
#define	KW_TEXTURE_LIGHTING	366
#define	KW_TEXTURE_MIPMAP	367
#define	KW_TEXTURE_SCALE	368
#define	KW_TEX_SET	369
#define	KW_TEX_SET_SELECT	370
#define	KW_THING	371
#define	KW_TRANSFORM	372
#define	KW_TRANSPARENT	373
#define	KW_TRIANGLE	374
#define	KW_TRIGGER	375
#define	KW_UV	376
#define	KW_UVA	377
#define	KW_UVEC	378
#define	KW_UV_SHIFT	379
#define	KW_V	380
#define	KW_VERTEX	381
#define	KW_VERTICES	382
#define	KW_VVEC	383
#define	KW_W	384
#define	KW_WARP	385
#define	KW_WORLD	386
#define	KW_yes	387
#define	KW_no	388
#define	KW_none	389
#define	KW_linear	390
#define	KW_inverse	391
#define	KW_realistic	392
#define	STRING	393
#define	NUMBER	394

