
/*  A Bison parser, made from plugins/aws/skinlang.bsn
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse awsparse
#define yylex awslex
#define yyerror awserror
#define yylval awslval
#define yychar awschar
#define yydebug awsdebug
#define yynerrs awsnerrs
#define	TOKEN_NUM	257
#define	TOKEN_STR	258
#define	TOKEN_ATTR	259
#define	TOKEN_SKIN	260
#define	TOKEN_FOR	261
#define	TOKEN_WINDOW	262
#define	TOKEN_FROM	263
#define	TOKEN_COMPONENT	264
#define	TOKEN_CONNECT	265
#define	TOKEN_IS	266
#define	NEG	267



#include "cssysdef.h"
#include "csgeom/csrect.h"
#include "csutil/scfstr.h"
#include "csutil/csdllist.h"
#include "aws.h"
#include "awsprefs.h"
#include <stdio.h>


typedef union {
  char   *str;     /* For returning titles and handles to items. */
  int     val;     /* For returning numbers                      */
  csRect *rect;    /* For returning rectangular regions          */
  awsKey *key;     /* For returning keys to various definition items */
} YYSTYPE;


extern int awslex(YYSTYPE *awslval);
extern int awserror(char *s);
extern int awslineno;
static void mapsourcetosink(iAws *windowmgr, unsigned long signal, iString *sinkname, iString *triggername);

/// This is locally global variable that holds keys for a little while
static awsKeyContainer kcont[32];
/// This is the locally global variable that holds window keys.
static awsKeyContainer wkcont;
/// This is the locally global variable that holds skin keys.
static awsKeyContainer skcont;
/// This is the level we're on.
static int			   klevel=0;

/// This is the parser parameter
#define YYPARSE_PARAM windowmgr


#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		127
#define	YYFLAG		-32768
#define	YYNTBASE	28

#define YYTRANSLATE(x) ((unsigned)(x) <= 267 ? yytranslate[x] : 44)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    20,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    25,
    26,    16,    15,    24,    14,     2,    17,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    23,     2,     2,
    13,    22,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    19,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    27,     2,    21,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    18
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     6,     8,    10,    13,    21,    23,    26,
    30,    34,    42,    56,    57,    63,    64,    73,    75,    78,
    82,    86,   100,   105,   113,   115,   118,   124,   128,   136,
   140,   148,   162,   164,   167,   173,   175,   177,   179,   183,
   187,   191,   195,   198
};

static const short yyrhs[] = {    -1,
    28,    29,     0,    20,     0,    41,     0,    38,     0,     1,
    21,     0,    43,    14,    22,     5,    23,    23,     5,     0,
    30,     0,    31,    30,     0,     5,    23,     4,     0,     5,
    23,    43,     0,     5,    23,    43,    24,    43,    24,    43,
     0,     5,    23,    25,    43,    24,    43,    26,    14,    25,
    43,    24,    43,    26,     0,     0,    11,    33,    27,    31,
    21,     0,     0,    10,     4,    12,     4,    27,    34,    35,
    21,     0,    32,     0,    35,    32,     0,     5,    23,     4,
     0,     5,    23,    43,     0,     5,    23,    25,    43,    24,
    43,    26,    14,    25,    43,    24,    43,    26,     0,    11,
    27,    31,    21,     0,    10,     4,    12,     4,    27,    35,
    21,     0,    36,     0,    37,    36,     0,     8,     4,    27,
    37,    21,     0,     5,    23,     4,     0,     5,    23,    43,
    24,    43,    24,    43,     0,     5,    23,    43,     0,     5,
    23,    25,    43,    24,    43,    26,     0,     5,    23,    25,
    43,    24,    43,    26,    14,    25,    43,    24,    43,    26,
     0,    39,     0,    40,    39,     0,     6,     4,    27,    40,
    21,     0,     5,     0,     3,     0,    42,     0,    43,    15,
    43,     0,    43,    14,    43,     0,    43,    16,    43,     0,
    43,    17,    43,     0,    14,    43,     0,    25,    43,    26,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    81,    82,    85,    86,    87,    88,    96,    99,   100,   103,
   104,   105,   106,   107,   108,   113,   115,   122,   123,   127,
   128,   129,   130,   134,   141,   142,   145,   157,   158,   159,
   160,   161,   164,   165,   168,   177,   191,   192,   193,   194,
   195,   196,   197,   198
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TOKEN_NUM",
"TOKEN_STR","TOKEN_ATTR","TOKEN_SKIN","TOKEN_FOR","TOKEN_WINDOW","TOKEN_FROM",
"TOKEN_COMPONENT","TOKEN_CONNECT","TOKEN_IS","'='","'-'","'+'","'*'","'/'","NEG",
"'^'","'\\n'","'}'","'>'","':'","','","'('","')'","'{'","input","line","connection_item",
"connection_item_list","component_item","@1","@2","component_item_list","window_item",
"window_item_list","window","skin_item","skin_item_list","skin","constant_item",
"exp", NULL
};
#endif

