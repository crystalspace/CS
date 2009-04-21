Copyright (C)2005 by Eric Sunshine <sunshine@sunshineco.com>

The document explains how to build the Crystal Space documentation in Microsoft
Compressed HTML format (CHM), which is the standard Windows help file format.


Requirements
------------
o HTML Help Workshop 1.4 (msdn.microsoft.com)
o Perl (www.perl.org or www.activatestate.com)


Generation
----------
These instructions apply to Unix-like platforms or installations, such as
GNU/Linux, Msys/Mingw on Windows, and Cygwin on Windows, which employ `make' or
`jam' when building Crystal Space.  Consult the "Preparing Windows" or
"Preparing GNU/Linux" sections below to learn how to prepare your development
environment for building CHM format documentation. Once the environment has
been configured, follow these steps to actually generate the documentation:

1. Configure Crystal Space's build environment in the normal fashion by
   running `./configure'.

2. Invoke `jam manualchm' (or `make manualchm') to convert the Crystal Space
   user manual to CHM format. The generated file will reside at
   out/docs/chm/manual/csmanual.chm.

3. Invoke `jam apichm' (or `make apichm') to convert the Crystal Space API
   reference to CHM format. The generated file will reside at
   out/docs/chm/api/csapi.chm.

4. Optionally remove generated files (after copying the CHM files elsewhere) by
   invoking `jam cleandoc' or `jam manualchmclean' or `jam apichmclean'.


Preparing Windows
-----------------
Generation of CHM format help files requires hhc.exe, the Microsoft HTML Help
compiler, and Perl.

1. Download the Microsoft HTML Help Workshop installer, htmlhelp.exe:

   http://msdn.microsoft.com/library/en-us/htmlhelp/html/hwMicrosoftHTMLHelpDownloads.asp

   Run the downloaded program to install HTML Help Workshop which contains
   hhc.exe, the HTML Help compiler.

2. Install the ActivePerl distribution from ActiveState:

   http://www.activestate.com/Products/ActivePerl/

3. Edit your PATH environment variable to ensure that it mentions that
   directory in which hhc.exe resides (typically, c:\Program Files\HTML Help
   Workshop) and the path in which perl.exe resides so that the Crystal Space
   configure script can find them.  When you run the Crystal Space configure
   script, verify that it actually locates hhc.exe and perl.exe after printing
   "checking for hhc..." and "checking for perl...", respectively.


Preparing GNU/Linux
-------------------
It is possible to create Microsoft CHM format help files on GNU/Linux by
running hhc.exe, the HTML Help Compiler, via Wine. The following instructions
explain how to create a bare-minimum Wine installation capable of running
hhc.exe.

 1. Install cabextract.

    http://www.kyz.uklinux.net/cabextract.php

 2. Install Wine.

    http://www.winehq.org/

 3. Install WineTools.

    http://www.von-thadden.de/Joachim/WineTools/

    Choose the latest recommended version of WineTools corresponding to your
    version of Wine. Note that some versions of WineTools hardcode a URL to a
    defunct SourceForge download server. If WineTools hangs when attempting to
    download certain packages, it is likely caused by this problem. Resolve the
    issue by editing /usr/bin/wt2 (or /usr/local/bin/wt2) and performing a
    global search/replace of http://mesh.dl.sourceforge.net/ with your
    preferred active SourceForge download server.

    Run WineTools by invoking `wt2' on the command-line.

 4. From WineTools `Base setup' menu choose `Create a fake Windows drive'.

 5. From WineTools `Base setup' menu install `TrueType Font Arial'.

 6. From WineTools `Base setup' menu install `DCOM98'.

    Exit WineTools.

 7. Download the Microsoft Visual C++ runtime installer which is contained in
    vc6redistsetup_enu.exe:

    http://support.microsoft.com/support/kb/articles/Q259/4/03.ASP

    Extract from vc6redistsetup_enu.exe the actual installer, vcredist.exe, and
    run it:

    % cabextract vc6redistsetup_enu.exe
    % wine vcredist.exe

 8. Install the Microsoft HTML Help Workshop installer, htmlhelp.exe:

    http://msdn.microsoft.com/library/en-us/htmlhelp/html/hwMicrosoftHTMLHelpDownloads.asp

    Extract the installer into a directory named `htmlhelp':

    % cabextract -d htmlhelp htmlhelp.exe

    Patch the INF file to avoid a Wine bug:

    % cd htmlhelp
    % sed -i -e 's/;.*$//' htmlhelp.inf

    Install the HTML Help Workshop:

    % wine setup.exe
    % cd ..

 9. Install the Microsoft Foundation Classes update, MFC40i.exe:

    http://download.microsoft.com/download/sql65/Patch/6.5/WIN98/EN-US/MFC40i.exe

    Extract the `mfc40.dll' file and copy it to the Wine `windows\system'
    directory:

    % cabextract -d MFC40i MFC40i.exe
    % cp -a MFC40i/mfc40.dll ~/.wine/fake_windows/windows/system/

10. Create a Unix shell script named `hhc' which runs hhc.exe via Wine.

    % echo '#! /bin/sh' > hhc
    % echo 'exec wine "c:/Program Files/HTML Help Workshop/hhc.exe" $@' >> hhc
    % chmod +x hhc

    Test the script by invoking `hhc' at the command-line. It should print out
    an hhc.exe usage message.

    Place the script into a directory mentioned by your PATH environment
    variable so that the Crystal Space configure script can find it.  When you
    run the Crystal Space configure script, verify that it actually locates the
    `hhc' script after printing "checking for hhc...".

11. If you want to be able to test-browse the generated Crystal Space CHM
    documentation files using hh.exe, the HTML Help viewer, then you will also
    need to install Internet Explorer 6 in Wine. This step is optional; if you
    want only to generate CHM files, but do not care about reading them, then
    you can skip this step.

    From WineTools `Base setup' menu install `Internet Explorer 6.0 SP1'.

    For convenience, you may want to create a Unix shell script named `hh'
    which runs hh.exe via Wine.

    % echo '#! /bin/sh' > hh
    % echo 'exec wine "c:/windows/hh.exe" $@' >> hh
    % chmod +x hh

    Move the script to a directory mentioned by PATH, and test it by invoking
    `hh c:/windows/help/htmlhelp.chm' at the command-line.

---

The very useful information in steps 7 through 10 of "Preparing GNU/Linux" was
gleaned from the slightly outdated but still quite helpful page:

http://htmlhelp.berlios.de/howto/mshh4wine.php
