project(
    'malachi',
    'cpp',
    version: '0.1',
    default_options: ['warning_level=2', 'cpp_std=c++20'],
)

catch2_dep = dependency('catch2-with-main', required: true)

mupdf_dep = dependency('mupdf', required: true)

sqlite_dep = dependency('sqlite3', required: true)

config_include_dir = include_directories('include/config')

config = static_library(
    'config',
    sources: ['lib/config/Config.cpp'],
    include_directories: config_include_dir,
    install: false,
)

config_dep = declare_dependency(
    link_with: [config],
    include_directories: config_include_dir,
)

executable(
    'malachi',
    sources: ['bin/main.cpp'],
    dependencies: [mupdf_dep, sqlite_dep, config_dep],
)

config_test = executable(
    'config_test',
    sources: ['test/config_test.cpp'],
    dependencies: [catch2_dep, config_dep],
)

test('config_test', config_test)
