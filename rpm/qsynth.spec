#
# spec file for package qsynth
#
# Copyright (C) 2003-2024, rncbc aka Rui Nuno Capela. All rights reserved.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

Summary:	A fluidsynth Qt GUI Interface
Name:		qsynth
Version:	0.9.91
Release:	1.1
License:	GPL-2.0-or-later
Group:		Productivity/Multimedia/Sound/Midi
Source:		%{name}-%{version}.tar.gz
URL:		https://qsynth.sourceforge.io/
#Packager:	rncbc.org

%if 0%{?fedora_version} >= 34 || 0%{?suse_version} > 1500 || ( 0%{?sle_version} == 150200 && 0%{?is_opensuse} )
%define qt_major_version  6
%else
%define qt_major_version  5
%endif

BuildRequires:	coreutils
BuildRequires:	pkgconfig
BuildRequires:	glibc-devel
BuildRequires:	cmake >= 3.15
%if 0%{?sle_version} >= 150200 && 0%{?is_opensuse}
BuildRequires:	gcc10 >= 10
BuildRequires:	gcc10-c++ >= 10
%define _GCC	/usr/bin/gcc-10
%define _GXX	/usr/bin/g++-10
%else
BuildRequires:	gcc >= 10
BuildRequires:	gcc-c++ >= 10
%define _GCC	/usr/bin/gcc
%define _GXX	/usr/bin/g++
%endif
%if 0%{qt_major_version} == 6
%if 0%{?sle_version} == 150200 && 0%{?is_opensuse}
BuildRequires:	qtbase6.7-static >= 6.7
BuildRequires:	qttools6.7-static
BuildRequires:	qttranslations6.7-static
BuildRequires:	qtsvg6.7-static
%else
BuildRequires:	cmake(Qt6LinguistTools)
BuildRequires:	pkgconfig(Qt6Core)
BuildRequires:	pkgconfig(Qt6Gui)
BuildRequires:	pkgconfig(Qt6Widgets)
BuildRequires:	pkgconfig(Qt6Svg)
BuildRequires:	pkgconfig(Qt6Network)
%endif
%else
BuildRequires:	cmake(Qt5LinguistTools)
BuildRequires:	pkgconfig(Qt5Core)
BuildRequires:	pkgconfig(Qt5Gui)
BuildRequires:	pkgconfig(Qt5Widgets)
BuildRequires:	pkgconfig(Qt5Svg)
BuildRequires:	pkgconfig(Qt5Network)
%endif

%if %{defined fedora}
BuildRequires:	jack-audio-connection-kit-devel
%else
BuildRequires:	pkgconfig(jack)
%endif
BuildRequires:	pkgconfig(alsa)

BuildRequires:	libfluidsynth-devel

%description
Qsynth is a fluidsynth GUI front-end application, written in C++ around the
Qt framework, using Qt Designer. Eventually it may evolve into a softsynth
management application allowing the user to control and manage a variety of
command line softsynths.


%prep
%setup -q

%build
%if 0%{?sle_version} == 150200 && 0%{?is_opensuse}
source /opt/qt6.7-static/bin/qt6.7-static-env.sh
%endif
CXX=%{_GXX} CC=%{_GCC} \
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -Wno-dev -B build
cmake --build build %{?_smp_mflags}

%install
DESTDIR="%{buildroot}" \
cmake --install build


