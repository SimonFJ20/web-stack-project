
scriptlang_sources = files(
    'parser.cpp',
    'lexer.cpp',
)

scriptlang_inc = include_directories('.')

fmt_dep = dependency('fmt')

scriptlang_lib = static_library(
    'web-scriptlang',
    scriptlang_sources,
    include_directories: [
        scriptlang_inc,
    ],
    dependencies: [
        fmt_dep,
        utils_dep,
    ],
)

scriptlang_dep = declare_dependency(
    link_with: scriptlang_lib,
    include_directories: [
        scriptlang_inc,
    ],
)
