#include <iostream>
#include <cstring>
#include <array>

#include <d3dcompiler.h>
#include <d3d11_4.h>

#include <windows.h>
#include <windowsx.h>

#include "../common/com.h"
#include "../common/str.h"

#undef ENUM_NAME
#define ENUM_NAME(e) case e: return #e;

std::string GetFormatName(DXGI_FORMAT Format) {
  switch (Format) {
    ENUM_NAME(DXGI_FORMAT_UNKNOWN);
    ENUM_NAME(DXGI_FORMAT_R32G32B32A32_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_R32G32B32A32_FLOAT);
    ENUM_NAME(DXGI_FORMAT_R32G32B32A32_UINT);
    ENUM_NAME(DXGI_FORMAT_R32G32B32A32_SINT);
    ENUM_NAME(DXGI_FORMAT_R32G32B32_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_R32G32B32_FLOAT);
    ENUM_NAME(DXGI_FORMAT_R32G32B32_UINT);
    ENUM_NAME(DXGI_FORMAT_R32G32B32_SINT);
    ENUM_NAME(DXGI_FORMAT_R16G16B16A16_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_R16G16B16A16_FLOAT);
    ENUM_NAME(DXGI_FORMAT_R16G16B16A16_UNORM);
    ENUM_NAME(DXGI_FORMAT_R16G16B16A16_UINT);
    ENUM_NAME(DXGI_FORMAT_R16G16B16A16_SNORM);
    ENUM_NAME(DXGI_FORMAT_R16G16B16A16_SINT);
    ENUM_NAME(DXGI_FORMAT_R32G32_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_R32G32_FLOAT);
    ENUM_NAME(DXGI_FORMAT_R32G32_UINT);
    ENUM_NAME(DXGI_FORMAT_R32G32_SINT);
    ENUM_NAME(DXGI_FORMAT_R32G8X24_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
    ENUM_NAME(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_X32_TYPELESS_G8X24_UINT);
    ENUM_NAME(DXGI_FORMAT_R10G10B10A2_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_R10G10B10A2_UNORM);
    ENUM_NAME(DXGI_FORMAT_R10G10B10A2_UINT);
    ENUM_NAME(DXGI_FORMAT_R11G11B10_FLOAT);
    ENUM_NAME(DXGI_FORMAT_R8G8B8A8_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_R8G8B8A8_UNORM);
    ENUM_NAME(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    ENUM_NAME(DXGI_FORMAT_R8G8B8A8_UINT);
    ENUM_NAME(DXGI_FORMAT_R8G8B8A8_SNORM);
    ENUM_NAME(DXGI_FORMAT_R8G8B8A8_SINT);
    ENUM_NAME(DXGI_FORMAT_R16G16_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_R16G16_FLOAT);
    ENUM_NAME(DXGI_FORMAT_R16G16_UNORM);
    ENUM_NAME(DXGI_FORMAT_R16G16_UINT);
    ENUM_NAME(DXGI_FORMAT_R16G16_SNORM);
    ENUM_NAME(DXGI_FORMAT_R16G16_SINT);
    ENUM_NAME(DXGI_FORMAT_R32_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_D32_FLOAT);
    ENUM_NAME(DXGI_FORMAT_R32_FLOAT);
    ENUM_NAME(DXGI_FORMAT_R32_UINT);
    ENUM_NAME(DXGI_FORMAT_R32_SINT);
    ENUM_NAME(DXGI_FORMAT_R24G8_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_D24_UNORM_S8_UINT);
    ENUM_NAME(DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_X24_TYPELESS_G8_UINT);
    ENUM_NAME(DXGI_FORMAT_R8G8_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_R8G8_UNORM);
    ENUM_NAME(DXGI_FORMAT_R8G8_UINT);
    ENUM_NAME(DXGI_FORMAT_R8G8_SNORM);
    ENUM_NAME(DXGI_FORMAT_R8G8_SINT);
    ENUM_NAME(DXGI_FORMAT_R16_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_R16_FLOAT);
    ENUM_NAME(DXGI_FORMAT_D16_UNORM);
    ENUM_NAME(DXGI_FORMAT_R16_UNORM);
    ENUM_NAME(DXGI_FORMAT_R16_UINT);
    ENUM_NAME(DXGI_FORMAT_R16_SNORM);
    ENUM_NAME(DXGI_FORMAT_R16_SINT);
    ENUM_NAME(DXGI_FORMAT_R8_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_R8_UNORM);
    ENUM_NAME(DXGI_FORMAT_R8_UINT);
    ENUM_NAME(DXGI_FORMAT_R8_SNORM);
    ENUM_NAME(DXGI_FORMAT_R8_SINT);
    ENUM_NAME(DXGI_FORMAT_A8_UNORM);
    ENUM_NAME(DXGI_FORMAT_R1_UNORM);
    ENUM_NAME(DXGI_FORMAT_R9G9B9E5_SHAREDEXP);
    ENUM_NAME(DXGI_FORMAT_R8G8_B8G8_UNORM);
    ENUM_NAME(DXGI_FORMAT_G8R8_G8B8_UNORM);
    ENUM_NAME(DXGI_FORMAT_BC1_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_BC1_UNORM);
    ENUM_NAME(DXGI_FORMAT_BC1_UNORM_SRGB);
    ENUM_NAME(DXGI_FORMAT_BC2_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_BC2_UNORM);
    ENUM_NAME(DXGI_FORMAT_BC2_UNORM_SRGB);
    ENUM_NAME(DXGI_FORMAT_BC3_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_BC3_UNORM);
    ENUM_NAME(DXGI_FORMAT_BC3_UNORM_SRGB);
    ENUM_NAME(DXGI_FORMAT_BC4_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_BC4_UNORM);
    ENUM_NAME(DXGI_FORMAT_BC4_SNORM);
    ENUM_NAME(DXGI_FORMAT_BC5_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_BC5_UNORM);
    ENUM_NAME(DXGI_FORMAT_BC5_SNORM);
    ENUM_NAME(DXGI_FORMAT_B5G6R5_UNORM);
    ENUM_NAME(DXGI_FORMAT_B5G5R5A1_UNORM);
    ENUM_NAME(DXGI_FORMAT_B8G8R8A8_UNORM);
    ENUM_NAME(DXGI_FORMAT_B8G8R8X8_UNORM);
    ENUM_NAME(DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM);
    ENUM_NAME(DXGI_FORMAT_B8G8R8A8_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    ENUM_NAME(DXGI_FORMAT_B8G8R8X8_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_B8G8R8X8_UNORM_SRGB);
    ENUM_NAME(DXGI_FORMAT_BC6H_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_BC6H_UF16);
    ENUM_NAME(DXGI_FORMAT_BC6H_SF16);
    ENUM_NAME(DXGI_FORMAT_BC7_TYPELESS);
    ENUM_NAME(DXGI_FORMAT_BC7_UNORM);
    ENUM_NAME(DXGI_FORMAT_BC7_UNORM_SRGB);
    ENUM_NAME(DXGI_FORMAT_AYUV);
    ENUM_NAME(DXGI_FORMAT_Y410);
    ENUM_NAME(DXGI_FORMAT_Y416);
    ENUM_NAME(DXGI_FORMAT_NV12);
    ENUM_NAME(DXGI_FORMAT_P010);
    ENUM_NAME(DXGI_FORMAT_P016);
    ENUM_NAME(DXGI_FORMAT_420_OPAQUE);
    ENUM_NAME(DXGI_FORMAT_YUY2);
    ENUM_NAME(DXGI_FORMAT_Y210);
    ENUM_NAME(DXGI_FORMAT_Y216);
    ENUM_NAME(DXGI_FORMAT_NV11);
    ENUM_NAME(DXGI_FORMAT_AI44);
    ENUM_NAME(DXGI_FORMAT_IA44);
    ENUM_NAME(DXGI_FORMAT_P8);
    ENUM_NAME(DXGI_FORMAT_A8P8);
    ENUM_NAME(DXGI_FORMAT_B4G4R4A4_UNORM);
    default: return std::to_string(Format);
  }
}


std::string GetFormatFlagName(D3D11_FORMAT_SUPPORT Flag) {
  switch (Flag) {
    ENUM_NAME(D3D11_FORMAT_SUPPORT_BUFFER);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_IA_INDEX_BUFFER);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_SO_BUFFER);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_TEXTURE1D);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_TEXTURE2D);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_TEXTURE3D);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_TEXTURECUBE);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_SHADER_LOAD);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_SHADER_SAMPLE);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_SHADER_SAMPLE_COMPARISON);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_SHADER_SAMPLE_MONO_TEXT);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_MIP);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_MIP_AUTOGEN);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_RENDER_TARGET);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_BLENDABLE);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_DEPTH_STENCIL);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_CPU_LOCKABLE);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_DISPLAY);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_CAST_WITHIN_BIT_LAYOUT);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_MULTISAMPLE_LOAD);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_SHADER_GATHER);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_BACK_BUFFER_CAST);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_TYPED_UNORDERED_ACCESS_VIEW);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_SHADER_GATHER_COMPARISON);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_DECODER_OUTPUT);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT);
    ENUM_NAME(D3D11_FORMAT_SUPPORT_VIDEO_ENCODER);
    default: return std::to_string(Flag);
  }
}


