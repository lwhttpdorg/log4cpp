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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERSION="$(tr -d '\r\n' < "${SCRIPT_DIR}/VERSION")"

echo "==> Creating source tarball (version ${VERSION})..."
cd ..
tar -czf "${HOME}/rpmbuild/SOURCES/liblog4cpp-${VERSION}.tar.gz" log4cpp/

echo "==> Writing spec with VERSION from ${SCRIPT_DIR}/VERSION..."
sed "s/^%define _version .*/%define _version ${VERSION}/" log4cpp/liblog4cpp.spec > "${HOME}/rpmbuild/SPECS/liblog4cpp.spec"

echo "==> Building RPM package..."
rpmbuild -ba ~/rpmbuild/SPECS/liblog4cpp.spec

echo "==> RPM build completed successfully."
