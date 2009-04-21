%define name     crystalspace
%define version  1.3
%define release  0.svn20070721.1
%define prefix   /usr
%define csprefix crystalspace-%{version}


%{?dist: %{expand: %%define %dist 1}}

%{?fc1:%define _without_xorg 1}
%{?el3:%define _without_xorg 1}
%{?rh9:%define _without_xorg 1}
%{?rh8:%define _without_xorg 1}
%{?rh7:%define _without_xorg 1}
%{?el2:%define _without_xorg 1}
%{?rh6:%define _without_xorg 1}

%define with_DEBUG 0
%{?_without_debug: %{expand: %%global with_DEBUG 0}}
%{?_with_debug: %{expand: %%global with_DEBUG 1}}

# (vk) This should prevent symbol stripping.
%define __spec_install_post /usr/lib/rpm/brp-compress || :
# (vk) Disable RPM's external debug info package
%define debug_package %{nil}

%define with_PERL 0
%{?_without_perl: %{expand: %%global with_PERL 0}}
%{?_with_perl: %{expand: %%global with_PERL 1}}

%define with_SHARED 0
%{?_without_shared: %{expand: %%global with_SHARED 0}}
%{?_with_shared: %{expand: %%global with_SHARED 1}}

Group: Development/C++
Source: http://www.crystalspace3d.org/cvs-snapshots/bzip2/cs-current-snapshot.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-root
URL: http://www.crystalspace3d.org/

BuildRequires: zlib-devel, libpng-devel, libjpeg-devel, libmng-devel
BuildRequires: lcms-devel, libogg-devel, libvorbis-devel, alsa-lib-devel
BuildRequires: gcc-c++, XFree86-devel, freetype-devel, lib3ds = 1.2.0
BuildRequires: cal3d-devel >= 0.11.0, ode-devel >= 0.6
#BuildRequires: Cg >= 1.4, cegui-devel >= 0.4.1
%{?_without_xorg:BuildRequires: XFree86-Mesa-libGLU}
%{!?_without_xorg:BuildRequires: xorg-x11-Mesa-libGLU}

#Requires:
#Obsoletes:

# Main rpm and .src.rpm packages
Summary: Crystal Space free 3D SDK
Name: %{name}
Release: %{release}
Version: %{version}
License: LGPL

%description
Crystal Space is a free (LGPL) and portable 3D SDK
written in C++.

# Utils package
%package -n %{name}-utils
Summary: Utilities for Crystal Space free 3D SDK.
Group: Development/C++
Provides:       %{name}-utils = %{version}-%{release}
Requires:       %{name} = %{version}-%{release}
%description -n %{name}-utils
Utilities for Crystal Space free 3D SDK.

# Demos package
%package -n %{name}-demos
Summary: Demos for Crystal Space free 3D SDK.
Group: Development/C++
Provides:       %{name}-demos = %{version}-%{release}
Requires:       %{name} = %{version}-%{release}
Requires:       %{name}-utils = %{version}-%{release}
%description -n %{name}-demos
Demos for Crystal Space free 3D SDK.

# Dev package
%package -n %{name}-devel
Summary: C++ headers and link libraries for Crystal Space free 3D SDK.
Group: Development/C++
Provides:       %{name}-devel = %{version}-%{release}
Requires:       %{name} = %{version}-%{release}
%description -n %{name}-devel
Headers and link libraries needed for building
projects based upon the Crystal Space 3D SDK.

# Docs package
%package -n %{name}-doc
Summary: Documentation for Crystal Space free 3D SDK
Group: Development/C++
Provides:       %{name}-doc = %{version}-%{release}
%description -n %{name}-doc
Documentation (manual and public API reference)
for Crystal Space free 3D SDK.

%prep
%setup -n CS

%build
sh  configure \
%if %{with_DEBUG}
 --enable-debug \
%endif
%if %{with_PERL}
 --with-perl \
%endif
%if %{with_SHARED}
  --enable-shared \
%endif
 --prefix=%{prefix} \
 --libdir=%{prefix}/lib \
 --datadir=%{prefix}/share \
 --sysconfdir=/etc

%{__make} %{?_smp_mflags} all

%install
DESTDIR=%{buildroot} %{__make} %{?_smp_mflags} install

%post -n %{name}-utils
for map in castle flarge partsys terrain terrainf ; \
  do %{_bindir}/cslight -video=null $map ; \
  done ;

%clean
%{__rm} -rf %{buildroot}

%files -n %{name}
%defattr(-,root,root)

