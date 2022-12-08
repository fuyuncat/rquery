Name:           rquery
Version:        %{latestversion}
Release:        1%{?dist}
Summary:        RQuery is a regular expression (Capturing Group and Named Group supported) based text searching engine.

Group:          fuyuncat
License:        GPL-3.0
URL:            https://www.hellodba.com
Source0:        https://github.com/fuyuncat/rquery/archive/refs/tags/%{name}-%{version}.tar.gz

Packager:       fuyuncat
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

%global debug_package %{nil}

%description
RQuery is a powerful tool can search string block/file/folder using regular/delimiter/wildcard/ expression parttern, and filter/group calculate/sort the matched result. One command can do what grep/xgrep/sort/uniq/awk/wc/sed/cut/tr can do and more.

%prep
%setup -q -n %{name}-%{version}

%build
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
mkdir -p %{buildroot}/usr/bin/
cp rq %{buildroot}/usr/bin/

%files
%defattr(755,root,root)
/usr/bin/rq
%doc

%changelog