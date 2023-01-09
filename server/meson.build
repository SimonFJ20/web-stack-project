
server_sources = files(
    'main.cpp',
)

fmt_dep = dependency('fmt')

server_exe = executable(
    'web-server',
    server_sources,
    win_subsystem: 'console',
    dependencies: [
        fmt_dep,
        utils_dep,
        markup_dep,
    ],
)
