/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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
     TOKEN_STR = 259,
     TOKEN_ATTR = 260,
     TOKEN_SKIN = 261,
     TOKEN_FOR = 262,
     TOKEN_WINDOW = 263,
     TOKEN_FROM = 264,
     TOKEN_COMPONENT = 265,
     TOKEN_CONNECT = 266,
     TOKEN_IS = 267,
     NEG = 268
   };
#endif
#define TOKEN_NUM 258
#define TOKEN_STR 259
#define TOKEN_ATTR 260
#define TOKEN_SKIN 261
#define TOKEN_FOR 262
#define TOKEN_WINDOW 263
#define TOKEN_FROM 264
#define TOKEN_COMPONENT 265
#define TOKEN_CONNECT 266
#define TOKEN_IS 267
#define NEG 268




/* Copy the first part of user declarations.  */
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
#line 31 "plugins/aws/skinpars.yy"
typedef union YYSTYPE {
  char   *str;			/* For returning titles and handles to items. */
  int     val;			/* For returning numbers                      */
  csRect *rect;			/* For returning rectangular regions          */
  awsKey *key;     		/* For returning keys to various definition items */
  awsComponentNode *comp;	/* for returning windows		      */
  awsKeyContainer *keycont;	/* for returning KeyContainers		      */
  awsSkinNode *skin;		/* for returning skins			      */
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 136 "plugins/aws/skinpars.cpp"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */
#line 42 "plugins/aws/skinpars.yy"


extern int awslex(YYSTYPE *awslval);
extern int awserror(char *s);
extern int awslineno;

/// This is the parser parameter
#define YYPARSE_PARAM windowmgr



/* Line 214 of yacc.c.  */
#line 158 "plugins/aws/skinpars.cpp"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

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
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

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
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   266

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  28
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  15
/* YYNRULES -- Number of rules. */
#define YYNRULES  44
/* YYNRULES -- Number of states. */
#define YYNSTATES  130

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   268

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    18
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    16,    24,
      26,    29,    33,    37,    45,    59,    64,    72,    78,    80,
      83,    87,    91,   105,   110,   118,   120,   123,   129,   133,
     141,   145,   153,   167,   169,   172,   178,   180,   182,   184,
     188,   192,   196,   200,   203
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      29,     0,    -1,    -1,    29,    30,    -1,    20,    -1,    40,
      -1,    37,    -1,     1,    21,    -1,    42,    14,    22,     5,
      23,    23,     5,    -1,    31,    -1,    32,    31,    -1,     5,
      23,     4,    -1,     5,    23,    42,    -1,     5,    23,    42,
      24,    42,    24,    42,    -1,     5,    23,    25,    42,    24,
      42,    26,    14,    25,    42,    24,    42,    26,    -1,    11,
      27,    32,    21,    -1,    10,     4,    12,     4,    27,    34,
      21,    -1,     8,     4,    27,    34,    21,    -1,    33,    -1,
      34,    33,    -1,     5,    23,     4,    -1,     5,    23,    42,
      -1,     5,    23,    25,    42,    24,    42,    26,    14,    25,
      42,    24,    42,    26,    -1,    11,    27,    32,    21,    -1,
      10,     4,    12,     4,    27,    34,    21,    -1,    35,    -1,
      36,    35,    -1,     8,     4,    27,    36,    21,    -1,     5,
      23,     4,    -1,     5,    23,    42,    24,    42,    24,    42,
      -1,     5,    23,    42,    -1,     5,    23,    25,    42,    24,
      42,    26,    -1,     5,    23,    25,    42,    24,    42,    26,
      14,    25,    42,    24,    42,    26,    -1,    38,    -1,    39,
      38,    -1,     6,     4,    27,    39,    21,    -1,     5,    -1,
       3,    -1,    41,    -1,    42,    15,    42,    -1,    42,    14,
      42,    -1,    42,    16,    42,    -1,    42,    17,    42,    -1,
      14,    42,    -1,    25,    42,    26,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    79,    79,    80,    83,    84,    85,    86,    94,    99,
     104,   112,   114,   116,   118,   120,   127,   136,   147,   152,
     161,   163,   165,   167,   174,   186,   191,   199,   214,   216,
     218,   220,   222,   227,   232,   240,   254,   263,   264,   265,
     266,   267,   268,   269,   270
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOKEN_NUM", "TOKEN_STR", "TOKEN_ATTR", 
  "TOKEN_SKIN", "TOKEN_FOR", "TOKEN_WINDOW", "TOKEN_FROM", 
  "TOKEN_COMPONENT", "TOKEN_CONNECT", "TOKEN_IS", "'='", "'-'", "'+'", 
  "'*'", "'/'", "NEG", "'^'", "'\\n'", "'}'", "'>'", "':'", "','", "'('", 
  "')'", "'{'", "$accept", "input", "line", "connection_item", 
  "connection_item_list", "component_item", "component_item_list", 
  "window_item", "window_item_list", "window", "skin_item", 
  "skin_item_list", "skin", "constant_item", "exp", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,    61,    45,    43,    42,    47,   268,    94,
      10,   125,    62,    58,    44,    40,    41,   123
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    28,    29,    29,    30,    30,    30,    30,    31,    32,
      32,    33,    33,    33,    33,    33,    33,    33,    34,    34,
      35,    35,    35,    35,    35,    36,    36,    37,    38,    38,
      38,    38,    38,    39,    39,    40,    41,    42,    42,    42,
      42,    42,    42,    42,    42
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     2,     7,     1,
       2,     3,     3,     7,    13,     4,     7,     5,     1,     2,
       3,     3,    13,     4,     7,     1,     2,     5,     3,     7,
       3,     7,    13,     1,     2,     5,     1,     1,     1,     3,
       3,     3,     3,     2,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     1,     0,     0,     0,     4,     3,     6,     5,
       7,     0,     0,     0,     0,     0,    33,     0,     0,     0,
       0,    25,     0,     0,    35,    34,     0,     0,     0,    27,
      26,    37,    28,    36,     0,     0,    38,    30,    20,     0,
      21,     0,     0,     9,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    23,    10,     0,     0,
      44,    40,    39,    41,    42,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    18,     0,     0,    31,
      29,     0,     0,     0,     0,     0,    24,    19,     0,     0,
       0,    11,     0,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,     8,     0,     0,     0,     0,
      17,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    16,    32,    22,     0,     0,     0,     0,     0,    14
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,     7,    43,    44,    76,    77,    21,    22,     8,
      16,    17,     9,    36,    45
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -89
static const short yypact[] =
{
     -89,    50,   -89,   -11,    14,    28,   -89,   -89,   -89,   -89,
     -89,     7,    10,    36,     4,    22,   -89,    12,    32,    57,
      38,   -89,   113,    17,   -89,   -89,    35,    54,    -1,   -89,
     -89,   -89,   -89,   -89,    -1,    -1,   -89,   187,   -89,    -1,
     245,    58,    -1,   -89,    43,   249,   -89,   111,    -1,    -1,
      -1,    -1,    -1,   124,    40,   141,   -89,   -89,    49,    -1,
     -89,    -9,    -9,   -89,   -89,   191,    -1,   247,    67,   145,
      -1,   149,    56,    69,    77,    64,   -89,    72,    61,    85,
     245,    89,    83,    79,    95,    -1,   -89,   -89,    86,    88,
      96,   -89,    -1,   202,   247,   106,    91,   110,    -1,    -1,
     128,    -1,    90,   102,   -89,   -89,   206,   217,    -1,   221,
     -89,   247,    -1,    -1,   162,    -1,   109,   166,   170,   108,
     245,   -89,   -89,   -89,   107,    -1,   232,    -1,   183,   -89
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -89,   -89,   -89,   -43,    46,   -72,   -88,   114,   -89,   -89,
     116,   -89,   -89,   -89,   -23
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      37,    57,    31,    40,    33,    87,   102,    50,    51,    18,
      10,    46,    47,    34,    19,    20,    53,    15,    11,    55,
      31,    32,    33,   116,    42,    61,    62,    63,    64,    65,
      87,    34,    12,    24,    13,    61,    69,    14,    31,    38,
      33,    15,    35,    71,    87,    23,    31,    80,    33,    34,
       2,     3,    31,    57,    33,    26,     4,    34,     5,    93,
      39,    27,    54,    34,    56,    28,    41,    67,    42,   100,
       6,    68,    78,    83,    42,   106,   107,    72,   109,    82,
      73,    84,    74,    75,    88,   114,    31,    91,    33,   117,
     118,    85,   120,    86,    31,    72,    33,    34,    73,    89,
      74,    75,   126,    90,   128,    34,    94,    95,    92,    97,
     103,   110,   104,    98,    72,   105,    42,    73,    18,    74,
      75,    99,   124,    19,    20,    48,    49,    50,    51,   111,
     121,    96,   125,    25,    29,    59,    30,    60,    48,    49,
      50,    51,    48,    49,    50,    51,     0,     0,    66,     0,
      60,     0,   108,     0,    60,    48,    49,    50,    51,    48,
      49,    50,    51,    48,    49,    50,    51,    60,     0,     0,
       0,    79,     0,     0,     0,    81,    48,    49,    50,    51,
      48,    49,    50,    51,    48,    49,    50,    51,   119,     0,
       0,     0,   122,     0,     0,     0,   123,    48,    49,    50,
      51,    48,    49,    50,    51,    48,    49,    50,    51,   129,
       0,    52,     0,     0,     0,    70,    48,    49,    50,    51,
      48,    49,    50,    51,     0,     0,   101,     0,     0,     0,
     112,    48,    49,    50,    51,    48,    49,    50,    51,     0,
       0,   113,     0,     0,     0,   115,    48,    49,    50,    51,
       0,     0,    72,     0,     0,    73,   127,    74,    75,    48,
      49,    50,    51,    58,    49,    50,    51
};

static const yysigned_char yycheck[] =
{
      23,    44,     3,    26,     5,    77,    94,    16,    17,     5,
      21,    34,    35,    14,    10,    11,    39,     5,     4,    42,
       3,     4,     5,   111,    25,    48,    49,    50,    51,    52,
     102,    14,     4,    21,    27,    58,    59,    27,     3,     4,
       5,     5,    25,    66,   116,    23,     3,    70,     5,    14,
       0,     1,     3,    96,     5,    23,     6,    14,     8,    82,
      25,     4,     4,    14,    21,    27,    12,    27,    25,    92,
      20,    22,     5,     4,    25,    98,    99,     5,   101,    23,
       8,     4,    10,    11,    23,   108,     3,     4,     5,   112,
     113,    27,   115,    21,     3,     5,     5,    14,     8,    14,
      10,    11,   125,    14,   127,    14,    27,    12,    25,    23,
       4,    21,    21,    25,     5,     5,    25,     8,     5,    10,
      11,    25,    14,    10,    11,    14,    15,    16,    17,    27,
      21,    85,    25,    17,    21,    24,    22,    26,    14,    15,
      16,    17,    14,    15,    16,    17,    -1,    -1,    24,    -1,
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

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    29,     0,     1,     6,     8,    20,    30,    37,    40,
      21,     4,     4,    27,    27,     5,    38,    39,     5,    10,
      11,    35,    36,    23,    21,    38,    23,     4,    27,    21,
      35,     3,     4,     5,    14,    25,    41,    42,     4,    25,
      42,    12,    25,    31,    32,    42,    42,    42,    14,    15,
      16,    17,    24,    42,     4,    42,    21,    31,    14,    24,
      26,    42,    42,    42,    42,    42,    24,    27,    22,    42,
      24,    42,     5,     8,    10,    11,    33,    34,     5,    26,
      42,    26,    23,     4,     4,    27,    21,    33,    23,    14,
      14,     4,    25,    42,    27,    12,    32,    23,    25,    25,
      42,    24,    34,     4,    21,     5,    42,    42,    24,    42,
      21,    27,    24,    24,    42,    24,    34,    42,    42,    26,
      42,    21,    26,    26,    14,    25,    42,    24,    42,    26
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
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
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
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
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
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
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

#if YYMAXDEPTH == 0
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
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

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
	short *yyss1 = yyss;


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
	short *yyss1 = yyss;
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
#line 84 "plugins/aws/skinpars.yy"
    { static_awsparser->AddGlobalSkinDef(yyvsp[0].skin); ;}
    break;

  case 6:
#line 85 "plugins/aws/skinpars.yy"
    { static_awsparser->AddGlobalWindowDef(yyvsp[0].comp); ;}
    break;

  case 7:
#line 86 "plugins/aws/skinpars.yy"
    { yyerrok;      ;}
    break;

  case 8:
#line 95 "plugins/aws/skinpars.yy"
    { yyval.key = static_awsparser->MapSourceToSink (yyvsp[-6].val, yyvsp[-3].str, yyvsp[0].str); free(yyvsp[-3].str); free(yyvsp[0].str); ;}
    break;

  case 9:
#line 100 "plugins/aws/skinpars.yy"
    { awsKeyContainer* kc = new awsKeyContainer;
		  if (yyvsp[0].key) kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;}
    break;

  case 10:
#line 105 "plugins/aws/skinpars.yy"
    { awsKeyContainer* kc = yyvsp[-1].keycont;
		  if (yyvsp[0].key) kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;}
    break;

  case 11:
#line 113 "plugins/aws/skinpars.yy"
    {  yyval.key = new awsStringKey(yyvsp[-2].str, yyvsp[0].str); free(yyvsp[-2].str); free(yyvsp[0].str); ;}
    break;

  case 12:
#line 115 "plugins/aws/skinpars.yy"
    {  yyval.key = new awsIntKey(yyvsp[-2].str, yyvsp[0].val); free(yyvsp[-2].str); ;}
    break;

  case 13:
#line 117 "plugins/aws/skinpars.yy"
    {  yyval.key = new awsRGBKey(yyvsp[-6].str, yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val); free(yyvsp[-6].str); ;}
    break;

  case 14:
#line 119 "plugins/aws/skinpars.yy"
    {  yyval.key = new awsRectKey(yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-12].str); ;}
    break;

  case 15:
#line 121 "plugins/aws/skinpars.yy"
    { awsConnectionNode *cn = new awsConnectionNode();
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume (kc);
		  kc->DecRef();
		  yyval.key = cn;
		;}
    break;

  case 16:
#line 128 "plugins/aws/skinpars.yy"
    { awsComponentNode *cn = new awsComponentNode(yyvsp[-5].str, yyvsp[-3].str);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key = cn;
		  free(yyvsp[-5].str);
		  free(yyvsp[-3].str);
		;}
    break;

  case 17:
#line 137 "plugins/aws/skinpars.yy"
    { awsComponentNode *cn = new awsComponentNode(yyvsp[-3].str, "Window");
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key = cn;
		  free(yyvsp[-3].str);
		;}
    break;

  case 18:
#line 148 "plugins/aws/skinpars.yy"
    { awsKeyContainer* keycontainer = new awsKeyContainer;
		  keycontainer->Add(yyvsp[0].key);
		  yyval.keycont = keycontainer;
		;}
    break;

  case 19:
#line 153 "plugins/aws/skinpars.yy"
    { awsKeyContainer* keycontainer = yyvsp[-1].keycont;
		  keycontainer->Add(yyvsp[0].key);
		  yyval.keycont = keycontainer;
		;}
    break;

  case 20:
#line 162 "plugins/aws/skinpars.yy"
    { yyval.key = new awsStringKey(yyvsp[-2].str, yyvsp[0].str); free(yyvsp[-2].str); free(yyvsp[0].str); ;}
    break;

  case 21:
#line 164 "plugins/aws/skinpars.yy"
    { yyval.key = new awsIntKey(yyvsp[-2].str, yyvsp[0].val); free(yyvsp[-2].str); ;}
    break;

  case 22:
#line 166 "plugins/aws/skinpars.yy"
    { yyval.key = new awsRectKey(yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-12].str); ;}
    break;

  case 23:
