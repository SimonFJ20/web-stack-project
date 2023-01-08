project(
    'web-stack-project',
    'cpp',
    version: '0.1',
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++20',
    ],
)

fmt_dep = dependency('fmt')
sdl2_dep = dependency('sdl2')

server_sources = []
subdir('server')
server_exe = executable(
    'web-server',
    server_sources,
    win_subsystem: 'console',
    include_directories: [
        include_directories('server'),
    ],
    dependencies: [
        fmt_dep,
        sdl2_dep,
    ],
)

browser_sources = []
subdir('browser')
browser_exe = executable(
    'web-browser',
    browser_sources,
    win_subsystem: 'console',
    include_directories: [
        include_directories('browser'),
    ],
    dependencies: [
        fmt_dep,
        sdl2_dep,
    ],
)
