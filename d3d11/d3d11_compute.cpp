#include <array>
#include <iostream>
#include <vector>

#include <d3dcompiler.h>
#include <d3d11_1.h>
#include <dxgi1_6.h>

#include <windows.h>
#include <windowsx.h>

#include <cstring>
#include <string>
#include <sstream>

#include "../common/com.h"
#include "../common/str.h"

const std::string g_computeShaderCode =
  "RWTexture2D<float4> t_uav : register(u0);\n"
  "[numthreads(8,8,1)]\n"
  "void main(uint3 pos : SV_DispatchThreadID) {\n"
  "  t_uav[pos.xy] = float4(float((pos.x ^ pos.y) & 4).xxx, 1.0f);\n"
  "}\n";

class TriangleApp {
  
public:
  
  TriangleApp(HINSTANCE instance, HWND window)
  : m_window(window) {
    HRESULT status = D3D11CreateDevice(
      nullptr, D3D_DRIVER_TYPE_HARDWARE,
      nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
      &m_device, nullptr, &m_context);

    if (FAILED(status)) {
      std::cerr << "Failed to create D3D11 device" << std::endl;
      return;
    }

    Com<IDXGIDevice> dxgiDevice;

    if (FAILED(m_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice)))) {
      std::cerr << "Failed to query DXGI device" << std::endl;
      return;
    }

    if (FAILED(dxgiDevice->GetAdapter(&m_adapter))) {
      std::cerr << "Failed to query DXGI adapter" << std::endl;
      return;
    }

    if (FAILED(m_adapter->GetParent(IID_PPV_ARGS(&m_factory)))) {
      std::cerr << "Failed to query DXGI factory" << std::endl;
      return;
    }

    DXGI_SWAP_CHAIN_DESC1 swapDesc = { };
    swapDesc.Width          = m_windowSizeW;
    swapDesc.Height         = m_windowSizeH;
    swapDesc.Format         = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.Stereo         = FALSE;
    swapDesc.SampleDesc     = { 1, 0 };
    swapDesc.BufferUsage    = DXGI_USAGE_UNORDERED_ACCESS;
    swapDesc.BufferCount    = 3;
    swapDesc.Scaling        = DXGI_SCALING_STRETCH;
    swapDesc.SwapEffect     = DXGI_SWAP_EFFECT_DISCARD;
    swapDesc.AlphaMode      = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapDesc.Flags          = 0;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc;
    fsDesc.RefreshRate      = { 0, 0 };
    fsDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    fsDesc.Scaling          = DXGI_MODE_SCALING_UNSPECIFIED;
    fsDesc.Windowed         = TRUE;
    
    Com<IDXGISwapChain1> swapChain;
    Com<IDXGISwapChain4> swapChain4;
    if (FAILED(m_factory->CreateSwapChainForHwnd(m_device.ptr(), m_window, &swapDesc, &fsDesc, nullptr, &swapChain))) {
      std::cerr << "Failed to create DXGI swap chain" << std::endl;
      return;
    }

    if (FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain)))) {
      std::cerr << "Failed to query DXGI swap chain interface" << std::endl;
      return;
    }

    Com<ID3DBlob> computeShaderBlob;
    
    if (FAILED(D3DCompile(g_computeShaderCode.data(), g_computeShaderCode.size(),
        "Vertex shader", nullptr, nullptr, "main", "cs_5_0", 0, 0, &computeShaderBlob, nullptr))) {
      std::cerr << "Failed to compile compute shader" << std::endl;
      return;
    }
    if (FAILED(m_device->CreateComputeShader(
        computeShaderBlob->GetBufferPointer(),
        computeShaderBlob->GetBufferSize(),
        nullptr, &m_cs))) {
      std::cerr << "Failed to create compute shader" << std::endl;
      return;
    }
  }
  
  
  ~TriangleApp() {
    m_context->ClearState();
  }
  
  
  bool run() {
    if (!beginFrame())
      return false;

    m_context->CSSetShader(m_cs.ptr(), nullptr, 0);
    m_context->CSSetUnorderedAccessViews(0, 1, &m_uav, nullptr);
    m_context->Dispatch((m_windowSizeW + 7) / 8, (m_windowSizeH + 7) / 8, 1);

    return SUCCEEDED(m_swapChain->Present(0, 0));
  }


  bool beginFrame() {
    // Make sure we can actually render to the window
    RECT windowRect = { 0, 0, 1024, 600 };
    GetClientRect(m_window, &windowRect);
    
    uint32_t newWindowSizeW = uint32_t(windowRect.right - windowRect.left);
    uint32_t newWindowSizeH = uint32_t(windowRect.bottom - windowRect.top);
    
    if (m_windowSizeW != newWindowSizeW || m_windowSizeH != newWindowSizeH || m_uav == nullptr) {
      m_uav = nullptr;
      m_context->ClearState();

      DXGI_SWAP_CHAIN_DESC desc;
      m_swapChain->GetDesc(&desc);

      if (FAILED(m_swapChain->ResizeBuffers(desc.BufferCount,
          newWindowSizeW, newWindowSizeH, DXGI_FORMAT_R8G8B8A8_UNORM, desc.Flags))) {
        std::cerr << "Failed to resize back buffers" << std::endl;
        return false;
      }
      
      Com<ID3D11Texture2D> backBuffer;
      if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
        std::cerr << "Failed to get swap chain back buffer" << std::endl;
        return false;
      }
      
      D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
      uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
      uavDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM;
      uavDesc.Texture2D     = { 0u };
      
      if (FAILED(m_device->CreateUnorderedAccessView(backBuffer.ptr(), &uavDesc, &m_uav))) {
        std::cerr << "Failed to create unordered access view" << std::endl;
        return false;
      }

      m_windowSizeW = newWindowSizeW;
      m_windowSizeH = newWindowSizeH;
    }

    return true;
  }

private:
  
  HWND                          m_window;
  uint32_t                      m_windowSizeW = 1024;
  uint32_t                      m_windowSizeH = 600;
  
  Com<IDXGIFactory3>            m_factory;
  Com<IDXGIAdapter>             m_adapter;
  Com<ID3D11Device>             m_device;
  Com<ID3D11DeviceContext>      m_context;
  Com<IDXGISwapChain>           m_swapChain;

  Com<ID3D11UnorderedAccessView> m_uav;
  Com<ID3D11ComputeShader>      m_cs;

};

LRESULT CALLBACK WindowProc(HWND hWnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  WNDCLASSEXW wc = { };
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = HBRUSH(COLOR_WINDOW);
  wc.lpszClassName = L"WindowClass";
  RegisterClassExW(&wc);

  HWND hWnd = CreateWindowExW(0, L"WindowClass", L"D3D11 compute",
    WS_OVERLAPPEDWINDOW, 300, 300, 1024, 600,
    nullptr, nullptr, hInstance, nullptr);
  ShowWindow(hWnd, nCmdShow);

  TriangleApp app(hInstance, hWnd);

  MSG msg;

  while (true) {
    if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
      
      if (msg.message == WM_QUIT)
        return msg.wParam;
    } else {
      if (!app.run())
        break;
    }
  }

  return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProcW(hWnd, message, wParam, lParam);
}
