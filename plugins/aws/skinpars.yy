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

/*********
  Bison definitions for skin files.
 *********/

/** Note:
 * The proper command-line for compiling this file with bison is:
 *
 *    bison --no-lines -d -p aws -o skinpars.cpp skinpars.yy
 *
 */

%{

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

%}



%pure_parser
%union {
  char   *str;			/* For returning titles and handles to items. */
  int     val;			/* For returning integer numbers              */
  float  fval;			/* For returning non-integer numbers          */
  csRect *rect;			/* For returning rectangular regions          */
  awsKey *key;     		/* For returning keys to various definition items */
  awsComponentNode *comp;	/* for returning windows		      */
  awsKeyContainer *keycont;	/* for returning KeyContainers		      */
  awsSkinNode *skin;		/* for returning skins			      */
}


%{

extern int awslex(YYSTYPE *awslval);
extern int awserror(char *s);
extern int awslineno;

/// This is the parser parameter
#define YYPARSE_PARAM windowmgr

%}

%token <val>  TOKEN_NUM
%token <fval> TOKEN_FLOAT
%token <str>  TOKEN_STR
%token <str>  TOKEN_ATTR
%token        TOKEN_SKIN
%token        TOKEN_FOR
%token        TOKEN_WINDOW
%token        TOKEN_FROM
%token        TOKEN_COMPONENT
%token	      TOKEN_CONNECT
%token        TOKEN_IS

%type  <fval>	expf
%type  <val>	exp constant_item
%type  <key>	skin_item window_item component_item connection_item
%type  <keycont> component_item_list connection_item_list window_item_list skin_item_list
%type  <comp>	window
%type  <skin>	skin

%right '='
%left '-' '+'
%left '*' '/'
%left NEG     /* Negation--unary minus */
%right '^'    /* Exponentiation        */


%%

input:          /* empty */
               | input line
;

line:            '\n'
               | skin                   { static_awsparser->AddGlobalSkinDef($1); }
               | window                 { static_awsparser->AddGlobalWindowDef($1); }
               | error '}'              { yyerrok;      }
;

/*************************************
  Definitions for parsing a window def
 *************************************/

connection_item:
	exp '-' '>' TOKEN_ATTR ':' ':' TOKEN_ATTR
		{ $$ = static_awsparser->MapSourceToSink ($1, $4, $7); free($4); free($7); }
;

connection_item_list:
	connection_item
		{ awsKeyContainer* kc = new awsKeyContainer((iAws*)windowmgr);
		  if ($1) kc->Add($1);
		  $$ = kc;
		}
	| connection_item_list connection_item
		{ awsKeyContainer* kc = $1;
		  if ($2) kc->Add($2);
		  $$ = kc;
		}
;

component_item:
	TOKEN_ATTR ':' TOKEN_STR
		{  $$ = new awsStringKey((iAws*)windowmgr, $1, $3); free($1); free($3); }
	| TOKEN_ATTR ':' expf
		{  $$ = new awsFloatKey((iAws*)windowmgr, $1, $3); free($1); }
	| TOKEN_ATTR ':' exp
		{  $$ = new awsIntKey((iAws*)windowmgr, $1, $3); free($1); }
	| TOKEN_ATTR ':' exp ',' exp ',' exp
		{  $$ = new awsRGBKey((iAws*)windowmgr, $1, $3, $5, $7); free($1); }
	| TOKEN_ATTR ':' '(' exp ',' exp ')' '-' '(' exp ',' exp ')'
		{  $$ = new awsRectKey((iAws*)windowmgr, $1, csRect($4, $6, $10, $12)); free($1); }
	| TOKEN_CONNECT	'{' connection_item_list '}'
		{ awsConnectionNode *cn = new awsConnectionNode((iAws*)windowmgr);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) $3;
		  cn->Consume (kc);
		  kc->DecRef();
		  $$ = cn;
		}
	| TOKEN_COMPONENT TOKEN_STR TOKEN_IS TOKEN_STR '{' component_item_list '}'
		{ awsComponentNode *cn = new awsComponentNode((iAws*)windowmgr, $2, $4);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) $6;
		  cn->Consume(kc);
                  kc->DecRef();
		  $$ = cn;
		  free($2);
		  free($4);
		}
	| TOKEN_WINDOW TOKEN_STR '{' component_item_list '}'
		{ awsComponentNode *cn = new awsComponentNode((iAws*)windowmgr, $2, "Window");
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) $4;
		  cn->Consume(kc);
                  kc->DecRef();
		  $$ = cn;
		  free($2);
		}
;

component_item_list:
	component_item
		{ awsKeyContainer* keycontainer = new awsKeyContainer((iAws*)windowmgr);
		  keycontainer->Add($1);
		  $$ = keycontainer;
		}
	| component_item_list component_item
		{ awsKeyContainer* keycontainer = $1;
		  keycontainer->Add($2);
		  $$ = keycontainer;
		}
;


