project(
    'malachi',
    'c',
    version: '0.1',
    default_options: ['warning_level=2', 'c_std=gnu17'],
)

mupdf_dep = dependency('mupdf', required: true)

sqlite_dep = dependency('sqlite3', required: true)

config_include_dir = include_directories('include/config')

config = static_library(
    'config',
    sources: ['lib/config/config.c'],
    include_directories: config_include_dir,
    install: false,
)

config_dep = declare_dependency(
    link_with: [config],
    include_directories: config_include_dir,
)

executable(
    'malachi',
    sources: ['bin/main.c'],
    dependencies: [mupdf_dep, sqlite_dep, config_dep],
)

config_test = executable(
    'config_test',
    sources: ['test/config_test.c'],
    dependencies: [config_dep],
)

test('config_test', config_test)
