#include <cstring>
#include <iostream>
#include <functional>
#include <array>

#include <d3dcompiler.h>
#include <d3d11_2.h>

#include <windows.h>
#include <windowsx.h>

#include "../common/com.h"
#include "../common/str.h"

class TiledResourceTestApp {

public:

  TiledResourceTestApp() {
    Com<ID3D11Device> device;

    std::vector<D3D_FEATURE_LEVEL> fl = {
      D3D_FEATURE_LEVEL_12_1,
      D3D_FEATURE_LEVEL_12_0,
    };

    if (FAILED(D3D11CreateDevice(
          nullptr, D3D_DRIVER_TYPE_HARDWARE,
          nullptr, 0, fl.data(), fl.size(), D3D11_SDK_VERSION,
          &device, nullptr, nullptr))) {
      std::cout << "Failed to create D3D11 device" << std::endl;
      return;
    }

    if (FAILED(device->QueryInterface(IID_PPV_ARGS(&m_device)))) {
      std::cout << "Failed to query ID3D11Device2" << std::endl;
      return;
    }

    m_device->GetImmediateContext2(&m_context);

    D3D11_FEATURE_DATA_D3D11_OPTIONS1 options1;
    m_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS1, &options1, sizeof(options1));
    m_tier = options1.TiledResourcesTier;

