project(
    'web-stack-project',
    'cpp',
    version: '0.1',
    default_options: [
        'warning_level=3',
        'cpp_std=c++20',
    ],
)

fmt_dep = dependency('fmt')
sdl2_dep = dependency('sdl2')

add_project_arguments(
    '-Wno-gnu-statement-expression-from-macro-expansion',
    language: 'cpp',
)

common_sources = []
subdir('utils')
subdir('scriptlang')
subdir('markup')

browser_sources = common_sources
subdir('browser')

executable(
    'web-browser',
    browser_sources,
    win_subsystem: 'console',
    include_directories: [
        include_directories('.'),
        include_directories('browser'),
    ],
    dependencies: [
        fmt_dep,
        sdl2_dep,
    ],
)

server_sources = common_sources
subdir('server')

executable(
    'web-server',
    server_sources,
    win_subsystem: 'console',
    include_directories: [
        include_directories('.'),
        include_directories('server'),
    ],
    dependencies: [
        fmt_dep,
    ],
)
