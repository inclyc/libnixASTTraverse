libnixtDeps = [ nix_expr, nix_main, nix_cmd, boost ]

libnixdInc = include_directories('../include')

libnixt = library('nixt',
                  'Displacement.cpp',
                  'Name.cpp',
                  'ParentMap.cpp',
                  'Serialize.cpp',
                  include_directories: libnixdInc,
                  dependencies: libnixtDeps,
                  install: true
)

pkgconfig.generate(name: 'libnixt',
                   version: 'nightly',
                   description: 'nix tooling',
                   subdirs: [ 'nixt' ],
                   libraries: libnixt
)


nixt = declare_dependency(include_directories: libnixdInc,
                          dependencies: libnixtDeps,
                          link_with: libnixt
)