#line 168 "plugins/aws/skinpars.yy"
    { awsConnectionNode *cn = new awsConnectionNode();
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key=cn;
		;}
    break;

  case 24:
#line 175 "plugins/aws/skinpars.yy"
    { awsComponentNode *cn = new awsComponentNode(yyvsp[-5].str, yyvsp[-3].str);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  cn->Consume(kc);
                  kc->DecRef();
		  yyval.key=cn;
		  free(yyvsp[-5].str);
		  free(yyvsp[-3].str);
		;}
    break;

  case 25:
#line 187 "plugins/aws/skinpars.yy"
    { awsKeyContainer* cnt = new awsKeyContainer;
		cnt->Add(yyvsp[0].key);
		yyval.keycont = cnt;
	      ;}
    break;

  case 26:
#line 192 "plugins/aws/skinpars.yy"
    { awsKeyContainer* cnt = yyvsp[-1].keycont;
	        cnt->Add(yyvsp[0].key);
	        yyval.keycont = cnt;
	      ;}
    break;

  case 27:
#line 200 "plugins/aws/skinpars.yy"
    { awsComponentNode *win = new awsComponentNode(yyvsp[-3].str, "Default");
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
		  win->Consume(kc);
                  kc->DecRef();
		  yyval.comp = win;
		  free(yyvsp[-3].str);
		;}
    break;

  case 28:
