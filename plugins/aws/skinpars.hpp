#ifndef BISON____CS_PLUGINS_AWS_SKINPARS_HPP
# define BISON____CS_PLUGINS_AWS_SKINPARS_HPP

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


#endif /* not BISON____CS_PLUGINS_AWS_SKINPARS_HPP */