static const short yyr1[] = {     0,
    28,    28,    29,    29,    29,    29,    30,    31,    31,    32,
    32,    32,    32,    33,    32,    34,    32,    35,    35,    36,
    36,    36,    36,    36,    37,    37,    38,    39,    39,    39,
    39,    39,    40,    40,    41,    42,    43,    43,    43,    43,
    43,    43,    43,    43
};

static const short yyr2[] = {     0,
     0,     2,     1,     1,     1,     2,     7,     1,     2,     3,
     3,     7,    13,     0,     5,     0,     8,     1,     2,     3,
     3,    13,     4,     7,     1,     2,     5,     3,     7,     3,
     7,    13,     1,     2,     5,     1,     1,     1,     3,     3,
     3,     3,     2,     3
};

static const short yydefact[] = {     1,
     0,     0,     0,     0,     3,     2,     5,     4,     6,     0,
     0,     0,     0,     0,    33,     0,     0,     0,     0,    25,
     0,     0,    35,    34,     0,     0,     0,    27,    26,    37,
    28,    36,     0,     0,    38,    30,    20,     0,    21,     0,
     0,     8,     0,     0,    43,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    23,     9,     0,     0,    44,    40,
    39,    41,    42,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    14,    18,     0,     0,    31,    29,     0,     0,
     0,     0,    24,    19,     0,     0,     0,    10,     0,    11,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     7,
     0,     0,     0,     0,    16,    15,     0,     0,     0,     0,
     0,     0,     0,     0,    12,     0,    32,    22,     0,    17,
     0,     0,     0,     0,    13,     0,     0
};

static const short yydefgoto[] = {     1,
     6,    42,    43,    74,    82,   111,    75,    20,    21,     7,
    15,    16,     8,    35,    44
};

static const short yypact[] = {-32768,
   103,    -8,     6,    19,-32768,-32768,-32768,-32768,-32768,     7,
    13,    45,     4,     1,-32768,    12,    28,    49,    29,-32768,
    -3,    17,-32768,-32768,    34,    42,    77,-32768,-32768,-32768,
-32768,-32768,    77,    77,-32768,   182,-32768,    77,   231,    56,
    77,-32768,    41,   238,-32768,   105,    77,    77,    77,    77,
    77,   110,    36,   127,-32768,-32768,    27,    77,-32768,   -11,
   -11,-32768,-32768,   186,    77,   141,    59,   140,    77,   144,
    38,    61,-32768,-32768,    95,    48,    60,   231,    63,    65,
    71,    57,-32768,-32768,    66,    67,    68,-32768,    77,   197,
    91,    77,    92,    77,    77,   123,    77,    69,    73,-32768,
   201,   212,    77,   216,-32768,-32768,    77,    77,   148,    77,
   141,   161,   165,    93,   231,   107,-32768,-32768,    83,-32768,
    77,   227,    77,   169,-32768,   113,-32768
};

