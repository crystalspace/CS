Summary: CrystalSpace free 3d engine
Name: crystalspace
Version: cvs20030114
Release: 1
Group: unknown
License: LGPL
Source: cs-20030114.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
URL: http://crystal.sourceforge.net
#Requires: 
#Obsoletes: 

%description
Crystal Space is a free (LGPL) and portable 3D Development Kit written in C++. It supports: true six degree's of freedom, colored lighting, mipmapping, portals, mirrors, alpha transparency, reflective surfaces, 3D sprites (frame based or with skeletal animation), procedural textures, radiosity, particle systems, halos, volumetric fog, scripting (using Python or other languages), 8-bit, 16-bit, and 32-bit display support, OpenGL and software renderering, font support, hierarchical transformations, etc.

%prep
%setup -n CS

%build
sh ./configure --prefix=%buildroot/usr/local/crystal
make -k all

%install
make -k install

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%doc docs
/usr/local/crystal/bin/blocks
/usr/local/crystal/bin/cs-config
/usr/local/crystal/bin/cs2xml
/usr/local/crystal/bin/g2dtest
/usr/local/crystal/bin/gfxtest
/usr/local/crystal/bin/isomap
/usr/local/crystal/bin/isotest
/usr/local/crystal/bin/levtool
/usr/local/crystal/bin/map2cs
/usr/local/crystal/bin/md32spr
/usr/local/crystal/bin/mdl2spr
/usr/local/crystal/bin/milk2spr
/usr/local/crystal/bin/perftest
/usr/local/crystal/bin/python.cex
/usr/local/crystal/bin/scfreg
/usr/local/crystal/bin/tbconvert
/usr/local/crystal/bin/vsh
/usr/local/crystal/bin/walktest
/usr/local/crystal/data/awsdef.zip
/usr/local/crystal/data/blocks.zip
/usr/local/crystal/data/config/*.cfg
/usr/local/crystal/data/csws.zip
/usr/local/crystal/data/fancycon.zip
/usr/local/crystal/data/flarge/world
/usr/local/crystal/data/isomap/world
/usr/local/crystal/data/partsys/world
/usr/local/crystal/data/standard.zip
/usr/local/crystal/data/stdtex.zip
/usr/local/crystal/docs/README.html
/usr/local/crystal/docs/history.old
/usr/local/crystal/docs/history.txt
/usr/local/crystal/docs/html/build/platform/win32/cygwin/*.jpg
/usr/local/crystal/docs/html/cs_*.html
/usr/local/crystal/docs/html/index.html
/usr/local/crystal/docs/html/plugins/engine/*.jpg
/usr/local/crystal/docs/html/tutorial/howto/kdevproj/*.jpg
/usr/local/crystal/docs/html/tutorial/map2cs/*.jpg
/usr/local/crystal/docs/html/tutorial/map2cs/*.png
/usr/local/crystal/docs/html/tutorial/wincvs/*.jpg
/usr/local/crystal/docs/pubapi/*.html
/usr/local/crystal/docs/pubapi/*.gif
   
/usr/local/crystal/include/*.h
/usr/local/crystal/include/csengine/*.h
/usr/local/crystal/include/csgeom/*.h
/usr/local/crystal/include/csgfx/*.h
/usr/local/crystal/include/cssys/*.h
/usr/local/crystal/include/cssys/macosx/*.h
/usr/local/crystal/include/cssys/unix/*.h
/usr/local/crystal/include/cssys/win32/*.h
/usr/local/crystal/include/cstool/*.h
/usr/local/crystal/include/csutil/*.h
/usr/local/crystal/include/csws/*.h
/usr/local/crystal/include/iaws/*.h
/usr/local/crystal/include/iengine/*.h
/usr/local/crystal/include/igeom/*.h
/usr/local/crystal/include/igraphic/*.h
/usr/local/crystal/include/imap/*.h
/usr/local/crystal/include/imesh/*.h
/usr/local/crystal/include/imesh/thing/*.h
/usr/local/crystal/include/inetwork/*.h
/usr/local/crystal/include/isound/*.h
/usr/local/crystal/include/iutil/*.h
/usr/local/crystal/include/ivaria/*.h
/usr/local/crystal/include/ivideo/*.h
/usr/local/crystal/include/ivideo/effects/*.h
/usr/local/crystal/include/ivideo/shader/*.h

/usr/local/crystal/install.log

/usr/local/crystal/lib/*.so
   
/usr/local/crystal/scf.cfg
/usr/local/crystal/scripts/python/*.py
/usr/local/crystal/uninst
/usr/local/crystal/vfs.cfg

/usr/local/crystal/docs/html/plugins/engine/*.png
/usr/local/crystal/docs/html/tutorial/howto/msvcproj/*.jpg

/usr/local/crystal/docs/pubapi/doxygen.css
/usr/local/crystal/lib/*.a

%changelog
* Tue Jan 14 2003 Eric Sunshine <sunshine@users.sourceforge.net>
- Upgraded for new Autoconf project configuration used by Crystal Space.

* Tue Dec 24 2002 Che <newrpms.sunsite.dk>
- Some adaptions for latest cvs tarball

* Thu Nov 14 2002 Che <newrpms.sunsite.dk>
- Initial rpm release
