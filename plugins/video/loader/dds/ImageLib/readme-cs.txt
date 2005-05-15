Jason Dorie's ImageLib, downloaded from http://jasondorie.com/web/main.html .

License is "It's also free to anyone who wants to use it, for any reason. 
Even commercially."

Notes:
- CS is cross-platform, hence all assembler code has been replaced with its
  C++ equivalent.
- The ImageLib comes with an MMX-optimized and plain C++ implementation of
  CodeBook. For the same reason as above, the latter is used.
- Various other portability-related changes.
- Everything was stuffed into a namespace.
- The files CodeBook4MMX.*, Load*.* and Usage.cpp are only here for 
  completeness' sake, but aren't used by CS itself.
