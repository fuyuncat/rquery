Name:           rquery
Version:        main
Release:        1%{?dist}
Summary:        RQuery is a regular expression (Capturing Group and Named Group supported) based text searching engine.

Group:          fuyuncat
License:        GPL-3.0
URL:            https://www.hellodba.com
Source0:        https://github.com/fuyuncat/rquery/archive/refs/tags/%{name}-%{version}.tar.gz

Packager:       fuyuncat
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:  gcc
BuildRequires:  make
Requires:       bash

%description
RQuery is a regular expression (Capturing Group and Named Group supported) based text searching engine.

%prep
%setup -q -n %{name}-%{version}


%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --sysconfdir=/etc
make %{?_smp_mflags}


%install
make install DESTDIR=%{buildroot}


%files
%doc

%changelog
