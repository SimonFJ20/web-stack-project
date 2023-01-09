
utils_sources = files()

utils_inc = include_directories('.')

fmt_dep = dependency('fmt')

utils_lib = static_library(
    'web-utils',
    utils_sources,
    include_directories: [
        utils_inc,
    ],
    dependencies: [
        fmt_dep,
    ],
)

utils_dep = declare_dependency(
    link_with: utils_lib,
    include_directories: [
        utils_inc,
    ],
)
