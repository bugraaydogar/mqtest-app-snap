project('mqtest-client', 'c', default_options: [
        'c_std=gnu17', 'warning_level=3', 'debug=true', 'optimization=2'
])
add_project_arguments('-D_FORTIFY_SOURCE=2', '-Wformat', '-Wformat-security',
        '-fstack-protector-strong', '-fstack-clash-protection', language: 'c',
)
cc = meson.get_compiler('c')
rt_dep = cc.find_library('rt', required: true)
executable('client', 'client.c', dependencies: rt_dep, install: true)