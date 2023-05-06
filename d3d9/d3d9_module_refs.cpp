#include <array>
#include <cstring>
#include <iostream>
#include <vector>

#include <d3d9.h>
#include <d3dcompiler.h>

#include "../common/com.h"
#include "../common/error.h"
#include "../common/str.h"

struct Extent2D {
    uint32_t w, h;
};

class TriangleApp {
  
public:
  
  TriangleApp(HINSTANCE instance, HWND window)
  : m_window(window) {
    // This test does not link against d3d9 directly.


    // Test instance alone:
    {
      HMODULE hD3D9 = LoadLibraryA("d3d9.dll");
      if (!hD3D9)
        throw Error("Failed to load D3D9 library");

      auto pDirect3DCreate9Ex = (decltype(Direct3DCreate9Ex)*)GetProcAddress(hD3D9, "Direct3DCreate9Ex");

      IDirect3D9Ex* pD3D9 = nullptr;
      HRESULT status = pDirect3DCreate9Ex(D3D_SDK_VERSION, &pD3D9);

      if (FAILED(status))
        throw Error("Failed to create D3D9 interface");

      D3DPRESENT_PARAMETERS params;
      getPresentParams(params);

      IDirect3DDevice9Ex* pDevice = nullptr;
      status = pD3D9->CreateDeviceEx(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        m_window,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &params,
        nullptr,
        &pDevice);
      if (FAILED(status))
        throw Error("Failed to create D3D9 device");

      hD3D9 = GetModuleHandleA("d3d9.dll");
      std::cout << "hD3D9: After CreateDevice: " << hD3D9 << std::endl;

      FreeLibrary(hD3D9);
      hD3D9 = nullptr;
      
      hD3D9 = GetModuleHandleA("d3d9.dll");
      std::cout << "hD3D9: After FreeLibrary and CreateDevice: " << hD3D9 << std::endl;
    }
  }
 
  
  void getPresentParams(D3DPRESENT_PARAMETERS& params) {
    params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
    params.BackBufferCount = 1;
    params.BackBufferFormat = D3DFMT_X8R8G8B8;
    params.BackBufferWidth = m_windowSize.w;
    params.BackBufferHeight = m_windowSize.h;
    params.EnableAutoDepthStencil = 0;
    params.Flags = 0;
    params.FullScreen_RefreshRateInHz = 0;
    params.hDeviceWindow = m_window;
    params.MultiSampleQuality = 0;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.Windowed = TRUE;
  }
    
private:
  
  HWND                          m_window;
  Extent2D                      m_windowSize = { 1024, 600 };
  
};

LRESULT CALLBACK WindowProc(HWND hWnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);

int main(int argc, char** argv) {
  HINSTANCE hInstance = GetModuleHandle(NULL);

  HWND hWnd;
  WNDCLASSEXW wc;
  ZeroMemory(&wc, sizeof(WNDCLASSEX));
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
  wc.lpszClassName = L"WindowClass1";
  RegisterClassExW(&wc);

  hWnd = CreateWindowExW(0,
    L"WindowClass1",
    L"Our First Windowed Program",
    WS_OVERLAPPEDWINDOW,
    300, 300,
    640, 480,
    nullptr,
    nullptr,
    hInstance,
    nullptr);
  ShowWindow(hWnd, 0);

  MSG msg;
  
  try {
    TriangleApp app(hInstance, hWnd);
    return 0;
  } catch (const Error& e) {
    std::cerr << e.message() << std::endl;
    return msg.wParam;
  }
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}
