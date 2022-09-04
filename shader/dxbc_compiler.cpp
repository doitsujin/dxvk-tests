#include <iostream>
#include <iterator>
#include <fstream>
#include <limits>

#include <d3d11.h>
#include <windows.h>

#include "../common/com.h"
#include "../common/str.h"

MIDL_INTERFACE("bb8a4fb9-3935-4762-b44b-35189a26414a")
ID3D11VkExtShader : public IUnknown {
  virtual HRESULT STDMETHODCALLTYPE GetSpirvCode(
          SIZE_T*                 pCodeSize,
          void*                   pCode) = 0;
};

#ifdef _MSC_VER
struct __declspec(uuid("bb8a4fb9-3935-4762-b44b-35189a26414a")) ID3D11VkExtShader;
#else
__CRT_UUID_DECL(ID3D11VkExtShader, 0xbb8a4fb9,0x3935,0x4762,0xb4,0x4b,0x35,0x18,0x9a,0x26,0x41,0x4a);
#endif

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
  int     argc = 0;
  LPWSTR* argv = CommandLineToArgvW(
    GetCommandLineW(), &argc);  
  
  if (argc < 3) {
    std::cerr << "Usage: dxbc-compiler input.dxbc output.spv" << std::endl;
    return 1;
  }

  Com<ID3D11Device> device;

  HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
    nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &device, nullptr, nullptr);

  if (FAILED(hr)) {
    std::cerr << "Failed to create D3D11 device" << std::endl;
    return 1;
  }

  std::string ifileName = fromws(argv[1]);
  std::ifstream ifile(ifileName, std::ios::binary);
  ifile.ignore(std::numeric_limits<std::streamsize>::max());
  std::streamsize length = ifile.gcount();
  ifile.clear();
  
  ifile.seekg(0, std::ios_base::beg);
  std::vector<char> dxbcCode(length);
  ifile.read(dxbcCode.data(), length);

  Com<ID3D11DeviceChild> shader;

  Com<ID3D11VertexShader> vs;
  Com<ID3D11HullShader> hs;
  Com<ID3D11DomainShader> ds;
  Com<ID3D11GeometryShader> gs;
  Com<ID3D11PixelShader> ps;
  Com<ID3D11ComputeShader> cs;

  if (SUCCEEDED(device->CreateVertexShader(dxbcCode.data(), dxbcCode.size(), nullptr, &vs)))
    shader = vs.ptr();
  else if (SUCCEEDED(device->CreateHullShader(dxbcCode.data(), dxbcCode.size(), nullptr, &hs)))
    shader = hs.ptr();
  else if (SUCCEEDED(device->CreateDomainShader(dxbcCode.data(), dxbcCode.size(), nullptr, &ds)))
    shader = ds.ptr();
  else if (SUCCEEDED(device->CreateGeometryShader(dxbcCode.data(), dxbcCode.size(), nullptr, &gs)))
    shader = gs.ptr();
  else if (SUCCEEDED(device->CreatePixelShader(dxbcCode.data(), dxbcCode.size(), nullptr, &ps)))
    shader = ps.ptr();
  else if (SUCCEEDED(device->CreateComputeShader(dxbcCode.data(), dxbcCode.size(), nullptr, &cs)))
    shader = cs.ptr();

  if (shader == nullptr) {
    std::cerr << "Failed to create shader" << std::endl;
    return 1;
  }

  Com<ID3D11VkExtShader> extShader;

  if (FAILED(shader->QueryInterface(IID_PPV_ARGS(&extShader)))) {
    std::cerr << "Failed to query ID3D11VkExtShader from shader" << std::endl;
    return 1;
  }

  SIZE_T size = 0;
  extShader->GetSpirvCode(&size, nullptr);
  std::vector<char> spirvCode(size);
  extShader->GetSpirvCode(&size, spirvCode.data());

  std::ofstream ofile(fromws(argv[2]), std::ios::binary);
  ofile.write(spirvCode.data(), size);
  return 0;
}
