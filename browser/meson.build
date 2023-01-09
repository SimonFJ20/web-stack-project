
browser_sources = files(
    'main.cpp',
)

fmt_dep = dependency('fmt')
sdl2_dep = dependency('sdl2')

executable(
    'web-browser',
    browser_sources,
    win_subsystem: 'console',
    dependencies: [
        fmt_dep,
        sdl2_dep,
        utils_dep,
        markup_dep,
        scriptlang_dep,
    ],
)
