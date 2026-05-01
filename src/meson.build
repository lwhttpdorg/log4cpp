cpp = meson.get_compiler('cpp')

src_files = files(
    'lib/appender/console_appender.cpp',
    'lib/appender/file_appender.cpp',
    'lib/appender/socket_appender.cpp',
    'lib/common/common.cpp',
    'lib/common/json.cpp',
    'lib/common/log_net.cpp',
    'lib/common/log_utils.cpp',
    'lib/config/appender.cpp',
    'lib/config/log4cpp.cpp',
    'lib/config/logger.cpp',
    'lib/logger/logger_proxy.cpp',
    'lib/logger/real_logger.cpp',
    'lib/manager/logger_manager.cpp',
    'lib/pattern/log_pattern.cpp',
)

inc_public = include_directories('../include')
inc_private = include_directories('include')

compile_args = []
link_args = []
deps = []

# Platform specifics
if host_machine.system() == 'windows'
    compile_args += ['-DUNICODE', '-D_UNICODE']
    if cpp.get_id() == 'msvc'
        compile_args += ['-DWIN32_LEAN_AND_MEAN', '-D_CRT_SECURE_NO_WARNINGS']
        if host_machine.cpu_family() == 'x86_64'
            compile_args += '-D_AMD64_'
        elif host_machine.cpu_family() == 'x86'
            compile_args += '-D_X86_'
        endif
    endif
    ws2_32 = cpp.find_library('ws2_32', required: true)
    deps += ws2_32
elif host_machine.system() in ['linux', 'darwin', 'freebsd', 'netbsd', 'openbsd', 'sunos']
    thread_dep = dependency('threads', required: true)
    deps += thread_dep
endif

# GNU warnings
if cpp.get_id() == 'gcc'
    compile_args += [
        '-Wall', '-Wold-style-cast', '-Wextra', '-Wshadow', '-Wpointer-arith',
        '-Wwrite-strings', '-Woverloaded-virtual', '-Wstringop-overflow',
        '-Wno-pessimizing-move', '-Wno-uninitialized', '-Wno-unused-parameter',
        '-Wno-unused-function', '-Wno-unused-variable', '-Wno-unused-value',
        '-pipe',
    ]
endif

# Apply extra sanitizer / coverage flags from root
compile_args += asan_extra_args + cov_compile_args
link_args += cov_link_args

log4cpp_lib = shared_library(
    'log4cpp',
    src_files,
    include_directories: [inc_public, inc_private],
    cpp_args: compile_args,
    link_args: link_args,
    dependencies: deps,
    install: true,
    version: meson.project_version(),
    soversion: meson.project_version().split('.')[0],
)

log4cpp_dep = declare_dependency(
    link_with: log4cpp_lib,
    include_directories: inc_public,
)
