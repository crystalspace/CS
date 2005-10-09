/*
    Copyright (C) 2001 by Christopher Nelson

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*******
  Flex definition for the .skn file parser.

  to compile do:  flex -L skinlang.flx
 *********/

%{

/* MSVC 8 complains about deprecated methods in the generated code. Although
 * inclusion of cssysdef.h as the very first header would enable a workaround
 * for that, we can't ensure that; hence, just disable the warning here. */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable:4996)
#endif

#include "cssysdef.h"
#include "csgeom/csrect.h"
#include "awsprefs.h"
#include "awsparser.h"

#include "iutil/vfs.h"

#include <stdlib.h>
#include <string.h>

#include "skinpars.hpp"

// We have no control over this generated code, so silence some warnings.
#if defined(CS_COMPILER_MSVC)
#pragma warning(disable:4065)
#pragma warning(disable:4102)
#endif

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
\/\/.*\n 		/* eat comments */
#.*\n                   /* eat comments */
.                       return awstext[0];
%%
