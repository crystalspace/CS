/* A Bison parser, made from plugins/aws/skinlang.bsn
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse awsparse
#define yylex awslex
#define yyerror awserror
#define yylval awslval
#define yychar awschar
#define yydebug awsdebug
#define yynerrs awsnerrs
# define	TOKEN_NUM	257
# define	TOKEN_STR	258
# define	TOKEN_ATTR	259
# define	TOKEN_SKIN	260
# define	TOKEN_FOR	261
# define	TOKEN_WINDOW	262
# define	TOKEN_FROM	263
# define	TOKEN_COMPONENT	264
# define	TOKEN_CONNECT	265
# define	TOKEN_IS	266
# define	NEG	267



#include "cssysdef.h"
#include "csgeom/csrect.h"
#include "csutil/scfstr.h"
#include "csutil/csdllist.h"
#include "aws.h"
#include "awsprefs.h"
#include "awsparser.h"
#include <stdio.h>


#ifndef YYSTYPE
typedef union {
  char   *str;			/* For returning titles and handles to items. */
  int     val;			/* For returning numbers                      */
  csRect *rect;			/* For returning rectangular regions          */
  awsKey *key;     		/* For returning keys to various definition items */
  awsComponentNode *comp;	/* for returning windows		      */
  awsKeyContainer *keycont;	/* for returning KeyContainers		      */
  awsSkinNode *skin;		/* for returning skins			      */
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif


extern int awslex(YYSTYPE *awslval);
extern int awserror(char *s);
extern int awslineno;

/// This is the parser parameter
#define YYPARSE_PARAM windowmgr


#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		130
#define	YYFLAG		-32768
#define	YYNTBASE	28

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 267 ? yytranslate[x] : 42)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      20,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      25,    26,    16,    15,    24,    14,     2,    17,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    23,     2,
       2,    13,    22,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    19,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    27,     2,    21,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    18
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     1,     4,     6,     8,    10,    13,    21,    23,
      26,    30,    34,    42,    56,    61,    69,    75,    77,    80,
      84,    88,   102,   107,   115,   117,   120,   126,   130,   138,
     142,   150,   164,   166,   169,   175,   177,   179,   181,   185,
     189,   193,   197,   200
};
static const short yyrhs[] =
{
      -1,    28,    29,     0,    20,     0,    39,     0,    36,     0,
       1,    21,     0,    41,    14,    22,     5,    23,    23,     5,
       0,    30,     0,    31,    30,     0,     5,    23,     4,     0,
       5,    23,    41,     0,     5,    23,    41,    24,    41,    24,
      41,     0,     5,    23,    25,    41,    24,    41,    26,    14,
      25,    41,    24,    41,    26,     0,    11,    27,    31,    21,
       0,    10,     4,    12,     4,    27,    33,    21,     0,     8,
       4,    27,    33,    21,     0,    32,     0,    33,    32,     0,
       5,    23,     4,     0,     5,    23,    41,     0,     5,    23,
      25,    41,    24,    41,    26,    14,    25,    41,    24,    41,
      26,     0,    11,    27,    31,    21,     0,    10,     4,    12,
       4,    27,    33,    21,     0,    34,     0,    35,    34,     0,
       8,     4,    27,    35,    21,     0,     5,    23,     4,     0,
       5,    23,    41,    24,    41,    24,    41,     0,     5,    23,
      41,     0,     5,    23,    25,    41,    24,    41,    26,     0,
       5,    23,    25,    41,    24,    41,    26,    14,    25,    41,
      24,    41,    26,     0,    37,     0,    38,    37,     0,     6,
       4,    27,    38,    21,     0,     5,     0,     3,     0,    40,
       0,    41,    15,    41,     0,    41,    14,    41,     0,    41,
      16,    41,     0,    41,    17,    41,     0,    14,    41,     0,
      25,    41,    26,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,    77,    78,    81,    82,    83,    84,    91,    96,   102,
     109,   112,   114,   116,   118,   126,   134,   144,   150,   158,
     161,   163,   165,   173,   183,   189,   196,   211,   214,   216,
     218,   220,   224,   230,   237,   251,   260,   261,   262,   263,
     264,   265,   266,   267
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "TOKEN_NUM", "TOKEN_STR", "TOKEN_ATTR", 
  "TOKEN_SKIN", "TOKEN_FOR", "TOKEN_WINDOW", "TOKEN_FROM", 
  "TOKEN_COMPONENT", "TOKEN_CONNECT", "TOKEN_IS", "'='", "'-'", "'+'", 
  "'*'", "'/'", "NEG", "'^'", "'\\n'", "'}'", "'>'", "':'", "','", "'('", 
  "')'", "'{'", "input", "line", "connection_item", 
  "connection_item_list", "component_item", "component_item_list", 
  "window_item", "window_item_list", "window", "skin_item", 
  "skin_item_list", "skin", "constant_item", "exp", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    28,    28,    29,    29,    29,    29,    30,    31,    31,
      32,    32,    32,    32,    32,    32,    32,    33,    33,    34,
      34,    34,    34,    34,    35,    35,    36,    37,    37,    37,
      37,    37,    38,    38,    39,    40,    41,    41,    41,    41,
      41,    41,    41,    41
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     0,     2,     1,     1,     1,     2,     7,     1,     2,
       3,     3,     7,    13,     4,     7,     5,     1,     2,     3,
       3,    13,     4,     7,     1,     2,     5,     3,     7,     3,
       7,    13,     1,     2,     5,     1,     1,     1,     3,     3,
       3,     3,     2,     3
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       1,     0,     0,     0,     0,     3,     2,     5,     4,     6,
       0,     0,     0,     0,     0,    32,     0,     0,     0,     0,
      24,     0,     0,    34,    33,     0,     0,     0,    26,    25,
      36,    27,    35,     0,     0,    37,    29,    19,     0,    20,
       0,     0,     8,     0,     0,    42,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     9,     0,     0,    43,
      39,    38,    40,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    17,     0,     0,    30,    28,
       0,     0,     0,     0,     0,    23,    18,     0,     0,     0,
      10,     0,    11,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     7,     0,     0,     0,     0,    16,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      15,    31,    21,     0,     0,     0,     0,     0,    13,     0,
       0
};

