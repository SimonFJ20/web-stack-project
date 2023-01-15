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

sources = []
subdir('browser')

executable(
    'web-browser',
    sources,
    win_subsystem: 'console',
    include_directories: [],
    override_options: [
        'werror=true'
    ],
    dependencies: [
        fmt_dep,
        sdl2_dep,
    ],
)
