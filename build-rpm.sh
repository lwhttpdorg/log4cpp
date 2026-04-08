#!/bin/bash
set -e

usage() {
    echo "Usage: $0 [clean] [--arch <ARCH>]"
    echo
    echo "Options:"
    echo "  clean          Clean rpmbuild directory"
    echo "  --arch <ARCH>  Cross-compile for the specified architecture"
    echo "                 Supported: aarch64, armhf, mips, mips64"
    echo
    echo "Examples:"
    echo "  $0                    # native build"
    echo "  $0 --arch aarch64    # cross-compile for aarch64"
    echo "  $0 clean             # clean build artifacts"
    exit 0
}

resolve_arch() {
    local arch="$1"
    case "$arch" in
        aarch64|arm64)
            RPM_TARGET_ARCH="aarch64"
            CMAKE_TOOLCHAIN="cross/aarch64-linux-gnu.cmake"
            CROSS_COMPILER_PREFIX="aarch64-linux-gnu"
            ;;
        armhf|arm)
            RPM_TARGET_ARCH="armv7hl"
            CMAKE_TOOLCHAIN="cross/arm-linux-gnueabihf.cmake"
            CROSS_COMPILER_PREFIX="arm-linux-gnueabihf"
            ;;
        mips)
            RPM_TARGET_ARCH="mips"
            CMAKE_TOOLCHAIN="cross/mips-linux-gnu.cmake"
            CROSS_COMPILER_PREFIX="mips-linux-gnu"
            ;;
        mips64)
            RPM_TARGET_ARCH="mips64"
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
DO_CLEAN=0
RPM_TARGET_ARCH=$(uname -m)

while [[ $# -gt 0 ]]; do
    case "$1" in
        clean)
            DO_CLEAN=1
            shift
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

# Resolve architecture settings (needed before dependency check)
if [[ "$CROSS_BUILD" -eq 1 ]]; then
    resolve_arch "$TARGET_ARCH"
fi

echo "===> Clean"
rm -rf ~/rpmbuild/BUILD/* ~/rpmbuild/BUILDROOT/*
find ~/rpmbuild/RPMS -name "*${RPM_TARGET_ARCH}.rpm" -delete 2>/dev/null || true

if [[ "$DO_CLEAN" -eq 1 ]]; then
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

if [[ "$CROSS_BUILD" -eq 1 ]]; then
    # Add cross-compiler packages to dependency check
    REQUIRED_PKGS+=("gcc-${CROSS_COMPILER_PREFIX}" "gcc-c++-${CROSS_COMPILER_PREFIX}")
fi

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

if [[ "$CROSS_BUILD" -eq 1 ]]; then
    echo "==> Cross-compiling for ${RPM_TARGET_ARCH} (toolchain: ${CMAKE_TOOLCHAIN})"
    export CC="${CROSS_COMPILER_PREFIX}-gcc"
    export CXX="${CROSS_COMPILER_PREFIX}-g++"
    rpmbuild -ba ~/rpmbuild/SPECS/liblog4cpp.spec \
        --target "${RPM_TARGET_ARCH}" \
            --define "_smp_mflags -j$(nproc)" \
        --define "__cmake_in_source_build 1" \
        --define "_cmake_extra_args -DCMAKE_TOOLCHAIN_FILE=${SCRIPT_DIR}/${CMAKE_TOOLCHAIN}"
else
    rpmbuild -ba ~/rpmbuild/SPECS/liblog4cpp.spec --define "_smp_mflags -j$(nproc)"
fi

echo "==> RPM build completed successfully."
