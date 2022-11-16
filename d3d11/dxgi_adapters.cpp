#include <iostream>
#include <vector>

#include <dxgi1_6.h>

#include <windows.h>
#include <windowsx.h>

#include "../common/com.h"
#include "../common/str.h"

int main(int argc, char **argv) {
  Com<IDXGIFactory1> factory;
  
  if (CreateDXGIFactory1(__uuidof(IDXGIFactory1),
      reinterpret_cast<void**>(&factory)) != S_OK) {
    std::cerr << "Failed to create DXGI factory" << std::endl;
    return 1;
  }
  
  Com<IDXGIAdapter1> adapter;
  
  for (UINT i = 0; factory->EnumAdapters1(i, &adapter) == S_OK; i++) {
    DXGI_ADAPTER_DESC adapterDesc;
    
    if (adapter->GetDesc(&adapterDesc) != S_OK) {
      std::cerr << "Failed to get DXGI adapter info" << std::endl;
      return 1;
    }
    
    DXGI_ADAPTER_DESC desc;
    
    if (adapter->GetDesc(&desc) != S_OK) {
      std::cerr << "Failed to get DXGI adapter info" << std::endl;
      return 1;
    }
    
    std::cout << format("Adapter ", i, ":") << std::endl;
    std::cout << format(" ", desc.Description) << std::endl;
    std::cout << format(" Vendor: ", desc.VendorId) << std::endl;
    std::cout << format(" Device: ", desc.DeviceId) << std::endl;
    std::cout << format(" Dedicated RAM: ", desc.DedicatedVideoMemory) << std::endl;
    std::cout << format(" Shared RAM: ", desc.SharedSystemMemory) << std::endl;
    
    Com<IDXGIOutput> baseOutput;
    
    for (UINT j = 0; adapter->EnumOutputs(j, &baseOutput) == S_OK; j++) {
      Com<IDXGIOutput6> output;
      baseOutput->QueryInterface(__uuidof(IDXGIOutput6), reinterpret_cast<void**>(&output));

      std::vector<DXGI_MODE_DESC> modes;
      
      DXGI_OUTPUT_DESC1 desc;
      
      if (output->GetDesc1(&desc) != S_OK) {
        std::cerr << "Failed to get DXGI output info" << std::endl;
        return 1;
      }
      
      std::cout << format(" Output ", j, ":") << std::endl;
      std::cout << format("  ", desc.DeviceName) << std::endl;
      std::cout << format("  Coordinates: ",
        desc.DesktopCoordinates.left, ",",
        desc.DesktopCoordinates.top, ":",
        desc.DesktopCoordinates.right - desc.DesktopCoordinates.left, "x",
        desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top) << std::endl;
      std::cout << format("  AttachedToDesktop: ", desc.AttachedToDesktop ? "true" : "false") << std::endl;
      std::cout << format("  Rotation: ", desc.Rotation) << std::endl;
      std::cout << format("  Monitor: ", desc.Monitor) << std::endl;
      std::cout << format("  BitsPerColor: ", desc.BitsPerColor) << std::endl;
      std::cout << format("  ColorSpace: ", desc.ColorSpace) << std::endl;
      std::cout << format("  RedPrimary: ", desc.RedPrimary[0], " ", desc.RedPrimary[1]) << std::endl;
      std::cout << format("  GreenPrimary: ", desc.GreenPrimary[0], " ", desc.GreenPrimary[1]) << std::endl;
      std::cout << format("  BluePrimary: ", desc.BluePrimary[0], " ", desc.BluePrimary[1]) << std::endl;
      std::cout << format("  WhitePoint: ", desc.WhitePoint[0], " ", desc.WhitePoint[1]) << std::endl;
      std::cout << format("  MinLuminance: ", desc.MinLuminance) << std::endl;
      std::cout << format("  MaxLuminance: ", desc.MaxLuminance) << std::endl;
      std::cout << format("  MaxFullFrameLuminance: ", desc.MaxFullFrameLuminance) << std::endl;
      
      HRESULT status = S_OK;
      UINT    displayModeCount = 0;
      
      do {
        if (output->GetDisplayModeList(
          DXGI_FORMAT_R8G8B8A8_UNORM,
          DXGI_ENUM_MODES_SCALING,
          &displayModeCount, nullptr) != S_OK) {
          std::cerr << "Failed to get DXGI output display mode count" << std::endl;
          return 1;
        }
        
        modes.resize(displayModeCount);
        
        status = output->GetDisplayModeList(
          DXGI_FORMAT_R8G8B8A8_UNORM,
          DXGI_ENUM_MODES_SCALING,
          &displayModeCount, modes.data());
      } while (status == DXGI_ERROR_MORE_DATA);
      
      if (status != S_OK) {
        std::cerr << "Failed to get DXGI output display mode list" << std::endl;
        return 1;
      }
      
      for (auto mode : modes) {
        std::cout << format("  ",
          mode.Width, "x", mode.Height, " @ ",
          mode.RefreshRate.Numerator / mode.RefreshRate.Denominator,
          mode.Scaling == DXGI_MODE_SCALING_CENTERED ? " (native)" : "") << std::endl;

        //test matching modes
        DXGI_MODE_DESC matched_mode{ 0 };
        status = output->FindClosestMatchingMode(&mode, &matched_mode, nullptr);

        if (status != S_OK) {
            std::cerr << "Failed to get matching mode" << std::endl;
            return 1;
        }
        
        if (matched_mode.Width != mode.Width ||
            matched_mode.Height != mode.Height ||
            matched_mode.RefreshRate.Numerator != mode.RefreshRate.Numerator ||
            matched_mode.RefreshRate.Denominator != mode.RefreshRate.Denominator ||
            matched_mode.Format != mode.Format)
        {
            std::cerr << "Matched mode is incorrect" << std::endl;
            return 1;
        }
      }
    }
  }
  
  return 0;
}
