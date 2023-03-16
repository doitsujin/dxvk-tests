#include <cstring>
#include <iostream>
#include <functional>

#include <dxgi1_6.h>
#include <d3d11on12.h>
#include <d3dcompiler.h>

#include <windows.h>
#include <windowsx.h>

#include "../common/com.h"
#include "../common/str.h"

const std::string g_vertexShaderCode =
  "float4 main(float2 v_pos : IN_POSITION) : SV_POSITION {\n"
  "  return float4(v_pos, 0.0f, 1.0f);\n"
  "}\n";

const std::string g_pixelShaderCode =
  "float4 main() : SV_TARGET {\n"
  "  return float4(0.25f, 0.25f, 0.25f, 1.0f);\n"
  "}\n";

struct Vertex {
  float x, y;
};

class D3D11On12App {

public:

  D3D11On12App(HINSTANCE hInstance, int nCmdShow) {
    m_initialized =
      createWindow(hInstance, nCmdShow) &&
      createDXGIFactory() &&
      createD3D12Device() &&
      createD3D12CommandList() &&
      createD3D12Resources() &&
      createD3D11On12Device() &&
      createD3D11Resources() &&
      createDXGISwapChain();
  }

  ~D3D11On12App() {
    if (m_d3d12Event)
      CloseHandle(m_d3d12Event);
  }

  bool renderFrame() {
    m_d3d12Allocator->Reset();
    m_d3d12CommandList->Reset(m_d3d12Allocator.ptr(), nullptr);

    // Use D3D11 to render to the shared image
    std::vector<ID3D11Resource*> resources = { m_d3d11RenderTarget.ptr(), m_d3d11VertexBuffer.ptr(), m_d3d11IndexBuffer.ptr() };

    m_d3d11on12Device->AcquireWrappedResources(resources.data(), resources.size());
    m_d3d11Context->ClearState();

    FLOAT color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    m_d3d11Context->OMSetRenderTargets(1, &m_d3d11RenderTargetView, nullptr);
    m_d3d11Context->ClearRenderTargetView(m_d3d11RenderTargetView.ptr(), color);

    m_d3d11Context->VSSetShader(m_d3d11VertexShader.ptr(), nullptr, 0);
    m_d3d11Context->PSSetShader(m_d3d11PixelShader.ptr(), nullptr, 0);

    m_d3d11Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_d3d11Context->IASetInputLayout(m_d3d11InputLayout.ptr());

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = 1280.0f;
    viewport.Height = 720.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_d3d11Context->RSSetViewports(1, &viewport);

    uint32_t vsStride = sizeof(Vertex);
    uint32_t vsOffset = 0;

    m_d3d11Context->IASetVertexBuffers(0, 1, &m_d3d11VertexBuffer, &vsStride, &vsOffset);
    m_d3d11Context->IASetIndexBuffer(m_d3d11IndexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);
    m_d3d11Context->DrawIndexed(3, 0, 0);

    m_d3d11on12Device->ReleaseWrappedResources(resources.data(), resources.size());
    m_d3d11Context->Flush();

    // Copy the shared image to the D3D12 swap chain
    Com<ID3D12Resource> backBuffer;

    if (FAILED(m_dxgiSwapChain->GetBuffer(m_dxgiSwapChain->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&backBuffer)))) {
      std::cerr << "Failed to acquire back buffer" << std::endl;
      return false;
    }

    D3D12_RESOURCE_BARRIER preCopyBarrier = { };
    preCopyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    preCopyBarrier.Transition.pResource = backBuffer.ptr();
    preCopyBarrier.Transition.Subresource = 0;
    preCopyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    preCopyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

    m_d3d12CommandList->ResourceBarrier(1, &preCopyBarrier);

    D3D12_TEXTURE_COPY_LOCATION dstRegion = { };
    dstRegion.pResource = backBuffer.ptr();
    dstRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstRegion.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION srcRegion = { };
    srcRegion.pResource = m_d3d12RenderTarget.ptr();
    srcRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcRegion.SubresourceIndex = 0;

    m_d3d12CommandList->CopyTextureRegion(&dstRegion, 0, 0, 0, &srcRegion, nullptr);

