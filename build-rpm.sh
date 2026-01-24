#!/bin/bash
set -e

if [[ "$1" == "clean" ]]; then
    echo "==> Removing entire rpmbuild directory..."
    rm -rf ~/rpmbuild
    echo "==> Clean completed."
    exit 0
fi

# Required dependencies
REQUIRED_PKGS=(
    rpm-build
    rpmdevtools
    cmake
    gcc-c++
    json-devel
)

echo "==> Checking required dependencies..."

MISSING_PKGS=()

for pkg in "${REQUIRED_PKGS[@]}"; do
    if ! rpm -q $pkg &>/dev/null; then
        MISSING_PKGS+=("$pkg")
    fi
done

if [ ${#MISSING_PKGS[@]} -ne 0 ]; then
    echo "Missing dependencies detected:"
    for pkg in "${MISSING_PKGS[@]}"; do
        echo "  - $pkg"
    done
    echo
    echo "Please install them manually, for example:"
    echo "  sudo dnf install ${MISSING_PKGS[*]}"
    echo
    exit 1
else
    echo "All required dependencies are installed."
fi

echo "==> Setting up rpmbuild tree..."
rpmdev-setuptree

echo "==> Creating source tarball..."
cd ..
tar -czf ~/rpmbuild/SOURCES/liblog4cpp-4.0.4.tar.gz log4cpp/

echo "==> Copying spec file..."
cp log4cpp/liblog4cpp.spec ~/rpmbuild/SPECS/

echo "==> Building RPM package..."
rpmbuild -ba ~/rpmbuild/SPECS/liblog4cpp.spec

echo "==> RPM build completed successfully."