window_item:
	TOKEN_ATTR ':' TOKEN_STR
		{ $$ = new awsStringKey((iAws*)windowmgr, $1, $3); free($1); free($3); }
	| TOKEN_ATTR ':' expf
		{ $$ = new awsFloatKey((iAws*)windowmgr, $1, $3); free($1); }
	| TOKEN_ATTR ':' exp
		{ $$ = new awsIntKey((iAws*)windowmgr, $1, $3); free($1); }
	| TOKEN_ATTR ':' '(' exp ',' exp ')' '-' '(' exp ',' exp ')'
		{ $$ = new awsRectKey((iAws*)windowmgr, $1, csRect($4, $6, $10, $12)); free($1); }
	| TOKEN_CONNECT '{' connection_item_list '}'
		{ awsConnectionNode *cn = new awsConnectionNode((iAws*)windowmgr);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) $3;
		  cn->Consume(kc);
                  kc->DecRef();
		  $$=cn;
		}
        | TOKEN_COMPONENT TOKEN_STR TOKEN_IS TOKEN_STR '{' component_item_list '}'
		{ awsComponentNode *cn = new awsComponentNode((iAws*)windowmgr, $2, $4);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) $6;
		  cn->Consume(kc);
                  kc->DecRef();
		  $$=cn;
		  free($2);
		  free($4);
		}
;

window_item_list:
	window_item
	      { awsKeyContainer* cnt = new awsKeyContainer((iAws*)windowmgr);
		cnt->Add($1);
		$$ = cnt;
	      }
	| window_item_list window_item
	      { awsKeyContainer* cnt = $1;
	        cnt->Add($2);
	        $$ = cnt;
	      }
;

window:
	TOKEN_WINDOW TOKEN_STR  '{' window_item_list '}'
		{ awsComponentNode *win = new awsComponentNode((iAws*)windowmgr, $2, "Default");
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) $4;
		  win->Consume(kc);
                  kc->DecRef();
		  $$ = win;
		  free($2);
		}
;

/*************************************
  Definitions for parsing a skin def
 *************************************/

skin_item:
	TOKEN_ATTR ':' TOKEN_STR
		{ $$ = new awsStringKey((iAws*)windowmgr, $1, $3); free($1); free($3); }
	| TOKEN_ATTR ':' exp ',' exp ',' exp
		{ $$ = new awsRGBKey((iAws*)windowmgr, $1, $3, $5, $7); free($1); }
	| TOKEN_ATTR ':' expf
		{ $$ = new awsFloatKey((iAws*)windowmgr, $1, $3); free($1); }
	| TOKEN_ATTR ':' exp
		{ $$ = new awsIntKey((iAws*)windowmgr, $1, $3); free($1); }
	| TOKEN_ATTR ':' '(' exp ',' exp ')'
		{ $$ = new awsPointKey((iAws*)windowmgr, $1, csPoint($4, $6)); free($1); }
	| TOKEN_ATTR ':' '(' exp ',' exp ')' '-' '(' exp ',' exp ')'
		{ $$ = new awsRectKey((iAws*)windowmgr, $1, csRect($4, $6, $10, $12)); free($1); }
;

skin_item_list:
	skin_item
		{ awsKeyContainer* kc = new awsKeyContainer((iAws*)windowmgr);
		  kc->Add($1);
		  $$ = kc;
		}
	| skin_item_list skin_item
		{ awsKeyContainer* kc = $1;
		  kc->Add($2);
		  $$ = kc;
		}
;

skin:
	TOKEN_SKIN TOKEN_STR '{' skin_item_list '}'
		{ awsSkinNode *skin = new awsSkinNode((iAws*)windowmgr, $2);
                  iAwsKeyContainer* kc = (iAwsKeyContainer*) $4;
                  skin->Consume(kc);
                  kc->DecRef();
		  $$ = skin;
		  free($2);
		}
;

/*************************************
  Definitions for parsing an expression
 *************************************/
constant_item:
	TOKEN_ATTR
		{ int v = 0;
		  if (!static_awsparser->GetConstantValue($1, v))
		    static_awsparser->ReportError ("Constant %s is not defined.", $1);
		  $$ = v;
		  free($1);
		}
;

expf:	TOKEN_FLOAT		{ $$ = $1; }
	| expf '+' expf		{ $$ = $1 + $3; }
	| expf '-' expf		{ $$ = $1 - $3; }
	| expf '*' expf		{ $$ = $1 * $3; }
	| expf '/' expf		{ $$ = $1 / $3; }
	| '-' expf %prec NEG	{ $$ = -$2; }
	| '(' expf ')'		{ $$ = $2; }
;

exp:    TOKEN_NUM		{ $$ = $1; }
	| constant_item		{ $$ = $1; }
	| exp '+' exp		{ $$ = $1 + $3; }
	| exp '-' exp		{ $$ = $1 - $3; }
	| exp '*' exp		{ $$ = $1 * $3; }
	| exp '/' exp		{ $$ = $1 / $3; }
	| '-' exp %prec NEG	{ $$ = -$2; }
	| '(' exp ')'		{ $$ = $2; }
;

%%

int
awserror(char *s)
{
  static_awsparser->ReportError (s);
  return 0;
}
