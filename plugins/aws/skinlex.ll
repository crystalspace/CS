
/*******
  Flex definition for the .skn file parser.

  to compile do:  flex -L skinlang.flx
 *********/

%{

#include "cssysdef.h"
#include "csgeom/csrect.h"
#include "awsprefs.h"
#include "awsparser.h"

#include "iutil/vfs.h"

#include <stdlib.h>
#include <string.h>

#include "skinpars.hpp"

#ifdef YY_PROTO
#define YY_DECL int yylex YY_PROTO(( YYSTYPE *awslval ))
#else
#define YY_DECL int yylex ( YYSTYPE *awslval )
#endif

/* change input to read from vfs */
#define YY_INPUT(buf, result, max_size)			\
{							\
  result = static_awsparser->Read(buf, max_size);	\
}

%}

%option noyywrap
%option yylineno
%option never-interactive
%option prefix="aws"


%%
component                                        return TOKEN_COMPONENT;
connect											 return TOKEN_CONNECT;
is                                               return TOKEN_IS;
skin                    /* printf("skin-");   */ return TOKEN_SKIN;
window                  /* printf("window-"); */ return TOKEN_WINDOW;
for                     /* printf("for-");    */ return TOKEN_FOR;
from                    /* printf("from-");   */ return TOKEN_FROM;
[0-9]+                  /* printf("<int>-");  */ awslval->val = atoi(awstext);   return TOKEN_NUM;
[0-9]*\.[0-9]+          /* printf("<float>-"); */ awslval->fval = atof(awstext);   return TOKEN_FLOAT;
[a-zA-Z]+[a-zA-Z0-9]*   /* printf("<attr>-"); */ awslval->str = strdup(awstext); return TOKEN_ATTR;
\"[^\n\"]*\"             /* printf("<str>-");  */ awstext[awsleng-1]=0; awslval->str = strdup(awstext+1); return TOKEN_STR;
[ \t\r\n]+              /* eat all spaces and stuff */
.                       /* printf("%c-", awstext[0]); */ return awstext[0];
\/\/.*\n 		/* eat comments */
.                       return awstext[0];
%%
