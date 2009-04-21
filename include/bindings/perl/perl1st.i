#ifdef SWIGPERL5

/*
    The Perl headers define these macros, which go unused and conflict with
    names in CS, so we undef them here before anything else is included.
*/

%{
  #undef Copy
  #undef MAXXCOUNT
  #undef MAXY_SIZE
  #undef MAXYCOUNT
  #undef Move
  #undef New
  #undef Newc
  #undef Newz
  #undef Pause
  #undef Renew
  #undef Renewc
  #undef Safefree
  #undef StructCopy
  #undef Zero
%}

/*
    Very recent versions of Swig #undef the Perl ENTER macro, yet we require
    the macro for our custom functions which access PerlAPI.
*/
%{
#ifndef ENTER
#define ENTER push_scope()
#endif
%}

#endif // SWIGPERL5
