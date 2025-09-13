project(
    'malachi',
    'c',
    version: '0.1.0',
    default_options: ['warning_level=2', 'c_std=gnu17'],
)

add_project_arguments('-D_XOPEN_SOURCE=600', language: 'c')

if get_option('optimization') != '0'
    add_project_arguments('-D_FORTIFY_SOURCE=2', language: 'c')
endif

libgit2_dep = dependency('libgit2', required: true)

mupdf_dep = dependency('mupdf', required: false)

sqlite_dep = dependency('sqlite3', required: true)

project_config = configure_file(
    input: 'include/project.h.in',
    output: 'project.h.in',
    configuration: {
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
    replace_string: '@MALACHI_COMMIT_SHORT_HASH@',
)

platform_sources = []
if host_machine.system() == 'darwin'
    platform_sources += ['src/cmd/malachi/platmac.c']
else
    platform_sources += ['src/cmd/malachi/platxdg.c']
endif

filter_sources = []
if mupdf_dep.found()
    filter_sources += ['src/cmd/malachi/filtmupdf.c']
endif

test_sources = ['src/cmd/malachi/testconf.c', 'src/cmd/malachi/testplat.c']
if host_machine.system() == 'darwin'
    test_sources += ['src/cmd/malachi/testconfmac.c']
else
    test_sources += ['src/cmd/malachi/testconfxdg.c']
endif

malachi = executable(
    'malachi',
    sources: [
        'src/cmd/malachi/config.c',
        'src/cmd/malachi/filt.c',
        'src/cmd/malachi/main.c',
        'src/cmd/malachi/path.c',
        'src/cmd/malachi/test.c',
        'src/cmd/malachi/util.c',
        platform_sources,
        filter_sources,
        test_sources,
        project_h,
    ],
    dependencies: [libgit2_dep, mupdf_dep, sqlite_dep],
    install: true,
)

test('config_test', malachi, args: ['-tconfig'])
test('platform_test', malachi, args: ['-tplatform'])
