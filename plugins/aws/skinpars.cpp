/* A Bison parser, made from plugins/aws/skinpars.yy
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
# define	TOKEN_FLOAT	258
# define	TOKEN_STR	259
# define	TOKEN_ATTR	260
# define	TOKEN_SKIN	261
# define	TOKEN_FOR	262
# define	TOKEN_WINDOW	263
# define	TOKEN_FROM	264
# define	TOKEN_COMPONENT	265
# define	TOKEN_CONNECT	266
# define	TOKEN_IS	267
# define	NEG	268

#line 12 "plugins/aws/skinpars.yy"


#include "cssysdef.h"
#include "csgeom/csrect.h"
#include "csutil/scfstr.h"
#include "aws.h"
#include "awsprefs.h"
#include "awsparser.h"
#include <stdio.h>

// bison 1.875 outputs wrong code (an attribute after a label is not allowed in
// g++
#define __attribute__(x)


#line 31 "plugins/aws/skinpars.yy"
#ifndef YYSTYPE
typedef union {
  char   *str;			/* For returning titles and handles to items. */
  int     val;			/* For returning integer numbers              */
  float  fval;			/* For returning non-integer numbers          */
  csRect *rect;			/* For returning rectangular regions          */
  awsKey *key;     		/* For returning keys to various definition items */
  awsComponentNode *comp;	/* for returning windows		      */
  awsKeyContainer *keycont;	/* for returning KeyContainers		      */
  awsSkinNode *skin;		/* for returning skins			      */
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#line 43 "plugins/aws/skinpars.yy"


extern int awslex(YYSTYPE *awslval);
extern int awserror(char *s);
extern int awslineno;

/// This is the parser parameter
#define YYPARSE_PARAM windowmgr

