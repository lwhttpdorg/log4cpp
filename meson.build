project(
    'log4cpp',
    'cpp',
    version: files('VERSION'),
    default_options: [
        'cpp_std=c++17',
        'buildtype=release',
        'warning_level=0',
    ],
    meson_version: '>=1.1.0',
)

fs = import('fs')
pkg = import('pkgconfig')

# Options
build_demo = get_option('build_demo')
enable_tests = get_option('enable_tests')
enable_coverage = get_option('enable_coverage')

# Meson uses the built-in 'b_sanitize' option for sanitizers.
# We only add supplementary flags that Meson does not handle automatically.
sanitize_opt = get_option('b_sanitize')

asan_extra_args = []
if sanitize_opt != 'none'
    cpp_id = meson.get_compiler('cpp').get_id()
    if cpp_id in ['gcc', 'clang']
        if get_option('buildtype') == 'debug' and sanitize_opt.contains('address')
            asan_extra_args += '-fno-omit-frame-pointer'
        endif
    elif host_machine.system() == 'windows'
        warning('MSVC AddressSanitizer is not fully supported in this configuration.')
    endif
endif

# Coverage flags (GNU only)
cov_compile_args = []
cov_link_args = []
if enable_coverage
    if meson.get_compiler('cpp').get_id() == 'gcc'
        cov_compile_args = ['--coverage', '-O0', '-g']
        cov_link_args = ['--coverage']
    else
        warning('Code coverage is only supported with GCC; enable_coverage is ignored.')
    endif
endif

# Subdirectories
subdir('src')

if build_demo
    subdir('demo')
endif

if enable_tests
    subdir('test')
endif

# Install headers
install_subdir('include', install_dir: get_option('includedir'), strip_directory: true)

# pkg-config file
pkg.generate(
    log4cpp_lib,
    filebase: 'log4cpp',
    name: 'liblog4cpp',
    description: 'A log4j-style C++ logging library',
)