    D3D12_RESOURCE_BARRIER postCopyBarrier = { };
    postCopyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postCopyBarrier.Transition.pResource = m_d3d12RenderTarget.ptr();
    postCopyBarrier.Transition.Subresource = 0;
    postCopyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    postCopyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    D3D12_RESOURCE_BARRIER prePresentBarrier = { };
    prePresentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    prePresentBarrier.Transition.pResource = backBuffer.ptr();
    prePresentBarrier.Transition.Subresource = 0;
    prePresentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    prePresentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    m_d3d12CommandList->ResourceBarrier(1, &postCopyBarrier);
    m_d3d12CommandList->ResourceBarrier(1, &prePresentBarrier);
    m_d3d12CommandList->Close();

    ID3D12CommandList* cmdList = m_d3d12CommandList.ptr();
    m_d3d12Queue->ExecuteCommandLists(1, &cmdList);

    m_dxgiSwapChain->Present(1, 0);

    // We're lazy with synchronization, just stall at the end of each frame
    m_d3d12Queue->Signal(m_d3d12Fence.ptr(), ++m_frameId);
    m_d3d12Fence->SetEventOnCompletion(m_frameId, m_d3d12Event);

    while (WaitForSingleObject(m_d3d12Event, INFINITE))
      continue;

    return true;
  }

  int run() {
    if (!m_initialized)
      return 1;

    MSG msg;

    while (true) {
      if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        
        if (msg.message == WM_QUIT)
          return msg.wParam;
      } else {
        if (!renderFrame())
          break;
      }
    }

    return msg.wParam;
  }


