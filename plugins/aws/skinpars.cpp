/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse awsparse
#define yylex   awslex
#define yyerror awserror
#define yylval  awslval
#define yychar  awschar
#define yydebug awsdebug
#define yynerrs awsnerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOKEN_NUM = 258,
     TOKEN_FLOAT = 259,
     TOKEN_STR = 260,
     TOKEN_ATTR = 261,
     TOKEN_SKIN = 262,
     TOKEN_FOR = 263,
     TOKEN_WINDOW = 264,
     TOKEN_FROM = 265,
     TOKEN_COMPONENT = 266,
     TOKEN_CONNECT = 267,
     TOKEN_IS = 268,
     NEG = 269
   };
#endif
#define TOKEN_NUM 258
#define TOKEN_FLOAT 259
#define TOKEN_STR 260
#define TOKEN_ATTR 261
#define TOKEN_SKIN 262
#define TOKEN_FOR 263
#define TOKEN_WINDOW 264
#define TOKEN_FROM 265
#define TOKEN_COMPONENT 266
#define TOKEN_CONNECT 267
#define TOKEN_IS 268
#define NEG 269




/* Copy the first part of user declarations.  */



#include "cssysdef.h"
#include "csgeom/csrect.h"
#include "csutil/scfstr.h"
#include "aws.h"
#include "awsprefs.h"
#include "awsparser.h"
#include <stdio.h>

// Bison 1.875 outputs bogus code -- an attribute after a label is not
// allowed in g++, so work around the problem.
#define __attribute__(x)

// MSVC 7.1 complains about switch statement without any 'case' labels in
// generated yydestruct(). We have no control over this generated code, so
// silence the warning.
#if defined(CS_COMPILER_MSVC)
#pragma warning(disable:4065)
#endif



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
  char   *str;			/* For returning titles and handles to items. */
  int     val;			/* For returning integer numbers              */
  float  fval;			/* For returning non-integer numbers          */
  csRect *rect;			/* For returning rectangular regions          */
  awsKey *key;     		/* For returning keys to various definition items */
  awsComponentNode *comp;	/* for returning windows		      */
  awsKeyContainer *keycont;	/* for returning KeyContainers		      */
  awsSkinNode *skin;		/* for returning skins			      */
} YYSTYPE;
/* Line 191 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */



extern int awslex(YYSTYPE *awslval);
extern int awserror(char *s);
extern int awslineno;

/// This is the parser parameter
#define YYPARSE_PARAM windowmgr



/* Line 214 of yacc.c.  */


