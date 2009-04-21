/*
    Copyright (C) 2003 Rene Jager <renej_frog@users.sourceforge.net>

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

/*
  SWIG interface for Crystal Space

  This file includes scripting language specific SWIG files.  Search for
  'pythpre' (pre-include phase) and 'pythpost' (post-include phase) for places
  where to include files for new scripting languages.

  Python (renej_frog@users.sourceforge.net):
    Used for the new "cspython" plugin based on python 2.2 including almost
    complete access to CS.

    Additionally, provides CS as a python 2.2 module name "cspace".  To use
    this, make sure cspace.py and _cspace.so in directory
    $CRYSTAL/scripts/python can be found by python; and don't forget to set the
    CRYSTAL environment variable.  See $CRYSTAL/scripts/python/pysimpcd.py for
    an example of usage.

  Perl (oktal@gmx.co.uk):
    Used for the "csperl5" plugin based on perl 5.8 including access to most
    parts of CS (although some ealier versions are probably supported).

    Additionally, provices CS as a perl 5.8 module name "cspace".  Make sure
    the directory containing cspace.pm and cspace.so, $CRYSTAL/scripts/perl5,
    is in perl's @INC array.

  Java (renej_frog@users.sourceforge.net):
    A wrapper for using CS from Java is provided: crystalspace.jar and
    libcsjava.so.

  Note:
    Tested with swig 1.3.17 and up, swig 1.1 won't work!

  Thanks to:
    Norman Kramer <norman@users.sourceforge.net> who made me think
      about sequence of declarations in SWIG file.
    Mark Gossage <mark@gossage.cjb.net> who made me think about
      preventing handling of smart pointers by SWIG to reduce
      size of generated code.
    Mat Sutcliffe <oktal@gmx.co.uk> for input, Perl includes,
      and wrapping SCF and other macros.
    Eric Sunshine <sunshine@sunshineco.com> for all the hours spent tracking
      down and fixing bugs.
*/

/*
  SWIG's preprocessor can be used to "replace" a macro by a function in the
  target scripting language using the following:

  In C header file:

    #define X(arg) ...

  which as a function would have the prototype:

    int X (int i);

  Now in the SWIG *.i file do:

    #define _X(a) X(a)
    #undef X
    int _X (int i);

  and there will be a "X" function in the scripting language available.
*/

%ignore iBase::~iBase(); // We replace iBase dtor with one that calls DecRef().
			 // Swig already knows not to delete an SCF pointer.

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) INTERFACE_POST(x)

%define cs_apply_all_interfaces
    APPLY_FOR_ALL_INTERFACES_POST
%enddef

%define cs_lang_include(file)
  #if defined(SWIGPYTHON)
    %include bindings/python/ ## file
  #elif defined(SWIGPERL5)
    %include bindings/perl/ ## file
  #elif defined(SWIGRUBY)
    %include bindings/ruby/ ## file
  #elif defined(SWIGTCL8)
    %include bindings/tcl/ ## file
  #elif defined(SWIGJAVA)
    %include bindings/java/ ## file
  #elif defined(SWIGLUA)
    %include bindings/lua/ ## file
  #endif
%enddef

#undef new