%files
%license LICENSE
%doc README TRANSLATORS ChangeLog
#dir %{_datadir}/applications
%dir %{_datadir}/icons/hicolor
%dir %{_datadir}/icons/hicolor/32x32
%dir %{_datadir}/icons/hicolor/32x32/apps
%dir %{_datadir}/icons/hicolor/scalable
%dir %{_datadir}/icons/hicolor/scalable/apps
%dir %{_datadir}/%{name}
%dir %{_datadir}/%{name}/translations
%dir %{_datadir}/%{name}/palette
%dir %{_datadir}/metainfo
#dir %{_datadir}/man
#dir %{_datadir}/man/man1
#dir %{_datadir}/man/fr
#dir %{_datadir}/man/fr/man1
%{_bindir}/%{name}
%{_datadir}/applications/org.rncbc.%{name}.desktop
%{_datadir}/icons/hicolor/32x32/apps/org.rncbc.%{name}.png
%{_datadir}/icons/hicolor/scalable/apps/org.rncbc.%{name}.svg
%{_datadir}/%{name}/translations/%{name}_*.qm
%{_datadir}/metainfo/org.rncbc.%{name}.metainfo.xml
%{_datadir}/man/man1/%{name}.1.gz
%{_datadir}/man/fr/man1/%{name}.1.gz
%{_datadir}/%{name}/palette/*.conf


%changelog
* Wed May  1 2024 Rui Nuno Capela <rncbc@rncbc.org> 0.9.91
- A Spring'24 Release Candidate 2.
* Wed Apr 10 2024 Rui Nuno Capela <rncbc@rncbc.org> 0.9.90
- A Spring'24 Release Candidate.
* Wed Jan 24 2024 Rui Nuno Capela <rncbc@rncbc.org> 0.9.13
- A Winter'24 Release.
* Sat Sep  9 2023 Rui Nuno Capela <rncbc@rncbc.org> 0.9.12
- An End-of-Summer'23 Release.
* Thu Jun  1 2023 Rui Nuno Capela <rncbc@rncbc.org> 0.9.11
- A Spring'23 Release.
* Thu Mar 23 2023 Rui Nuno Capela <rncbc@rncbc.org> 0.9.10
- An Early-Spring'23 Release.
* Wed Dec 28 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.9
- An End-of-Year'22 Release.
* Mon Oct  3 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.8
- An Early-Autumn'22 Release.
* Sat Apr  2 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.7
- A Spring'22 Release.
* Sat Feb 12 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.6
- A Mid-Winter'22 Release.
* Sat Jan  9 2022 Rui Nuno Capela <rncbc@rncbc.org> 0.9.5
- A Winter'22 Release.
* Sun Jul  4 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.4
- Early-Summer'21 release.
* Tue May 11 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.3
- Spring'21 release.
* Sun Mar 14 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.2
- End-of-Winter'21 release.
* Sun Feb  7 2021 Rui Nuno Capela <rncbc@rncbc.org> 0.9.1
- Winter'21 release.
* Thu Dec 17 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.9.0
- Winter'20 release.
* Fri Jul 31 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.6.3
- Summer'20 release.
* Tue Mar 24 2020 Rui Nuno Capela <rncbc@rncbc.org> 0.6.2
- Spring'20 release.
* Sun Dec 22 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.6.1
- Winter'19 release.
* Thu Oct 17 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.6.0
- Autumn'19 release.
* Fri Jul 12 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.5.7
- Summer'19 release.
* Thu Apr 11 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.5.6
- Spring-Break'19 release.
* Mon Mar 11 2019 Rui Nuno Capela <rncbc@rncbc.org> 0.5.5
- Pre-LAC2019 release frenzy.
* Wed Dec 05 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.5.4
- End of Autumn'18 Release.
* Wed Oct 10 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.5.3
- Autumn'18 Release.
* Sun Jul 22 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.5.2
- Summer'18 Release.
* Mon May 21 2018 Rui Nuno Capela <rncbc@rncbc.org> 0.5.1
- Pre-LAC2018 release frenzy.
* Sat Dec 16 2017 Rui Nuno Capela <rncbc@rncbc.org> 0.5.0
- End of Autumn'17 release.
* Thu Apr 27 2017 Rui Nuno Capela <rncbc@rncbc.org> 0.4.4
- Pre-LAC2017 release frenzy.
* Mon Nov 14 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.4.3
- A Fall'16 release.
* Wed Sep 14 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.4.2
- End of Summer'16 release.
* Tue Apr  5 2016 Rui Nuno Capela <rncbc@rncbc.org> 0.4.1
- Spring'16 release frenzy.
* Mon Sep  7 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.4.0
- Summer'15 release frenzy.
* Wed Mar 25 2015 Rui Nuno Capela <rncbc@rncbc.org> 0.3.9
- Pre-LAC2015 release frenzy.
* Tue Dec 31 2013 Rui Nuno Capela <rncbc@rncbc.org> 0.3.8
- A fifth of a Jubilee release.
* Tue Apr 16 2013 Rui Nuno Capela <rncbc@rncbc.org> 0.3.7
- Spring cleaning sale.
* Thu Apr 07 2011 Rui Nuno Capela <rncbc@rncbc.org> 0.3.6
- Slip release.
* Mon May 17 2010 Rui Nuno Capela <rncbc@rncbc.org>
- Standard desktop icon fixing.
* Tue Apr 27 2010 Rui Nuno Capela <rncbc@rncbc.org> 0.3.5
- Overdue release 0.3.5.
* Sun May 10 2009 Rui Nuno Capela <rncbc@rncbc.org>
- New release 0.3.4.
- Added russian translation.
* Mon Jun 23 2008 Rui Nuno Capela <rncbc@rncbc.org>
- Added german and spanish translations.
* Wed Dec 19 2007 Rui Nuno Capela <rncbc@rncbc.org>
- Minor stealth fixes and season greetings.
* Mon Jul 16 2007 Rui Nuno Capela <rncbc@rncbc.org>
- Shallowed bug-fix release.
* Tue Jul 03 2007 Rui Nuno Capela <rncbc@rncbc.org>
- Qt4 migration was complete.
* Mon Jun 25 2007 Rui Nuno Capela <rncbc@rncbc.org>
- Application icon is now installed to (prefix)/share/pixmaps.
- Declared fundamental build and run-time requirements.
- Destination install directory prefix is now in spec.
- Spec is now a bit more openSUSE compliant.
* Thu Jun 01 2006 Rui Nuno Capela <rncbc@rncbc.org>
- Changed deprecated copyright attribute to license.
* Sun Oct 2 2005 Rui Nuno Capela <rncbc@rncbc.org>
- Made public as for 0.2.4 release.
* Wed Aug 24 2005 Rui Nuno Capela <rncbc@rncbc.org>
- Created initial qsynth.spec
