test_d3d9_deps = [ lib_d3d9, lib_d3dcompiler_47 ]

args = {
  'dependencies': test_d3d9_deps,
  'install': true
}

executable('d3d9-triangle', files('d3d9_triangle.cpp'), gui_app: true, kwargs: args)
executable('d3d9-present', files('d3d9_present.cpp'), gui_app: true, kwargs: args)
executable('d3d9-nv12', files('d3d9_nv12.cpp'), gui_app: true, kwargs: args)