#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
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
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
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
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   296

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  29
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  16
/* YYNRULES -- Number of rules. */
#define YYNRULES  54
/* YYNRULES -- Number of states. */
#define YYNSTATES  149

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   269

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    19
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    16,    24,
      26,    29,    33,    37,    41,    49,    63,    68,    76,    82,
      84,    87,    91,    95,    99,   113,   118,   126,   128,   131,
     137,   141,   149,   153,   157,   165,   179,   181,   184,   190,
     192,   194,   198,   202,   206,   210,   213,   217,   219,   221,
     225,   229,   233,   237,   240
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      30,     0,    -1,    -1,    30,    31,    -1,    21,    -1,    41,
      -1,    38,    -1,     1,    22,    -1,    44,    15,    23,     6,
      24,    24,     6,    -1,    32,    -1,    33,    32,    -1,     6,
      24,     5,    -1,     6,    24,    43,    -1,     6,    24,    44,
      -1,     6,    24,    44,    25,    44,    25,    44,    -1,     6,
      24,    26,    44,    25,    44,    27,    15,    26,    44,    25,
      44,    27,    -1,    12,    28,    33,    22,    -1,    11,     5,
      13,     5,    28,    35,    22,    -1,     9,     5,    28,    35,
      22,    -1,    34,    -1,    35,    34,    -1,     6,    24,     5,
      -1,     6,    24,    43,    -1,     6,    24,    44,    -1,     6,
      24,    26,    44,    25,    44,    27,    15,    26,    44,    25,
      44,    27,    -1,    12,    28,    33,    22,    -1,    11,     5,
      13,     5,    28,    35,    22,    -1,    36,    -1,    37,    36,
      -1,     9,     5,    28,    37,    22,    -1,     6,    24,     5,
      -1,     6,    24,    44,    25,    44,    25,    44,    -1,     6,
      24,    43,    -1,     6,    24,    44,    -1,     6,    24,    26,
      44,    25,    44,    27,    -1,     6,    24,    26,    44,    25,
      44,    27,    15,    26,    44,    25,    44,    27,    -1,    39,
      -1,    40,    39,    -1,     7,     5,    28,    40,    22,    -1,
       6,    -1,     4,    -1,    43,    16,    43,    -1,    43,    15,
      43,    -1,    43,    17,    43,    -1,    43,    18,    43,    -1,
      15,    43,    -1,    26,    43,    27,    -1,     3,    -1,    42,
      -1,    44,    16,    44,    -1,    44,    15,    44,    -1,    44,
      17,    44,    -1,    44,    18,    44,    -1,    15,    44,    -1,
      26,    44,    27,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   107,   107,   108,   111,   112,   113,   114,   122,   127,
     132,   140,   142,   144,   146,   148,   150,   157,   166,   177,
     182,   191,   193,   195,   197,   199,   206,   218,   223,   231,
     246,   248,   250,   252,   254,   256,   261,   266,   274,   288,
     297,   298,   299,   300,   301,   302,   303,   306,   307,   308,
     309,   310,   311,   312,   313
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOKEN_NUM", "TOKEN_FLOAT", "TOKEN_STR",
  "TOKEN_ATTR", "TOKEN_SKIN", "TOKEN_FOR", "TOKEN_WINDOW", "TOKEN_FROM",
  "TOKEN_COMPONENT", "TOKEN_CONNECT", "TOKEN_IS", "'='", "'-'", "'+'",
  "'*'", "'/'", "NEG", "'^'", "'\\n'", "'}'", "'>'", "':'", "','", "'('",
  "')'", "'{'", "$accept", "input", "line", "connection_item",
  "connection_item_list", "component_item", "component_item_list",
  "window_item", "window_item_list", "window", "skin_item",
  "skin_item_list", "skin", "constant_item", "expf", "exp", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,    61,    45,    43,    42,    47,   269,
      94,    10,   125,    62,    58,    44,    40,    41,   123
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    29,    30,    30,    31,    31,    31,    31,    32,    33,
      33,    34,    34,    34,    34,    34,    34,    34,    34,    35,
      35,    36,    36,    36,    36,    36,    36,    37,    37,    38,
      39,    39,    39,    39,    39,    39,    40,    40,    41,    42,
      43,    43,    43,    43,    43,    43,    43,    44,    44,    44,
      44,    44,    44,    44,    44
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     2,     7,     1,
       2,     3,     3,     3,     7,    13,     4,     7,     5,     1,
       2,     3,     3,     3,    13,     4,     7,     1,     2,     5,
       3,     7,     3,     3,     7,    13,     1,     2,     5,     1,
       1,     3,     3,     3,     3,     2,     3,     1,     1,     3,
       3,     3,     3,     2,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     1,     0,     0,     0,     4,     3,     6,     5,
       7,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,    27,     0,     0,    38,    37,     0,     0,     0,    29,
      28,    47,    40,    30,    39,     0,     0,    48,    32,    33,
      21,     0,    22,    23,     0,     0,     0,     9,     0,     0,
       0,    45,    53,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    25,    10,     0,
      46,     0,    54,     0,     0,    42,    41,    43,    44,    50,
      49,    51,    52,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,     0,     0,    34,    31,     0,
       0,     0,     0,     0,    26,    20,     0,     0,     0,    11,
       0,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    16,     8,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      17,    35,    24,     0,     0,     0,     0,     0,    15
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,     7,    47,    48,    94,    95,    21,    22,     8,
      16,    17,     9,    37,    53,    49
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -100
static const short int yypact[] =
{
    -100,    63,  -100,     3,    15,    30,  -100,  -100,  -100,  -100,
    -100,    19,    23,    37,     5,    38,  -100,    28,    42,    62,
      64,  -100,   131,     4,  -100,  -100,    54,    69,   104,  -100,
    -100,  -100,  -100,  -100,  -100,    75,    75,  -100,   270,   210,
    -100,    75,   270,   274,    86,   104,   104,  -100,   100,   278,
      75,  -100,  -100,   160,   130,     0,     0,     0,     0,   104,
     104,   104,   104,   104,   143,    65,   164,  -100,  -100,    26,
    -100,   104,  -100,     0,     0,   -12,   -12,  -100,  -100,    27,
      27,  -100,  -100,   221,   104,   272,    94,   168,   104,   181,
      78,   107,   109,    89,  -100,    77,    81,   103,   274,   105,
      70,    96,   114,   104,  -100,  -100,   115,   123,   124,  -100,
      75,   270,   225,   272,   149,   110,   146,   104,   104,   147,
     104,   122,   101,  -100,  -100,   236,   240,   104,   251,  -100,
     272,   104,   104,   185,   104,   129,   189,   202,   141,   274,
    -100,  -100,  -100,   140,   104,   255,   104,   206,  -100
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -100,  -100,  -100,   -47,    66,   -93,   -99,   145,  -100,  -100,
     154,  -100,  -100,  -100,    -2,   -23
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      39,    68,   105,    43,    32,    57,    58,    31,    32,    33,
      34,    18,    52,    54,   121,    73,    19,    20,    64,    35,
      11,    38,    52,    66,    42,    10,    74,    66,   105,    31,
      36,   135,    34,    51,    15,    12,    79,    80,    81,    82,
      83,    45,   105,    15,    61,    62,    79,    13,    87,    86,
      24,    14,    46,    75,    76,    77,    78,    31,    32,    40,
      34,    89,    23,     2,     3,    98,    26,    27,    68,    35,
       4,    51,     5,    31,    32,   109,    34,   112,    31,    32,
      41,    34,    44,    90,     6,    35,    91,   119,    92,    93,
      35,    65,    28,    85,   125,   126,   110,   128,   111,   104,
      96,    50,   100,    31,   133,   106,    34,    31,   136,   137,
      34,   139,   101,    31,   102,    45,    34,   103,   107,    45,
     108,   145,    67,   147,   113,    45,    46,   114,    90,   130,
      46,    91,   123,    92,    93,    90,    46,    18,    91,   116,
      92,    93,    19,    20,   129,    59,    60,    61,    62,   117,
     118,   140,   124,    29,   122,    71,   143,    72,    59,    60,
      61,    62,    59,    60,    61,    62,   144,    30,    84,   115,
      72,    25,   127,     0,    72,    55,    56,    57,    58,    59,
      60,    61,    62,    59,    60,    61,    62,    70,     0,     0,
       0,    72,     0,     0,     0,    97,    59,    60,    61,    62,
      59,    60,    61,    62,    59,    60,    61,    62,    99,     0,
       0,     0,   138,     0,     0,     0,   141,    59,    60,    61,
      62,    59,    60,    61,    62,    59,    60,    61,    62,   142,
       0,     0,     0,   148,     0,    63,    59,    60,    61,    62,
      59,    60,    61,    62,     0,     0,    88,     0,     0,     0,
     120,    59,    60,    61,    62,    59,    60,    61,    62,     0,
       0,   131,     0,     0,     0,   132,    59,    60,    61,    62,
      59,    60,    61,    62,     0,     0,   134,     0,    90,     0,
     146,    91,     0,    92,    93,    55,    56,    57,    58,    59,
      60,    61,    62,    69,    60,    61,    62
};

