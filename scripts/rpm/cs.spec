%define name    crystalspace
%define version 20040628
%define release 1
%define prefix	/usr

%define with_DEBUG 0
%{?_without_debug: %{expand: %%global with_DEBUG 0}}
%{?_with_debug: %{expand: %%global with_DEBUG 1}}
                                                                                                         
%define with_NR 0
%{?_without_newrenderer: %{expand: %%global with_NR 0}}
%{?_with_newrenderer: %{expand: %%global with_NR 1}}
                                                                                                         
%define with_PERL 0
%{?_without_perl: %{expand: %%global with_PERL 0}}
%{?_with_perl: %{expand: %%global with_PERL 1}}

Group: Applications/Development
Source: http://crystal.sourceforge.net/cvs-snapshots/bzip2/cs-current-snapshot.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-root
URL: http://crystal.sourceforge.net/
#Requires: 
#Obsoletes:

# Main rpm and .src.rpm packages
Summary: Crystal Space free 3D SDK
Name: %{name}
Release: %{release}
Version: %{version}
License: LGPL

%description
Crystal Space is a free (LGPL) and portable 3D SDK written in C++.

# Dev package
%package -n %{name}-devel
Summary: C++ headers for Crystal Space free 3D SDK.
Group: Application/Development
Provides:       %{name}-devel = %{version}-%{release}
%description -n %{name}-devel
Headers and files needed for building a Crystal Space apps

# Docs package
%package -n %{name}-doc
Summary: Documentation for Crystal Space free 3D SDK
Group: Application/Development
Provides:       %{name}-doc = %{version}-%{release}
%description -n %{name}-doc
Documentation (manual and public API reference) for CrystalSpace free 3D SDK.

%prep
%setup -n CS

%build
sh configure \
%if %{with_DEBUG}
 --enable-debug \
%endif
%if %{with_NR}
 --enable-new-renderer \
%endif
%if %{with_PERL}
 --with-perl \
%endif
--prefix=%{prefix}

make all

%install
DESTDIR=%{buildroot} make install

%if ! %{with_NR}
 CRYSTAL=%{buildroot}%{prefix} \
 CS_DATADIR=%{buildroot}%{prefix}/share/crystal \
 CS_MAPDIR=%{buildroot}%{prefix}/share/crystal/maps \
 %{buildroot}%{prefix}/bin/cslight -canvas=null2d -video=null flarge

 CRYSTAL=%{buildroot}%{prefix} \
 CS_DATADIR=%{buildroot}%{prefix}/share/crystal \
 CS_MAPDIR=%{buildroot}%{prefix}/share/crystal/maps \
 %{buildroot}%{prefix}/bin/cslight -canvas=null2d -video=null partsys
%endif

%clean
rm -rf "$RPM_BUILD_ROOT"

