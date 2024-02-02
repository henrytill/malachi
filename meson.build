project(
    'malachi',
    'c',
    version: '0.1',
    default_options: ['warning_level=2', 'c_std=c11'],
)

mupdf_dep = dependency('mupdf', required: true)

sqlite_dep = dependency('sqlite3', required: true)

platform_include_dir = include_directories('include/platform')

platform_dep = declare_dependency(
    include_directories: platform_include_dir,
)

path_include_dir = include_directories('include/path')

path_lib = static_library(
    'path',
    sources: ['lib/path/path.c'],
    include_directories: path_include_dir,
    dependencies: [platform_dep],
    install: false,
)

path_dep = declare_dependency(
    link_with: [path_lib],
    include_directories: path_include_dir,
)

config_include_dir = include_directories('include/config')

config_lib = static_library(
    'config',
    sources: ['lib/config/config.c'],
    include_directories: config_include_dir,
    dependencies: [path_dep, platform_dep],
    install: false,
)

config_dep = declare_dependency(
    link_with: [config_lib],
    include_directories: config_include_dir,
)

executable(
    'malachi',
    sources: ['bin/main.c'],
    dependencies: [
        config_dep,
        platform_dep,
        mupdf_dep,
        sqlite_dep,
    ],
)

config_test = executable(
    'config_test',
    sources: ['test/config_test.c'],
    dependencies: [config_dep, platform_dep],
)

test('config_test', config_test)
