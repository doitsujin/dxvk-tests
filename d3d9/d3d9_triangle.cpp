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

const std::string g_vertexShaderCode = R"(

struct VS_INPUT {
  float3 Position : POSITION;
};

struct VS_OUTPUT {
  float4 Position : POSITION;
};

VS_OUTPUT main( VS_INPUT IN ) {
  VS_OUTPUT OUT;
  OUT.Position = float4(IN.Position, 0.6f);

  return OUT;
}

)";

const std::string g_pixelShaderCode = R"(

struct VS_OUTPUT {
  float4 Position : POSITION;
};

struct PS_OUTPUT {
  float4 Colour   : COLOR;
};

sampler g_texDepth : register( s0 );

PS_OUTPUT main( VS_OUTPUT IN ) {
  PS_OUTPUT OUT;

  OUT.Colour = tex2D(g_texDepth, float2(0, 0));
  OUT.Colour = 1.0;

  return OUT;
}


)";

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

    auto CheckSRGBFormat = [&](D3DFORMAT fmt, const char* name) {
      HRESULT status = m_d3d->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_TEXTURE, fmt);
      std::cout << format("(linear) ", name, ": ", SUCCEEDED(status) ? "ok" : "nope") << std::endl;

      status = m_d3d->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_QUERY_SRGBREAD, D3DRTYPE_TEXTURE, fmt);
      std::cout << format("(srgb) ", name, ": ", SUCCEEDED(status) ? "ok" : "nope") << std::endl;
    };

    CheckSRGBFormat(D3DFMT_R5G6B5,       "R5G6B5");
    CheckSRGBFormat(D3DFMT_X1R5G5B5,     "X1R5G5B5");
    CheckSRGBFormat(D3DFMT_A1R5G5B5,     "A1R5G5B5");
    CheckSRGBFormat(D3DFMT_A4R4G4B4,     "A4R4G4B4");
    CheckSRGBFormat(D3DFMT_X4R4G4B4,     "X4R4G4B4");
    CheckSRGBFormat(D3DFMT_G16R16,       "G16R16");
    CheckSRGBFormat(D3DFMT_A2R10G10B10,  "A2R10G10B10");
    CheckSRGBFormat(D3DFMT_A16B16G16R16, "A16B16G16R16");

    //

    DWORD quality;
    status = m_d3d->CheckDepthStencilMatch(adapter, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_D24S8);
    status = m_d3d->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, D3DFMT_A8R8G8B8);
    status = m_d3d->CheckDeviceFormatConversion(adapter, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8);
    status = m_d3d->CheckDeviceMultiSampleType(adapter, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, FALSE, D3DMULTISAMPLE_NONE, &quality);
    status = m_d3d->CheckDeviceMultiSampleType(adapter, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, TRUE, D3DMULTISAMPLE_NONE, &quality);
    status = m_d3d->CheckDeviceType(adapter, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8, FALSE);
    status = m_d3d->CheckDeviceType(adapter, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, D3DFMT_A8R8G8B8, TRUE);

    // NULL
    constexpr D3DFORMAT NullFormat = D3DFORMAT(MAKEFOURCC('N', 'U', 'L', 'L'));

    status = m_d3d->CheckDepthStencilMatch(adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, NullFormat, D3DFMT_D24S8);
    status = m_d3d->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, NullFormat);
    status = m_d3d->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, NullFormat);
    status = m_d3d->CheckDeviceFormatConversion(adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, NullFormat);
    status = m_d3d->CheckDeviceType(adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, NullFormat, false);
    status = m_d3d->CheckDeviceType(adapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, NullFormat, true);
    //

    D3DPRESENT_PARAMETERS params;
    getPresentParams(params);

    status = m_d3d->CreateDeviceEx(
      adapter,
      D3DDEVTYPE_HAL,
      m_window,
      D3DCREATE_HARDWARE_VERTEXPROCESSING,
      &params,
      nullptr,
      &m_device);

    if (FAILED(status))
        throw Error("Failed to create D3D9 device");

    // Funny Swapchain Refcounting
    // "One of the things COM does really well, is lifecycle management"
    // Implicit Swapchain
    {
      IDirect3DSurface9* pSurface1 = nullptr;
      IDirect3DSurface9* pSurface2 = nullptr;
      status = m_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface1);
      D3DPRESENT_PARAMETERS newParams = params;
      newParams.BackBufferWidth  = 10;
      newParams.BackBufferHeight = 10;
      status = m_device->Reset(&newParams);
      status = m_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface2);

      IDirect3DSwapChain9* pSwapChain2 = nullptr;
      IDirect3DSwapChain9* pSwapChain3 = nullptr;
      status = pSurface1->GetContainer(__uuidof(IDirect3DSwapChain9), reinterpret_cast<void**>(&pSwapChain2));
      status = pSurface2->GetContainer(__uuidof(IDirect3DSwapChain9), reinterpret_cast<void**>(&pSwapChain3));

      printf("E_NOINTERFACE! for pSwapchain2");
      status = m_device->Reset(&params);
    }
    // Additional swapchain
    {
      IDirect3DSwapChain9* pSwapChain2 = nullptr;
      IDirect3DSwapChain9* pSwapChain3 = nullptr;
      IDirect3DSwapChain9* pSwapChain4 = nullptr;
      IDirect3DSurface9* pSurface = nullptr;
      status = m_device->CreateAdditionalSwapChain(&params, &pSwapChain2);
      status = pSwapChain2->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pSurface);
      status = pSurface->GetContainer(__uuidof(IDirect3DSwapChain9), reinterpret_cast<void**>(&pSwapChain3));
      pSwapChain2->Release();
      UINT count = pSwapChain2->Release();
      printf("Count: %u - Should be 0 and swapchain dead!", count);
      status = pSurface->GetContainer(__uuidof(IDirect3DSwapChain9), reinterpret_cast<void**>(&pSwapChain4));
      // E_NOINTERFACE !
      printf("E_NOINTERFACE!");
    }

    m_device->AddRef();

    Com<IDirect3DSurface9> backbuffer;
    m_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

    m_device->AddRef();

    Com<IDirect3DSwapChain9> swapchain;
    m_device->GetSwapChain(0, &swapchain);

    m_device->AddRef();

    DWORD bias = 0xDEADBEEF;
    status = m_device->GetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, &bias);
    status = m_device->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, MAKEFOURCC('G', 'E', 'T', '4'));
    status = m_device->GetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, &bias);
    status = m_device->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, MAKEFOURCC('G', 'E', 'T', '1'));
    status = m_device->GetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, &bias);

    // Vertex Shader
    {
      Com<ID3DBlob> blob;

      status = D3DCompile(
        g_vertexShaderCode.data(),
        g_vertexShaderCode.length(),
        nullptr, nullptr, nullptr,
        "main",
        "vs_2_0",
        0, 0, &blob,
        nullptr);

      if (FAILED(status))
        throw Error("Failed to compile vertex shader");

      status = m_device->CreateVertexShader(reinterpret_cast<const DWORD*>(blob->GetBufferPointer()), &m_vs);

      if (FAILED(status))
        throw Error("Failed to create vertex shader");
    }

    // Pixel Shader
    {
      Com<ID3DBlob> blob;

      status = D3DCompile(
        g_pixelShaderCode.data(),
        g_pixelShaderCode.length(),
        nullptr, nullptr, nullptr,
        "main",
        "ps_2_0",
        0, 0, &blob,
        nullptr);

      if (FAILED(status))
        throw Error("Failed to compile pixel shader");

      status = m_device->CreatePixelShader(reinterpret_cast<const DWORD*>(blob->GetBufferPointer()), &m_ps);

      if (FAILED(status))
        throw Error("Failed to create pixel shader");
    }

    m_device->SetVertexShader(m_vs.ptr());
    m_device->SetPixelShader(m_ps.ptr());

    m_device->AddRef();

    Com<IDirect3DSurface9> nullSurface;
    status = m_device->CreateRenderTarget(64, 64, D3DFORMAT(MAKEFOURCC('N', 'U', 'L', 'L')), D3DMULTISAMPLE_NONE, 0, FALSE, &nullSurface, nullptr);

    status = m_device->ColorFill(nullSurface.ptr(), nullptr, D3DCOLOR_RGBA(255, 0, 0, 255));

    Com<IDirect3DTexture9> defaultTexture;
    status = m_device->CreateTexture(64, 64, 1, 0, D3DFMT_DXT3, D3DPOOL_DEFAULT,   &defaultTexture, nullptr);

    m_device->AddRef();

    Com<IDirect3DSurface9> surface;
    status = defaultTexture->GetSurfaceLevel(0, &surface);

    m_device->AddRef();

    Com<IDirect3DTexture9> sysmemTexture;
    status = m_device->CreateTexture(64, 64, 1, 0, D3DFMT_DXT3, D3DPOOL_SYSTEMMEM, &sysmemTexture, nullptr);

    Com<IDirect3DSurface9> offscreenSurface;
    //status = m_device->CreateOffscreenPlainSurfaceEx(64, 64, D3DFMT_DXT3, D3DPOOL_DEFAULT, &offscreenSurface, nullptr, 0);
    status = m_device->CreateOffscreenPlainSurfaceEx(64, 64, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &offscreenSurface, nullptr, 0);

    D3DLOCKED_RECT offscreenLock;
    status = offscreenSurface->LockRect(&offscreenLock, nullptr, 0);

    std::memset(offscreenLock.pBits, 0xFF, offscreenLock.Pitch * (64 / 4));

    status = offscreenSurface->UnlockRect();

    //status = m_device->ColorFill(offscreenSurface.ptr(), nullptr, D3DCOLOR_ARGB(255, 255, 0, 0));

    D3DLOCKED_RECT sysmemLock;
    status = sysmemTexture->LockRect(0, &sysmemLock, nullptr, 0);

    //D3DLOCKED_RECT offscreenLock;
    status = offscreenSurface->LockRect(&offscreenLock, nullptr, 0);

    std::memcpy(sysmemLock.pBits, offscreenLock.pBits, offscreenLock.Pitch * (64 / 4));

    sysmemTexture->UnlockRect(0);
    offscreenSurface->UnlockRect();

    status = m_device->UpdateTexture(sysmemTexture.ptr(), defaultTexture.ptr());

    status = m_device->SetTexture(0, defaultTexture.ptr());

    Com<IDirect3DSurface9> rt;
    status = m_device->CreateRenderTarget(1280, 720, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &rt, nullptr);

    m_device->AddRef();

    Com<IDirect3DSurface9> rt2;
    status = m_device->CreateRenderTarget(1280, 720, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &rt2, nullptr);

    m_device->AddRef();

    rt2 = nullptr;

    m_device->AddRef();

    RECT stretchRect1 = { 0, 0, 640, 720 };
    RECT stretchRect2 = { 640, 0, 1280, 720 };
    status = m_device->StretchRect(rt.ptr(), &stretchRect1, rt.ptr(), &stretchRect2, D3DTEXF_LINEAR);

    /// 

    Com<IDirect3DSurface9> ds;
    //status = m_device->CreateDepthStencilSurface(1274, 695, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, FALSE, &ds, nullptr);
    status = m_device->CreateDepthStencilSurface(1280, 720, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, FALSE, &ds, nullptr);

    status = m_device->SetDepthStencilSurface(ds.ptr());
    status = m_device->SetRenderState(D3DRS_ZWRITEENABLE, 1);
    status = m_device->SetRenderState(D3DRS_ZENABLE, 1);
    status = m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);



    std::array<float, 9> vertices = {
      0.0f, 0.5f, 0.0f,
      0.5f, -0.5f, 0.0f,
      -0.5f, -0.5f, 0.0f,
    };

    const size_t vbSize = vertices.size() * sizeof(float);

    status = m_device->CreateVertexBuffer(vbSize, 0, 0, D3DPOOL_DEFAULT, &m_vb, nullptr);
    if (FAILED(status))
      throw Error("Failed to create vertex buffer");

    void* data = nullptr;
    status = m_vb->Lock(0, 0, &data, 0);
    if (FAILED(status))
      throw Error("Failed to lock vertex buffer");

    std::memcpy(data, vertices.data(), vbSize);

    status = m_vb->Unlock();
    if (FAILED(status))
      throw Error("Failed to unlock vertex buffer");

    m_device->SetStreamSource(0, m_vb.ptr(), 0, 3 * sizeof(float));

    std::array<D3DVERTEXELEMENT9, 2> elements;

    elements[0].Method = 0;
    elements[0].Offset = 0;
    elements[0].Stream = 0;
    elements[0].Type = D3DDECLTYPE_FLOAT3;
    elements[0].Usage = D3DDECLUSAGE_POSITION;
    elements[0].UsageIndex = 0;

    elements[1] = D3DDECL_END();

    HRESULT result = m_device->CreateVertexDeclaration(elements.data(), &m_decl);
    if (FAILED(result))
      throw Error("Failed to create vertex decl");

    m_device->SetVertexDeclaration(m_decl.ptr());

    ///

    Com<IDirect3DTexture9> myRT;
    status = m_device->CreateTexture(512, 256, 1, 0, D3DFMT_DXT1, D3DPOOL_DEFAULT, &myRT, nullptr);
    
    Com<IDirect3DSurface9> myRTSurf;
    myRT->GetSurfaceLevel(0, &myRTSurf);

    Com<IDirect3DTexture9> myCopyThing;
    status = m_device->CreateTexture(512, 256, 1, 0, D3DFMT_DXT1, D3DPOOL_DEFAULT, &myCopyThing, nullptr);

    Com<IDirect3DSurface9> myCopyThingSurf;
    myCopyThing->GetSurfaceLevel(0, &myCopyThingSurf);

    status = m_device->StretchRect(myRTSurf.ptr(), nullptr, myCopyThingSurf.ptr(), nullptr, D3DTEXF_NONE);

    D3DLOCKED_RECT rect;
    status = myCopyThing->LockRect(0, &rect, nullptr, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);

    m_device->SetRenderState(D3DRS_ALPHAREF, 256 + 255);
    m_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL);
    m_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
  }
  
  void run() {
    this->adjustBackBuffer();

    m_device->BeginScene();

    m_device->Clear(
      0,
      nullptr,
      D3DCLEAR_TARGET,
      D3DCOLOR_RGBA(44, 62, 80, 0),
      0,
      0);

    m_device->Clear(
      0,
      nullptr,
      D3DCLEAR_ZBUFFER,
      0,
      0.5f,
      0);

    m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

    m_device->EndScene();

    m_device->PresentEx(
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      0);
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
  
  Com<IDirect3D9Ex>             m_d3d;
  Com<IDirect3DDevice9Ex>       m_device;

  Com<IDirect3DVertexShader9>   m_vs;
  Com<IDirect3DPixelShader9>    m_ps;
  Com<IDirect3DVertexBuffer9>   m_vb;
  Com<IDirect3DVertexDeclaration9> m_decl;
  
};

LRESULT CALLBACK WindowProc(HWND hWnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
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
