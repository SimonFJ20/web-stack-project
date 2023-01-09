
markup_sources = files(
    'parser.cpp',
    'lexer.cpp',
)

markup_inc = include_directories('.')

fmt_dep = dependency('fmt')

markup_lib = static_library(
    'web-markup',
    markup_sources,
    include_directories: [
        markup_inc,
    ],
    dependencies: [
        fmt_dep,
        utils_dep,
    ],
)

markup_dep = declare_dependency(
    link_with: markup_lib,
    include_directories: [
        markup_inc,
    ],
)