#---------------------MAIN-------------------------
%files -n %{name}
%defattr(-,root,root)
%{prefix}/bin/*
%{prefix}/etc/crystal/*
%{prefix}/lib/crystal/*
%{prefix}/share/crystal/*

#---------------------DOC-------------------------
%files -n %{name}-doc
%defattr(-,root,root)
%docdir docs
%{prefix}/share/doc/crystal/README.html
%{prefix}/share/doc/crystal/html/manual/*.html
%{prefix}/share/doc/crystal/html/manual/build/platform/win32/cygwin/*.jpg
%{prefix}/share/doc/crystal/html/manual/tutorial/howto/msvc6proj/*.jpg
%{prefix}/share/doc/crystal/html/manual/tutorial/howto/msvc7proj/*.jpg
%{prefix}/share/doc/crystal/html/manual/tutorial/howto/kdevproj/*.jpg
%{prefix}/share/doc/crystal/html/manual/tutorial/map2cs/*.png
%{prefix}/share/doc/crystal/html/manual/tutorial/map2cs/*.jpg
%{prefix}/share/doc/crystal/html/manual/tutorial/wincvs/*.jpg
%{prefix}/share/doc/crystal/html/manual/plugins/engine/*.jpg
%{prefix}/share/doc/crystal/html/manual/plugins/engine/*.png

%{prefix}/share/doc/crystal/history.txt
%{prefix}/share/doc/crystal/history.old

%{prefix}/share/doc/crystal/html/api/*

#---------------------DEV-------------------------
%files -n %{name}-devel
%defattr(-,root,root)
%{prefix}/lib/*.a
%{prefix}/include/*.h
%{prefix}/include/csws/*.h
%{prefix}/include/iaws/*.h
%{prefix}/include/imap/*.h
%{prefix}/include/csgfx/*.h
%{prefix}/include/igeom/*.h
%{prefix}/include/imesh/*.h
%{prefix}/include/iutil/*.h
%{prefix}/include/csgeom/*.h
%{prefix}/include/cstool/*.h
%{prefix}/include/csutil/*.h
%{prefix}/include/csutil/*.inc
%{prefix}/include/csutil/win32/*.h
%{prefix}/include/csutil/win32/*.inc
%{prefix}/include/csutil/win32/*.fun
%{prefix}/include/csutil/macosx/*.h
%{prefix}/include/csutil/unix/*.h

%{prefix}/include/isound/*.h
%{prefix}/include/ivaria/*.h
%{prefix}/include/ivaria/*.i
%{prefix}/include/ivideo/*.h
%{prefix}/include/ivideo/shader/*.h
%{prefix}/include/itexture/*.h
%{prefix}/include/iengine/*.h
%{prefix}/include/iengine/rendersteps/*.h
%{prefix}/include/inetwork/*.h
%{prefix}/include/igraphic/*.h

%changelog
* Mon Jun 28 2004 Vincent Knecht <vknecht@users.sourceforge.net> 20040628-1
- Added conditional build flags to enable debug, NR and perl plugin.
- Disabled relighting of levels when building NR. Crashes for me.

* Mon May 31 2004 Vincent Knecht <vknecht@users.sourceforge.net> 20040531-2
- Added %files entries for .inc and .fun files in include/csutil/

* Tue May 11 2004 Vincent Knecht <vknecht@users.sourceforge.net> 20040511-1
- Prefixed source file name with URL.
- Building is done by the make emulation layer, which will automatically
  call system installed jam if present, or use CS provided jam.

* Sun May 9 2004 Vincent Knecht <vknecht@users.sourceforge.net> 20040509-1
- Building is done by jam provided with Crystal Space if not installed
- Specified null2d canvas for levels relighting
- Added %{prefix}/include/ivaria/*.i to devel %files section
- Removed reference to include/csappframe/ dir

* Sat Feb 28 2004 Vincent Knecht <vknecht@users.sourceforge.net> 20040228-1
- Added lightmaps cache computation for flarge and partsys

* Sat Feb 28 2004 Philip Wyett <philip@wyett.net>
- Removed reference to 'include/imesh/thing/ dir.

* Tue Feb 10 2004 Eric Sunshine <sunshine@users.sourceforge.net>
- Unification and clean up of package descriptions.

* Mon Feb 9 2004 Vincent Knecht <vknecht@users.sourceforge.net> 20040209-3
- Using /usr for %{prefix}
- Fixed bogus doc package by using %docdir instead of %doc

* Fri Feb 7 2004 Vincent Knecht <vknecht@users.sourceforge.net> 20040207-1
- using jam to build and install
- Update %files section according to latest cvs snapshot
- Use %{prefix} instead of hardcoded /usr/local/crystal
- splitted %files section in multiple packages ('main', -devel, -doc)

* Tue Jan 14 2003 Eric Sunshine <sunshine@users.sourceforge.net>
- Upgraded for new Autoconf project configuration used by Crystal Space.

* Tue Dec 24 2002 Che <newrpms.sunsite.dk>
- Some adaptions for latest cvs tarball

* Thu Nov 14 2002 Che <newrpms.sunsite.dk>
- Initial rpm release
