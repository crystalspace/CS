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
#define	KW_ADD	260
#define	KW_ALPHA	261
#define	KW_ATTENUATION	262
#define	KW_BEZIER	263
#define	KW_CAMERA	264
#define	KW_CENTER	265
#define	KW_CIRCLE	266
#define	KW_CLIP	267
#define	KW_COLLECTION	268
#define	KW_COLOR	269
#define	KW_COLORS	270
#define	KW_CONVEX	271
#define	KW_COPY	272
#define	KW_COSFACT	273
#define	KW_CURVECENTER	274
#define	KW_CURVECONTROL	275
#define	KW_CURVESCALE	276
#define	KW_DETAIL	277
#define	KW_DITHER	278
#define	KW_DYNAMIC	279
#define	KW_F	280
#define	KW_FILE	281
#define	KW_FIRST	282
#define	KW_FIRST_LEN	283
#define	KW_FLATCOL	284
#define	KW_FOG	285
#define	KW_FOR_2D	286
#define	KW_FOR_3D	287
#define	KW_FORWARD	288
#define	KW_FRAME	289
#define	KW_GOURAUD	290
#define	KW_HALO	291
#define	KW_HEIGHTMAP	292
#define	KW_IDENTITY	293
#define	KW_KEY	294
#define	KW_KEYCOLOR	295
#define	KW_LEN	296
#define	KW_LIBRARY	297
#define	KW_LIGHT	298
#define	KW_LIGHTING	299
#define	KW_LIMB	300
#define	KW_MATRIX	301
#define	KW_MERGE_NORMALS	302
#define	KW_MERGE_TEXELS	303
#define	KW_MERGE_VERTICES	304
#define	KW_MIPMAP	305
#define	KW_MIRROR	306
#define	KW_MIXMODE	307
#define	KW_MOVE	308
#define	KW_MOVEABLE	309
#define	KW_MULTIPLY	310
#define	KW_MULTIPLY2	311
#define	KW_NODE	312
#define	KW_ORIG	313
#define	KW_PLANE	314
#define	KW_POLYGON	315
#define	KW_PORTAL	316
#define	KW_POSITION	317
#define	KW_RADIUS	318
#define	KW_ROT	319
#define	KW_ROT_X	320
#define	KW_ROT_Y	321
#define	KW_ROT_Z	322
#define	KW_SCALE	323
#define	KW_SCALE_X	324
#define	KW_SCALE_Y	325
#define	KW_SCALE_Z	326
#define	KW_SCRIPT	327
#define	KW_SECOND	328
#define	KW_SECOND_LEN	329
#define	KW_SECTOR	330
#define	KW_SKELETON	331
#define	KW_SKYDOME	332
#define	KW_SOUND	333
#define	KW_SOUNDS	334
#define	KW_SPRITE	335
#define	KW_SPRITE2D	336
#define	KW_START	337
#define	KW_STATBSP	338
#define	KW_STATIC	339
#define	KW_TEMPLATE	340
#define	KW_TERRAIN	341
#define	KW_TEX	342
#define	KW_TEXLEN	343
#define	KW_TEXNR	344
#define	KW_TEXTURE	345
#define	KW_TEXTURES	346
#define	KW_TEXTURE_LIGHTING	347
#define	KW_TEXTURE_MIPMAP	348
#define	KW_TEXTURE_SCALE	349
#define	KW_TEX_SET	350
#define	KW_TEX_SET_SELECT	351
#define	KW_THING	352
#define	KW_TRANSFORM	353
#define	KW_TRANSPARENT	354
#define	KW_TRIANGLE	355
#define	KW_TRIGGER	356
#define	KW_UPWARD	357
#define	KW_UV	358
#define	KW_UVA	359
#define	KW_UVEC	360
#define	KW_UV_SHIFT	361
#define	KW_V	362
#define	KW_VERTEX	363
#define	KW_VERTICES	364
#define	KW_VVEC	365
#define	KW_W	366
#define	KW_WARP	367
#define	KW_WORLD	368
#define	KW_yes	369
#define	KW_no	370
#define	KW_none	371
#define	KW_linear	372
#define	KW_inverse	373
#define	KW_realistic	374
#define	PARSER_VERSION	375
#define	STRING	376
#define	NUMBER	377

