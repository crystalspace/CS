%define name    crystalspace
%define version 0.99
%define release 10
%define prefix	/usr

%define with_DEBUG 0
%{?_without_debug: %{expand: %%global with_DEBUG 0}}
%{?_with_debug: %{expand: %%global with_DEBUG 1}}
                                                                                                         
%define with_PERL 0
%{?_without_perl: %{expand: %%global with_PERL 0}}
%{?_with_perl: %{expand: %%global with_PERL 1}}

Group: Development/C++
Source: http://www.crystalspace3d.org/cvs-snapshots/bzip2/cs-current-snapshot.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-root
URL: http://www.crystalspace3d.org/
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
for CrystalSpace free 3D SDK.

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
 --prefix=%{prefix} \
 --libdir=%{prefix}/lib \
 --datadir=%{prefix}/share \
 --sysconfdir=/etc

make all

%install
DESTDIR=%{buildroot} make install

%post -n %{name}-utils
%{_bindir}/cslight -canvas=null -video=null flarge
%{_bindir}/cslight -canvas=null -video=null partsys

%clean
rm -rf "$RPM_BUILD_ROOT"

%files -n %{name}
%defattr(-,root,root)

%{_sysconfdir}/%{name}/*
%exclude %{_sysconfdir}/%{name}/awstest.cfg
%exclude %{_sysconfdir}/%{name}/awstut.cfg
%exclude %{_sysconfdir}/%{name}/csbumptest.cfg
%exclude %{_sysconfdir}/%{name}/csdemo.cfg
%exclude %{_sysconfdir}/%{name}/cswstest.cfg
%exclude %{_sysconfdir}/%{name}/g2dtest.cfg
%exclude %{_sysconfdir}/%{name}/heightmapgen.cfg
%exclude %{_sysconfdir}/%{name}/lighter.xml
%exclude %{_sysconfdir}/%{name}/map2cs.cfg
%exclude %{_sysconfdir}/%{name}/walktest.cfg
%exclude %{_sysconfdir}/%{name}/waterdemo.cfg

%{_libdir}/%{name}/*

%{_datadir}/%{name}/data/*
%exclude %{_datadir}/%{name}/data/maps/*

%files -n %{name}-utils
%defattr(-,root,root)

%{_bindir}/*
%exclude %{_bindir}/cs-config
%exclude %{_bindir}/*.cex

%{_datadir}/%{name}/data/maps/*
%{_datadir}/%{name}/conversion/*

%{_sysconfdir}/%{name}/awstest.cfg
%{_sysconfdir}/%{name}/awstut.cfg
%{_sysconfdir}/%{name}/csbumptest.cfg
%{_sysconfdir}/%{name}/csdemo.cfg
%{_sysconfdir}/%{name}/cswstest.cfg
%{_sysconfdir}/%{name}/g2dtest.cfg
%{_sysconfdir}/%{name}/heightmapgen.cfg
%{_sysconfdir}/%{name}/lighter.xml
%{_sysconfdir}/%{name}/map2cs.cfg
%{_sysconfdir}/%{name}/walktest.cfg
%{_sysconfdir}/%{name}/waterdemo.cfg

%files -n %{name}-doc
%defattr(-,root,root)

%docdir docs
%{_datadir}/doc/%{name}-%{version}/*

%files -n %{name}-devel
%defattr(-,root,root)

%{_bindir}/cs-config
%{_libdir}/*.a

# (vk) Scripting related files are here for now
%{_bindir}/*.cex
%{_datadir}/%{name}/bindings/*

%{_datadir}/%{name}/build/*

%{prefix}/include/%{name}/*

%changelog
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
