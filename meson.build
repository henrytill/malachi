project(
    'malachi',
    'cpp',
    version: '0.1.0',
    default_options: ['warning_level=2', 'cpp_std=c++20'],
)

libgit2_dep = dependency('libgit2', required: true)

mupdf_dep = dependency('mupdf', required: false)

sqlite_dep = dependency('sqlite3', required: true)

include_dir = include_directories('include')

configure_file(
    output: 'project.h',
    configuration: {
        'MALACHI_VERSION_MAJOR': meson.project_version().split('.')[0],
        'MALACHI_VERSION_MINOR': meson.project_version().split('.')[1],
        'MALACHI_VERSION_PATCH': meson.project_version().split('.')[2],
        'MALACHI_HAVE_MUPDF': mupdf_dep.found(),
    },
)

root_dir = include_directories('.')

executable(
    'malachi',
    sources: ['src/config.cpp', 'src/main.cpp'],
    dependencies: [libgit2_dep, mupdf_dep, sqlite_dep],
    include_directories: [include_dir, root_dir],
    install: true,
)

catch2_dep = dependency('catch2', fallback: ['catch2', 'catch2_dep'])

platform_test = executable(
    'platform_test',
    sources: 'test/platform_test.cpp',
    dependencies: catch2_dep,
    include_directories: include_dir,
)

config_test = executable(
    'config_test',
    sources: ['test/config_test.cpp', 'src/config.cpp'],
    dependencies: catch2_dep,
    include_directories: include_dir,
)

test('platform_test', platform_test)

test('config_test', config_test)
