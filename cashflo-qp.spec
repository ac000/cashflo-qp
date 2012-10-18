Summary:	Interest & Discount Calculator
Name:		cashflo-qp
Version:	001
Release:	1.otl%{?dist}
Group:		Applications/Productivity
License:	GPLv2
URL:		https://github.com/opentechlabs/cashflo
Vendor:		OpenTech Labs
Packager:	Andrew Clayton <andrew@opentechlabs.co.uk>
Source0:	cashflo-qp-%{version}.tar
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires:	gtk3-devel
Requires:	gtk3

%description
A simple application to show the difference between paying a supplier
after a longer number of days and earning interest on that or paying
them quicker in return for a discount.

%prep
%setup -q

%build
make

%install
rm -rf $RPM_BUILD_ROOT
install -Dp -m0755 cashflo-qp $RPM_BUILD_ROOT/%{_bindir}/cashflo-qp
install -Dp -m0644 cashflo-qp.desktop $RPM_BUILD_ROOT/%{_datadir}/applications/cashflo-qp.desktop
install -Dp -m0644 cashflo-qp.png $RPM_BUILD_ROOT/%{_datadir}/pixmaps/cashflo-qp.png
install -Dp -m0644 cashflo-qp.glade $RPM_BUILD_ROOT/%{_datadir}/cashflo/cashflo-qp.glade

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/cashflo-qp
%{_datadir}/applications/cashflo-qp.desktop
%{_datadir}/pixmaps/cashflo-qp.png
%{_datadir}/cashflo/cashflo-qp.glade
%doc README COPYING

%changelog
* Thu Oct 18 2012 Andrew Clayton <andrew@opentechlabs.co.uk> - 001-1.otl
- Initial release
