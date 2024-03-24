project(
    'malachi',
    'cpp',
    version: '0.1',
    default_options: ['warning_level=2', 'cpp_std=c++20'],
)

catch2 = subproject('catch2', default_options: 'tests=false')

catch2_dep = catch2.get_variable('catch2_dep')

poppler_dep = dependency('poppler-cpp', required: true)

sqlite_dep = dependency('sqlite3', required: true)

platform_include_dir = include_directories('include/platform')

platform_dep = declare_dependency(
  include_directories: platform_include_dir,
)

config_include_dir = include_directories('include/config')

config = static_library(
    'config',
    sources: ['lib/config/config.cpp'],
    include_directories: config_include_dir,
    dependencies: [platform_dep],
    install: false,
)

config_dep = declare_dependency(
    link_with: [config],
    include_directories: config_include_dir,
    dependencies: [platform_dep],
)

executable(
    'malachi',
    sources: ['bin/main.cpp'],
    dependencies: [poppler_dep, sqlite_dep, config_dep],
)

config_test = executable(
    'config_test',
    sources: ['test/config_test.cpp'],
    dependencies: [catch2_dep, config_dep],
)

platform_test = executable(
    'platform_test',
    sources: ['test/platform_test.cpp'],
    dependencies: [catch2_dep, platform_dep],
)

test('config_test', config_test)

test('platform_test', platform_test)