#line 215 "plugins/aws/skinpars.yy"
    { yyval.key = new awsStringKey(yyvsp[-2].str, yyvsp[0].str); free(yyvsp[-2].str); free(yyvsp[0].str); ;}
    break;

  case 29:
#line 217 "plugins/aws/skinpars.yy"
    { yyval.key = new awsRGBKey(yyvsp[-6].str, yyvsp[-4].val, yyvsp[-2].val, yyvsp[0].val); free(yyvsp[-6].str); ;}
    break;

  case 30:
#line 219 "plugins/aws/skinpars.yy"
    { yyval.key = new awsIntKey(yyvsp[-2].str, yyvsp[0].val); free(yyvsp[-2].str); ;}
    break;

  case 31:
#line 221 "plugins/aws/skinpars.yy"
    { yyval.key = new awsPointKey(yyvsp[-6].str, csPoint(yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-6].str); ;}
    break;

  case 32:
#line 223 "plugins/aws/skinpars.yy"
    { yyval.key = new awsRectKey(yyvsp[-12].str, csRect(yyvsp[-9].val, yyvsp[-7].val, yyvsp[-3].val, yyvsp[-1].val)); free(yyvsp[-12].str); ;}
    break;

  case 33:
#line 228 "plugins/aws/skinpars.yy"
    { awsKeyContainer* kc = new awsKeyContainer;
		  kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;}
    break;

  case 34:
