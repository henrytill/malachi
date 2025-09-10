project(
    'malachi',
    'c',
    version: '0.1.0',
    default_options: ['warning_level=2', 'c_std=c17'],
)

libgit2_dep = dependency('libgit2', required: true)

mupdf_dep = dependency('mupdf', required: false)

sqlite_dep = dependency('sqlite3', required: true)

include_dir = include_directories('include')

root_dir = include_directories('.')

configure_file(
    output: 'project.h',
    configuration: {
        'MALACHI_VERSION_MAJOR': meson.project_version().split('.')[0],
        'MALACHI_VERSION_MINOR': meson.project_version().split('.')[1],
        'MALACHI_VERSION_PATCH': meson.project_version().split('.')[2],
        'MALACHI_HAVE_MUPDF': mupdf_dep.found(),
    },
)

executable(
    'malachi',
    sources: ['src/config.c', 'src/main.c', 'src/path.c'],
    dependencies: [libgit2_dep, mupdf_dep, sqlite_dep],
    include_directories: [include_dir, root_dir],
    install: true,
)

test_include_dir = include_directories('test')

platform_test = executable(
    'platform_test',
    sources: ['test/platform_test.c', 'src/path.c'],
    include_directories: [include_dir, test_include_dir],
)

config_test_sources = ['test/config_test.c', 'src/config.c', 'src/path.c']
if host_machine.system() == 'darwin'
    config_test_sources += ['test/config_test_macos.c']
else
    config_test_sources += ['test/config_test_xdg.c']
endif

config_test = executable(
    'config_test',
    sources: config_test_sources,
    include_directories: [include_dir, test_include_dir],
)

test('platform_test', platform_test)
test('config_test', config_test)