static const short int yycheck[] =
{
      23,    48,    95,    26,     4,    17,    18,     3,     4,     5,
       6,     6,    35,    36,   113,    15,    11,    12,    41,    15,
       5,    23,    45,    46,    26,    22,    26,    50,   121,     3,
      26,   130,     6,    35,     6,     5,    59,    60,    61,    62,
      63,    15,   135,     6,    17,    18,    69,    28,    71,    23,
      22,    28,    26,    55,    56,    57,    58,     3,     4,     5,
       6,    84,    24,     0,     1,    88,    24,     5,   115,    15,
       7,    73,     9,     3,     4,     5,     6,   100,     3,     4,
      26,     6,    13,     6,    21,    15,     9,   110,    11,    12,
      15,     5,    28,    28,   117,   118,    26,   120,   100,    22,
       6,    26,    24,     3,   127,    24,     6,     3,   131,   132,
       6,   134,     5,     3,     5,    15,     6,    28,    15,    15,
      15,   144,    22,   146,    28,    15,    26,    13,     6,    28,
      26,     9,    22,    11,    12,     6,    26,     6,     9,    24,
      11,    12,    11,    12,    22,    15,    16,    17,    18,    26,
      26,    22,     6,    22,     5,    25,    15,    27,    15,    16,
      17,    18,    15,    16,    17,    18,    26,    22,    25,   103,
      27,    17,    25,    -1,    27,    15,    16,    17,    18,    15,
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

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    30,     0,     1,     7,     9,    21,    31,    38,    41,
      22,     5,     5,    28,    28,     6,    39,    40,     6,    11,
      12,    36,    37,    24,    22,    39,    24,     5,    28,    22,
      36,     3,     4,     5,     6,    15,    26,    42,    43,    44,
       5,    26,    43,    44,    13,    15,    26,    32,    33,    44,
      26,    43,    44,    43,    44,    15,    16,    17,    18,    15,
      16,    17,    18,    25,    44,     5,    44,    22,    32,    15,
      27,    25,    27,    15,    26,    43,    43,    43,    43,    44,
      44,    44,    44,    44,    25,    28,    23,    44,    25,    44,
       6,     9,    11,    12,    34,    35,     6,    27,    44,    27,
      24,     5,     5,    28,    22,    34,    24,    15,    15,     5,
      26,    43,    44,    28,    13,    33,    24,    26,    26,    44,
      25,    35,     5,    22,     6,    44,    44,    25,    44,    22,
      28,    25,    25,    44,    25,    35,    44,    44,    27,    44,
      22,    27,    27,    15,    26,    44,    25,    44,    27
};

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
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


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
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

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

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
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

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

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

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
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

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
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
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


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

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:

    { static_awsparser->AddGlobalSkinDef(yyvsp[0].skin); ;}
    break;

  case 6:

    { static_awsparser->AddGlobalWindowDef(yyvsp[0].comp); ;}
    break;

  case 7:

    { yyerrok;      ;}
    break;

  case 8:

    { yyval.key = static_awsparser->MapSourceToSink (yyvsp[-6].val, yyvsp[-3].str, yyvsp[0].str); free(yyvsp[-3].str); free(yyvsp[0].str); ;}
    break;

  case 9:

    { awsKeyContainer* kc = new awsKeyContainer((iAws*)windowmgr);
		  if (yyvsp[0].key) kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;}
    break;

  case 10:

    { awsKeyContainer* kc = yyvsp[-1].keycont;
		  if (yyvsp[0].key) kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;}
    break;

  case 11:

    {  yyval.key = new awsStringKey((iAws*)windowmgr, yyvsp[-2].str, yyvsp[0].str); free(yyvsp[-2].str); free(yyvsp[0].str); ;}
    break;

  case 12:

    {  yyval.key = new awsFloatKey((iAws*)windowmgr, yyvsp[-2].str, yyvsp[0].fval); free(yyvsp[-2].str); ;}
    break;

  case 13:

    {  yyval.key = new awsIntKey((iAws*)windowmgr, yyvsp[-2].str, yyvsp[0].val); free(yyvsp[-2].str); ;}
    break;

  case 14:

    {  yyval.key = new awsRGBKey((iAws*)windowmgr, yyvsp[-6].str, yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val); free(yyvsp[-6].str); ;}
    break;

  case 15:

    {  yyval.key = new awsRectKey((iAws*)windowmgr, yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-12].str); ;}
    break;

  case 16:

    { awsConnectionNode *cn = new awsConnectionNode((iAws*)windowmgr);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume (kc);
		  kc->DecRef();
		  yyval.key = cn;
		;}
    break;

  case 17:

    { awsComponentNode *cn = new awsComponentNode((iAws*)windowmgr, yyvsp[-5].str, yyvsp[-3].str);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key = cn;
		  free(yyvsp[-5].str);
		  free(yyvsp[-3].str);
		;}
    break;

  case 18:

    { awsComponentNode *cn = new awsComponentNode((iAws*)windowmgr, yyvsp[-3].str, "Window");
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key = cn;
		  free(yyvsp[-3].str);
		;}
    break;

  case 19:

    { awsKeyContainer* keycontainer = new awsKeyContainer((iAws*)windowmgr);
		  keycontainer->Add(yyvsp[0].key);
		  yyval.keycont = keycontainer;
		;}
    break;

  case 20:

    { awsKeyContainer* keycontainer = yyvsp[-1].keycont;
		  keycontainer->Add(yyvsp[0].key);
		  yyval.keycont = keycontainer;
		;}
    break;

  case 21:

    { yyval.key = new awsStringKey((iAws*)windowmgr, yyvsp[-2].str, yyvsp[0].str); free(yyvsp[-2].str); free(yyvsp[0].str); ;}
    break;

  case 22:

    { yyval.key = new awsFloatKey((iAws*)windowmgr, yyvsp[-2].str, yyvsp[0].fval); free(yyvsp[-2].str); ;}
    break;

  case 23:

    { yyval.key = new awsIntKey((iAws*)windowmgr, yyvsp[-2].str, yyvsp[0].val); free(yyvsp[-2].str); ;}
    break;

  case 24:

    { yyval.key = new awsRectKey((iAws*)windowmgr, yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-12].str); ;}
    break;

  case 25:

    { awsConnectionNode *cn = new awsConnectionNode((iAws*)windowmgr);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key=cn;
		;}
    break;

  case 26:

    { awsComponentNode *cn = new awsComponentNode((iAws*)windowmgr, yyvsp[-5].str, yyvsp[-3].str);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key=cn;
		  free(yyvsp[-5].str);
		  free(yyvsp[-3].str);
		;}
    break;

  case 27:

    { awsKeyContainer* cnt = new awsKeyContainer((iAws*)windowmgr);
		cnt->Add(yyvsp[0].key);
		yyval.keycont = cnt;
	      ;}
    break;

  case 28:

    { awsKeyContainer* cnt = yyvsp[-1].keycont;
	        cnt->Add(yyvsp[0].key);
	        yyval.keycont = cnt;
	      ;}
    break;

  case 29:

    { awsComponentNode *win = new awsComponentNode((iAws*)windowmgr, yyvsp[-3].str, "Default");
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  win->Consume(kc);
                  kc->DecRef();
		  yyval.comp = win;
		  free(yyvsp[-3].str);
		;}
    break;

  case 30:

    { yyval.key = new awsStringKey((iAws*)windowmgr, yyvsp[-2].str, yyvsp[0].str); free(yyvsp[-2].str); free(yyvsp[0].str); ;}
    break;

  case 31:

    { yyval.key = new awsRGBKey((iAws*)windowmgr, yyvsp[-6].str, yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val); free(yyvsp[-6].str); ;}
    break;

  case 32:

    { yyval.key = new awsFloatKey((iAws*)windowmgr, yyvsp[-2].str, yyvsp[0].fval); free(yyvsp[-2].str); ;}
    break;

  case 33:

    { yyval.key = new awsIntKey((iAws*)windowmgr, yyvsp[-2].str, yyvsp[0].val); free(yyvsp[-2].str); ;}
    break;

  case 34:

    { yyval.key = new awsPointKey((iAws*)windowmgr, yyvsp[-6].str, csPoint(yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-6].str); ;}
    break;

  case 35:

    { yyval.key = new awsRectKey((iAws*)windowmgr, yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-12].str); ;}
    break;

  case 36:

    { awsKeyContainer* kc = new awsKeyContainer((iAws*)windowmgr);
		  kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;}
    break;

  case 37:

    { awsKeyContainer* kc = yyvsp[-1].keycont;
		  kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;}
    break;

  case 38:

    { awsSkinNode *skin = new awsSkinNode((iAws*)windowmgr, yyvsp[-3].str);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
                  skin->Consume(kc);
                  kc->DecRef();
		  yyval.skin = skin;
		  free(yyvsp[-3].str);
		;}
    break;

  case 39:

    { int v = 0;
		  if (!static_awsparser->GetConstantValue(yyvsp[0].str, v))
		    static_awsparser->ReportError ("Constant %s is not defined.", yyvsp[0].str);
		  yyval.val = v;
		  free(yyvsp[0].str);
		;}
    break;

  case 40:

    { yyval.fval = yyvsp[0].fval; ;}
    break;

  case 41:

    { yyval.fval = yyvsp[-2].fval + yyvsp[0].fval; ;}
    break;

  case 42:

    { yyval.fval = yyvsp[-2].fval - yyvsp[0].fval; ;}
    break;

  case 43:

    { yyval.fval = yyvsp[-2].fval * yyvsp[0].fval; ;}
    break;

  case 44:

    { yyval.fval = yyvsp[-2].fval / yyvsp[0].fval; ;}
    break;

  case 45:

    { yyval.fval = -yyvsp[0].fval; ;}
    break;

  case 46:

    { yyval.fval = yyvsp[-1].fval; ;}
    break;

  case 47:

    { yyval.val = yyvsp[0].val; ;}
    break;

  case 48:

    { yyval.val = yyvsp[0].val; ;}
    break;

  case 49:

    { yyval.val = yyvsp[-2].val + yyvsp[0].val; ;}
    break;

  case 50:

    { yyval.val = yyvsp[-2].val - yyvsp[0].val; ;}
    break;

  case 51:

    { yyval.val = yyvsp[-2].val * yyvsp[0].val; ;}
    break;

  case 52:

    { yyval.val = yyvsp[-2].val / yyvsp[0].val; ;}
    break;

  case 53:

    { yyval.val = -yyvsp[0].val; ;}
    break;

  case 54:

    { yyval.val = yyvsp[-1].val; ;}
    break;


    }

/* Line 1010 of yacc.c.  */


  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
	  yychar = YYEMPTY;

	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

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

