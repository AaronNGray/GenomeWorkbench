Name:       gbench
Version:    $version
Release:    $release
Source0:    gbench-linux-i386-%{version}-%{release}.tgz
Source1:    gbench-linux-x86_64-%{version}-%{release}.tgz
Summary:    NCBI Genome Workbench is an integrated application for viewing and analyzing sequence data.
Exclusiveos: linux
Group:      NCBI/GBENCH
License:    Public Domain
BuildArch:  i386 x86_64
BuildRoot:  /var/tmp/%{name}-buildroot

%define    _use_internal_dependency_generator 0

%description
NCBI Genome Workbench is an integrated application for viewing and analyzing
sequence data. With Genome Workbench, you can view data in publically available
sequence databases at NCBI, and mix this data with your own private data.
Genome Workbench can display sequence data in many ways, including graphical
sequence views, various alignment views, phylogenetic tree views, and tabular
views of data. It can also align your private data to data in public databases,
display your data in the context of public data, and retrieve BLAST results.

%prep
rm -rfv %{name}-%{version}
mkdir -p %{name}-%{version}
cd %{name}-%{version}
%ifarch i386
tar xzvf %{SOURCE0}
%else
tar xzvf %{SOURCE1}
%endif

%build
 
%define __find_requires %{_builddir}/gbench-req

cat <<'EOF' >%{_builddir}/%{name}-prov
#!/bin/sh
echo $(echo $(cat) | %{__find_provides} | grep -v ".so")
EOF
chmod +x %{_builddir}/%{name}-prov

%define __find_provides %{_builddir}/%{name}-prov

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
cd %{name}-%{version}
cp -av * $RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/opt/ncbi/gbench-%{version}
/usr/share/applications

