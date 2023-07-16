test_shader_deps = [ lib_dxgi, lib_d3d11, lib_d3dcompiler_47 ]

args = {
  'dependencies': test_shader_deps,
  'install': true
}

executable('hlsl-compiler', files('hlsl_compiler.cpp'), gui_app: true, kwargs: args)
executable('dxbc-disasm', files('dxbc_disasm.cpp'), gui_app: true, kwargs: args)
executable('dxbc-compiler', files('dxbc_compiler.cpp'), gui_app: true, kwargs: args)
