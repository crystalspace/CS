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
  csPVector2 vect2;
  // A 3D point
  csPVector3 vect;
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
#define	KW_CAMERA	267
#define	KW_CENTER	268
#define	KW_CIRCLE	269
#define	KW_CLIP	270
#define	KW_COLLECTION	271
#define	KW_COLOR	272
#define	KW_COLORS	273
#define	KW_CONVEX	274
#define	KW_COPY	275
#define	KW_COSFACT	276
#define	KW_CURVECENTER	277
#define	KW_CURVECONTROL	278
#define	KW_CURVESCALE	279
#define	KW_DETAIL	280
#define	KW_DITHER	281
#define	KW_DYNAMIC	282
#define	KW_F	283
#define	KW_FILE	284
#define	KW_FIRST	285
#define	KW_FIRST_LEN	286
#define	KW_FLATCOL	287
#define	KW_FOG	288
#define	KW_FOR_2D	289
#define	KW_FOR_3D	290
#define	KW_FORWARD	291
#define	KW_FRAME	292
#define	KW_GOURAUD	293
#define	KW_HALO	294
#define	KW_HEIGHTMAP	295
#define	KW_IDENTITY	296
#define	KW_KEY	297
#define	KW_KEYCOLOR	298
#define	KW_LEN	299
#define	KW_LIBRARY	300
#define	KW_LIGHT	301
#define	KW_LIGHTING	302
#define	KW_LIGHTX	303
#define	KW_LIMB	304
#define	KW_MATRIX	305
#define	KW_MERGE_NORMALS	306
#define	KW_MERGE_TEXELS	307
#define	KW_MERGE_VERTICES	308
#define	KW_MIPMAP	309
#define	KW_MIRROR	310
#define	KW_MIXMODE	311
#define	KW_MOVE	312
#define	KW_MOVEABLE	313
#define	KW_MULTIPLY	314
#define	KW_MULTIPLY2	315
#define	KW_NODE	316
#define	KW_ORIG	317
#define	KW_PLANE	318
#define	KW_POLYGON	319
#define	KW_PORTAL	320
#define	KW_POSITION	321
#define	KW_PRIMARY_ACTIVE	322
#define	KW_PRIMARY_INACTIVE	323
#define	KW_RADIUS	324
#define	KW_ROT	325
#define	KW_ROT_X	326
#define	KW_ROT_Y	327
#define	KW_ROT_Z	328
#define	KW_SCALE	329
#define	KW_SCALE_X	330
#define	KW_SCALE_Y	331
#define	KW_SCALE_Z	332
#define	KW_SCRIPT	333
#define	KW_SECOND	334
#define	KW_SECONDARY_ACTIVE	335
#define	KW_SECONDARY_INACTIVE	336
#define	KW_SECOND_LEN	337
#define	KW_SECTOR	338
#define	KW_SKELETON	339
#define	KW_SKYDOME	340
#define	KW_SOUND	341
#define	KW_SOUNDS	342
#define	KW_SPRITE	343
#define	KW_SPRITE2D	344
#define	KW_START	345
#define	KW_STATBSP	346
#define	KW_STATELESS	347
#define	KW_STATIC	348
#define	KW_TEMPLATE	349
#define	KW_TERRAIN	350
#define	KW_TEX	351
#define	KW_TEXLEN	352
#define	KW_TEXNR	353
#define	KW_TEXTURE	354
#define	KW_TEXTURES	355
#define	KW_TEXTURE_LIGHTING	356
#define	KW_TEXTURE_MIPMAP	357
#define	KW_TEXTURE_SCALE	358
#define	KW_TEX_SET	359
#define	KW_TEX_SET_SELECT	360
#define	KW_THING	361
#define	KW_TRANSFORM	362
#define	KW_TRANSPARENT	363
#define	KW_TRIANGLE	364
#define	KW_TRIGGER	365
#define	KW_UPWARD	366
#define	KW_UV	367
#define	KW_UVA	368
#define	KW_UVEC	369
#define	KW_UV_SHIFT	370
#define	KW_V	371
#define	KW_VERTEX	372
#define	KW_VERTICES	373
#define	KW_VVEC	374
#define	KW_W	375
#define	KW_WARP	376
#define	KW_WORLD	377
#define	KW_yes	378
#define	KW_no	379
#define	KW_none	380
#define	KW_linear	381
#define	KW_inverse	382
#define	KW_realistic	383
#define	PARSER_VERSION	384
#define	STRING	385
#define	NUMBER	386

