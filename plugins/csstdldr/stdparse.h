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
#define	KW_ADD	259
#define	KW_ALPHA	260
#define	KW_ATTENUATION	261
#define	KW_BEZIER	262
#define	KW_CAMERA	263
#define	KW_CENTER	264
#define	KW_CIRCLE	265
#define	KW_CLIP	266
#define	KW_COLLECTION	267
#define	KW_COLOR	268
#define	KW_COLORS	269
#define	KW_CONVEX	270
#define	KW_COPY	271
#define	KW_CURVECENTER	272
#define	KW_CURVECONTROL	273
#define	KW_CURVESCALE	274
#define	KW_DETAIL	275
#define	KW_DITHER	276
#define	KW_DYNAMIC	277
#define	KW_F	278
#define	KW_FILE	279
#define	KW_FIRST	280
#define	KW_FIRST_LEN	281
#define	KW_FLATCOL	282
#define	KW_FOG	283
#define	KW_FOR_2D	284
#define	KW_FOR_3D	285
#define	KW_FORWARD	286
#define	KW_FRAME	287
#define	KW_GOURAUD	288
#define	KW_HALO	289
#define	KW_HEIGHTMAP	290
#define	KW_IDENTITY	291
#define	KW_KEY	292
#define	KW_KEYCOLOR	293
#define	KW_LEN	294
#define	KW_LIBRARY	295
#define	KW_LIGHT	296
#define	KW_LIGHTING	297
#define	KW_LIMB	298
#define	KW_MATERIAL	299
#define	KW_MATRIX	300
#define	KW_MIPMAP	301
#define	KW_MIRROR	302
#define	KW_MIXMODE	303
#define	KW_MOVE	304
#define	KW_MOVEABLE	305
#define	KW_MULTIPLY	306
#define	KW_MULTIPLY2	307
#define	KW_NODE	308
#define	KW_ORIG	309
#define	KW_PLANE	310
#define	KW_POLYGON	311
#define	KW_PORTAL	312
#define	KW_POSITION	313
#define	KW_RADIUS	314
#define	KW_ROT	315
#define	KW_ROT_X	316
#define	KW_ROT_Y	317
#define	KW_ROT_Z	318
#define	KW_SCALE	319
#define	KW_SCALE_X	320
#define	KW_SCALE_Y	321
#define	KW_SCALE_Z	322
#define	KW_SCRIPT	323
#define	KW_SECOND	324
#define	KW_SECOND_LEN	325
#define	KW_SECTOR	326
#define	KW_SKELETON	327
#define	KW_SKYDOME	328
#define	KW_SOUND	329
#define	KW_SOUNDS	330
#define	KW_SPRITE	331
#define	KW_SPRITE2D	332
#define	KW_START	333
#define	KW_STATBSP	334
#define	KW_STATIC	335
#define	KW_TEMPLATE	336
#define	KW_TERRAIN	337
#define	KW_TEX	338
#define	KW_TEXLEN	339
#define	KW_TEXMAP	340
#define	KW_TEXNR	341
#define	KW_TEXTURE	342
#define	KW_TEXTURES	343
#define	KW_TEXTURE_LIGHTING	344
#define	KW_TEXTURE_MIPMAP	345
#define	KW_TEXTURE_SCALE	346
#define	KW_TEX_SET	347
#define	KW_TEX_SET_SELECT	348
#define	KW_THING	349
#define	KW_TRANSFORM	350
#define	KW_TRANSPARENT	351
#define	KW_TRIANGLE	352
#define	KW_UPWARD	353
#define	KW_UV	354
#define	KW_UVA	355
#define	KW_UVEC	356
#define	KW_UV_SHIFT	357
#define	KW_V	358
#define	KW_VERTEX	359
#define	KW_VERTICES	360
#define	KW_VVEC	361
#define	KW_W	362
#define	KW_WORLD	363
#define	KW_ZFILL	364
#define	KW_yes	365
#define	KW_no	366
#define	KW_none	367
#define	KW_linear	368
#define	KW_inverse	369
#define	KW_realistic	370
#define	PARSER_VERSION	371
#define	STRING	372
#define	NUMBER	373