static const short yypgoto[] = {-32768,
-32768,   -42,    18,   -71,-32768,-32768,     3,    94,-32768,-32768,
   114,-32768,-32768,-32768,   -22
};


#define	YYLAST		255


static const short yytable[] = {    36,
    56,    17,    39,    84,    49,    50,    18,    19,    17,    10,
    45,    46,     9,    18,    19,    52,    14,    28,    54,    30,
    31,    32,    11,    22,    60,    61,    62,    63,    64,    30,
    33,    32,    23,    12,    60,    68,    30,    37,    32,    13,
    33,    34,    70,    30,    84,    32,    78,    33,    67,    14,
    25,    41,    26,    40,    33,    27,    56,    90,    38,    53,
    80,    55,    66,    76,    81,    41,    96,    30,    88,    32,
    85,   101,   102,    86,   104,    30,    87,    32,    33,    30,
   109,    32,    91,    92,   112,   113,    33,   115,    93,    89,
    33,    94,    95,   106,    98,   105,   100,    41,   122,    71,
   124,    41,   126,     2,    72,    73,   119,   121,     3,    99,
     4,    71,   127,   116,    29,    83,    72,    73,    47,    48,
    49,    50,     5,    47,    48,    49,    50,   120,    58,    24,
    59,     0,     0,    65,     0,    59,    47,    48,    49,    50,
    47,    48,    49,    50,     0,    71,   103,     0,    59,     0,
    72,    73,    59,    47,    48,    49,    50,    47,    48,    49,
    50,    47,    48,    49,    50,    77,     0,     0,     0,    79,
     0,     0,     0,   114,    47,    48,    49,    50,    47,    48,
    49,    50,    47,    48,    49,    50,   117,     0,     0,     0,
   118,     0,     0,     0,   125,    47,    48,    49,    50,    47,
    48,    49,    50,     0,     0,    51,     0,     0,     0,    69,
    47,    48,    49,    50,    47,    48,    49,    50,     0,     0,
    97,     0,     0,     0,   107,    47,    48,    49,    50,    47,
    48,    49,    50,     0,     0,   108,     0,     0,     0,   110,
    47,    48,    49,    50,    47,    48,    49,    50,     0,     0,
   123,    57,    48,    49,    50
};

