#include <array>
#include <cstring>
#include <iostream>
#include <vector>


#define D3D_DEBUG_INFO
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
    HRESULT status = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_d3d);

    if (FAILED(status))
      throw Error("Failed to create D3D9 interface");

    UINT adapter = D3DADAPTER_DEFAULT;

    D3DADAPTER_IDENTIFIER9 adapterId;
    m_d3d->GetAdapterIdentifier(adapter, 0, &adapterId);

    std::cout << format("Using adapter: ", adapterId.Description) << std::endl;

    std::vector<D3DDISPLAYMODEEX> modes;
    D3DDISPLAYMODEFILTER filter = {
         sizeof(D3DDISPLAYMODEFILTER),
         D3DFMT_X8R8G8B8, D3DSCANLINEORDERING_PROGRESSIVE
    };
    uint32_t modeCount = m_d3d->GetAdapterModeCountEx(adapter, &filter);
    modes.resize(modeCount);

    for (uint32_t i = 0; i < modeCount; i++) {
        modes[i].Size = sizeof(D3DDISPLAYMODEEX);
        status = m_d3d->EnumAdapterModesEx(adapter, &filter, i, &modes[i]);
        if (FAILED(status))
            throw Error("Failed to create D3D9 device");
    }

    D3DPRESENT_PARAMETERS params;
    getPresentParams(params);

    D3DDISPLAYMODEEX mode = {};
    Extent2D displaySize = { 0, 0 };
    for (uint32_t i = 0; i < modeCount; i++) {
        if (modes[i].Width == params.BackBufferWidth && modes[i].Height == params.BackBufferHeight && modes[i].RefreshRate == params.FullScreen_RefreshRateInHz) {
            mode = modes[i];
        }

        if (modes[i].Width >= displaySize.w && modes[i].Height >= displaySize.h) {
          displaySize = { modes[i].Width, modes[i].Height };
        }
    }

    status = m_d3d->CreateDeviceEx(
      adapter,
      D3DDEVTYPE_HAL,
      m_window,
      D3DCREATE_HARDWARE_VERTEXPROCESSING,
      &params,
      m_fullscreen ? &mode : nullptr,
      &m_device);

    if (FAILED(status))
        throw Error("Failed to create D3D9 device");
      
    status = m_device->Reset(&params);

    if (m_fullscreen) {
        displaySize = { mode.Width, mode.Height };
    }
    status = m_device->CreateOffscreenPlainSurface(displaySize.w, displaySize.h, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_frontBufferData, nullptr);
    if (FAILED(status))
        throw Error("Failed to create offscreen surface");
    status = m_device->CreateOffscreenPlainSurface(displaySize.w, displaySize.h, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_frontBufferDataDefault, nullptr);
    if (FAILED(status))
        throw Error("Failed to create offscreen surface");
  }

  const std::array<D3DCOLOR, 6> COLORS = {
    D3DCOLOR_RGBA(255, 0, 0, 0),
    D3DCOLOR_RGBA(0, 255, 0, 0),
    D3DCOLOR_RGBA(0, 0, 255, 0),
    D3DCOLOR_RGBA(0, 255, 255, 0),
    D3DCOLOR_RGBA(255, 255, 0, 0),
    D3DCOLOR_RGBA(255, 0, 255, 0),
  };

  void testFrontbuffer(IDirect3DSurface9* backbuffer) {
      m_device->BeginScene();
      m_device->SetRenderTarget(0, backbuffer);

      if (m_frameCounter < m_backbufferCount) {
          m_device->Clear(
              0,
              nullptr,
              D3DCLEAR_TARGET,
              COLORS[m_frameCounter % m_backbufferCount],
              0,
              0);
      }
      m_device->EndScene();

      m_device->PresentEx(
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          0);
  }

  void testPartialPresent(IDirect3DSurface9* backbuffer) {
      m_device->BeginScene();
      m_device->SetRenderTarget(0, backbuffer);

      m_device->Clear(
        0,
        nullptr,
        D3DCLEAR_TARGET,
        COLORS[m_frameCounter % COLORS.size()],
        0,
        0);
      m_device->EndScene();

      static std::array<RECT, 7> s_dstRects = {
        RECT { 0, 0, 64, 64 },
        RECT { 64, 0, 128, 64 },
        RECT { 128, 0, 192, 64 },
        RECT { 192, 0, 256, 64 },
        RECT { 256, 0, 320, 64 },
        RECT { 320, 0, 384, 64 },
        RECT { 384, 0, 448, 64 },
      };

      m_device->PresentEx(
        nullptr,
        &s_dstRects[m_frameCounter % s_dstRects.size()],
        nullptr,
        nullptr,
        0);
  }

  void testGetFrontBufferData(IDirect3DSurface9* backbuffer) {
      m_device->BeginScene();
      m_device->SetRenderTarget(0, backbuffer);

      if (m_frameCounter < m_backbufferCount) {
          m_device->Clear(
              0,
              nullptr,
              D3DCLEAR_TARGET,
              COLORS[m_frameCounter % m_backbufferCount],
              0,
              0);
      }

      IDirect3DSurface9* surf = m_frontBufferData.ptr();
      HRESULT status = m_device->GetFrontBufferData(0, surf);
      if (FAILED(status))
          throw Error("GetFrontBufferData failed");

      D3DLOCKED_RECT lockedRect = {};
      m_frontBufferData->LockRect(&lockedRect, nullptr, 0);
      D3DCOLOR* data = reinterpret_cast<D3DCOLOR*>(lockedRect.pBits);
      m_frontBufferData->UnlockRect();

      // UpdateSurface requires the same format, GetFrontBufferData requires XRGB and thats not supported as a backbuffer format
      // So we have to use StretchRect instead, which requires D3DPOOL_DEFAULT textures.

      status = m_device->UpdateSurface(m_frontBufferData.ptr(), nullptr, m_frontBufferDataDefault.ptr(), nullptr);
      if (FAILED(status))
          throw Error("UpdateSurface failed");

      RECT srcRect = { 0, 0, 64, 64 };
      RECT dstRect = { 0, 64, 64, 128 };
      status = m_device->StretchRect(m_frontBufferDataDefault.ptr(), &srcRect, backbuffer, &dstRect, D3DTEXF_LINEAR);
      if (FAILED(status))
          throw Error("StretchRect failed");

      m_device->EndScene();

      m_device->PresentEx(
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          0);
  }
 
  void run() {
    if (!m_fullscreen)
      this->adjustBackBuffer();

    Com<IDirect3DSurface9> backbuffer;
    m_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

    std::cerr << "Backbuffer index: " << (m_frameCounter % m_backbufferCount) << std::endl;
    std::cerr << "Frame index: " << (m_frameCounter) << std::endl;

    testPartialPresent(backbuffer.ptr());
    //testFrontbuffer(backbuffer.ptr());
    //testGetFrontBufferData(backbuffer.ptr());

    Sleep(2000);

    m_frameCounter++;
  }
  
  void adjustBackBuffer() {
    RECT windowRect = { 0, 0, 1024, 600 };
    GetClientRect(m_window, &windowRect);

    Extent2D newSize = {
      static_cast<uint32_t>(windowRect.right - windowRect.left),
      static_cast<uint32_t>(windowRect.bottom - windowRect.top),
    };

    if (m_windowSize.w != newSize.w
     || m_windowSize.h != newSize.h) {
      m_windowSize = newSize;

      D3DPRESENT_PARAMETERS params;
      getPresentParams(params);
      HRESULT status = m_device->ResetEx(&params, nullptr);

      if (FAILED(status))
        throw Error("Device reset failed");
    }
  }
  
  void getPresentParams(D3DPRESENT_PARAMETERS& params) {
      if (!m_fullscreen) {
        params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
        params.BackBufferCount = m_backbufferCount;
        params.BackBufferFormat = D3DFMT_X8R8G8B8;
        params.BackBufferWidth = m_windowSize.w;
        params.BackBufferHeight = m_windowSize.h;
        params.EnableAutoDepthStencil = false;
        params.Flags = 0;
        params.FullScreen_RefreshRateInHz = 0;
        params.hDeviceWindow = m_window;
        params.MultiSampleQuality = 0;
        params.MultiSampleType = D3DMULTISAMPLE_NONE;
        params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
        params.SwapEffect = m_swapEffect;
        params.Windowed = true;
      } else {
        params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
        params.BackBufferCount = m_backbufferCount;
        params.BackBufferFormat = D3DFMT_X8R8G8B8;
        params.BackBufferWidth = m_windowSize.w;
        params.BackBufferHeight = m_windowSize.h;
        params.EnableAutoDepthStencil = false;
        params.Flags = 0;
        params.FullScreen_RefreshRateInHz = 144;
        params.hDeviceWindow = m_window;
        params.MultiSampleQuality = 0;
        params.MultiSampleType = D3DMULTISAMPLE_NONE;
        params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
        params.SwapEffect = m_swapEffect;
        params.Windowed = false;
      }
  }
    
private:
  
  HWND                          m_window;
  Extent2D                      m_windowSize = { 1920, 1080};
  bool                          m_fullscreen = false;
  uint32_t                      m_backbufferCount = 1;
  D3DSWAPEFFECT                 m_swapEffect = D3DSWAPEFFECT_DISCARD;

  Com<IDirect3D9Ex>             m_d3d;
  Com<IDirect3DDevice9Ex>       m_device;
  
  Com<IDirect3DSurface9> m_frontBufferData;
  Com<IDirect3DSurface9> m_frontBufferDataDefault;

  uint32_t m_frameCounter = 0;
  
};

LRESULT CALLBACK WindowProc(HWND hWnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);

int main(int argc, char** argv) {
  HINSTANCE hInstance = GetModuleHandle(nullptr);
  int nCmdShow = SW_SHOWDEFAULT;
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
  ShowWindow(hWnd, nCmdShow);

  MSG msg;
  
  try {
    TriangleApp app(hInstance, hWnd);
  
    while (true) {
      if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        if (msg.message == WM_QUIT)
          return msg.wParam;
      } else {
        app.run();
      }
    }
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

  return DefWindowProcW(hWnd, message, wParam, lParam);
}
