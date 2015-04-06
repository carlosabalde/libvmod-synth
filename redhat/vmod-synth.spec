Summary: Synth VMOD for Varnish
Name: vmod-synth
Version: 0.1
Release: 1%{?dist}
License: BSD
URL: https://github.com/carlosabalde/libvmod-synth
Group: System Environment/Daemons
Source0: libvmod-synth.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Requires: varnish > 4.0.2
BuildRequires: make, python-docutils, varnish >= 4.0.2, varnish-libs-devel >= 4.0.2

%description
Synth VMOD for Varnish

%prep
%setup -n libvmod-synth

%build
./configure VMODDIR=%{VMODDIR} --prefix=/usr/ --docdir='${datarootdir}/doc/%{name}'
%{__make} %{?_smp_mflags}
%{__make} %{?_smp_mflags} check

%install
[ %{buildroot} != "/" ] && %{__rm} -rf %{buildroot}
%{__make} install DESTDIR=%{buildroot}

%clean
[ %{buildroot} != "/" ] && %{__rm} -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_libdir}/varnish/vmods/
%doc /usr/share/doc/%{name}/*
%{_mandir}/man?/*

%changelog
* Sun Jul 20 2014 Carlos Abalde <carlos.abalde@gmail.com> - 0.1-0.20140720
- Initial version.
