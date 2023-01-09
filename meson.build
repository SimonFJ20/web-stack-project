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


utils_sources = []
subdir('utils')

scriptlang_sources = []
subdir('scriptlang')

markup_sources = []
subdir('markup')

common_sources = utils_sources + scriptlang_sources + markup_sources


browser_sources = [] + common_sources
subdir('browser')
browser_sources

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

server_sources = [] + common_sources
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