#line 233 "plugins/aws/skinpars.yy"
    { awsKeyContainer* kc = yyvsp[-1].keycont;
		  kc->Add(yyvsp[0].key);
		  yyval.keycont = kc;
		;}
    break;

  case 35:
#line 241 "plugins/aws/skinpars.yy"
    { awsSkinNode *skin = new awsSkinNode(yyvsp[-3].str);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) yyvsp[-1].keycont;
                  skin->Consume(kc);
                  kc->DecRef();
		  yyval.skin = skin;
		  free(yyvsp[-3].str);
		;}
    break;

  case 36:
#line 255 "plugins/aws/skinpars.yy"
    { int v = 0;
		  if (!static_awsparser->GetConstantValue(yyvsp[0].str, v))
		    static_awsparser->ReportError ("Constant %s is not defined.", yyvsp[0].str);
		  yyval.val = v;
		  free(yyvsp[0].str);
		;}
    break;

  case 37:
#line 263 "plugins/aws/skinpars.yy"
    { yyval.val = yyvsp[0].val; ;}
    break;

  case 38:
#line 264 "plugins/aws/skinpars.yy"
    { yyval.val = yyvsp[0].val; ;}
    break;

  case 39:
#line 265 "plugins/aws/skinpars.yy"
    { yyval.val = yyvsp[-2].val + yyvsp[0].val; ;}
    break;

  case 40:
#line 266 "plugins/aws/skinpars.yy"
    { yyval.val = yyvsp[-2].val - yyvsp[0].val; ;}
    break;

  case 41:
#line 267 "plugins/aws/skinpars.yy"
    { yyval.val = yyvsp[-2].val * yyvsp[0].val; ;}
    break;

  case 42:
#line 268 "plugins/aws/skinpars.yy"
    { yyval.val = yyvsp[-2].val / yyvsp[0].val; ;}
    break;

  case 43:
#line 269 "plugins/aws/skinpars.yy"
    { yyval.val = -yyvsp[0].val; ;}
    break;

  case 44:
#line 270 "plugins/aws/skinpars.yy"
    { yyval.val = yyvsp[-1].val; ;}
    break;


    }

/* Line 991 of yacc.c.  */
#line 1424 "plugins/aws/skinpars.cpp"

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
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__)
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
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
      yyvsp--;
      yystate = *--yyssp;

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


#line 273 "plugins/aws/skinpars.yy"


int
awserror(char *s)
{
  static_awsparser->ReportError (s);
  return 0;
}


