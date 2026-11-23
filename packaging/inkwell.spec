Name:           inkwell
Version:        0.1.0
Release:        1%{?dist}
Summary:        Fast, native handwritten note-taking and PDF annotation application

License:        MIT
URL:            https://github.com/inkwell-notes/inkwell
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  gcc-c++
BuildRequires:  ninja-build
BuildRequires:  desktop-file-utils
BuildRequires:  qt6-qtbase-devel >= 6.4
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qtsvg-devel
BuildRequires:  qt6-qtwayland-devel
BuildRequires:  poppler-qt6-devel
BuildRequires:  sqlite-devel
BuildRequires:  tesseract-devel
BuildRequires:  leptonica-devel

Requires:       qt6-qtbase
Requires:       qt6-qtdeclarative
Requires:       qt6-qtsvg
Requires:       qt6-qtwayland
Requires:       poppler-utils
Requires:       tesseract

%description
Inkwell is a distraction-free, vector-based digital notebook and PDF annotation
application crafted natively for Fedora Linux and KDE Plasma. It delivers
low-latency inking with true pressure sensitivity for drawing tablets
(such as Huion and OpenTabletDriver), customizable pens, shapes, ruler,
protractor, lasso editing, and offline SQLite storage.

%prep
%autosetup

%build
%cmake -G Ninja -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop

%files
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg

%changelog
* Mon Nov 23 2026 Inkwell Team <inkwell@fedoraproject.org> - 0.1.0-1
- Initial Fedora RPM release of Inkwell