#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		149
#define	YYFLAG		-32768
#define	YYNTBASE	29

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 268 ? yytranslate[x] : 44)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      21,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      26,    27,    17,    16,    25,    15,     2,    18,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    24,     2,
       2,    14,    23,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    20,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    28,     2,    22,     2,     2,     2,     2,
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
       6,     7,     8,     9,    10,    11,    12,    13,    19
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     1,     4,     6,     8,    10,    13,    21,    23,
      26,    30,    34,    38,    46,    60,    65,    73,    79,    81,
      84,    88,    92,    96,   110,   115,   123,   125,   128,   134,
     138,   146,   150,   154,   162,   176,   178,   181,   187,   189,
     191,   195,   199,   203,   207,   210,   214,   216,   218,   222,
     226,   230,   234,   237
};
static const short yyrhs[] =
{
      -1,    29,    30,     0,    21,     0,    40,     0,    37,     0,
       1,    22,     0,    43,    15,    23,     6,    24,    24,     6,
       0,    31,     0,    32,    31,     0,     6,    24,     5,     0,
       6,    24,    42,     0,     6,    24,    43,     0,     6,    24,
      43,    25,    43,    25,    43,     0,     6,    24,    26,    43,
      25,    43,    27,    15,    26,    43,    25,    43,    27,     0,
      12,    28,    32,    22,     0,    11,     5,    13,     5,    28,
      34,    22,     0,     9,     5,    28,    34,    22,     0,    33,
       0,    34,    33,     0,     6,    24,     5,     0,     6,    24,
      42,     0,     6,    24,    43,     0,     6,    24,    26,    43,
      25,    43,    27,    15,    26,    43,    25,    43,    27,     0,
      12,    28,    32,    22,     0,    11,     5,    13,     5,    28,
      34,    22,     0,    35,     0,    36,    35,     0,     9,     5,
      28,    36,    22,     0,     6,    24,     5,     0,     6,    24,
      43,    25,    43,    25,    43,     0,     6,    24,    42,     0,
       6,    24,    43,     0,     6,    24,    26,    43,    25,    43,
      27,     0,     6,    24,    26,    43,    25,    43,    27,    15,
      26,    43,    25,    43,    27,     0,    38,     0,    39,    38,
       0,     7,     5,    28,    39,    22,     0,     6,     0,     4,
       0,    42,    16,    42,     0,    42,    15,    42,     0,    42,
      17,    42,     0,    42,    18,    42,     0,    15,    42,     0,
      26,    42,    27,     0,     3,     0,    41,     0,    43,    16,
      43,     0,    43,    15,    43,     0,    43,    17,    43,     0,
      43,    18,    43,     0,    15,    43,     0,    26,    43,    27,
       0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,    82,    83,    86,    87,    88,    89,    96,   101,   107,
     114,   117,   119,   121,   123,   125,   132,   141,   151,   157,
     165,   168,   170,   172,   174,   181,   192,   198,   205,   220,
     223,   225,   227,   229,   231,   235,   241,   248,   262,   272,
     273,   274,   275,   276,   277,   278,   280,   281,   282,   283,
     284,   285,   286,   287
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "TOKEN_NUM", "TOKEN_FLOAT", "TOKEN_STR", 
  "TOKEN_ATTR", "TOKEN_SKIN", "TOKEN_FOR", "TOKEN_WINDOW", "TOKEN_FROM", 
  "TOKEN_COMPONENT", "TOKEN_CONNECT", "TOKEN_IS", "'='", "'-'", "'+'", 
  "'*'", "'/'", "NEG", "'^'", "'\\n'", "'}'", "'>'", "':'", "','", "'('", 
  "')'", "'{'", "input", "line", "connection_item", 
  "connection_item_list", "component_item", "component_item_list", 
  "window_item", "window_item_list", "window", "skin_item", 
  "skin_item_list", "skin", "constant_item", "expf", "exp", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    29,    29,    30,    30,    30,    30,    31,    32,    32,
      33,    33,    33,    33,    33,    33,    33,    33,    34,    34,
      35,    35,    35,    35,    35,    35,    36,    36,    37,    38,
      38,    38,    38,    38,    38,    39,    39,    40,    41,    42,
      42,    42,    42,    42,    42,    42,    43,    43,    43,    43,
      43,    43,    43,    43
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     0,     2,     1,     1,     1,     2,     7,     1,     2,
       3,     3,     3,     7,    13,     4,     7,     5,     1,     2,
       3,     3,     3,    13,     4,     7,     1,     2,     5,     3,
       7,     3,     3,     7,    13,     1,     2,     5,     1,     1,
       3,     3,     3,     3,     2,     3,     1,     1,     3,     3,
       3,     3,     2,     3
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       1,     0,     0,     0,     0,     3,     2,     5,     4,     6,
       0,     0,     0,     0,     0,    35,     0,     0,     0,     0,
      26,     0,     0,    37,    36,     0,     0,     0,    28,    27,
      46,    39,    29,    38,     0,     0,    47,    31,    32,    20,
       0,    21,    22,     0,     0,     0,     8,     0,     0,     0,
      44,    52,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,     9,     0,    45,
       0,    53,     0,     0,    41,    40,    42,    43,    49,    48,
      50,    51,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    18,     0,     0,    33,    30,     0,     0,
       0,     0,     0,    25,    19,     0,     0,     0,    10,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,     7,     0,     0,     0,     0,    17,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    16,
      34,    23,     0,     0,     0,     0,     0,    14,     0,     0
};

static const short yydefgoto[] =
{
       1,     6,    46,    47,    93,    94,    20,    21,     7,    15,
      16,     8,    36,    52,    48
};

