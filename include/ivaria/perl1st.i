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
  #undef Renew
  #undef Renewc
  #undef Safefree
  #undef StructCopy
  #undef Zero
%}

#endif // SWIGPERL5
