#ifndef BISON_PLUGINS_AWS_SLP_HPP
# define BISON_PLUGINS_AWS_SLP_HPP

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


#endif /* not BISON_PLUGINS_AWS_SLP_HPP */