static const short yypact[] =
{
  -32768,    63,    -2,    20,    30,-32768,-32768,-32768,-32768,-32768,
      15,    19,    45,     5,    38,-32768,    28,    42,    62,    64,
  -32768,   131,     4,-32768,-32768,    54,    69,   104,-32768,-32768,
  -32768,-32768,-32768,-32768,    75,    75,-32768,   270,   210,-32768,
      75,   270,   274,    86,   104,   104,-32768,   100,   278,    75,
  -32768,-32768,   160,   130,     0,     0,     0,     0,   104,   104,
     104,   104,   104,   143,    65,   164,-32768,-32768,    26,-32768,
     104,-32768,     0,     0,   -12,   -12,-32768,-32768,    27,    27,
  -32768,-32768,   221,   104,   272,    94,   168,   104,   181,    78,
     107,   109,    89,-32768,    77,    81,   103,   274,   105,    70,
      96,   114,   104,-32768,-32768,   115,   123,   124,-32768,    75,
     270,   225,   272,   149,   110,   146,   104,   104,   147,   104,
     122,   101,-32768,-32768,   236,   240,   104,   251,-32768,   272,
     104,   104,   185,   104,   129,   189,   202,   141,   274,-32768,
  -32768,-32768,   140,   104,   255,   104,   206,-32768,   167,-32768
};

static const short yypgoto[] =
{
  -32768,-32768,   -46,    67,   -92,   -98,   150,-32768,-32768,   157,
  -32768,-32768,-32768,    -1,   -22
};


#define	YYLAST		296


static const short yytable[] =
{
      38,    67,   104,    42,    31,    56,    57,    30,    31,    32,
      33,    17,    51,    53,   120,    72,    18,    19,    63,    34,
       9,    37,    51,    65,    41,    10,    73,    65,   104,    30,
      35,   134,    33,    50,    14,    11,    78,    79,    80,    81,
      82,    44,   104,    12,    60,    61,    78,    13,    86,    85,
      23,    14,    45,    74,    75,    76,    77,    30,    31,    39,
      33,    88,    22,   148,     2,    97,    25,    26,    67,    34,
       3,    50,     4,    30,    31,   108,    33,   111,    30,    31,
      40,    33,    43,    89,     5,    34,    90,   118,    91,    92,
      34,    64,    27,    84,   124,   125,   109,   127,   110,   103,
      95,    49,    99,    30,   132,   105,    33,    30,   135,   136,
      33,   138,   100,    30,   101,    44,    33,   102,   106,    44,
     107,   144,    66,   146,   112,    44,    45,   113,    89,   129,
      45,    90,   122,    91,    92,    89,    45,    17,    90,   115,
      91,    92,    18,    19,   128,    58,    59,    60,    61,   116,
     117,   139,   123,    28,   121,    70,   142,    71,    58,    59,
      60,    61,    58,    59,    60,    61,   143,   149,    83,   114,
      71,    29,   126,    24,    71,    54,    55,    56,    57,    58,
      59,    60,    61,    58,    59,    60,    61,    69,     0,     0,
       0,    71,     0,     0,     0,    96,    58,    59,    60,    61,
      58,    59,    60,    61,    58,    59,    60,    61,    98,     0,
       0,     0,   137,     0,     0,     0,   140,    58,    59,    60,
      61,    58,    59,    60,    61,    58,    59,    60,    61,   141,
       0,     0,     0,   147,     0,    62,    58,    59,    60,    61,
      58,    59,    60,    61,     0,     0,    87,     0,     0,     0,
     119,    58,    59,    60,    61,    58,    59,    60,    61,     0,
       0,   130,     0,     0,     0,   131,    58,    59,    60,    61,
      58,    59,    60,    61,     0,     0,   133,     0,    89,     0,
     145,    90,     0,    91,    92,    54,    55,    56,    57,    58,
      59,    60,    61,    68,    59,    60,    61
};

