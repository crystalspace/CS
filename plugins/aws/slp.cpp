/* A Bison parser, made from skinlang.bsn
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
#include <stdio.h>


#ifndef YYSTYPE
typedef union {
  char   *str;     /* For returning titles and handles to items. */
  int     val;     /* For returning numbers                      */
  csRect *rect;    /* For returning rectangular regions          */
  awsKey *key;     /* For returning keys to various definition items */
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif


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


#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		133
#define	YYFLAG		-32768
#define	YYNTBASE	28

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 267 ? yytranslate[x] : 45)

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
      26,    30,    34,    42,    56,    57,    63,    64,    73,    74,
      81,    83,    86,    90,    94,   108,   113,   121,   123,   126,
     132,   136,   144,   148,   156,   170,   172,   175,   181,   183,
     185,   187,   191,   195,   199,   203,   206
};
static const short yyrhs[] =
{
      -1,    28,    29,     0,    20,     0,    42,     0,    39,     0,
       1,    21,     0,    44,    14,    22,     5,    23,    23,     5,
       0,    30,     0,    31,    30,     0,     5,    23,     4,     0,
       5,    23,    44,     0,     5,    23,    44,    24,    44,    24,
      44,     0,     5,    23,    25,    44,    24,    44,    26,    14,
      25,    44,    24,    44,    26,     0,     0,    11,    33,    27,
      31,    21,     0,     0,    10,     4,    12,     4,    27,    34,
      36,    21,     0,     0,     8,     4,    27,    35,    36,    21,
       0,    32,     0,    36,    32,     0,     5,    23,     4,     0,
       5,    23,    44,     0,     5,    23,    25,    44,    24,    44,
      26,    14,    25,    44,    24,    44,    26,     0,    11,    27,
      31,    21,     0,    10,     4,    12,     4,    27,    36,    21,
       0,    37,     0,    38,    37,     0,     8,     4,    27,    38,
      21,     0,     5,    23,     4,     0,     5,    23,    44,    24,
      44,    24,    44,     0,     5,    23,    44,     0,     5,    23,
      25,    44,    24,    44,    26,     0,     5,    23,    25,    44,
      24,    44,    26,    14,    25,    44,    24,    44,    26,     0,
      40,     0,    41,    40,     0,     6,     4,    27,    41,    21,
       0,     5,     0,     3,     0,    43,     0,    44,    15,    44,
       0,    44,    14,    44,     0,    44,    16,    44,     0,    44,
      17,    44,     0,    14,    44,     0,    25,    44,    26,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,    81,    82,    85,    86,    87,    88,    96,    99,   100,
     103,   104,   105,   106,   107,   107,   113,   113,   121,   121,
     130,   131,   135,   136,   137,   138,   142,   149,   150,   153,
     165,   166,   167,   168,   169,   172,   173,   176,   185,   199,
     200,   201,   202,   203,   204,   205,   206
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
  "connection_item_list", "component_item", "@1", "@2", "@3", 
  "component_item_list", "window_item", "window_item_list", "window", 
  "skin_item", "skin_item_list", "skin", "constant_item", "exp", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    28,    28,    29,    29,    29,    29,    30,    31,    31,
      32,    32,    32,    32,    33,    32,    34,    32,    35,    32,
      36,    36,    37,    37,    37,    37,    37,    38,    38,    39,
      40,    40,    40,    40,    40,    41,    41,    42,    43,    44,
      44,    44,    44,    44,    44,    44,    44
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     0,     2,     1,     1,     1,     2,     7,     1,     2,
       3,     3,     7,    13,     0,     5,     0,     8,     0,     6,
       1,     2,     3,     3,    13,     4,     7,     1,     2,     5,
       3,     7,     3,     7,    13,     1,     2,     5,     1,     1,
       1,     3,     3,     3,     3,     2,     3
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       1,     0,     0,     0,     0,     3,     2,     5,     4,     6,
       0,     0,     0,     0,     0,    35,     0,     0,     0,     0,
      27,     0,     0,    37,    36,     0,     0,     0,    29,    28,
      39,    30,    38,     0,     0,    40,    32,    22,     0,    23,
       0,     0,     8,     0,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,     9,     0,     0,    46,
      42,    41,    43,    44,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    20,     0,     0,    33,    31,
       0,     0,     0,     0,     0,    26,    21,     0,     0,     0,
      10,     0,    11,    18,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     7,     0,     0,     0,     0,     0,
      16,    15,     0,     0,     0,     0,    19,     0,     0,     0,
       0,    12,     0,    34,    24,     0,    17,     0,     0,     0,
       0,    13,     0,     0
};

static const short yydefgoto[] =
{
       1,     6,    42,    43,    75,    84,   117,   101,    76,    20,
      21,     7,    15,    16,     8,    35,    44
};