private:

  HWND                            m_window = nullptr;

  Com<IDXGIFactory6>              m_dxgiFactory;
  Com<IDXGIAdapter4>              m_dxgiAdapter;
  Com<IDXGISwapChain4>            m_dxgiSwapChain;

  Com<ID3D11Device>               m_d3d11Device;
  Com<ID3D11DeviceContext>        m_d3d11Context;
  Com<ID3D11On12Device>           m_d3d11on12Device;
  Com<ID3D11Buffer>               m_d3d11VertexBuffer;
  Com<ID3D11Buffer>               m_d3d11IndexBuffer;
  Com<ID3D11Texture2D>            m_d3d11RenderTarget;
  Com<ID3D11RenderTargetView>     m_d3d11RenderTargetView;
  Com<ID3D11VertexShader>         m_d3d11VertexShader;
  Com<ID3D11PixelShader>          m_d3d11PixelShader;
  Com<ID3D11InputLayout>          m_d3d11InputLayout;

  Com<ID3D12Device>               m_d3d12Device;
  Com<ID3D12CommandQueue>         m_d3d12Queue;
  Com<ID3D12CommandAllocator>     m_d3d12Allocator;
  Com<ID3D12GraphicsCommandList>  m_d3d12CommandList;
  Com<ID3D12Resource>             m_d3d12RenderTarget;
  Com<ID3D12Heap>                 m_d3d12Heap;
  Com<ID3D12Resource>             m_d3d12VertexBuffer;
  Com<ID3D12Resource>             m_d3d12IndexBuffer;
  Com<ID3D12Fence>                m_d3d12Fence;

  HANDLE                          m_d3d12Event = nullptr;

  UINT64                          m_frameId = 0;

  bool m_initialized = false;

  bool createWindow(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEXW wc = { };
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = windowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = HBRUSH(COLOR_WINDOW);
    wc.lpszClassName = L"WindowClass";
    RegisterClassExW(&wc);

    m_window = CreateWindowExW(0, L"WindowClass", L"D3D11on12 triangle",
      WS_OVERLAPPEDWINDOW, 300, 300, 1280, 720,
      nullptr, nullptr, hInstance, nullptr);
    ShowWindow(m_window, nCmdShow);

    return true;
  }

  bool createDXGIFactory() {
    HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgiFactory));

    if (FAILED(hr)) {
      std::cerr << "Failed to create DXGI factory" << std::endl;
      return false;
    }

    hr = m_dxgiFactory->EnumAdapterByGpuPreference(0,
      DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_dxgiAdapter));

    if (FAILED(hr)) {
      std::cerr << "Failed to query DXGI adapter" << std::endl;
      return false;
    }

    return true;
  }

  bool createD3D12Device() {
    HRESULT hr = D3D12CreateDevice(m_dxgiAdapter.ptr(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&m_d3d12Device));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D12 device" << std::endl;
      return false;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = { };
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    hr = m_d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_d3d12Queue));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D12 command queue" << std::endl;
      return false;
    }

    return true;
  }

  bool createD3D12CommandList() {
    HRESULT hr = m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_d3d12Allocator));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D12 command allocator" << std::endl;
      return false;
    }

    hr = m_d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
      m_d3d12Allocator.ptr(), nullptr, IID_PPV_ARGS(&m_d3d12CommandList));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D12 command list" << std::endl;
      return false;
    }

    hr = m_d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D12 fence" << std::endl;
      return false;
    }

    m_d3d12Event = CreateEventW(nullptr, FALSE, FALSE, nullptr);

    if (FAILED(!m_d3d12Event)) {
      std::cerr << "Failed to create fence event" << std::endl;
      return false;
    }

    return true;
  }

  bool createD3D12Resources() {
    D3D12_HEAP_PROPERTIES deviceHeapProperties = { };
    deviceHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC rtDesc = { };
    rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rtDesc.Width = 1280;
    rtDesc.Height = 720;
    rtDesc.DepthOrArraySize = 1;
    rtDesc.MipLevels = 1;
    rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtDesc.SampleDesc = { 1, 0 };
    rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    HRESULT hr = m_d3d12Device->CreateCommittedResource(&deviceHeapProperties, D3D12_HEAP_FLAG_NONE, &rtDesc,
      D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&m_d3d12RenderTarget));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D12 render target" << std::endl;
      return false;
    }

    D3D12_HEAP_DESC heapDesc = { };
    heapDesc.SizeInBytes = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT * 2;
    heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

    hr = m_d3d12Device->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_d3d12Heap));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D12 heap" << std::endl;
      return false;
    }

    D3D12_RESOURCE_DESC bufferDesc = { };
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.SampleDesc = { 1, 0 };
    bufferDesc.MipLevels = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    hr = m_d3d12Device->CreatePlacedResource(m_d3d12Heap.ptr(), 0, &bufferDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_d3d12VertexBuffer));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D12 vertex buffer" << std::endl;
      return false;
    }

    hr = m_d3d12Device->CreatePlacedResource(m_d3d12Heap.ptr(), D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, &bufferDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_d3d12IndexBuffer));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D12 index buffer" << std::endl;
      return false;
    }

    return true;
  }

  bool createD3D11On12Device() {
    Com<IUnknown> commandQueue = m_d3d12Queue.ptr();

    HRESULT hr = D3D11On12CreateDevice(
      m_d3d12Device.ptr(), 0, nullptr, 0,
      &commandQueue, 1, 0, &m_d3d11Device, &m_d3d11Context, nullptr);

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D11 device" << std::endl;
      return false;
    }

    hr = m_d3d11Device->QueryInterface(IID_PPV_ARGS(&m_d3d11on12Device));

    if (FAILED(hr)) {
      std::cerr << "Failed to retrieve ID3D11On12Device interface" << std::endl;
      return false;
    }

    return true;
  }

  bool createD3D11Resources() {
    D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };

    HRESULT hr = m_d3d11on12Device->CreateWrappedResource(
      m_d3d12RenderTarget.ptr(), &d3d11Flags,
      D3D12_RESOURCE_STATE_RENDER_TARGET,
      D3D12_RESOURCE_STATE_COPY_SOURCE,
      IID_PPV_ARGS(&m_d3d11RenderTarget));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D11 render target" << std::endl;
      return false;
    }

    hr = m_d3d11Device->CreateRenderTargetView(m_d3d11RenderTarget.ptr(), nullptr, &m_d3d11RenderTargetView);

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D11 render target view" << std::endl;
      return false;
    }

    d3d11Flags = { D3D11_BIND_VERTEX_BUFFER };

    hr = m_d3d11on12Device->CreateWrappedResource(
      m_d3d12VertexBuffer.ptr(), &d3d11Flags,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      IID_PPV_ARGS(&m_d3d11VertexBuffer));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D11 vertex buffer" << std::endl;
      return false;
    }

    d3d11Flags = { D3D11_BIND_INDEX_BUFFER };

    hr = m_d3d11on12Device->CreateWrappedResource(
      m_d3d12IndexBuffer.ptr(), &d3d11Flags,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      IID_PPV_ARGS(&m_d3d11IndexBuffer));

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D11 vertex buffer" << std::endl;
      return false;
    }

    Com<ID3DBlob> vertexShaderBlob;
    Com<ID3DBlob> pixelShaderBlob;
    
    if (FAILED(D3DCompile(g_vertexShaderCode.data(), g_vertexShaderCode.size(),
        "Vertex shader", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderBlob, nullptr))) {
      std::cerr << "Failed to compile vertex shader" << std::endl;
      return false;
    }
    
    if (FAILED(D3DCompile(g_pixelShaderCode.data(), g_pixelShaderCode.size(),
        "Pixel shader", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShaderBlob, nullptr))) {
      std::cerr << "Failed to compile pixel shader" << std::endl;
      return false;
    }
    
    if (FAILED(m_d3d11Device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr, &m_d3d11VertexShader))) {
      std::cerr << "Failed to create vertex shader" << std::endl;
      return false;
    }
    
    if (FAILED(m_d3d11Device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr, &m_d3d11PixelShader))) {
      std::cerr << "Failed to create pixel shader" << std::endl;
      return false;
    }

    std::array<D3D11_INPUT_ELEMENT_DESC, 1> vertexFormatDesc = {{
      { "IN_POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    }};
    
    if (FAILED(m_d3d11Device->CreateInputLayout(
        vertexFormatDesc.data(),
        vertexFormatDesc.size(),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        &m_d3d11InputLayout))) {
      std::cerr << "Failed to create input layout" << std::endl;
      return false;
    }

    std::array<uint32_t, 3> indexData = {{ 0, 1, 2 }};
    std::array<Vertex, 3> vertexData = {{
      Vertex { -0.3f, -0.3f },
      Vertex {  0.0f,  0.3f },
      Vertex {  0.3f, -0.3f },
    }};

    D3D11_BOX vboBox = { 0, 0, 0, uint32_t(sizeof(vertexData)), 1, 1 };
    D3D11_BOX iboBox = { 0, 0, 0, uint32_t(sizeof(indexData)), 1, 1 };

    std::vector<ID3D11Resource*> resources = { m_d3d11VertexBuffer.ptr(), m_d3d11IndexBuffer.ptr() };

    m_d3d11on12Device->AcquireWrappedResources(resources.data(), resources.size());

    m_d3d11Context->UpdateSubresource(m_d3d11VertexBuffer.ptr(), 0, &vboBox, vertexData.data(), 0, 0);
    m_d3d11Context->UpdateSubresource(m_d3d11IndexBuffer.ptr(), 0, &iboBox, indexData.data(), 0, 0);

    m_d3d11on12Device->ReleaseWrappedResources(resources.data(), resources.size());
    m_d3d11Context->Flush();
    return true;
  }

  bool createDXGISwapChain() {
    Com<IDXGISwapChain1> swapChain;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
    swapChainDesc.Width = 1280;
    swapChainDesc.Height = 720;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 3;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

    HRESULT hr = m_dxgiFactory->CreateSwapChainForHwnd(m_d3d12Queue.ptr(),
      m_window, &swapChainDesc, nullptr, nullptr, &swapChain);

    if (FAILED(hr)) {
      std::cerr << "Failed to create D3D12 swap chain" << std::endl;
      return false;
    }

    if (FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&m_dxgiSwapChain)))) {
      std::cerr << "Failed to query swap chain interface" << std::endl;
      return false;
    }

    return true;
  }

  static LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
      case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
  }

};

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
  D3D11On12App app(hInstance, nCmdShow);
  return app.run();
}
