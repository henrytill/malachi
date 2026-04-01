project(
    'malachi',
    'cpp',
    version: '0.1.0',
    default_options: ['warning_level=2', 'cpp_std=c++20'],
)

perl = find_program('perl', required: false)
if perl.found()
    perl_modules = ['Cwd', 'File::Basename', 'File::Spec', 'Getopt::Std', 'Git', 'JSON::XS', 'JSON::PP']
    foreach module : perl_modules
        result = run_command(perl, '-e', 'use @0@;'.format(module), check: false)
        if result.returncode() == 0
            message('Perl module @0@ found: YES'.format(module))
        else
            warning('Perl module @0@ found: NO'.format(module))
        endif
    endforeach
endif

libgit2_dep = dependency('libgit2', required: true)

mupdf_dep = dependency('mupdf', required: false)

sqlite_dep = dependency('sqlite3', required: true)

catch2_dep = dependency('catch2', fallback: ['catch2', 'catch2_dep'])

yyjson_dep = dependency('yyjson', fallback: ['yyjson', 'yyjson_dep'])

include_dir = include_directories('include')

root_dir = include_directories('.')

fs = import('fs')
schema = fs.read('src/schema.sql')
schema_escaped = schema.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n')
schema_h = configure_file(
    input: 'include/schema.h.in',
    output: 'schema.h',
    configuration: {'MALACHI_SCHEMA_SQL': '"' + schema_escaped + '"'},
)

configure_file(
    output: 'project.h',
    configuration: {
        'MALACHI_VERSION_MAJOR': meson.project_version().split('.')[0],
        'MALACHI_VERSION_MINOR': meson.project_version().split('.')[1],
        'MALACHI_VERSION_PATCH': meson.project_version().split('.')[2],
        'MALACHI_HAVE_MUPDF': mupdf_dep.found(),
    },
)

malachi_sources = [
    'src/config.cpp',
    'src/logging.cpp',
    'src/db.cpp',
    'src/status.cpp',
    'src/filter.cpp',
    'src/parser.cpp',
    'src/main.cpp',
    schema_h,
]

malachi_deps = [libgit2_dep, sqlite_dep, yyjson_dep]

if mupdf_dep.found()
    malachi_sources += ['src/filter_mupdf.cpp']
    malachi_deps += [mupdf_dep]
endif

executable(
    'malachi',
    sources: malachi_sources,
    dependencies: malachi_deps,
    include_directories: [include_dir, root_dir],
    install: true,
)

parser_test = executable(
    'parser_test',
    sources: [
        'src/logging.cpp',
        'src/parser.cpp',
        'test/parser_test.cpp',
    ],
    dependencies: [catch2_dep, yyjson_dep],
    include_directories: [include_dir, root_dir],
)

test('parser_test', parser_test)

db_test = executable(
    'db_test',
    sources: [
        'src/config.cpp',
        'src/logging.cpp',
        'src/db.cpp',
        'src/status.cpp',
        'test/db_test.cpp',
        schema_h,
    ],
    dependencies: [catch2_dep, sqlite_dep],
    include_directories: [include_dir, root_dir],
)

test('db_test', db_test)

platform_test = executable(
    'platform_test',
    sources: 'test/platform_test.cpp',
    dependencies: catch2_dep,
    include_directories: include_dir,
)

config_test = executable(
    'config_test',
    sources: ['src/config.cpp', 'test/config_test.cpp'],
    dependencies: catch2_dep,
    include_directories: include_dir,
)

test('platform_test', platform_test)

test('config_test', config_test)

if not get_option('prefix').startswith(meson.current_source_dir())
    install_data('bin/git-crawl', install_dir: get_option('bindir'))
    install_man('man/man1/malachi.1')
endif
