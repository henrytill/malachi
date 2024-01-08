project(
    'malachi',
    'c',
    version: '0.1',
    default_options: ['warning_level=2', 'c_std=gnu17'],
)

mupdf_dep = dependency('mupdf', required: true)

sqlite_dep = dependency('sqlite3', required: true)

platform_include_dir = include_directories('include/platform')

platform_dep = declare_dependency(
    include_directories: platform_include_dir,
)

alloc_include_dir = include_directories('include/alloc')

alloc = static_library(
    'alloc',
    sources: ['lib/alloc/alloc.c'],
    include_directories: alloc_include_dir,
    install: false,
)

alloc_dep = declare_dependency(
    link_with: [alloc],
    include_directories: alloc_include_dir,
)

path_include_dir = include_directories('include/path')

path = static_library(
    'path',
    sources: ['lib/path/path.c'],
    include_directories: path_include_dir,
    dependencies: [alloc_dep, platform_dep],
    install: false,
)

path_dep = declare_dependency(
    link_with: [path],
    include_directories: path_include_dir,
)

config_include_dir = include_directories('include/config')

config = static_library(
    'config',
    sources: ['lib/config/config.c'],
    include_directories: config_include_dir,
    dependencies: [
        alloc_dep,
        platform_dep,
        path_dep,
    ],
    install: false,
)

config_dep = declare_dependency(
    link_with: [config],
    include_directories: config_include_dir,
)

executable(
    'malachi',
    sources: ['bin/main.c'],
    dependencies: [mupdf_dep, sqlite_dep, config_dep, platform_dep],
)

config_test = executable(
    'config_test',
    sources: ['test/config_test.c'],
    dependencies: [config_dep, platform_dep],
)

test('config_test', config_test)