%{_sysconfdir}/%{csprefix}/*
%exclude %{_sysconfdir}/%{csprefix}/autoexec.cfg
%exclude %{_sysconfdir}/%{csprefix}/csdemo.cfg
%exclude %{_sysconfdir}/%{csprefix}/g2dtest.cfg
%exclude %{_sysconfdir}/%{csprefix}/heightmapgen.cfg
%exclude %{_sysconfdir}/%{csprefix}/lighter.xml
%exclude %{_sysconfdir}/%{csprefix}/lighter2.cfg
%exclude %{_sysconfdir}/%{csprefix}/startme.cfg
%exclude %{_sysconfdir}/%{csprefix}/walktest.cfg
%exclude %{_sysconfdir}/%{csprefix}/waterdemo.cfg
%if %{with_SHARED}
%{_libdir}/*.so.*
%endif
%{_libdir}/%{csprefix}/*
%{_datadir}/%{csprefix}/data/*
%exclude %{_datadir}/%{csprefix}/data/startme.zip
%exclude %{_datadir}/%{csprefix}/data/ceguitest/
%exclude %{_datadir}/%{csprefix}/data/maps/*

%files -n %{name}-utils
%defattr(-,root,root)

%{_bindir}/*
%exclude %{_bindir}/cs-config*
%exclude %{_bindir}/ceguitest
%exclude %{_bindir}/csdemo
#%exclude %{_bindir}/isotest
#%exclude %{_bindir}/lghtngtest
%exclude %{_bindir}/startme
#%exclude %{_bindir}/waterdemo
%{_datadir}/%{csprefix}/data/maps/*
%{_datadir}/%{csprefix}/conversion/*
%{_sysconfdir}/%{csprefix}/autoexec.cfg
%{_sysconfdir}/%{csprefix}/heightmapgen.cfg
%{_sysconfdir}/%{csprefix}/lighter.xml
%{_sysconfdir}/%{csprefix}/lighter2.cfg
%{_sysconfdir}/%{csprefix}/walktest.cfg

%files -n %{name}-demos
%defattr(-,root,root)

%{_bindir}/ceguitest
%{_bindir}/csdemo
#%{_bindir}/isotest
#%{_bindir}/lghtngtest
%{_bindir}/startme
#%{_bindir}/waterdemo
%{_sysconfdir}/%{csprefix}/csdemo.cfg
%{_sysconfdir}/%{csprefix}/g2dtest.cfg
%{_sysconfdir}/%{csprefix}/startme.cfg
%{_sysconfdir}/%{csprefix}/waterdemo.cfg
%{_datadir}/%{csprefix}/data/startme.zip
%{_datadir}/%{csprefix}/data/ceguitest/

%files -n %{name}-doc
%defattr(-,root,root)

%docdir docs
%{_datadir}/doc/%{csprefix}/*

%files -n %{name}-devel
%defattr(-,root,root)

%{_bindir}/cs-config*
%if %{with_SHARED}
%else
%{_libdir}/*.a
%endif
# (vk) Scripting related files are here for now
%{_datadir}/%{csprefix}/bindings/*
%{_datadir}/%{csprefix}/build/*
%{_includedir}/%{csprefix}/*

%changelog
* Sat Jul 21 2007 Vincent Knecht <vknecht@users.sourceforge.net> 1.3-0.svn20070721.1
- Updated for AWS stuff removal and lighter2 addition.
- Updated for multiple install support (versioned directories and cs-config).

* Tue Oct 24 2006 Vincent Knecht <vknecht@users.sourceforge.net> 1.1-0.svn20061024.1
- Updated for SVN and new trunk version number.
- Disabled stripping even in optimize mode, it erases .crystalspace section.
- Disabled external debug info package generation.
- Added BuildRequires directives.

* Sun Mar 19 2006 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-0.cvs20060319.1
- Added shared lib option. Note that this option makes the package unusable as
  a SDK for now.

* Mon Oct 17 2005 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-0.cvs20051016.1
- Added demos subpackage, no stripping for debug build and SMP friendly macros
  for make.

* Fri Sep 23 2005 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-0.cvs20050924.1
- Added 'startme' stuff.

* Sun Apr 24 2005 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-0.cvs20050424.1
- Removed csbumptest.cfg occurences.
- Changed release numbering scheme so one knows the date of tested snapshot.
- Put maps relighting in a for loop. Added terrain and terrainf to the list.
- Changed occurences of %{name} to %{csprefix} where value is not dependent
  upon package name.

* Fri Mar 25 2005 Frank Richter <resqu@gmx.ch> 0.99-11
- The CSWS library has been retired.

* Sun Feb 27 2005 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-10
- Cleanup (using more wildcards, %exclude and %{_bindir}).

* Fri Feb 25 2005 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-9
- Upgraded for new csplugincommon include directory.

* Wed Jan 05 2005 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-8
- Made a separate package for utilities.

* Mon Dec 13 2004 Eric Sunshine <sunshine@sunshineco.com> 0.99-7
- The old-renderer has been retired.

* Sun Nov 28 2004 Eric Sunshine <sunsihne@sunshineco.com> 0.99-6
- Crystal Space now has its own domain: crystalspace3d.org.
- New renderer is now default (configure with --enable-old-renderer for old).

* Mon Nov 22 2004 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-5
- Moved cslight invokations to post-install step.
- Changed cslight invokations to use null instead of null2d.

* Thu Nov 04 2004 Eric Sunshine <sunshine@sunshineco.com> 0.99-4
- Upgraded for new location of installed map files.
- Improved the package descriptions.

* Sat Aug 07 2004 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-3
- Upgraded for new headers installation directory.

* Fri Jul 09 2004 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-2
- Simplified and re-enabled levels relighting.

* Thu Jul 08 2004 Vincent Knecht <vknecht@users.sourceforge.net> 0.99-1
- Adaptation for CS PACKAGE_NAME change.
- Changed Group: to Development/C++
- Disabled levels relighting since lightmaps aren't written where expected.

* Sun Jul 04 2004 Vincent Knecht <vknecht@users.sourceforge.net> 20040704-1
- Specified datadir, libdir and sysconfig switches at configure step.
- Specified CS_CONFIGDIR in cslight commands.
- Re-enabled levels relighting when building NR.

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
