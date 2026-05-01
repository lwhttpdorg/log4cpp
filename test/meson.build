gtest_dep = dependency('gtest', main: true, required: false)
if not gtest_dep.found()
    gtest_proj = subproject('gtest', default_options: ['warning_level=0'])
    gtest_dep = gtest_proj.get_variable('gtest_main_dep')
endif

test_includes = include_directories('include')
test_deps = []

if host_machine.system() == 'windows'
    ws2_32 = meson.get_compiler('cpp').find_library('ws2_32', required: true)
    test_deps += ws2_32
endif

test_configs = [
    'config_hot_reload_test_v1.json',
    'config_hot_reload_test_v2.json',
    'console_appender_stderr.json',
    'console_appender_stdout.json',
    'file_appender_test.json',
    'log4cpp_config_1.json',
    'log4cpp_config_2.json',
    'log4cpp.json',
    'serialize_test.json',
    'tcp_socket_appender_test.json',
    'udp_socket_appender_test.json',
]

foreach config : test_configs
    configure_file(
        input: 'config' / config,
        output: config,
        copy: true,
    )
endforeach

test_targets = {
    'load_config_tests': 'app/load_config_test.cpp',
    'log_pattern_tests': 'app/log_pattern_test.cpp',
    'log_truncate_tests': 'app/log_truncate_test.cpp',
    'console_appender_tests': 'app/console_appender_test.cpp',
    'file_appender_tests': 'app/file_appender_test.cpp',
    'serialize_test': 'app/serialize_test.cpp',
}

if host_machine.system() != 'windows'
    test_targets += {'config_hot_reload_tests': 'app/config_hot_reload_test.cpp'}
endif

foreach name, src : test_targets
    exe = executable(
        name,
        src,
        include_directories: [
            test_includes,
            include_directories('../src/include'),
        ],
        dependencies: [gtest_dep, log4cpp_dep] + test_deps,
        cpp_args: asan_extra_args + cov_compile_args,
        link_args: cov_link_args,
    )
    test(name, exe, timeout: 30, workdir: meson.current_build_dir(), protocol: 'gtest')
endforeach

# socket_appender_tests: split into individual gtest cases to avoid
# SIGPIPE from background threads when multiple tests run in one process.
socket_appender_exe = executable(
    'socket_appender_tests',
    'app/socket_appender_test.cpp',
    include_directories: [
        test_includes,
        include_directories('../src/include'),
    ],
    dependencies: [gtest_dep, log4cpp_dep] + test_deps,
    cpp_args: asan_extra_args + cov_compile_args,
    link_args: cov_link_args,
)

test('socket_appender_test.tcp_socket_appender_test', socket_appender_exe,
    args: ['--gtest_filter=socket_appender_test.tcp_socket_appender_test'],
    timeout: 30,
    workdir: meson.current_build_dir(),
    protocol: 'gtest',
)

test('socket_appender_test.udp_socket_appender_test', socket_appender_exe,
    args: ['--gtest_filter=socket_appender_test.udp_socket_appender_test'],
    timeout: 30,
    workdir: meson.current_build_dir(),
    protocol: 'gtest',
)
