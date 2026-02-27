# Release Notes for v4.0.5

This is a maintenance release focusing on code modernization, build improvements, and bug fixes.

**Release Date:** 2026-02-27

✨ **Features**
- Added code analysis support: introduce `code-analysis.sh` and `.clang-tidy` for static checking via clang-tidy
- Added build scripts: `build-deb.sh` and `build-rpm.sh` for easier DEB/RPM packaging
- Improved code coverage: new CMake option `ENABLE_LOG4CPP_COVERAGE` for coverage measurement on GNU platforms

🐛 **Bug Fixes**
- Fixed: Prevent exceptions from `std::filesystem::exists` when checking configuration file access
- Enhanced error handling for configuration file loading; fallback to default config on errors
- Various code refactors and type improvements (explicit underlying types for `enum class`), reducing warnings and potential errors

📝 **Documentation**
- Updated README and sample config files to explain new build scripts, coverage, and configuration options
- Improved code comments using modern C++ patterns; clarified namespace assignments