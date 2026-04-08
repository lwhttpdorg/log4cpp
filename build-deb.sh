#!/bin/bash
set -e

usage() {
    echo "Usage: $0 [clean] [--arch <ARCH>]"
    echo
    echo "Options:"
    echo "  clean          Clean previous build artifacts"
    echo "  --arch <ARCH>  Cross-compile for the specified architecture"
    echo "                 Supported: aarch64, armhf, mips, mips64"
    echo
    echo "Examples:"
    echo "  $0                    # native build"
    echo "  $0 --arch aarch64    # cross-compile for aarch64"
    echo "  $0 clean             # clean build artifacts"
    exit 0
}

# Map short arch names to dpkg architecture and cmake toolchain file
resolve_arch() {
    local arch="$1"
    case "$arch" in
        aarch64|arm64)
            DEB_HOST_ARCH="arm64"
            CMAKE_TOOLCHAIN="cross/aarch64-linux-gnu.cmake"
            CROSS_COMPILER_PREFIX="aarch64-linux-gnu"
            ;;
        armhf|arm)
            DEB_HOST_ARCH="armhf"
            CMAKE_TOOLCHAIN="cross/arm-linux-gnueabihf.cmake"
            CROSS_COMPILER_PREFIX="arm-linux-gnueabihf"
            ;;
        mips)
            DEB_HOST_ARCH="mips"
            CMAKE_TOOLCHAIN="cross/mips-linux-gnu.cmake"
            CROSS_COMPILER_PREFIX="mips-linux-gnu"
            ;;
        mips64)
            DEB_HOST_ARCH="mips64"
            CMAKE_TOOLCHAIN="cross/mips64-linux-gnuabi64.cmake"
            CROSS_COMPILER_PREFIX="mips64-linux-gnuabi64"
            ;;
        *)
            echo "Error: unsupported architecture '$arch'"
            echo "Supported: aarch64, armhf, mips, mips64"
            exit 1
            ;;
    esac
}

CROSS_BUILD=0
TARGET_ARCH=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        clean)
            echo "==> Running fakeroot debian/rules clean..."
            fakeroot debian/rules clean

            echo "==> Removing generated .deb files..."
            find .. -maxdepth 1 -type f -name "*.deb" -delete

            echo "==> Clean completed."
            exit 0
            ;;
        --arch|-a)
            CROSS_BUILD=1
            TARGET_ARCH="$2"
            shift 2
            ;;
        --help|-h)
            usage
            ;;
        *)
            echo "Unknown option: $1"
            usage
            ;;
    esac
done

# Required dependencies (common)
REQUIRED_PKGS=(
    dpkg-dev
    fakeroot
    debhelper
    nlohmann-json3-dev
)

if [[ "$CROSS_BUILD" -eq 1 ]]; then
    resolve_arch "$TARGET_ARCH"
    REQUIRED_PKGS+=("gcc-${CROSS_COMPILER_PREFIX}" "g++-${CROSS_COMPILER_PREFIX}")
fi

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

if [[ "$CROSS_BUILD" -eq 1 ]]; then
    echo "==> Cross-compiling for ${DEB_HOST_ARCH} (toolchain: ${CMAKE_TOOLCHAIN})"
    export CMAKE_TOOLCHAIN_FILE="$(pwd)/${CMAKE_TOOLCHAIN}"
    export CC="${CROSS_COMPILER_PREFIX}-gcc"
    export CXX="${CROSS_COMPILER_PREFIX}-g++"
    DEB_BUILD_OPTIONS="noddebs nocheck" dpkg-buildpackage -us -uc -b -j"$(nproc)" -a"${DEB_HOST_ARCH}"
else
    DEB_BUILD_OPTIONS="noddebs" dpkg-buildpackage -us -uc -b -j"$(nproc)"
fi

echo "==> Removing unnecessary build files..."
find .. -maxdepth 1 -type f \( -name "*.buildinfo" -o -name "*.changes" \) -delete

echo "==> Cleanup completed."
echo "==> Build finished successfully."