    std::cout << "Device supports tiled resources tier " << m_tier << std::endl;
    m_initialized = true;
  }

  int run() {
    if (!m_initialized)
      return 1;

    testCreateTilePool();
    testCreateTiledBuffer();
    testCreateTiledImage();
    testCreateTiledImage3D();
    testCreateMinMaxSampler();
    testGetResourceTiling();
    testMapBufferTiles();
    testMapImageTiles();
    return 0;
  }

  void testCreateTilePool() {
    Com<ID3D11Buffer> tilePool;

    D3D11_BUFFER_DESC tilePoolDesc = { };
    tilePoolDesc.ByteWidth  = 0;
    tilePoolDesc.Usage      = D3D11_USAGE_DEFAULT;
    tilePoolDesc.MiscFlags  = D3D11_RESOURCE_MISC_TILE_POOL;

    HRESULT hr = m_device->CreateBuffer(&tilePoolDesc, nullptr, &tilePool);
    std::cout << "Creating tile pool with size 0: hr = 0x" << std::hex << hr << std::endl;

    tilePool = nullptr;
    tilePoolDesc.ByteWidth  = 100 << 10;
    hr = m_device->CreateBuffer(&tilePoolDesc, nullptr, &tilePool);
    std::cout << "Creating tile pool with size 100k: hr = 0x" << std::hex << hr << std::endl;

    tilePool = nullptr;
    tilePoolDesc.ByteWidth  = 4u << 20;
    hr = m_device->CreateBuffer(&tilePoolDesc, nullptr, &tilePool);
    std::cout << "Creating tile pool with size 4M: hr = 0x" << std::hex << hr << std::endl;

    tilePool = nullptr;
    tilePoolDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = m_device->CreateBuffer(&tilePoolDesc, nullptr, &tilePool);
    std::cout << "Creating tile pool with CPU access: hr = 0x" << std::hex << hr << std::endl;

    tilePool = nullptr;
    tilePoolDesc.CPUAccessFlags = 0;
    tilePoolDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    hr = m_device->CreateBuffer(&tilePoolDesc, nullptr, &tilePool);
    std::cout << "Creating tile pool with bind flag: hr = 0x" << std::hex << hr << std::endl;
  }

  void testCreateTiledBuffer() {
    Com<ID3D11Buffer> tiledBuffer;

    D3D11_BUFFER_DESC bufferDesc = { };
    bufferDesc.ByteWidth  = 128u << 20;
    bufferDesc.Usage      = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags  = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags  = D3D11_RESOURCE_MISC_TILED;

    HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &tiledBuffer);
    std::cout << "Creating tiled buffer: hr = 0x" << std::hex << hr << std::endl;

    tiledBuffer = nullptr;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = m_device->CreateBuffer(&bufferDesc, nullptr, &tiledBuffer);
    std::cout << "Creating tiled buffer with CPU access: hr = 0x" << std::hex << hr << std::endl;
  }

  void testCreateTiledImage() {
    Com<ID3D11Texture2D> tiledImage;

    D3D11_TEXTURE2D_DESC texDesc = { };
    texDesc.Width      = 4096;
    texDesc.Height     = 4096;
    texDesc.MipLevels  = 0;
    texDesc.ArraySize  = 1;
    texDesc.SampleDesc = { 1, 0 };
    texDesc.Format     = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.Usage      = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags  = D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags  = D3D11_RESOURCE_MISC_TILED;

    HRESULT hr = m_device->CreateTexture2D(&texDesc, nullptr, &tiledImage);
    std::cout << "Creating tiled 2D texture (4096x4096, 0, 1): hr = 0x" << std::hex << hr << std::endl;

    texDesc.MipLevels  = 1;
    texDesc.ArraySize  = 10;

    tiledImage = nullptr;
    hr = m_device->CreateTexture2D(&texDesc, nullptr, &tiledImage);
    std::cout << "Creating tiled 2D texture (4096x4096, 1, 10): hr = 0x" << std::hex << hr << std::endl;

    texDesc.MipLevels  = 0;
    texDesc.ArraySize  = 10;

    tiledImage = nullptr;
    hr = m_device->CreateTexture2D(&texDesc, nullptr, &tiledImage);
    std::cout << "Creating tiled 2D texture (4096x4096, 0, 10): hr = 0x" << std::hex << hr << std::endl;
  }

  void testCreateTiledImage3D() {
    Com<ID3D11Texture3D> tiledImage;

    D3D11_TEXTURE3D_DESC texDesc = { };
    texDesc.Width      = 512;
    texDesc.Height     = 512;
    texDesc.Depth      = 512;
    texDesc.MipLevels  = 0;
    texDesc.Format     = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.Usage      = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags  = D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags  = D3D11_RESOURCE_MISC_TILED;

    HRESULT hr = m_device->CreateTexture3D(&texDesc, nullptr, &tiledImage);
    std::cout << "Creating tiled 3D texture (512x512x512, 0): hr = 0x" << std::hex << hr << std::endl;
  }

  void testCreateMinMaxSampler() {
    Com<ID3D11SamplerState> sampler;

    D3D11_SAMPLER_DESC desc = { };
    desc.Filter = D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MaxLOD   = 16.0f;

    HRESULT hr = m_device->CreateSamplerState(&desc, &sampler);
    std::cout << "Creating sampler with MINIMUM_MIN_MAG_MIPMAP_LINEAR: hr = 0x" << std::hex << hr << std::endl;

    sampler = nullptr;
    desc.Filter = D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;

    hr = m_device->CreateSamplerState(&desc, &sampler);
    std::cout << "Creating sampler with MAXIMUM_MIN_MAG_MIPMAP_LINEAR: hr = 0x" << std::hex << hr << std::endl;
  }

  void testGetResourceTiling() {
    struct TestEntry {
      DXGI_FORMAT Format;
      UINT Width;
      UINT Height;
      UINT Depth;
      BOOL Is3D;
      UINT MipLevels;
      UINT ArraySize;
    };

    std::vector<TestEntry> tests = {{
      { DXGI_FORMAT_UNKNOWN,          1048576,      1,      1, FALSE,    1,   1 },
      { DXGI_FORMAT_UNKNOWN,          1000000,      0,      0, FALSE,    1,   1 },
      { DXGI_FORMAT_UNKNOWN,             1024,      0,      0, FALSE,    1,   1 },
      { DXGI_FORMAT_R8G8B8A8_UNORM,       512,    512,      1, FALSE,    1,   1 },
      { DXGI_FORMAT_R8G8B8A8_UNORM,       512,    512,      1, FALSE,    1,  10 },
      { DXGI_FORMAT_R8G8B8A8_UNORM,       512,    512,      1, FALSE,   10,   1 },
      { DXGI_FORMAT_R8G8B8A8_UNORM,       512,    512,      1, FALSE,   10,  10 },
      { DXGI_FORMAT_R8G8B8A8_UNORM,      3711,   1417,      1, FALSE,   12,   1 },
      { DXGI_FORMAT_R16G16B16A16_FLOAT,  1024,   1024,      1, FALSE,    4,   1 },
      { DXGI_FORMAT_BC1_UNORM,           1024,   1024,      1, FALSE,   11,   1 },
      { DXGI_FORMAT_BC7_UNORM,           1024,   1024,      1, FALSE,   11,   1 },
      { DXGI_FORMAT_R8G8B8A8_UNORM,       512,    512,    512, TRUE,    10,   1 },
      { DXGI_FORMAT_R32G32B32A32_FLOAT,    73,     38,     24, TRUE,     5,   1 },
    }};

    uint32_t testCounter = 0;

    for (const auto& test : tests) {
      uint32_t testIndex = testCounter++;
      Com<ID3D11Resource> resource;

      if (!test.Format) {
        D3D11_BUFFER_DESC desc = { };
        desc.ByteWidth = test.Width;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_TILED;

        Com<ID3D11Buffer> buffer;

        if (FAILED(m_device->CreateBuffer(&desc, nullptr, &buffer))) {
          std::cout << "GetResourceTiling: Test " << testIndex << ": Failed to create buffer" << std::endl;
          continue;
        }

        buffer->QueryInterface(IID_PPV_ARGS(&resource));
      } else if (test.Is3D) {
        if (m_tier < D3D11_TILED_RESOURCES_TIER_3)
          continue;

        D3D11_TEXTURE3D_DESC desc3D = { };
        desc3D.Width = test.Width;
        desc3D.Height = test.Height;
        desc3D.Depth = test.Depth;
        desc3D.MipLevels = test.MipLevels;
        desc3D.Format = test.Format;
        desc3D.Usage = D3D11_USAGE_DEFAULT;
        desc3D.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc3D.MiscFlags = D3D11_RESOURCE_MISC_TILED;

        Com<ID3D11Texture3D> tex3D;

        if (FAILED(m_device->CreateTexture3D(&desc3D, nullptr, &tex3D))) {
          std::cout << "GetResourceTiling: Test " << testIndex << ": Failed to create 3D texture" << std::endl;
          continue;
        }

        tex3D->QueryInterface(IID_PPV_ARGS(&resource));
      } else {
        D3D11_TEXTURE2D_DESC desc2D = { };
        desc2D.Width = test.Width;
        desc2D.Height = test.Height;
        desc2D.MipLevels = test.MipLevels;
        desc2D.ArraySize = test.ArraySize;
        desc2D.SampleDesc = { 1, 0 };
        desc2D.Format = test.Format;
        desc2D.Usage = D3D11_USAGE_DEFAULT;
        desc2D.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc2D.MiscFlags = D3D11_RESOURCE_MISC_TILED;

        Com<ID3D11Texture2D> tex2D;

        if (FAILED(m_device->CreateTexture2D(&desc2D, nullptr, &tex2D))) {
          std::cout << "GetResourceTiling: Test " << testIndex << ": Failed to create 2D texture" << std::endl;
          continue;
        }

        tex2D->QueryInterface(IID_PPV_ARGS(&resource));
      }

      uint32_t subresourceCount = test.MipLevels * test.ArraySize;
      std::vector<D3D11_SUBRESOURCE_TILING> tilings(subresourceCount);

      uint32_t tileCount = 0;
      D3D11_PACKED_MIP_DESC packedInfo = { };
      D3D11_TILE_SHAPE tileShape = { };

      m_device->GetResourceTiling(resource.ptr(),
        &tileCount, &packedInfo, &tileShape,
        &subresourceCount, 0, tilings.data());

      std::cout << "GetResourceTiling: Test " << std::dec << testIndex << ": Tile count = " << tileCount
                << ", Subresources = " << subresourceCount
                << ", Tile shape = " << tileShape.WidthInTexels << "x" << tileShape.HeightInTexels << "x" << tileShape.DepthInTexels
                << ", Standard mips = " << uint32_t(packedInfo.NumStandardMips)
                << ", Packed mips = " << uint32_t(packedInfo.NumPackedMips) << " ("
                << packedInfo.StartTileIndexInOverallResource << ","
                << packedInfo.StartTileIndexInOverallResource + packedInfo.NumTilesForPackedMips << ")" << std::endl;
    }
  }

  void testMapBufferTiles() {
    Com<ID3D11Buffer> tilePool;
    Com<ID3D11Buffer> buffer1;
    Com<ID3D11Buffer> buffer2;
    Com<ID3D11Buffer> readback;

    D3D11_BUFFER_DESC tilePoolDesc = { };
    tilePoolDesc.ByteWidth = 64 << 16;
    tilePoolDesc.Usage = D3D11_USAGE_DEFAULT;
    tilePoolDesc.MiscFlags = D3D11_RESOURCE_MISC_TILE_POOL;

    if (FAILED(m_device->CreateBuffer(&tilePoolDesc, nullptr, &tilePool))) {
      std::cout << "Failed to create tile pool" << std::endl;
      return;
    }

    D3D11_BUFFER_DESC bufferDesc = { };
    bufferDesc.ByteWidth = 64 << 16;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_TILED;

    if (FAILED(m_device->CreateBuffer(&bufferDesc, nullptr, &buffer1))
     || FAILED(m_device->CreateBuffer(&bufferDesc, nullptr, &buffer2))) {
      std::cout << "Failed to create tiled buffer" << std::endl;
      return;
    }

    D3D11_BUFFER_DESC readbackDesc = { };
    readbackDesc.ByteWidth = 256;
    readbackDesc.Usage = D3D11_USAGE_STAGING;
    readbackDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    if (FAILED(m_device->CreateBuffer(&readbackDesc, nullptr, &readback))) {
      std::cout << "Failed to create readback buffer" << std::endl;
      return;
    }

    // Map entire buffer to whole tile pool first
    D3D11_TILED_RESOURCE_COORDINATE regionCoord = { 0, 0, 0, 0 };
    D3D11_TILE_REGION_SIZE regionSize = { 64, FALSE };

    UINT rangeFlags = 0;
    UINT rangeOffset = 0;
    UINT rangeSize = 64;

    std::cout << "Test: Map entrie buffer to linear region" << std::endl;

    HRESULT hr = m_context->UpdateTileMappings(buffer1.ptr(),
      1, &regionCoord, &regionSize, tilePool.ptr(),
      1, &rangeFlags, &rangeOffset, &rangeSize, 0);

    if (FAILED(hr)) {
      std::cout << "UpdateTileMappings failed: 0x " << std::hex << hr << std::endl;
      return;
    }

    // Initialize buffer with tile data. We'll count tiles from 1.
    for (uint32_t i = 0; i < 64; i++) {
      Com<ID3D11UnorderedAccessView> uav;

      D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
      uavDesc.Format = DXGI_FORMAT_R32_UINT;
      uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
      uavDesc.Buffer.FirstElement = 16384 * i;
      uavDesc.Buffer.NumElements = 16384;

      if (FAILED(m_device->CreateUnorderedAccessView(buffer1.ptr(), &uavDesc, &uav)))
        std::cout << "Failed to create buffer UAV for clears" << std::endl;

      const std::array<uint32_t, 4> color = { i + 1 }; 
      m_context->ClearUnorderedAccessViewUint(uav.ptr(), color.data());
    }

    validateBufferTileData(buffer1.ptr(), readback.ptr(), 64,
      [] (uint32_t tile) { return tile + 1; });

    // Test complex UpdateResourceTiling on the other buffer
    std::cout << "Test: Complex UpdateTileMappings on buffer" << std::endl;

    std::array<D3D11_TILED_RESOURCE_COORDINATE, 7> regionCoordArray = {{
      { 0 }, { 14 }, { 5 }, { 28 }, { 63 }, { 48 }, { 52 },
    }};

    std::array<D3D11_TILE_REGION_SIZE, 7> regionSizeArray = {{
      { 5 }, { 2 }, { 1 }, { 10 }, { 1 }, { 1 }, { 7 },
    }};

    std::array<uint32_t, 10> rangeFlagsArray = {{
      0, 0, 0,
      D3D11_TILE_RANGE_SKIP,
      D3D11_TILE_RANGE_REUSE_SINGLE_TILE,
      0, 0, 0,
      D3D11_TILE_RANGE_REUSE_SINGLE_TILE,
      0,
    }};

    std::array<uint32_t, 10> rangeOffsetArray = {{ 5,  7, 18,  0, 63, 44, 26, 11, 18, 31 }};
    std::array<uint32_t, 10> rangeSizeArray   = {{ 1,  1,  8,  1,  2,  1,  3,  1,  3,  6 }};

    hr = m_context->UpdateTileMappings(buffer2.ptr(),
      7, regionCoordArray.data(), regionSizeArray.data(), tilePool.ptr(),
      10, rangeFlagsArray.data(), rangeOffsetArray.data(), rangeSizeArray.data(), 0);

    if (FAILED(hr)) {
      std::cout << "UpdateTileMappings failed: 0x " << std::hex << hr << std::endl;
      return;
    }

    std::array<uint32_t, 64> expected = {{
      6,  8, 19, 20, 21, 24,  0,  0,
      0,  0,  0,  0,  0,  0, 22, 23,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0, 25, 26,  0, 64,
     64, 45, 27, 28, 29, 12,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
     19,  0,  0,  0, 19, 32, 33, 34,
     35, 36, 37,  0,  0,  0,  0, 19,
    }};

    validateBufferTileData(buffer2.ptr(), readback.ptr(), 64,
      [&expected] (uint32_t tile) { return expected[tile]; });

    // Test unmapping first buffer
    std::cout << "Test: Unmap all tiles" << std::endl;

    rangeFlags = D3D11_TILE_RANGE_NULL;
    hr = m_context->UpdateTileMappings(buffer1.ptr(),
      1, &regionCoord, &regionSize, tilePool.ptr(),
      1, &rangeFlags, &rangeOffset, &rangeSize, 0);

    if (FAILED(hr)) {
      std::cout << "UpdateTileMappings failed: 0x " << std::hex << hr << std::endl;
      return;
    }

    validateBufferTileData(buffer1.ptr(), readback.ptr(), 64,
      [&expected] (uint32_t tile) { return 0; });

    // Test CopyTileMappings by copying some tiles from the second buffer to
    // the first, but with some offsets applied to keep things interesting
    std::cout << "Test: CopyTileMappings on buffer" << std::endl;

    D3D11_TILED_RESOURCE_COORDINATE dstCopyCoord = { 1 };
    D3D11_TILED_RESOURCE_COORDINATE srcCopyCoord = { 2 };
    D3D11_TILE_REGION_SIZE copyRegionSize = { 60 };

    m_context->CopyTileMappings(
      buffer1.ptr(), &dstCopyCoord,
      buffer2.ptr(), &srcCopyCoord,
      &copyRegionSize, 0);

    validateBufferTileData(buffer1.ptr(), readback.ptr(), 64,
      [&expected] (uint32_t tile) {
        if (tile >= 1 && tile < 61)
          return expected[tile + 1];
        else
          return 0u;
      });
  }

  void testMapImageTiles() {
    Com<ID3D11Buffer> tilePool;
    Com<ID3D11Texture2D> texture1;
    Com<ID3D11Texture2D> texture2;
    Com<ID3D11Texture2D> readback;

    D3D11_TEXTURE2D_DESC texDesc = { };
    texDesc.Width      = 150;
    texDesc.Height     = 150;
    texDesc.MipLevels  = 1;
    texDesc.ArraySize  = 1;
    texDesc.SampleDesc = { 1, 0 };
    texDesc.Format     = DXGI_FORMAT_R32_UINT;
    texDesc.Usage      = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags  = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    texDesc.MiscFlags  = D3D11_RESOURCE_MISC_TILED;

    if (FAILED(m_device->CreateTexture2D(&texDesc, nullptr, &texture1))
     || FAILED(m_device->CreateTexture2D(&texDesc, nullptr, &texture2))) {
      std::cout << "Failed to create tiled 2D texture" << std::endl;
      return;
    }

    D3D11_TEXTURE2D_DESC readbackDesc = { };
    readbackDesc.Width      = 150;
    readbackDesc.Height     = 150;
    readbackDesc.MipLevels  = 1;
    readbackDesc.ArraySize  = 1;
    readbackDesc.SampleDesc = { 1, 0 };
    readbackDesc.Format     = DXGI_FORMAT_R32_UINT;
    readbackDesc.Usage      = D3D11_USAGE_STAGING;
    readbackDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    if (FAILED(m_device->CreateTexture2D(&readbackDesc, nullptr, &readback))) {
      std::cout << "Failed to create tiled 2D readback texture" << std::endl;
      return;
    }

    D3D11_PACKED_MIP_DESC packedInfo = { };
    D3D11_TILE_SHAPE tileShape = { };
    D3D11_SUBRESOURCE_TILING tiling = { };
    uint32_t tilingCount = 1;
    uint32_t tileCount = 0;

    m_device->GetResourceTiling(texture1.ptr(),
      &tileCount, &packedInfo, &tileShape,
      &tilingCount, 0, &tiling);

    D3D11_BUFFER_DESC tilePoolDesc = { };
    tilePoolDesc.ByteWidth = tileCount << 16;
    tilePoolDesc.Usage = D3D11_USAGE_DEFAULT;
    tilePoolDesc.MiscFlags = D3D11_RESOURCE_MISC_TILE_POOL;

    if (FAILED(m_device->CreateBuffer(&tilePoolDesc, nullptr, &tilePool))) {
      std::cout << "Failed to create tile pool" << std::endl;
      return;
    }

    // Map a region of the image
    D3D11_TILED_RESOURCE_COORDINATE regionCoord = { 0, 0, 0, 0 };
    D3D11_TILE_REGION_SIZE regionSize = { 4, TRUE, 2, 2, 1 };

    UINT rangeFlags = 0;
    UINT rangeOffset = 0;

    std::cout << "Test: Map image region" << std::endl;

    HRESULT hr = m_context->UpdateTileMappings(texture1.ptr(),
      1, &regionCoord, &regionSize, tilePool.ptr(),
      1, &rangeFlags, &rangeOffset, nullptr, 0);

    if (FAILED(hr)) {
      std::cout << "UpdateTileMappings failed: 0x" << std::hex << hr << std::endl;
      return;
    }

    Com<ID3D11UnorderedAccessView> uav;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
    uavDesc.Format = DXGI_FORMAT_R32_UINT;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

    if (FAILED(m_device->CreateUnorderedAccessView(texture1.ptr(), &uavDesc, &uav)))
      std::cout << "Failed to create image UAV for clears" << std::endl;

    const std::array<uint32_t, 4> color = { 1 }; 
    m_context->ClearUnorderedAccessViewUint(uav.ptr(), color.data());

    m_context->CopySubresourceRegion(
      readback.ptr(), 0, 0, 0, 0,
      texture1.ptr(), 0, nullptr);

    D3D11_MAPPED_SUBRESOURCE mapped = { };
    m_context->Map(readback.ptr(), 0, D3D11_MAP_READ, 0, &mapped);

    auto data = reinterpret_cast<const uint32_t*>(mapped.pData);

    for (uint32_t y = 0; y < tiling.HeightInTiles; y++) {
      for (uint32_t x = 0; x < tiling.HeightInTiles; x++) {
        uint32_t xCoord = x * tileShape.WidthInTexels;
        uint32_t yCoord = y * tileShape.HeightInTexels;

        uint32_t byteOffset = xCoord + yCoord * mapped.RowPitch / 4;
        std::cout << data[byteOffset] << " ";
      }
      std::cout << std::endl;
    }

    m_context->Unmap(readback.ptr(), 0);

    std::cout << "Test: CopyTiles" << std::endl;

    D3D11_BUFFER_DESC stagingDesc = { };
    stagingDesc.ByteWidth = 65536;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

    Com<ID3D11Buffer> stagingBuffer;

    if (FAILED(m_device->CreateBuffer(&stagingDesc, nullptr, &stagingBuffer))) {
      std::cout << "Failed to create staging buffer" << std::endl;
      return;
    }

    m_context->Map(stagingBuffer.ptr(), 0, D3D11_MAP_WRITE, 0, &mapped);
    std::memset(mapped.pData, 0xFF, 65536);
    m_context->Unmap(stagingBuffer.ptr(), 0);

    D3D11_TILED_RESOURCE_COORDINATE rcoord = { 1, 1 };
    D3D11_TILE_REGION_SIZE rsize = { 1 };

    m_context->CopyTiles(texture1.ptr(), &rcoord, &rsize, stagingBuffer.ptr(), 0,
      D3D11_TILE_COPY_SWIZZLED_TILED_RESOURCE_TO_LINEAR_BUFFER);
  }