static const short yydefgoto[] =
{
       1,     6,    42,    43,    75,    76,    20,    21,     7,    15,
      16,     8,    35,    44
};

static const short yypact[] =
{
  -32768,    50,   -11,    14,    28,-32768,-32768,-32768,-32768,-32768,
       7,    10,    36,     4,    22,-32768,    12,    32,    57,    38,
  -32768,   113,    17,-32768,-32768,    35,    54,    -1,-32768,-32768,
  -32768,-32768,-32768,    -1,    -1,-32768,   187,-32768,    -1,   245,
      58,    -1,-32768,    43,   249,-32768,   111,    -1,    -1,    -1,
      -1,    -1,   124,    40,   141,-32768,-32768,    49,    -1,-32768,
      -9,    -9,-32768,-32768,   191,    -1,   247,    67,   145,    -1,
     149,    56,    69,    77,    64,-32768,    72,    61,    85,   245,
      89,    83,    79,    95,    -1,-32768,-32768,    86,    88,    96,
  -32768,    -1,   202,   247,   106,    91,   110,    -1,    -1,   128,
      -1,    90,   102,-32768,-32768,   206,   217,    -1,   221,-32768,
     247,    -1,    -1,   162,    -1,   109,   166,   170,   108,   245,
  -32768,-32768,-32768,   107,    -1,   232,    -1,   183,-32768,   131,
  -32768
};

static const short yypgoto[] =
{
  -32768,-32768,   -42,    52,   -71,   -87,   112,-32768,-32768,   130,
  -32768,-32768,-32768,   -22
};


#define	YYLAST		266


static const short yytable[] =
{
      36,    56,    30,    39,    32,    86,   101,    49,    50,    17,
       9,    45,    46,    33,    18,    19,    52,    14,    10,    54,
      30,    31,    32,   115,    41,    60,    61,    62,    63,    64,
      86,    33,    11,    23,    12,    60,    68,    13,    30,    37,
      32,    14,    34,    70,    86,    22,    30,    79,    32,    33,
     129,     2,    30,    56,    32,    25,     3,    33,     4,    92,
      38,    26,    53,    33,    55,    27,    40,    66,    41,    99,
       5,    67,    77,    82,    41,   105,   106,    71,   108,    81,
      72,    83,    73,    74,    87,   113,    30,    90,    32,   116,
     117,    84,   119,    85,    30,    71,    32,    33,    72,    88,
      73,    74,   125,    89,   127,    33,    93,    94,    91,    96,
     102,   109,   103,    97,    71,   104,    41,    72,    17,    73,
      74,    98,   123,    18,    19,    47,    48,    49,    50,   110,
     120,   130,   124,    29,    28,    58,    95,    59,    47,    48,
      49,    50,    47,    48,    49,    50,    24,     0,    65,     0,
      59,     0,   107,     0,    59,    47,    48,    49,    50,    47,
      48,    49,    50,    47,    48,    49,    50,    59,     0,     0,
       0,    78,     0,     0,     0,    80,    47,    48,    49,    50,
      47,    48,    49,    50,    47,    48,    49,    50,   118,     0,
       0,     0,   121,     0,     0,     0,   122,    47,    48,    49,
      50,    47,    48,    49,    50,    47,    48,    49,    50,   128,
       0,    51,     0,     0,     0,    69,    47,    48,    49,    50,
      47,    48,    49,    50,     0,     0,   100,     0,     0,     0,
     111,    47,    48,    49,    50,    47,    48,    49,    50,     0,
       0,   112,     0,     0,     0,   114,    47,    48,    49,    50,
       0,     0,    71,     0,     0,    72,   126,    73,    74,    47,
      48,    49,    50,    57,    48,    49,    50
};

