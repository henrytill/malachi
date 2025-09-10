project(
    'malachi',
    'c',
    version: '0.1.0',
    default_options: ['warning_level=2', 'c_std=c17'],
)

libgit2_dep = dependency('libgit2', required: true)

mupdf_dep = dependency('mupdf', required: false)

sqlite_dep = dependency('sqlite3', required: true)

project_config = configure_file(
    input: 'include/project.h.in',
    output: 'project.h.in',
    configuration: {
        'MALACHI_HAVE_MUPDF': mupdf_dep.found(),
        'MALACHI_VERSION_MAJOR': meson.project_version().split('.')[0],
        'MALACHI_VERSION_MINOR': meson.project_version().split('.')[1],
        'MALACHI_VERSION_PATCH': meson.project_version().split('.')[2],
        'MALACHI_COMMIT_SHORT_HASH': '"@MALACHI_COMMIT_SHORT_HASH@"',
    },
)

project_h = vcs_tag(
    command: ['git', 'rev-parse', '--short', 'HEAD'],
    fallback: '',
    input: project_config,
    output: 'project.h',
    replace_string: '@MALACHI_COMMIT_SHORT_HASH@'
)

include_dir = include_directories('include')

root_dir = include_directories('.')

executable(
    'malachi',
    sources: ['src/config.c', 'src/main.c', 'src/path.c', project_h],
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