static const short yycheck[] =
{
      22,    47,    94,    25,     4,    17,    18,     3,     4,     5,
       6,     6,    34,    35,   112,    15,    11,    12,    40,    15,
      22,    22,    44,    45,    25,     5,    26,    49,   120,     3,
      26,   129,     6,    34,     6,     5,    58,    59,    60,    61,
      62,    15,   134,    28,    17,    18,    68,    28,    70,    23,
      22,     6,    26,    54,    55,    56,    57,     3,     4,     5,
       6,    83,    24,     0,     1,    87,    24,     5,   114,    15,
       7,    72,     9,     3,     4,     5,     6,    99,     3,     4,
      26,     6,    13,     6,    21,    15,     9,   109,    11,    12,
      15,     5,    28,    28,   116,   117,    26,   119,    99,    22,
       6,    26,    24,     3,   126,    24,     6,     3,   130,   131,
       6,   133,     5,     3,     5,    15,     6,    28,    15,    15,
      15,   143,    22,   145,    28,    15,    26,    13,     6,    28,
      26,     9,    22,    11,    12,     6,    26,     6,     9,    24,
      11,    12,    11,    12,    22,    15,    16,    17,    18,    26,
      26,    22,     6,    22,     5,    25,    15,    27,    15,    16,
      17,    18,    15,    16,    17,    18,    26,     0,    25,   102,
      27,    21,    25,    16,    27,    15,    16,    17,    18,    15,
      16,    17,    18,    15,    16,    17,    18,    27,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    27,    15,    16,    17,    18,
      15,    16,    17,    18,    15,    16,    17,    18,    27,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    27,    15,    16,    17,
      18,    15,    16,    17,    18,    15,    16,    17,    18,    27,
      -1,    -1,    -1,    27,    -1,    25,    15,    16,    17,    18,
      15,    16,    17,    18,    -1,    -1,    25,    -1,    -1,    -1,
      25,    15,    16,    17,    18,    15,    16,    17,    18,    -1,
      -1,    25,    -1,    -1,    -1,    25,    15,    16,    17,    18,
      15,    16,    17,    18,    -1,    -1,    25,    -1,     6,    -1,
      25,     9,    -1,    11,    12,    15,    16,    17,    18,    15,
      16,    17,    18,    15,    16,    17,    18
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

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

#line 315 "/usr/share/bison/bison.simple"


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
#line 87 "plugins/aws/skinpars.yy"
{ static_awsparser->AddGlobalSkinDef(yyvsp[0].skin); ;
    break;}
case 5:
#line 88 "plugins/aws/skinpars.yy"
{ static_awsparser->AddGlobalWindowDef(yyvsp[0].comp); ;
    break;}
case 6:
#line 89 "plugins/aws/skinpars.yy"
{ yyerrok;      ;
    break;}
case 7:
#line 98 "plugins/aws/skinpars.yy"
{ yyval.key = static_awsparser->MapSourceToSink (yyvsp[-6].val, yyvsp[-3].str, yyvsp[0].str); free(yyvsp[-3].str); free(yyvsp[0].str); ;
    break;}
case 8:
#line 103 "plugins/aws/skinpars.yy"
{ awsKeyContainer* kc = new awsKeyContainer;
		  if (yyvsp[0].key) kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;
    break;}
case 9:
#line 108 "plugins/aws/skinpars.yy"
{ awsKeyContainer* kc = yyvsp[-1].keycont;
		  if (yyvsp[0].key) kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;
    break;}
case 10:
#line 116 "plugins/aws/skinpars.yy"
{  yyval.key = new awsStringKey(yyvsp[-2].str, yyvsp[0].str); free(yyvsp[-2].str); free(yyvsp[0].str); ;
    break;}
case 11:
#line 118 "plugins/aws/skinpars.yy"
{  yyval.key = new awsFloatKey(yyvsp[-2].str, yyvsp[0].fval); free(yyvsp[-2].str); ;
    break;}
case 12:
#line 120 "plugins/aws/skinpars.yy"
{  yyval.key = new awsIntKey(yyvsp[-2].str, yyvsp[0].val); free(yyvsp[-2].str); ;
    break;}
case 13:
#line 122 "plugins/aws/skinpars.yy"
{  yyval.key = new awsRGBKey(yyvsp[-6].str, yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val); free(yyvsp[-6].str); ;
    break;}
case 14:
#line 124 "plugins/aws/skinpars.yy"
{  yyval.key = new awsRectKey(yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-12].str); ;
    break;}
