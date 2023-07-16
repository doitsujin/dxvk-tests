project('dxvk-tests', ['cpp'], version : 'v1.0', meson_version : '>= 0.49', default_options : [ 'cpp_std=c++17' ])

cpu_family = target_machine.cpu_family()
platform   = target_machine.system()

cpp = meson.get_compiler('cpp')
dxvk_is_msvc = cpp.get_id() == 'msvc'

compiler_args = [
  '-msse',
  '-msse2',
  '-msse3',
  '-mfpmath=sse',
  '-Wimplicit-fallthrough',
  # clang
  '-Wno-unused-private-field',
  '-Wno-microsoft-exception-spec',
  # windows
  '-DNOMINMAX',
  '-D_WIN32_WINNT=0xa00',
]

link_args = [
  '-static',
  '-static-libgcc',
  '-static-libstdc++',
  # We need to set the section alignment for debug symbols to
  # work properly as well as avoiding a memcpy from the Wine loader.
  '-Wl,--file-alignment=4096',
]

# Wine's built-in back traces only work with dwarf2 symbols
if get_option('debug')
  compiler_args += [
    '-gstrict-dwarf',
    '-gdwarf-2',
  ]
endif

# Enable stdcall fixup on 32-bit
if cpu_family == 'x86'
  link_args += [
    '-Wl,--enable-stdcall-fixup',
    '-Wl,--add-stdcall-alias',
]
endif

lib_d3d9    = cpp.find_library('d3d9')
lib_d3d11   = cpp.find_library('d3d11')
lib_d3d12   = cpp.find_library('d3d12')
lib_dxgi    = cpp.find_library('dxgi')

if dxvk_is_msvc
  lib_d3dcompiler_47 = cpp.find_library('d3dcompiler')
else
  lib_d3dcompiler_47 = cpp.find_library('d3dcompiler_47')
endif

add_project_arguments(cpp.get_supported_arguments(compiler_args), language: 'cpp')
add_project_link_arguments(cpp.get_supported_link_arguments(link_args), language: 'cpp')

exe_ext = ''
dll_ext = ''
def_spec_ext = '.def'

if cpu_family == 'x86' and cpp.has_header('d3d8.h', include_directories: include_directories('.', 'd3d8'))
  subdir('d3d8')
endif
subdir('d3d9')
subdir('d3d11')
subdir('shader')