static const short yypact[] =
{
  -32768,    94,   -15,     6,     9,-32768,-32768,-32768,-32768,-32768,
      18,    26,    29,     4,    42,-32768,    12,    45,    78,    56,
  -32768,   105,    17,-32768,-32768,    35,    75,    74,-32768,-32768,
  -32768,-32768,-32768,    74,    74,-32768,   182,-32768,    74,   240,
      82,    74,-32768,    27,   244,-32768,   106,    74,    74,    74,
      74,    74,   119,    62,   136,-32768,-32768,    41,    74,-32768,
       7,     7,-32768,-32768,   186,    74,   242,    91,   140,    74,
     144,    81,    93,    97,-32768,-32768,    -3,    88,    84,   240,
      99,    67,    85,   112,    90,-32768,-32768,    95,   100,   102,
  -32768,    74,   197,-32768,   124,    74,   126,    74,    74,   123,
      74,   242,   114,    59,-32768,   201,   212,    74,   216,    46,
  -32768,-32768,    74,    74,   157,    74,-32768,   242,   161,   165,
     115,   240,    98,-32768,-32768,   117,-32768,    74,   227,    74,
     178,-32768,   146,-32768
};

static const short yypgoto[] =
{
  -32768,-32768,   -42,    49,   -72,-32768,-32768,-32768,   -43,   127,
  -32768,-32768,   147,-32768,-32768,-32768,   -22
};


#define	YYLAST		261


static const short yytable[] =
{
      36,    56,    71,    39,    86,    72,     9,    73,    74,    17,
      10,    45,    46,    11,    18,    19,    52,    14,    85,    54,
      30,    31,    32,    49,    50,    60,    61,    62,    63,    64,
      30,    33,    32,    23,    14,    60,    68,    86,    30,    37,
      32,    33,    34,    70,    30,    12,    32,    79,    55,    33,
      86,    71,    41,    13,    72,    33,    73,    74,   109,    92,
      38,    56,    30,    67,    32,    22,    41,   116,    25,    99,
      30,    90,    32,    33,   122,   105,   106,    30,   108,    32,
     111,    33,    26,    27,    41,   114,    53,    40,    33,    66,
     118,   119,    91,   121,   132,     2,    77,    82,    88,    41,
       3,    83,     4,    71,    81,   128,    72,   130,    73,    74,
      17,    87,    93,    89,     5,    18,    19,    95,    96,   126,
      47,    48,    49,    50,    94,    97,    28,    98,   102,   125,
      58,   104,    59,    47,    48,    49,    50,    47,    48,    49,
      50,   110,   127,    65,   103,    59,   133,   107,    29,    59,
      47,    48,    49,    50,    47,    48,    49,    50,    47,    48,
      49,    50,    59,    24,     0,     0,    78,     0,     0,     0,
      80,    47,    48,    49,    50,    47,    48,    49,    50,    47,
      48,    49,    50,   120,     0,     0,     0,   123,     0,     0,
       0,   124,    47,    48,    49,    50,    47,    48,    49,    50,
      47,    48,    49,    50,   131,     0,    51,     0,     0,     0,
      69,    47,    48,    49,    50,    47,    48,    49,    50,     0,
       0,   100,     0,     0,     0,   112,    47,    48,    49,    50,
      47,    48,    49,    50,     0,     0,   113,     0,     0,     0,
     115,    47,    48,    49,    50,     0,     0,    71,     0,     0,
      72,   129,    73,    74,    47,    48,    49,    50,    57,    48,
      49,    50
};