static const short yycheck[] =
{
      22,    43,     3,    25,     5,    76,    93,    16,    17,     5,
      21,    33,    34,    14,    10,    11,    38,     5,     4,    41,
       3,     4,     5,   110,    25,    47,    48,    49,    50,    51,
     101,    14,     4,    21,    27,    57,    58,    27,     3,     4,
       5,     5,    25,    65,   115,    23,     3,    69,     5,    14,
       0,     1,     3,    95,     5,    23,     6,    14,     8,    81,
      25,     4,     4,    14,    21,    27,    12,    27,    25,    91,
      20,    22,     5,     4,    25,    97,    98,     5,   100,    23,
       8,     4,    10,    11,    23,   107,     3,     4,     5,   111,
     112,    27,   114,    21,     3,     5,     5,    14,     8,    14,
      10,    11,   124,    14,   126,    14,    27,    12,    25,    23,
       4,    21,    21,    25,     5,     5,    25,     8,     5,    10,
      11,    25,    14,    10,    11,    14,    15,    16,    17,    27,
      21,     0,    25,    21,    21,    24,    84,    26,    14,    15,
      16,    17,    14,    15,    16,    17,    16,    -1,    24,    -1,
      26,    -1,    24,    -1,    26,    14,    15,    16,    17,    14,
      15,    16,    17,    14,    15,    16,    17,    26,    -1,    -1,
      -1,    26,    -1,    -1,    -1,    26,    14,    15,    16,    17,
      14,    15,    16,    17,    14,    15,    16,    17,    26,    -1,
      -1,    -1,    26,    -1,    -1,    -1,    26,    14,    15,    16,
      17,    14,    15,    16,    17,    14,    15,    16,    17,    26,
      -1,    24,    -1,    -1,    -1,    24,    14,    15,    16,    17,
      14,    15,    16,    17,    -1,    -1,    24,    -1,    -1,    -1,
      24,    14,    15,    16,    17,    14,    15,    16,    17,    -1,
      -1,    24,    -1,    -1,    -1,    24,    14,    15,    16,    17,
      -1,    -1,     5,    -1,    -1,     8,    24,    10,    11,    14,
      15,    16,    17,    14,    15,    16,    17
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

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

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
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
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
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
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 4:
{ static_awsparser->AddGlobalSkinDef(yyvsp[0].skin); ;
    break;}
case 5:
{ static_awsparser->AddGlobalWindowDef(yyvsp[0].comp); ;
    break;}
case 6:
{ yyerrok;      ;
    break;}
case 7:
{ yyval.key = static_awsparser->MapSourceToSink (yyvsp[-6].val, yyvsp[-3].str, yyvsp[0].str); ;
    break;}
case 8:
{ awsKeyContainer* kc = new awsKeyContainer;
		  if (yyvsp[0].key) kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;
    break;}
case 9:
{ awsKeyContainer* kc = yyvsp[-1].keycont;
		  if (yyvsp[0].key) kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;
    break;}
case 10:
{  yyval.key = new awsStringKey(yyvsp[-2].str, yyvsp[0].str); ;
    break;}
case 11:
{  yyval.key = new awsIntKey(yyvsp[-2].str, yyvsp[0].val); ;
    break;}
case 12:
{  yyval.key = new awsRGBKey(yyvsp[-6].str, yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val); ;
    break;}
case 13:
{  yyval.key = new awsRectKey(yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); ;
    break;}
case 14:
{ awsConnectionNode *cn = new awsConnectionNode();
              iAwsKeyContainer* kc = SCF_QUERY_INTERFACE(yyvsp[-1].keycont, iAwsKeyContainer);
		  cn->Consume (kc);
              kc->DecRef();
		  delete yyvsp[-1].keycont;
		  yyval.key = cn;
		;
    break;}
case 15:
{ awsComponentNode *cn = new awsComponentNode(yyvsp[-5].str, yyvsp[-3].str);
              iAwsKeyContainer* kc = SCF_QUERY_INTERFACE(yyvsp[-1].keycont, iAwsKeyContainer);
		  cn->Consume(kc);
              kc->DecRef();
		  delete yyvsp[-1].keycont;
		  yyval.key = cn;
		;
    break;}
case 16:
{ awsComponentNode *cn = new awsComponentNode(yyvsp[-3].str, "Window");
              iAwsKeyContainer* kc = SCF_QUERY_INTERFACE(yyvsp[-1].keycont, iAwsKeyContainer);
		  cn->Consume(kc);
              kc->DecRef();
		  delete yyvsp[-1].keycont;
		  yyval.key = cn;
		;
    break;}
case 17:
{ awsKeyContainer* keycontainer = new awsKeyContainer;
		  keycontainer->Add(yyvsp[0].key);
		  yyval.keycont = keycontainer;
		;
    break;}
case 18:
{ awsKeyContainer* keycontainer = yyvsp[-1].keycont;
		  keycontainer->Add(yyvsp[0].key);
		  yyval.keycont = keycontainer;
		;
    break;}
case 19:
{ yyval.key = new awsStringKey(yyvsp[-2].str, yyvsp[0].str); ;
    break;}
case 20:
{ yyval.key = new awsIntKey(yyvsp[-2].str, yyvsp[0].val); ;
    break;}
case 21:
{ yyval.key = new awsRectKey(yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); ;
    break;}
case 22:
{ awsConnectionNode *cn = new awsConnectionNode();
              iAwsKeyContainer* kc = SCF_QUERY_INTERFACE(yyvsp[-1].keycont, iAwsKeyContainer);
		  cn->Consume(kc);
              kc->DecRef();
		  delete yyvsp[-1].keycont;
		  yyval.key=cn;
		;
    break;}
case 23:
{ awsComponentNode *cn = new awsComponentNode(yyvsp[-5].str, yyvsp[-3].str);
              iAwsKeyContainer* kc = SCF_QUERY_INTERFACE(yyvsp[-1].keycont, iAwsKeyContainer);
		  cn->Consume(kc);
              kc->DecRef();
		  delete yyvsp[-1].keycont;
		  yyval.key=cn;
		;
    break;}
case 24:
{ awsKeyContainer* cnt = new awsKeyContainer;
		cnt->Add(yyvsp[0].key);
		yyval.keycont = cnt;
	      ;
    break;}
case 25:
{ awsKeyContainer* cnt = yyvsp[-1].keycont;
	        cnt->Add(yyvsp[0].key);
	        yyval.keycont = cnt;
	      ;
    break;}
case 26:
{ awsComponentNode *win = new awsComponentNode(yyvsp[-3].str, "Default");
              iAwsKeyContainer* kc = SCF_QUERY_INTERFACE(yyvsp[-1].keycont, iAwsKeyContainer);
		  win->Consume(kc);
              kc->DecRef();
		  delete yyvsp[-1].keycont;
		  yyval.comp = win;
		;
    break;}
case 27:
{ yyval.key = new awsStringKey(yyvsp[-2].str, yyvsp[0].str); ;
    break;}
case 28:
{ yyval.key = new awsRGBKey(yyvsp[-6].str, yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val); ;
    break;}
case 29:
{ yyval.key = new awsIntKey(yyvsp[-2].str, yyvsp[0].val); ;
    break;}
case 30:
{ yyval.key = new awsPointKey(yyvsp[-6].str, csPoint(yyvsp[-3].val, yyvsp[-1].val)); ;
    break;}
case 31:
{ yyval.key = new awsRectKey(yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); ;
    break;}
case 32:
{ awsKeyContainer* kc = new awsKeyContainer;
		  kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;
    break;}
case 33:
{ awsKeyContainer* kc = yyvsp[-1].keycont;
		  kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;
    break;}
case 34:
{ awsSkinNode *skin = new awsSkinNode(yyvsp[-3].str);
              iAwsKeyContainer* kc = SCF_QUERY_INTERFACE(yyvsp[-1].keycont, iAwsKeyContainer);
              skin->Consume(kc);
              kc->DecRef();
              delete yyvsp[-1].keycont;
		  yyval.skin = skin;
		;
    break;}
case 35:
{ int v = 0;
		  if (!static_awsparser->GetConstantValue(yyvsp[0].str, v))
		    static_awsparser->ReportError ("Constant %s is not defined.", yyvsp[0].str);
		  yyval.val = v;
		;
    break;}
case 36:
{ yyval.val = yyvsp[0].val; ;
    break;}
case 37:
{ yyval.val = yyvsp[0].val; ;
    break;}
case 38:
{ yyval.val = yyvsp[-2].val + yyvsp[0].val; ;
    break;}
case 39:
{ yyval.val = yyvsp[-2].val - yyvsp[0].val; ;
    break;}
case 40:
{ yyval.val = yyvsp[-2].val * yyvsp[0].val; ;
    break;}
case 41:
{ yyval.val = yyvsp[-2].val / yyvsp[0].val; ;
    break;}
case 42:
{ yyval.val = -yyvsp[0].val; ;
    break;}
case 43:
{ yyval.val = yyvsp[-1].val; ;
    break;}
}



  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
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

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


int
awserror(char *s)
{
  static_awsparser->ReportError (s);
  return 0;
}