case 15:
#line 126 "plugins/aws/skinpars.yy"
{ awsConnectionNode *cn = new awsConnectionNode();
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume (kc);
		  kc->DecRef();
		  yyval.key = cn;
		;
    break;}
case 16:
#line 133 "plugins/aws/skinpars.yy"
{ awsComponentNode *cn = new awsComponentNode(yyvsp[-5].str, yyvsp[-3].str);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key = cn;
		  free(yyvsp[-5].str);
		  free(yyvsp[-3].str);
		;
    break;}
case 17:
#line 142 "plugins/aws/skinpars.yy"
{ awsComponentNode *cn = new awsComponentNode(yyvsp[-3].str, "Window");
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key = cn;
		  free(yyvsp[-3].str);
		;
    break;}
case 18:
#line 153 "plugins/aws/skinpars.yy"
{ awsKeyContainer* keycontainer = new awsKeyContainer;
		  keycontainer->Add(yyvsp[0].key);
		  yyval.keycont = keycontainer;
		;
    break;}
case 19:
#line 158 "plugins/aws/skinpars.yy"
{ awsKeyContainer* keycontainer = yyvsp[-1].keycont;
		  keycontainer->Add(yyvsp[0].key);
		  yyval.keycont = keycontainer;
		;
    break;}
case 20:
#line 167 "plugins/aws/skinpars.yy"
{ yyval.key = new awsStringKey(yyvsp[-2].str, yyvsp[0].str); free(yyvsp[-2].str); free(yyvsp[0].str); ;
    break;}
case 21:
#line 169 "plugins/aws/skinpars.yy"
{ yyval.key = new awsFloatKey(yyvsp[-2].str, yyvsp[0].fval); free(yyvsp[-2].str); ;
    break;}
case 22:
#line 171 "plugins/aws/skinpars.yy"
{ yyval.key = new awsIntKey(yyvsp[-2].str, yyvsp[0].val); free(yyvsp[-2].str); ;
    break;}
case 23:
#line 173 "plugins/aws/skinpars.yy"
{ yyval.key = new awsRectKey(yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-12].str); ;
    break;}
case 24:
#line 175 "plugins/aws/skinpars.yy"
{ awsConnectionNode *cn = new awsConnectionNode();
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key=cn;
		;
    break;}
case 25:
#line 182 "plugins/aws/skinpars.yy"
{ awsComponentNode *cn = new awsComponentNode(yyvsp[-5].str, yyvsp[-3].str);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key=cn;
		  free(yyvsp[-5].str);
		  free(yyvsp[-3].str);
		;
    break;}
case 26:
#line 194 "plugins/aws/skinpars.yy"
{ awsKeyContainer* cnt = new awsKeyContainer;
		cnt->Add(yyvsp[0].key);
		yyval.keycont = cnt;
	      ;
    break;}
case 27:
#line 199 "plugins/aws/skinpars.yy"
{ awsKeyContainer* cnt = yyvsp[-1].keycont;
	        cnt->Add(yyvsp[0].key);
	        yyval.keycont = cnt;
	      ;
    break;}
case 28:
#line 207 "plugins/aws/skinpars.yy"
{ awsComponentNode *win = new awsComponentNode(yyvsp[-3].str, "Default");
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  win->Consume(kc);
                  kc->DecRef();
		  yyval.comp = win;
		  free(yyvsp[-3].str);
		;
    break;}
case 29:
#line 222 "plugins/aws/skinpars.yy"
{ yyval.key = new awsStringKey(yyvsp[-2].str, yyvsp[0].str); free(yyvsp[-2].str); free(yyvsp[0].str); ;
    break;}
case 30:
#line 224 "plugins/aws/skinpars.yy"
{ yyval.key = new awsRGBKey(yyvsp[-6].str, yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val); free(yyvsp[-6].str); ;
    break;}
case 31:
#line 226 "plugins/aws/skinpars.yy"
{ yyval.key = new awsFloatKey(yyvsp[-2].str, yyvsp[0].fval); free(yyvsp[-2].str); ;
    break;}