static const short yycheck[] =
{
      22,    43,     5,    25,    76,     8,    21,    10,    11,     5,
       4,    33,    34,     4,    10,    11,    38,     5,    21,    41,
       3,     4,     5,    16,    17,    47,    48,    49,    50,    51,
       3,    14,     5,    21,     5,    57,    58,   109,     3,     4,
       5,    14,    25,    65,     3,    27,     5,    69,    21,    14,
     122,     5,    25,    27,     8,    14,    10,    11,   101,    81,
      25,   103,     3,    22,     5,    23,    25,    21,    23,    91,
       3,     4,     5,    14,   117,    97,    98,     3,   100,     5,
      21,    14,     4,    27,    25,   107,     4,    12,    14,    27,
     112,   113,    25,   115,     0,     1,     5,     4,    14,    25,
       6,     4,     8,     5,    23,   127,     8,   129,    10,    11,
       5,    23,    27,    14,    20,    10,    11,    27,    23,    21,
      14,    15,    16,    17,    12,    25,    21,    25,     4,    14,
      24,     5,    26,    14,    15,    16,    17,    14,    15,    16,
      17,    27,    25,    24,    95,    26,     0,    24,    21,    26,
      14,    15,    16,    17,    14,    15,    16,    17,    14,    15,
      16,    17,    26,    16,    -1,    -1,    26,    -1,    -1,    -1,
      26,    14,    15,    16,    17,    14,    15,    16,    17,    14,
      15,    16,    17,    26,    -1,    -1,    -1,    26,    -1,    -1,
      -1,    26,    14,    15,    16,    17,    14,    15,    16,    17,
      14,    15,    16,    17,    26,    -1,    24,    -1,    -1,    -1,
      24,    14,    15,    16,    17,    14,    15,    16,    17,    -1,
      -1,    24,    -1,    -1,    -1,    24,    14,    15,    16,    17,
      14,    15,    16,    17,    -1,    -1,    24,    -1,    -1,    -1,
      24,    14,    15,    16,    17,    -1,    -1,     5,    -1,    -1,
       8,    24,    10,    11,    14,    15,    16,    17,    14,    15,
      16,    17
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
{ ++klevel; /* go down a level (mid-action rule) */ ;
    break;}
case 19:
{ awsComponentNode *cn = new awsComponentNode(new scfString(yyvsp[-4].str), new scfString("Window"));
																				  ((awsKeyContainer*)cn)->Consume(&(kcont[klevel]));
																				  --klevel;  /* go up a level in recursion */
																				  kcont[klevel].Add(cn);
																				;
    break;}
case 20:
{ /*empty*/ ;
    break;}
case 21:
{ /*empty*/ ;
    break;}
case 22:
{ yyval.key = new awsStringKey(new scfString(yyvsp[-2].str), new scfString(yyvsp[0].str));          ;
    break;}
case 23:
{ yyval.key = new awsIntKey(new scfString(yyvsp[-2].str), yyvsp[0].val);                            ;
    break;}
case 24:
{ yyval.key = new awsRectKey(new scfString(yyvsp[-12].str), csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val));     ;
    break;}
case 25:
{ awsConnectionNode *cn = new awsConnectionNode();
																				  ((awsKeyContainer*)cn)->Consume(&(kcont[klevel]));
																				  yyval.key=cn;
			  																	;
    break;}
case 26:
{ awsComponentNode *cn = new awsComponentNode(new scfString(yyvsp[-5].str), new scfString(yyvsp[-3].str));
																				  ((awsKeyContainer*)cn)->Consume(&(kcont[klevel]));
																				   yyval.key=cn;
																				;
    break;}
case 27:
{ wkcont.Add(yyvsp[0].key); ;
    break;}
case 28:
{ wkcont.Add(yyvsp[0].key); ;
    break;}
case 29:
{
																				  awsComponentNode *win = new awsComponentNode(new scfString(yyvsp[-3].str), new scfString("Default"));
																				  ((awsKeyContainer*)win)->Consume(&wkcont);
																				  ((awsPrefManager *)((awsManager *)windowmgr)->GetPrefMgr())->AddWindowDef(win);
																				;
    break;}
case 30:
{ yyval.key = new awsStringKey(new scfString(yyvsp[-2].str), new scfString(yyvsp[0].str)); ;
    break;}
case 31:
{ yyval.key = new awsRGBKey(new scfString(yyvsp[-6].str), yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val);           ;
    break;}
case 32:
{ yyval.key = new awsIntKey(new scfString(yyvsp[-2].str), yyvsp[0].val);					 ;
    break;}
case 33:
{ yyval.key = new awsPointKey(new scfString(yyvsp[-6].str), csPoint(yyvsp[-3].val, yyvsp[-1].val));    ;
    break;}
case 34:
{ yyval.key = new awsRectKey(new scfString(yyvsp[-12].str), csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val));     ;
    break;}
case 35:
{ skcont.Add(yyvsp[0].key); ;
    break;}
case 36:
{ skcont.Add(yyvsp[0].key); ;
    break;}
case 37:
{ awsSkinNode *skin = new awsSkinNode(new scfString(yyvsp[-3].str));
										                      ((awsKeyContainer*)skin)->Consume(&skcont);
															  ((awsPrefManager *)((awsManager*)windowmgr)->GetPrefMgr())->AddSkinDef(skin);
															 ;
    break;}
case 38:
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
case 39:
{ yyval.val = yyvsp[0].val;      ;
    break;}
case 40:
{ yyval.val = yyvsp[0].val;      ;
    break;}
case 41:
{ yyval.val = yyvsp[-2].val + yyvsp[0].val; ;
    break;}
case 42:
{ yyval.val = yyvsp[-2].val - yyvsp[0].val; ;
    break;}
case 43:
{ yyval.val = yyvsp[-2].val * yyvsp[0].val; ;
    break;}
case 44:
{ yyval.val = yyvsp[-2].val / yyvsp[0].val; ;
    break;}
case 45:
{ yyval.val = -yyvsp[0].val;     ;
    break;}
case 46:
{ yyval.val = yyvsp[-1].val;      ;
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