private:

  Com<ID3D11Device2>          m_device;
  Com<ID3D11DeviceContext2>   m_context;

  D3D11_TILED_RESOURCES_TIER  m_tier;

  bool m_initialized = false;

  void validateBufferTileData(
          ID3D11Buffer*       tiledBuffer,
          ID3D11Buffer*       readbackBuffer,
          uint32_t            tileCount,
    const std::function<uint32_t (uint32_t)>& proc) {
    for (uint32_t i = 0; i < tileCount; i++) {
      D3D11_BOX box = { (i << 16), 0, 0, (i << 16) + uint32_t(sizeof(uint32_t)), 1, 1 };
      m_context->CopySubresourceRegion(readbackBuffer, 0,
        sizeof(uint32_t) * i, 0, 0, tiledBuffer, 0, &box);
    }

    D3D11_MAPPED_SUBRESOURCE mapped = { };
    m_context->Map(readbackBuffer, 0, D3D11_MAP_READ, 0, &mapped);

    auto data = reinterpret_cast<const uint32_t*>(mapped.pData);

    for (uint32_t i = 0; i < tileCount; i++) {
      if (data[i] != proc(i)) {
        std::cout << "At tile " << std::dec << i
                  << ", expected 0x" << std::hex << proc(i)
                  << ", got 0x" << std::hex << data[i] << std::endl;
        break;
      }
    }

    m_context->Unmap(readbackBuffer, 0);
  }

};

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
  TiledResourceTestApp app;
  return app.run();
}
