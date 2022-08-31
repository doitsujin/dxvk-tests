test_d3d11_deps = [ lib_dxgi, lib_d3d11, lib_d3dcompiler_47 ]

args = {
  'dependencies': test_d3d11_deps,
  'install': true
}

executable('d3d11-compute', files('d3d11_compute.cpp'), kwargs: args)
executable('dxgi-adapters', files('dxgi_adapters.cpp'), kwargs: args)