std::string GetFormatFlagName2(UINT Flag) {
  switch (Flag) {
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_ADD);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_BITWISE_OPS);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_COMPARE_STORE_OR_COMPARE_EXCHANGE);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_EXCHANGE);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_SIGNED_MIN_OR_MAX);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_UAV_ATOMIC_UNSIGNED_MIN_OR_MAX);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_UAV_TYPED_STORE);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_OUTPUT_MERGER_LOGIC_OP);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_TILED);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_SHAREABLE);
    ENUM_NAME(D3D11_FORMAT_SUPPORT2_MULTIPLANE_OVERLAY);
    default: return std::to_string(Flag);
  }
}


int main(int, char**) {
  Com<ID3D11Device> device;

  std::array<D3D_FEATURE_LEVEL, 9> featureLevels = {
    D3D_FEATURE_LEVEL_12_1,
    D3D_FEATURE_LEVEL_12_0,
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_1,
  };

  D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL();

  if (FAILED(D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featureLevels.data(), featureLevels.size(),
        D3D11_SDK_VERSION, &device, &featureLevel, nullptr))) {
    std::cerr << "Failed to create D3D11 device" << std::endl;
    return 1;
  }

  D3D11_FEATURE_DATA_THREADING                    featureThreading     = { };
  D3D11_FEATURE_DATA_DOUBLES                      featureDoubles       = { };
  D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT featureMinPrecision  = { };
  D3D11_FEATURE_DATA_D3D11_OPTIONS                featureD3D11Options  = { };
  D3D11_FEATURE_DATA_D3D11_OPTIONS1               featureD3D11Options1 = { };
  D3D11_FEATURE_DATA_D3D11_OPTIONS2               featureD3D11Options2 = { };
  D3D11_FEATURE_DATA_D3D11_OPTIONS3               featureD3D11Options3 = { };
  D3D11_FEATURE_DATA_D3D11_OPTIONS4               featureD3D11Options4 = { };
  D3D11_FEATURE_DATA_D3D11_OPTIONS5               featureD3D11Options5 = { };

  std::cout << "Feature level: " << (featureLevel >> 12) << "_" << ((featureLevel >> 8) & 0xf) << std::endl;

  if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &featureThreading, sizeof(featureThreading)))) {
    std::cout << "D3D11_FEATURE_THREADING:" << std::endl
              << "  DriverConcurrentCreates:          " << featureThreading.DriverConcurrentCreates << std::endl
              << "  DriverCommandLists:               " << featureThreading.DriverCommandLists << std::endl;
  }

  if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &featureDoubles, sizeof(featureDoubles)))) {
    std::cout << "D3D11_FEATURE_DOUBLES:" << std::endl
              << "  DoublePrecisionFloatShaderOps:    " << featureDoubles.DoublePrecisionFloatShaderOps << std::endl;
  }

  if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_SHADER_MIN_PRECISION_SUPPORT, &featureMinPrecision, sizeof(featureMinPrecision)))) {
    std::cout << "D3D11_FEATURE_SHADER_MIN_PRECISION_SUPPORT:" << std::endl
              << "  PixelShaderMinPrecision:          " << featureMinPrecision.PixelShaderMinPrecision << std::endl
              << "  AllOtherShaderStagesMinPrecision: " << featureMinPrecision.AllOtherShaderStagesMinPrecision << std::endl;
  }

  if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &featureD3D11Options, sizeof(featureD3D11Options)))) {
    std::cout << "D3D11_FEATURE_D3D11_OPTIONS:" << std::endl
              << "  OutputMergerLogicOp:              " << featureD3D11Options.OutputMergerLogicOp << std::endl
              << "  UAVOnlyRenderingForcedSampleCount: " << featureD3D11Options.UAVOnlyRenderingForcedSampleCount << std::endl
              << "  DiscardAPIsSeenByDriver:          " << featureD3D11Options.DiscardAPIsSeenByDriver << std::endl
              << "  FlagsForUpdateAndCopySeenByDriver: " << featureD3D11Options.FlagsForUpdateAndCopySeenByDriver << std::endl
              << "  ClearView:                        " << featureD3D11Options.ClearView << std::endl
              << "  CopyWithOverlap:                  " << featureD3D11Options.CopyWithOverlap << std::endl
              << "  ConstantBufferPartialUpdate:      " << featureD3D11Options.ConstantBufferPartialUpdate << std::endl
              << "  ConstantBufferOffsetting:         " << featureD3D11Options.ConstantBufferOffsetting << std::endl
              << "  MapNoOverwriteOnDynamicConstantBuffer: " << featureD3D11Options.MapNoOverwriteOnDynamicConstantBuffer << std::endl
              << "  MapNoOverwriteOnDynamicBufferSRV: " << featureD3D11Options.MapNoOverwriteOnDynamicBufferSRV << std::endl
              << "  MultisampleRTVWithForcedSampleCountOne: " << featureD3D11Options.MultisampleRTVWithForcedSampleCountOne << std::endl
              << "  SAD4ShaderInstructions:           " << featureD3D11Options.SAD4ShaderInstructions << std::endl
              << "  ExtendedDoublesShaderInstructions: " << featureD3D11Options.ExtendedDoublesShaderInstructions << std::endl
              << "  ExtendedResourceSharing:          " << featureD3D11Options.ExtendedResourceSharing << std::endl;
  }

  if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS1, &featureD3D11Options1, sizeof(featureD3D11Options1)))) {
    std::cout << "D3D11_FEATURE_D3D11_OPTIONS1:" << std::endl
              << "  TiledResourcesTier:               " << featureD3D11Options1.TiledResourcesTier << std::endl
              << "  MinMaxFiltering:                  " << featureD3D11Options1.MinMaxFiltering << std::endl
              << "  ClearViewAlsoSupportsDepthOnlyFormats: " << featureD3D11Options1.ClearViewAlsoSupportsDepthOnlyFormats << std::endl
              << "  MapOnDefaultBuffers:              " << featureD3D11Options1.MapOnDefaultBuffers << std::endl;

  }

  if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &featureD3D11Options2, sizeof(featureD3D11Options2)))) {
    std::cout << "D3D11_FEATURE_D3D11_OPTIONS2:" << std::endl
              << "  PSSpecifiedStencilRefSupported:   " << featureD3D11Options2.PSSpecifiedStencilRefSupported << std::endl
              << "  TypedUAVLoadAdditionalFormats:    " << featureD3D11Options2.TypedUAVLoadAdditionalFormats << std::endl
              << "  ROVsSupported:                    " << featureD3D11Options2.ROVsSupported << std::endl
              << "  ConservativeRasterizationTier:    " << featureD3D11Options2.ConservativeRasterizationTier << std::endl
              << "  MapOnDefaultTextures:             " << featureD3D11Options2.MapOnDefaultTextures << std::endl
              << "  TiledResourcesTier:               " << featureD3D11Options2.TiledResourcesTier << std::endl
              << "  StandardSwizzle:                  " << featureD3D11Options2.StandardSwizzle << std::endl
              << "  UnifiedMemoryArchitecture:        " << featureD3D11Options2.UnifiedMemoryArchitecture << std::endl;
  }

  if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &featureD3D11Options3, sizeof(featureD3D11Options3)))) {
    std::cout << "D3D11_FEATURE_D3D11_OPTIONS3:" << std::endl
              << "  VPAndRTArrayIndexFromAnyShaderFeedingRasterizer: " << featureD3D11Options3.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer << std::endl;
  }

  if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS4, &featureD3D11Options4, sizeof(featureD3D11Options4)))) {
    std::cout << "D3D11_FEATURE_D3D11_OPTIONS4:" << std::endl
              << "  ExtendedNV12SharedTextureSupported: " << featureD3D11Options4.ExtendedNV12SharedTextureSupported << std::endl;
  }

  if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS5, &featureD3D11Options5, sizeof(featureD3D11Options5)))) {
    std::cout << "D3D11_FEATURE_D3D11_OPTIONS5:" << std::endl
              << "  SharedResourceTier:               " << featureD3D11Options5.SharedResourceTier << std::endl;
  }

  for (UINT i  = UINT(DXGI_FORMAT_UNKNOWN);
            i <= UINT(DXGI_FORMAT_B4G4R4A4_UNORM);
            i++) {
    DXGI_FORMAT format = DXGI_FORMAT(i);
    UINT        flags  = 0;
    
    std::cout << GetFormatName(format) << ": " << std::endl;
    
    if (SUCCEEDED(device->CheckFormatSupport(format, &flags))) {
      for (uint32_t i = 0; i < 32; i++) {
        if (flags & (1 << i)) {
          std::cout << "  "
                    << GetFormatFlagName(D3D11_FORMAT_SUPPORT(1 << i))
                    << std::endl;
        }
      }

      D3D11_FEATURE_DATA_FORMAT_SUPPORT2 support2 = { };
      support2.InFormat = format;

      if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT2, &support2, sizeof(support2)))) {
        for (uint32_t i = 0; i < 32; i++) {
          if (support2.OutFormatSupport2 & (1u << i)) {
            std::cout << "  "
                      << GetFormatFlagName2(1u << i)
                      << std::endl;
          }
        }
      }
    } else {
      std::cout << "  Not supported" << std::endl;
    }
  }
  
  return 0;
}
