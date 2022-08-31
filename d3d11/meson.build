test_d3d11_deps = [ lib_dxgi, lib_d3d11, lib_d3dcompiler_47 ]

args = {
  'dependencies': test_d3d11_deps,
  'install': true
}

executable('d3d11-compute', files('d3d11_compute.cpp'), kwargs: args)
executable('d3d11-formats', files('d3d11_formats.cpp'), kwargs: args)
executable('d3d11-tiled', files('d3d11_tiled.cpp'), kwargs: args)
executable('d3d11-triangle', files('d3d11_triangle.cpp'), gui_app: true, kwargs: args)
executable('d3d11-video', files('d3d11_video.cpp'), gui_app: true, kwargs: args)
executable('dxgi-adapters', files('dxgi_adapters.cpp'), kwargs: args)

install_data('video_image.raw', install_dir : get_option('bindir'))
