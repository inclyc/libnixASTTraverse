nix_serialize = executable('nix-serialize',
                           'nix-serialize.cpp',
                           dependencies:  [ nixt, boost ],
                           install: true)

# regression_env = environment()

# regression_env.append('PATH', meson.current_build_dir())
# regression_env.set('MESON_BUILD_ROOT', meson.current_build_dir())

# if lit.found()
#     test('regression/nix-ast-dump', lit, env: regression_env, args: [ '-vv', meson.current_source_dir() + '/test' ], depends: [ nix_serialize ] )
# endif
