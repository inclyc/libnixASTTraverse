project('libnixt',
        ['c', 'cpp'],
        default_options : ['cpp_std=gnu++20'],
        version: 'nightly'
)

nix_main = dependency('nix-main')
nix_expr = dependency('nix-expr')
nix_cmd = dependency('nix-cmd')
boost = dependency('boost')
pkgconfig = import('pkgconfig')
lit = find_program('lit', required: false)

subdir('lib')

install_subdir('include', install_dir : 'include/nixt')

subdir('tools')