case 32:
#line 228 "plugins/aws/skinpars.yy"
{ yyval.key = new awsIntKey(yyvsp[-2].str, yyvsp[0].val); free(yyvsp[-2].str); ;
    break;}
case 33:
#line 230 "plugins/aws/skinpars.yy"
{ yyval.key = new awsPointKey(yyvsp[-6].str, csPoint(yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-6].str); ;
    break;}
case 34:
#line 232 "plugins/aws/skinpars.yy"
{ yyval.key = new awsRectKey(yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-12].str); ;
    break;}
case 35:
#line 237 "plugins/aws/skinpars.yy"
{ awsKeyContainer* kc = new awsKeyContainer;
		  kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;
    break;}
case 36:
#line 242 "plugins/aws/skinpars.yy"
{ awsKeyContainer* kc = yyvsp[-1].keycont;
		  kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;
    break;}
case 37:
#line 250 "plugins/aws/skinpars.yy"
{ awsSkinNode *skin = new awsSkinNode(yyvsp[-3].str);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
                  skin->Consume(kc);
                  kc->DecRef();
		  yyval.skin = skin;
		  free(yyvsp[-3].str);
		;
    break;}
case 38:
#line 264 "plugins/aws/skinpars.yy"
{ int v = 0;
		  if (!static_awsparser->GetConstantValue(yyvsp[0].str, v))
		    static_awsparser->ReportError ("Constant %s is not defined.", yyvsp[0].str);
		  yyval.val = v;
		  free(yyvsp[0].str);
		;
    break;}
case 39:
#line 272 "plugins/aws/skinpars.yy"
{ yyval.fval = yyvsp[0].fval; ;
    break;}
case 40:
#line 273 "plugins/aws/skinpars.yy"
{ yyval.fval = yyvsp[-2].fval + yyvsp[0].fval; ;
    break;}
case 41:
#line 274 "plugins/aws/skinpars.yy"
{ yyval.fval = yyvsp[-2].fval - yyvsp[0].fval; ;
    break;}
case 42:
#line 275 "plugins/aws/skinpars.yy"
{ yyval.fval = yyvsp[-2].fval * yyvsp[0].fval; ;
    break;}
case 43:
#line 276 "plugins/aws/skinpars.yy"
{ yyval.fval = yyvsp[-2].fval / yyvsp[0].fval; ;
    break;}
case 44:
#line 277 "plugins/aws/skinpars.yy"
{ yyval.fval = -yyvsp[0].fval; ;
    break;}
case 45:
#line 278 "plugins/aws/skinpars.yy"
{ yyval.fval = yyvsp[-1].fval; ;
    break;}
case 46:
#line 280 "plugins/aws/skinpars.yy"
{ yyval.val = yyvsp[0].val; ;
    break;}
case 47:
#line 281 "plugins/aws/skinpars.yy"
{ yyval.val = yyvsp[0].val; ;
    break;}
case 48:
#line 282 "plugins/aws/skinpars.yy"
{ yyval.val = yyvsp[-2].val + yyvsp[0].val; ;
    break;}
case 49:
#line 283 "plugins/aws/skinpars.yy"
{ yyval.val = yyvsp[-2].val - yyvsp[0].val; ;
    break;}
case 50:
#line 284 "plugins/aws/skinpars.yy"
{ yyval.val = yyvsp[-2].val * yyvsp[0].val; ;
    break;}
case 51:
#line 285 "plugins/aws/skinpars.yy"
{ yyval.val = yyvsp[-2].val / yyvsp[0].val; ;
    break;}
case 52:
#line 286 "plugins/aws/skinpars.yy"
{ yyval.val = -yyvsp[0].val; ;
    break;}
case 53:
#line 287 "plugins/aws/skinpars.yy"
{ yyval.val = yyvsp[-1].val; ;
    break;}
}

#line 705 "/usr/share/bison/bison.simple"


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
#line 290 "plugins/aws/skinpars.yy"


int
awserror(char *s)
{
  static_awsparser->ReportError (s);
  return 0;
}

