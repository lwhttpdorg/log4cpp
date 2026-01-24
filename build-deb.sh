#!/bin/bash
set -e

if [[ "$1" == "clean" ]]; then
    echo "==> Running fakeroot debian/rules clean..."
    fakeroot debian/rules clean

    echo "==> Removing generated .deb files..."
    find .. -maxdepth 1 -type f -name "*.deb" -delete

    echo "==> Clean completed."
    exit 0
fi

# Required dependencies
REQUIRED_PKGS=(
    dpkg-dev
    fakeroot
    debhelper
    nlohmann-json3-dev
)

echo "==> Checking required dependencies..."

MISSING_PKGS=()

for pkg in "${REQUIRED_PKGS[@]}"; do
    if ! dpkg -s "$pkg" &>/dev/null; then
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
    echo "  sudo apt install ${MISSING_PKGS[*]}"
    echo
    exit 1
else
    echo "All required dependencies are installed."
fi

echo "==> Cleaning previous build..."
fakeroot debian/rules clean

echo "==> Building the .deb package..."
DEB_BUILD_OPTIONS="noddebs" dpkg-buildpackage -us -uc -b -j"$(nproc)"

echo "==> Removing unnecessary build files..."
find .. -maxdepth 1 -type f \( -name "*.buildinfo" -o -name "*.changes" \) -delete

echo "==> Cleanup completed."
echo "==> Build finished successfully."
