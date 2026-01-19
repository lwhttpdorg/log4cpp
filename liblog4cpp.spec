# Define the version once to ensure consistency
%define _version 4.0.4

Name:           liblog4cpp
Version:        %{_version}
Release:        1%{?dist}
Summary:        A log4j-style C++ logging library
License:        LGPLv3
URL:            https://github.com/lwhttpdorg/log4cpp
Vendor:         log4cpp.org

# Source tarball expected in ~/rpmbuild/SOURCES/
Source0:        %{name}-%{version}.tar.gz

# Build-time dependencies: nlohmann-json is required only for compilation
BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  json-devel >= 3.0.0

%description
A log4j-style C++ logging library.

%package devel
Summary:        Development files for %{name}
# The development package must depend on the specific version of the runtime package
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%prep
# Unpack the source. -n specifies the directory name inside the tarball
%setup -q -n log4cpp

%build
# Standard CMake macro that handles build types and library paths (e.g., /usr/lib64)
%cmake -DPROJECT_VERSION=%{version}
%cmake_build

%install
# Install files into the virtual build root
%cmake_install

# Refresh dynamic linker cache after installation or removal
%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
# Runtime package: Contains the shared library and SONAME symlink
# %%{_libdir} automatically resolves to /usr/lib64 on 64-bit RPM systems
%{_libdir}/liblog4cpp.so.*

%files devel
# Development package: Contains headers, development symlinks, and pkg-config
# 1. Public header directory
%{_includedir}/log4cpp/
# 2. Unversioned symlink for linking during development (-llog4cpp)
%{_libdir}/liblog4cpp.so
# 3. Pkg-config metadata file
%{_libdir}/pkgconfig/log4cpp.pc

%changelog
* Mon Jan 19 2026 Developer <developer@log4cpp.org> - 4.0.4-1
- Initial native RPM release
- Internalized nlohmann-json dependency to hide implementation details
- Unified include path to /usr/include/log4cpp for standard API usage