static const short yycheck[] = {    22,
    43,     5,    25,    75,    16,    17,    10,    11,     5,     4,
    33,    34,    21,    10,    11,    38,     5,    21,    41,     3,
     4,     5,     4,    23,    47,    48,    49,    50,    51,     3,
    14,     5,    21,    27,    57,    58,     3,     4,     5,    27,
    14,    25,    65,     3,   116,     5,    69,    14,    22,     5,
    23,    25,     4,    12,    14,    27,    99,    80,    25,     4,
    23,    21,    27,     5,     4,    25,    89,     3,     4,     5,
    23,    94,    95,    14,    97,     3,    14,     5,    14,     3,
   103,     5,    12,    27,   107,   108,    14,   110,    23,    25,
    14,    25,    25,    21,     4,    27,     5,    25,   121,     5,
   123,    25,     0,     1,    10,    11,    14,    25,     6,    92,
     8,     5,     0,   111,    21,    21,    10,    11,    14,    15,
    16,    17,    20,    14,    15,    16,    17,    21,    24,    16,
    26,    -1,    -1,    24,    -1,    26,    14,    15,    16,    17,
    14,    15,    16,    17,    -1,     5,    24,    -1,    26,    -1,
    10,    11,    26,    14,    15,    16,    17,    14,    15,    16,
    17,    14,    15,    16,    17,    26,    -1,    -1,    -1,    26,
    -1,    -1,    -1,    26,    14,    15,    16,    17,    14,    15,
    16,    17,    14,    15,    16,    17,    26,    -1,    -1,    -1,
    26,    -1,    -1,    -1,    26,    14,    15,    16,    17,    14,
    15,    16,    17,    -1,    -1,    24,    -1,    -1,    -1,    24,
    14,    15,    16,    17,    14,    15,    16,    17,    -1,    -1,
    24,    -1,    -1,    -1,    24,    14,    15,    16,    17,    14,
    15,    16,    17,    -1,    -1,    24,    -1,    -1,    -1,    24,
    14,    15,    16,    17,    14,    15,    16,    17,    -1,    -1,
    24,    14,    15,    16,    17
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */

/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif


#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
YYPARSE_RETURN_TYPE
yyparse (void *);
#else
YYPARSE_RETURN_TYPE
yyparse (void);
#endif
#endif

YYPARSE_RETURN_TYPE
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
#ifndef YYSTACK_USE_ALLOCA
  int yyfree_stacks = 0;
#endif

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
#ifndef YYSTACK_USE_ALLOCA
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
#endif	    
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 4:
{ ;
    break;}
case 5:
{ ;
    break;}
case 6:
{ yyerrok;      ;
    break;}
case 7:
{ mapsourcetosink((iAws *)windowmgr, yyvsp[-6].val, new scfString(yyvsp[-3].str), new scfString(yyvsp[0].str)); ;
    break;}
case 10:
{ kcont[klevel].Add(new awsStringKey(new scfString(yyvsp[-2].str), new scfString(yyvsp[0].str)));         ;
    break;}
case 11:
{ kcont[klevel].Add(new awsIntKey(new scfString(yyvsp[-2].str), yyvsp[0].val));                           ;
    break;}
case 12:
{ kcont[klevel].Add(new awsRGBKey(new scfString(yyvsp[-6].str), yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val));                   ;
    break;}
case 13:
{ kcont[klevel].Add(new awsRectKey(new scfString(yyvsp[-12].str), csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)));    ;
    break;}
case 14:
{ ++klevel; /* go down a level (mid-action rule) */ ;
    break;}
case 15:
{ awsConnectionNode *cn = new awsConnectionNode();
																				  ((awsKeyContainer*)cn)->Consume(&(kcont[klevel]));
																				  --klevel;
																				  kcont[klevel].Add(cn);
																				;
    break;}
case 16:
{ ++klevel; /* go down a level (mid-action rule) */ ;
    break;}
case 17:
{ awsComponentNode *cn = new awsComponentNode(new scfString(yyvsp[-6].str), new scfString(yyvsp[-4].str));
																				  ((awsKeyContainer*)cn)->Consume(&(kcont[klevel]));
																				  --klevel;  /* go up a level in recursion */
																				  kcont[klevel].Add(cn);
																				;
    break;}
case 18:
{ /*empty*/ ;
    break;}
case 19:
{ /*empty*/ ;
    break;}
case 20:
{ yyval.key = new awsStringKey(new scfString(yyvsp[-2].str), new scfString(yyvsp[0].str));          ;
    break;}
case 21:
{ yyval.key = new awsIntKey(new scfString(yyvsp[-2].str), yyvsp[0].val);                            ;
    break;}
case 22:
{ yyval.key = new awsRectKey(new scfString(yyvsp[-12].str), csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val));     ;
    break;}
case 23:
{ awsConnectionNode *cn = new awsConnectionNode();
																				  ((awsKeyContainer*)cn)->Consume(&(kcont[klevel]));
																				  yyval.key=cn;
			  																	;
    break;}
case 24:
{ awsComponentNode *cn = new awsComponentNode(new scfString(yyvsp[-5].str), new scfString(yyvsp[-3].str));
																				  ((awsKeyContainer*)cn)->Consume(&(kcont[klevel]));
																				   yyval.key=cn;
																				;
    break;}
case 25:
{ wkcont.Add(yyvsp[0].key); ;
    break;}
