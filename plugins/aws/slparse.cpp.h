typedef union
{
  char *str;    /* For returning titles and handles to items. */
  int val;      /* For returning numbers                      */
  csRect *rect; /* For returning rectangular regions          */
  awsKey *key;  /* For returning keys to various definition items */
} YYSTYPE;
#define TOKEN_NUM       257
#define TOKEN_STR       258
#define TOKEN_ATTR      259
#define TOKEN_SKIN      260
#define TOKEN_FOR       261
#define TOKEN_WINDOW    262
#define TOKEN_FROM      263
#define TOKEN_COMPONENT 264
#define TOKEN_CONNECT   265
#define TOKEN_IS        266
#define NEG             267
