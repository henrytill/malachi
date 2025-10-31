project(
    'malachi',
    'c',
    version: '0.1.0',
    default_options: ['warning_level=3', 'c_std=c17'],
)

add_project_arguments('-D_XOPEN_SOURCE=600', language: 'c')

if get_option('optimization') != '0'
    add_project_arguments('-D_FORTIFY_SOURCE=2', language: 'c')
endif

if get_option('json_protocol')
    add_project_arguments('-DMALACHI_JSON_PROTOCOL', language: 'c')
endif

perl = find_program('perl', required: false)
if perl.found()
    perl_modules = ['Cwd', 'File::Basename', 'Getopt::Std', 'Git']

    foreach module : perl_modules
        result = run_command(perl, '-e', 'use @0@;'.format(module), check: false)
        if result.returncode() == 0
            message('Perl module @0@ found: YES'.format(module))
        else
            warning('Perl module @0@ found: NO'.format(module))
        endif
    endforeach
endif

mupdf_dep = dependency('mupdf', required: false)

sqlite_dep = dependency('sqlite3', required: true)

# yyjson typically doesn't provide pkg-config, try both methods
yyjson_dep = disabler()
if get_option('json_protocol')
    yyjson_dep = dependency('yyjson', required: false)
    if not yyjson_dep.found()
        cc = meson.get_compiler('c')
        yyjson_dep = cc.find_library('yyjson', required: true)
    endif
endif

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

fs = import('fs')
schema = fs.read('src/cmd/malachi/schema.sql')
schema_escaped = schema.replace('\\', '\\\\').replace('"', '\\"').replace(
    '\n',
    '\\n',
)

schema_h = configure_file(
    input: 'src/cmd/malachi/schema.h.in',
    output: 'schema.h',
    configuration: {'MALACHI_SCHEMA_SQL': '"' + schema_escaped + '"'},
)

platform_sources = []
if host_machine.system() == 'darwin'
    platform_sources += ['src/cmd/malachi/platmac.c']
else
    platform_sources += ['src/cmd/malachi/platxdg.c']
endif

parser_sources = []
if get_option('json_protocol')
    parser_sources += ['src/cmd/malachi/parserjson.c']
else
    parser_sources += ['src/cmd/malachi/parser.c']
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

malachi_deps = [sqlite_dep]
if get_option('json_protocol')
    malachi_deps += [yyjson_dep]
endif
if mupdf_dep.found()
    malachi_deps += [mupdf_dep]
endif

malachi = executable(
    'malachi',
    sources: [
        'src/cmd/malachi/malachi.c',
        'src/cmd/malachi/config.c',
        'src/cmd/malachi/db.c',
        'src/cmd/malachi/filt.c',
        'src/cmd/malachi/path.c',
        'src/cmd/malachi/test.c',
        'src/cmd/malachi/util.c',
        platform_sources,
        parser_sources,
        filter_sources,
        test_sources,
        project_h,
        schema_h,
    ],
    dependencies: malachi_deps,
    install: true,
)

if not get_option('prefix').startswith(meson.current_source_dir())
    install_data('bin/git-crawl', install_dir: get_option('bindir'))
endif

test('config_test', malachi, args: ['-tconfig'])
test('platform_test', malachi, args: ['-tplatform'])