case 26:
{ wkcont.Add(yyvsp[0].key); ;
    break;}
case 27:
{
																				  awsComponentNode *win = new awsComponentNode(new scfString(yyvsp[-3].str), new scfString("Default"));
																				  ((awsKeyContainer*)win)->Consume(&wkcont);
																				  ((awsPrefManager *)((awsManager *)windowmgr)->GetPrefMgr())->AddWindowDef(win);
																				;
    break;}
case 28:
{ yyval.key = new awsStringKey(new scfString(yyvsp[-2].str), new scfString(yyvsp[0].str)); ;
    break;}
case 29:
{ yyval.key = new awsRGBKey(new scfString(yyvsp[-6].str), yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val);           ;
    break;}
case 30:
{ yyval.key = new awsIntKey(new scfString(yyvsp[-2].str), yyvsp[0].val);					 ;
    break;}
case 31:
{ yyval.key = new awsPointKey(new scfString(yyvsp[-6].str), csPoint(yyvsp[-3].val, yyvsp[-1].val));    ;
    break;}
case 32:
{ yyval.key = new awsRectKey(new scfString(yyvsp[-12].str), csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val));     ;
    break;}
case 33:
{ skcont.Add(yyvsp[0].key); ;
    break;}
case 34:
{ skcont.Add(yyvsp[0].key); ;
    break;}
case 35:
{ awsSkinNode *skin = new awsSkinNode(new scfString(yyvsp[-3].str));
										                      ((awsKeyContainer*)skin)->Consume(&skcont);
															  ((awsPrefManager *)((awsManager*)windowmgr)->GetPrefMgr())->AddSkinDef(skin);
															 ;
    break;}
case 36:
{
																				  if (((awsManager *)windowmgr)->GetPrefMgr()->ConstantExists(yyvsp[0].str))
																				    {
																					  yyval.val = ((awsManager *)windowmgr)->GetPrefMgr()->GetConstantValue(yyvsp[0].str);
																					}
																				   else
																				    {
																					  printf("\taws window definition error: %s is not a registered constant.\n", yyvsp[0].str);
																					  yyval.val=0;
																					}
																				;
    break;}
case 37:
{ yyval.val = yyvsp[0].val;      ;
    break;}
case 38:
{ yyval.val = yyvsp[0].val;      ;
    break;}
case 39:
{ yyval.val = yyvsp[-2].val + yyvsp[0].val; ;
    break;}
case 40:
{ yyval.val = yyvsp[-2].val - yyvsp[0].val; ;
    break;}
case 41:
{ yyval.val = yyvsp[-2].val * yyvsp[0].val; ;
    break;}
case 42:
{ yyval.val = yyvsp[-2].val / yyvsp[0].val; ;
    break;}
case 43:
{ yyval.val = -yyvsp[0].val;     ;
    break;}
case 44:
{ yyval.val = yyvsp[-1].val;      ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */


  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
#ifndef YYSTACK_USE_ALLOCA
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
#endif
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
#ifndef YYSTACK_USE_ALLOCA
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
#endif    
  return 1;
}


int
awserror(char *s)
{
 printf("\taws definition parse error(%i): %s\n", awslineno, s);
 return 0;
}

void
mapsourcetosink(iAws *windowmgr, unsigned long signal, iString *sinkname, iString *triggername)
{

  if (windowmgr==NULL)
  {
    printf("\tinternal error: window manager parameter is null in mapsourcetosink!\n");
	return;
  }

  iAwsSink *sink = ((awsManager *)windowmgr)->GetSinkMgr()->FindSink(sinkname->GetData());

  if (sink==NULL)
  {
    printf("\tcould not find sink \"%s\" referred to in connection map.\n", sinkname->GetData());
	return;
  }

  unsigned long trigger = sink->GetTriggerID(triggername->GetData());

  kcont[klevel].Add(new awsConnectionKey(new scfString("connection"), sink, trigger, signal));
}
