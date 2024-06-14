#include <map>
#include <array>
#include <iostream>

#include <d3d8.h>
#include <d3d9caps.h>

#include "../common/com.h"
#include "../common/error.h"
#include "../common/str.h"

struct RGBVERTEX {
    FLOAT x, y, z, rhw;
    DWORD colour;
};

#define RGBT_FVF_CODES (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

// D3D9 caps
#ifdef _MSC_VER
#define D3DCAPS2_CANAUTOGENMIPMAP               0x40000000L
#define D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION    0x00000080L
#define D3DCAPS3_COPY_TO_VIDMEM                 0x00000100L
#define D3DCAPS3_COPY_TO_SYSTEMMEM              0x00000200L
#define D3DPMISCCAPS_INDEPENDENTWRITEMASKS      0x00004000L
#define D3DPMISCCAPS_PERSTAGECONSTANT           0x00008000L
#define D3DPMISCCAPS_FOGANDSPECULARALPHA        0x00010000L
#define D3DPMISCCAPS_SEPARATEALPHABLEND         0x00020000L
#define D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS    0x00040000L
#define D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING 0x00080000L
#define D3DPMISCCAPS_FOGVERTEXCLAMPED           0x00100000L

#define D3DPRASTERCAPS_SCISSORTEST              0x01000000L
#define D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS      0x02000000L
#define D3DPRASTERCAPS_DEPTHBIAS                0x04000000L 
#define D3DPRASTERCAPS_MULTISAMPLE_TOGGLE       0x08000000L

#define D3DLINECAPS_ANTIALIAS                   0x00000020L
#define D3DSTENCILCAPS_TWOSIDED                 0x00000100L
#define D3DPMISCCAPS_POSTBLENDSRGBCONVERT       0x00200000L
#define D3DPBLENDCAPS_SRCCOLOR2                 0x00004000L
#define D3DPBLENDCAPS_INVSRCCOLOR2              0x00008000L
#endif

// these don't seem to be included in any of the
// headers that should allegedly include them...
#define D3DDEVINFOID_VCACHE                     4

typedef struct _D3DDEVINFO_VCACHE {
  DWORD Pattern;
  DWORD OptMethod;
  DWORD CacheSize;
  DWORD MagicNumber;
} D3DDEVINFO_VCACHE, *LPD3DDEVINFO_VCACHE;

class RGBTriangle {
    
    public:

        static const UINT WINDOW_WIDTH  = 800;
        static const UINT WINDOW_HEIGHT = 800;

        RGBTriangle(HWND hWnd) : m_hWnd(hWnd) {
            decltype(Direct3DCreate8)* Direct3DCreate8 = nullptr;
            HMODULE hm = LoadLibraryA("d3d8.dll");
            Direct3DCreate8 = (decltype(Direct3DCreate8))GetProcAddress(hm, "Direct3DCreate8");

            // D3D Interface
            m_d3d = Direct3DCreate8(D3D_SDK_VERSION);
            if (m_d3d.ptr() == nullptr)
                throw Error("Failed to create D3D8 interface");

            D3DADAPTER_IDENTIFIER8 adapterId;
            HRESULT status = m_d3d->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &adapterId);
            if (FAILED(status))
                throw Error("Failed to get D3D8 adapter identifier");

            std::cout << format("Using adapter: ", adapterId.Description) << std::endl;

            // DWORD to hex printout
            char vendorID[16];
            char deviceID[16];
            sprintf(vendorID, "VendorId: %04lx", adapterId.VendorId);
            sprintf(deviceID, "DeviceId: %04lx", adapterId.DeviceId);
            std::cout << "  ~ " << vendorID << std::endl;
            std::cout << "  ~ " << deviceID << std::endl;

            std::cout << format("  ~ Driver: ", adapterId.Driver) << std::endl;

            // NVIDIA stores the driver version in the lower half of the lower DWORD
            if (adapterId.VendorId == uint32_t(0x10de)) {
                DWORD driverVersion = LOWORD(adapterId.DriverVersion.LowPart);
                DWORD majorVersion = driverVersion / 100;
                DWORD minorVersion = driverVersion % 100;
                std::cout << format("  ~ Version: ", majorVersion, ".", minorVersion) << std::endl;
            }
            // for other vendors simply list the entire DriverVersion long int
            else {
                DWORD product = HIWORD(adapterId.DriverVersion.HighPart);
                DWORD version = LOWORD(adapterId.DriverVersion.HighPart);
                DWORD subVersion = HIWORD(adapterId.DriverVersion.LowPart);
                DWORD build = LOWORD(adapterId.DriverVersion.LowPart);
                std::cout << format("  ~ Version: ", product, ".", version, ".",
                    subVersion, ".", build) << std::endl;
            }

            // D3D Device
            D3DDISPLAYMODE dm;
            status = m_d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm);
            if (FAILED(status))
                throw Error("Failed to get D3D8 adapter display mode");

            ZeroMemory(&m_pp, sizeof(m_pp));

            m_pp.Windowed = TRUE;
            m_pp.hDeviceWindow = m_hWnd;
            // set to D3DSWAPEFFECT_COPY or D3DSWAPEFFECT_FLIP for no VSync
            m_pp.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
            // according to D3D8 spec "0 is treated as 1" here
            m_pp.BackBufferCount = 0;
            m_pp.BackBufferWidth = WINDOW_WIDTH;
            m_pp.BackBufferHeight = WINDOW_HEIGHT;
            m_pp.BackBufferFormat = dm.Format;

            createDeviceWithFlags(&m_pp, D3DCREATE_HARDWARE_VERTEXPROCESSING, true);
        }

        // D3D Adapter Display Mode enumeration
        void listAdapterDisplayModes() {
            HRESULT          status;
            D3DDISPLAYMODE   amDM;
            UINT             adapterModeCount = 0;

            ZeroMemory(&amDM, sizeof(amDM));

            adapterModeCount = m_d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT);
            if (adapterModeCount == 0)
                throw Error("Failed to query for D3D8 adapter display modes");

            // these are all the possible adapter display formats, at least in theory
            std::map<D3DFORMAT, char const*> amDMFormats = { {D3DFMT_X8R8G8B8, "D3DFMT_X8R8G8B8"}, 
                                                             {D3DFMT_X1R5G5B5, "D3DFMT_X1R5G5B5"}, 
                                                             {D3DFMT_R5G6B5, "D3DFMT_R5G6B5"} };

            std::cout << std::endl << "Enumerating supported adapter display modes:" << std::endl;

            for (UINT i = 0; i < adapterModeCount; i++) {
                status =  m_d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &amDM);
                if (FAILED(status)) {
                    std::cout << format("    Failed to get adapter display mode ", i) << std::endl;
                } else {
                    std::cout << format("    ", amDMFormats[amDM.Format], " ", amDM.Width, 
                                        " x ", amDM.Height, " @ ", amDM.RefreshRate, " Hz") << std::endl;
                }
            }

            std::cout << format("Listed a total of ", adapterModeCount, " adapter display modes") << std::endl;
        }

        // D3D Device back buffer formats check
        void listBackBufferFormats(BOOL windowed) {
            resetOrRecreateDevice();

            HRESULT status;
            D3DPRESENT_PARAMETERS bbPP;

            memcpy(&bbPP, &m_pp, sizeof(m_pp));

            // these are all the possible backbuffer formats, at least in theory
            std::map<D3DFORMAT, char const*> bbFormats = { {D3DFMT_A8R8G8B8, "D3DFMT_A8R8G8B8"},
                                                           {D3DFMT_X8R8G8B8, "D3DFMT_X8R8G8B8"},
                                                           {D3DFMT_A1R5G5B5, "D3DFMT_A1R5G5B5"},
                                                           {D3DFMT_X1R5G5B5, "D3DFMT_X1R5G5B5"},
                                                           {D3DFMT_R5G6B5, "D3DFMT_R5G6B5"} };

            std::map<D3DFORMAT, char const*>::iterator bbFormatIter;

            std::cout << std::endl;
            if (windowed)
                std::cout << "Device back buffer format support (windowed):" << std::endl;
            else
                std::cout << "Device back buffer format support (full screen):" << std::endl;

            for (bbFormatIter = bbFormats.begin(); bbFormatIter != bbFormats.end(); bbFormatIter++) {
                status = m_d3d->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                                bbFormatIter->first, bbFormatIter->first,
                                                windowed);
                if (FAILED(status)) {
                    std::cout << format("  - The ", bbFormatIter->second, " format is not supported") << std::endl;
                } else {
                    bbPP.BackBufferFormat = bbFormatIter->first;

                    status = createDeviceWithFlags(&bbPP, D3DCREATE_HARDWARE_VERTEXPROCESSING, false);
                    if (FAILED(status)) {
                        std::cout << format("  ! The ", bbFormatIter->second, " format is supported, device creation FAILED") << std::endl;
                    } else {
                        std::cout << format("  + The ", bbFormatIter->second, " format is supported, device creation SUCCEEDED") << std::endl;
                    }
                }
            }
        }

        // D3D Device capabilities check
        void listDeviceCapabilities() {
            D3DCAPS8 caps8SWVP;
            D3DCAPS8 caps8HWVP;
            D3DCAPS8 caps8;

            // get the capabilities from the D3D device in SWVP mode
            createDeviceWithFlags(&m_pp, D3DCREATE_SOFTWARE_VERTEXPROCESSING, true);
            m_device->GetDeviceCaps(&caps8SWVP);

            // get the capabilities from the D3D device in HWVP mode
            createDeviceWithFlags(&m_pp, D3DCREATE_HARDWARE_VERTEXPROCESSING, true);
            m_device->GetDeviceCaps(&caps8HWVP);

            // get the capabilities from the D3D interface
            m_d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps8);

            std::cout << std::endl << "Listing device capabilities support:" << std::endl;

            if (caps8.Caps2 & D3DCAPS2_NO2DDURING3DSCENE)
                std::cout << "  + D3DCAPS2_NO2DDURING3DSCENE is supported" << std::endl;
            else
                std::cout << "  - D3DCAPS2_NO2DDURING3DSCENE is not supported" << std::endl;

            if (caps8.DevCaps & D3DDEVCAPS_QUINTICRTPATCHES)
                std::cout << "  + D3DDEVCAPS_QUINTICRTPATCHES is supported" << std::endl;
            else
                std::cout << "  - D3DDEVCAPS_QUINTICRTPATCHES is not supported" << std::endl;

            if (caps8.DevCaps & D3DDEVCAPS_RTPATCHES)
                std::cout << "  + D3DDEVCAPS_RTPATCHES is supported" << std::endl;
            else
                std::cout << "  - D3DDEVCAPS_RTPATCHES is not supported" << std::endl;

            if (caps8.DevCaps & D3DDEVCAPS_RTPATCHHANDLEZERO)
                std::cout << "  + D3DDEVCAPS_RTPATCHHANDLEZERO is supported" << std::endl;
            else
                std::cout << "  - D3DDEVCAPS_RTPATCHHANDLEZERO is not supported" << std::endl;

            if (caps8.DevCaps & D3DDEVCAPS_NPATCHES)
                std::cout << "  + D3DDEVCAPS_NPATCHES is supported" << std::endl;
            else
                std::cout << "  - D3DDEVCAPS_NPATCHES is not supported" << std::endl;

            // depends on D3DPRASTERCAPS_PAT
            if (caps8.PrimitiveMiscCaps & D3DPMISCCAPS_LINEPATTERNREP)
                std::cout << "  + D3DPMISCCAPS_LINEPATTERNREP is supported" << std::endl;
            else
                std::cout << "  - D3DPMISCCAPS_LINEPATTERNREP is not supported" << std::endl;

            // WineD3D supports it, but native drivers do not
            if (caps8.RasterCaps & D3DPRASTERCAPS_PAT)
                std::cout << "  + D3DPRASTERCAPS_PAT is supported" << std::endl;
            else
                std::cout << "  - D3DPRASTERCAPS_PAT is not supported" << std::endl;

            // this isn't typically exposed even on native
            if (caps8.RasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES)
                std::cout << "  + D3DPRASTERCAPS_ANTIALIASEDGES is supported" << std::endl;
            else
                std::cout << "  - D3DPRASTERCAPS_ANTIALIASEDGES is not supported" << std::endl;

            if (caps8.RasterCaps & D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE)
                std::cout << "  + D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE is supported" << std::endl;
            else
                std::cout << "  - D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE is not supported" << std::endl;

            if (caps8.VertexProcessingCaps & D3DVTXPCAPS_NO_VSDT_UBYTE4)
                std::cout << "  + D3DVTXPCAPS_NO_VSDT_UBYTE4 is supported" << std::endl;
            else
                std::cout << "  - D3DVTXPCAPS_NO_VSDT_UBYTE4 is not supported" << std::endl;

            std::cout << std::endl << "Listing device capability limits:" << std::endl;

            std::cout << format("  ~ MaxTextureWidth: ", caps8.MaxTextureWidth) << std::endl;
            std::cout << format("  ~ MaxTextureHeight: ", caps8.MaxTextureHeight) << std::endl;
            std::cout << format("  ~ MaxVolumeExtent: ", caps8.MaxVolumeExtent) << std::endl;
            std::cout << format("  ~ MaxTextureRepeat: ", caps8.MaxTextureRepeat) << std::endl;
            std::cout << format("  ~ MaxTextureAspectRatio: ", caps8.MaxTextureAspectRatio) << std::endl;
            std::cout << format("  ~ MaxAnisotropy: ", caps8.MaxAnisotropy) << std::endl;
            std::cout << format("  ~ MaxVertexW: ", caps8.MaxVertexW) << std::endl;
            std::cout << format("  ~ GuardBandLeft: ", caps8.GuardBandLeft) << std::endl;
            std::cout << format("  ~ GuardBandTop: ", caps8.GuardBandTop) << std::endl;
            std::cout << format("  ~ GuardBandRight: ", caps8.GuardBandRight) << std::endl;
            std::cout << format("  ~ GuardBandBottom: ", caps8.GuardBandBottom) << std::endl;
            std::cout << format("  ~ ExtentsAdjust: ", caps8.ExtentsAdjust) << std::endl;
            std::cout << format("  ~ MaxTextureBlendStages: ", caps8.MaxTextureBlendStages) << std::endl;
            std::cout << format("  ~ MaxSimultaneousTextures: ", caps8.MaxSimultaneousTextures) << std::endl;
            // may vary between interface and device modes (SWVP or HWVP)
            std::cout << format("  ~ MaxActiveLights: ", caps8.MaxActiveLights, " (I), ", caps8SWVP.MaxActiveLights,
                                " (SWVP), ", caps8HWVP.MaxActiveLights, " (HWVP)") << std::endl;
            // may vary between interface and device modes (SWVP or HWVP)
            std::cout << format("  ~ MaxUserClipPlanes: ", caps8.MaxUserClipPlanes, " (I), ", caps8SWVP.MaxUserClipPlanes,
                                " (SWVP), ", caps8HWVP.MaxUserClipPlanes, " (HWVP)") << std::endl;
            // may vary between interface and device modes (SWVP or HWVP)
            std::cout << format("  ~ MaxVertexBlendMatrices: ", caps8.MaxVertexBlendMatrices, " (I), ", caps8SWVP.MaxVertexBlendMatrices,
                                " (SWVP), ", caps8HWVP.MaxVertexBlendMatrices, " (HWVP)") << std::endl;
            // may vary between interface and device modes (SWVP or HWVP)
            std::cout << format("  ~ MaxVertexBlendMatrixIndex: ", caps8.MaxVertexBlendMatrixIndex, " (I), ", caps8SWVP.MaxVertexBlendMatrixIndex,
                                " (SWVP), ", caps8HWVP.MaxVertexBlendMatrixIndex, " (HWVP)") << std::endl;                         
            std::cout << format("  ~ MaxPointSize: ", caps8.MaxPointSize) << std::endl;
            std::cout << format("  ~ MaxPrimitiveCount: ", caps8.MaxPrimitiveCount) << std::endl;
            std::cout << format("  ~ MaxVertexIndex: ", caps8.MaxVertexIndex) << std::endl;
            std::cout << format("  ~ MaxStreams: ", caps8.MaxStreams) << std::endl;
            std::cout << format("  ~ MaxStreamStride: ", caps8.MaxStreamStride) << std::endl;
            std::cout << format("  ~ MaxVertexShaderConst: ", caps8.MaxVertexShaderConst) << std::endl;
            // typically FLT_MAX
            std::cout << format("  ~ MaxPixelShaderValue: ", caps8.MaxPixelShaderValue) << std::endl;
        }

        // D3D8 Devices shouldn't typically expose any D3D9 capabilities
        void listDeviceD3D9Capabilities() {
            D3DCAPS8 caps8;

            // get the capabilities from the D3D interface
            m_d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps8);

            std::cout << std::endl << "Listing device D3D9 capabilities support:" << std::endl;

            if (caps8.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)
                std::cout << "  + D3DCAPS2_CANAUTOGENMIPMAP is supported" << std::endl;
            else
                std::cout << "  - D3DCAPS2_CANAUTOGENMIPMAP is not supported" << std::endl;

            if (caps8.Caps3 & D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION)
                std::cout << "  + D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION is supported" << std::endl;
            else
                std::cout << "  - D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION is not supported" << std::endl;

            if (caps8.Caps3 & D3DCAPS3_COPY_TO_VIDMEM)
                std::cout << "  + D3DCAPS3_COPY_TO_VIDMEM is supported" << std::endl;
            else
                std::cout << "  - D3DCAPS3_COPY_TO_VIDMEM is not supported" << std::endl;

            if (caps8.Caps3 & D3DCAPS3_COPY_TO_SYSTEMMEM)
                std::cout << "  + D3DCAPS3_COPY_TO_SYSTEMMEM is supported" << std::endl;
            else
                std::cout << "  - D3DCAPS3_COPY_TO_SYSTEMMEM is not supported" << std::endl;

            if (caps8.PrimitiveMiscCaps & D3DPMISCCAPS_INDEPENDENTWRITEMASKS)
                std::cout << "  + D3DPMISCCAPS_INDEPENDENTWRITEMASKS is supported" << std::endl;
            else
                std::cout << "  - D3DPMISCCAPS_INDEPENDENTWRITEMASKS is not supported" << std::endl;

            if (caps8.PrimitiveMiscCaps & D3DPMISCCAPS_PERSTAGECONSTANT)
                std::cout << "  + D3DPMISCCAPS_PERSTAGECONSTANT is supported" << std::endl;
            else
                std::cout << "  - D3DPMISCCAPS_PERSTAGECONSTANT is not supported" << std::endl;

            if (caps8.PrimitiveMiscCaps & D3DPMISCCAPS_FOGANDSPECULARALPHA)
                std::cout << "  + D3DPMISCCAPS_FOGANDSPECULARALPHA is supported" << std::endl;
            else
                std::cout << "  - D3DPMISCCAPS_FOGANDSPECULARALPHA is not supported" << std::endl;

            if (caps8.PrimitiveMiscCaps & D3DPMISCCAPS_SEPARATEALPHABLEND)
                std::cout << "  + D3DPMISCCAPS_SEPARATEALPHABLEND is supported" << std::endl;
            else
                std::cout << "  - D3DPMISCCAPS_SEPARATEALPHABLEND is not supported" << std::endl;

            if (caps8.PrimitiveMiscCaps & D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS)
                std::cout << "  + D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS is supported" << std::endl;
            else
                std::cout << "  - D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS is not supported" << std::endl;

            if (caps8.PrimitiveMiscCaps & D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING)
                std::cout << "  + D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING is supported" << std::endl;
            else
                std::cout << "  - D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING is not supported" << std::endl;

            if (caps8.PrimitiveMiscCaps & D3DPMISCCAPS_FOGVERTEXCLAMPED)
                std::cout << "  + D3DPMISCCAPS_FOGVERTEXCLAMPED is supported" << std::endl;
            else
                std::cout << "  - D3DPMISCCAPS_FOGVERTEXCLAMPED is not supported" << std::endl;

            if (caps8.PrimitiveMiscCaps & D3DPMISCCAPS_POSTBLENDSRGBCONVERT)
                std::cout << "  + D3DPMISCCAPS_POSTBLENDSRGBCONVERT is supported" << std::endl;
            else
                std::cout << "  - D3DPMISCCAPS_POSTBLENDSRGBCONVERT is not supported" << std::endl;

            if (caps8.RasterCaps & D3DPRASTERCAPS_SCISSORTEST)
                std::cout << "  + D3DPRASTERCAPS_SCISSORTEST is supported" << std::endl;
            else
                std::cout << "  - D3DPRASTERCAPS_SCISSORTEST is not supported" << std::endl;

            if (caps8.RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS)
                std::cout << "  + D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS is supported" << std::endl;
            else
                std::cout << "  - D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS is not supported" << std::endl;

            if (caps8.RasterCaps & D3DPRASTERCAPS_DEPTHBIAS)
                std::cout << "  + D3DPRASTERCAPS_DEPTHBIAS is supported" << std::endl;
            else
                std::cout << "  - D3DPRASTERCAPS_DEPTHBIAS is not supported" << std::endl;

            if (caps8.RasterCaps & D3DPRASTERCAPS_MULTISAMPLE_TOGGLE)
                std::cout << "  + D3DPRASTERCAPS_MULTISAMPLE_TOGGLE is supported" << std::endl;
            else
                std::cout << "  - D3DPRASTERCAPS_MULTISAMPLE_TOGGLE is not supported" << std::endl;

            if (caps8.SrcBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR2)
                std::cout << "  + D3DPBLENDCAPS_INVSRCCOLOR2 is supported" << std::endl;
            else
                std::cout << "  - D3DPBLENDCAPS_INVSRCCOLOR2 is not supported" << std::endl;

            if (caps8.SrcBlendCaps & D3DPBLENDCAPS_SRCCOLOR2)
                std::cout << "  + D3DPBLENDCAPS_SRCCOLOR2 is supported" << std::endl;
            else
                std::cout << "  - D3DPBLENDCAPS_SRCCOLOR2 is not supported" << std::endl;
            
            if (caps8.LineCaps & D3DLINECAPS_ANTIALIAS)
                std::cout << "  + D3DLINECAPS_ANTIALIAS is supported" << std::endl;
            else
                std::cout << "  - D3DLINECAPS_ANTIALIAS is not supported" << std::endl;

            if (caps8.StencilCaps & D3DSTENCILCAPS_TWOSIDED)
                std::cout << "  + D3DSTENCILCAPS_TWOSIDED is supported" << std::endl;
            else
                std::cout << "  - D3DSTENCILCAPS_TWOSIDED is not supported" << std::endl;
        }

        void listVCacheQueryResult() {
            resetOrRecreateDevice();

            D3DDEVINFO_VCACHE vCache;
            HRESULT status = m_device->GetInfo(D3DDEVINFOID_VCACHE, &vCache, sizeof(D3DDEVINFO_VCACHE));

            std::cout << std::endl << "Listing VCache query result:" << std::endl;

            switch (status) {
                // NVIDIA will return D3D_OK and some values in vCache
                case D3D_OK:
                    std::cout << "  ~ Response: D3D_OK" << std::endl;
                    break;
                // ATI/AMD and Intel will return S_FALSE and all 0s in vCache
                case S_FALSE:
                    std::cout << "  ~ Response: S_FALSE" << std::endl;
                    break;
                case E_FAIL:
                    std::cout << "  ~ Response: E_FAIL" << std::endl;
                    break;
                // shouldn't be returned by any vendor
                case D3DERR_NOTAVAILABLE:
                    std::cout << "  ~ Response: D3DERR_NOTAVAILABLE" << std::endl;
                    break;
                case D3DERR_INVALIDCALL:
                    std::cout << "  ~ Response: D3DERR_INVALIDCALL" << std::endl;
                    break;
                default:
                    std::cout << "  ~ Response: UNKNOWN" << std::endl;
                    break;
            }

            if (SUCCEEDED(status)) {
                char pattern[] = "\0\0\0\0";
                if (vCache.Pattern == 0) {
                    pattern[0] = '0';
                } else {
                    for (UINT i = 0; i < 4; i++) {
                        pattern[i] = char(vCache.Pattern >> i * 8);
                    }
                }

                std::cout << format("  ~ Pattern: ", pattern) << std::endl;
                std::cout << format("  ~ OptMethod: ", vCache.OptMethod) << std::endl;
                std::cout << format("  ~ CacheSize: ", vCache.CacheSize) << std::endl;
                std::cout << format("  ~ MagicNumber: ", vCache.MagicNumber) << std::endl;
            }
        }

        void startTests() {
            m_totalTests  = 0;
            m_passedTests = 0;

            std::cout << std::endl << "Running D3D8 tests:" << std::endl;
        }

        // GetBackBuffer test (this shouldn't fail even with BackBufferCount set to 0)
        void testZeroBackBufferCount() {
            resetOrRecreateDevice();

            Com<IDirect3DSurface8> bbSurface;

            m_totalTests++;

            HRESULT status = m_device->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &bbSurface);
            if (FAILED(status)) {
                std::cout << "  - The GetBackBuffer test has failed" << std::endl;
            } else {
                m_passedTests++;
                std::cout << "  + The GetBackBuffer test has passed" << std::endl;
            }
        }

        // BeginScene & Reset test
        void testBeginSceneReset() {
            resetOrRecreateDevice();

            m_totalTests++;

            if (SUCCEEDED(m_device->BeginScene())) {
                HRESULT status = m_device->Reset(&m_pp);
                if (FAILED(status)) {
                    std::cout << "  - The BeginScene & Reset test has failed on Reset()" << std::endl;
                }
                else {
                    // Reset() should have cleared the state so this should work properly
                    if (SUCCEEDED(m_device->BeginScene())) {
                        m_passedTests++;
                        std::cout << "  + The BeginScene & Reset test has passed" << std::endl;
                    } else {
                        std::cout << "  - The BeginScene & Reset test has failed" << std::endl;
                    }
                }
            } else {
                throw Error("Failed to begin D3D8 scene");
            }

            // this call is expected fail in certain scenarios, but it makes no difference
            m_device->EndScene();
        }

        // SWVP Render State test (games like Massive Assault try to enable SWVP in PUREDEVICE mode)
        void testPureDeviceSetSWVPRenderState() {
            HRESULT status = createDeviceWithFlags(&m_pp, D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_PUREDEVICE, false);

            if (FAILED(status)) {
                std::cout << "  ~ The PUREDEVICE mode is not supported" << std::endl;
            } else {
                m_totalTests++;

                status = m_device->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, TRUE);
                if (FAILED(status)) {
                    std::cout << "  - The SWVP RS in PUREDEVICE mode test has failed" << std::endl;
                } else {
                    m_passedTests++;
                    std::cout << "  + The SWVP RS in PUREDEVICE mode test has passed" << std::endl;
                }
            }
        }

        // D3DPOOL_DEFAULT allocation & Reset + D3DERR_DEVICENOTRESET state test (2 in 1)
        void testDefaultPoolAllocationReset() {
            resetOrRecreateDevice();

            // create a temporary DS surface
            Com<IDirect3DSurface8> tempDS;
            m_device->CreateDepthStencilSurface(RGBTriangle::WINDOW_WIDTH, RGBTriangle::WINDOW_HEIGHT, 
                                                D3DFMT_D24X8, D3DMULTISAMPLE_NONE, &tempDS);

            m_totalTests++;
            // according to D3D8 docs, I quote: "Reset will fail unless the application releases all resources 
            // that are allocated in D3DPOOL_DEFAULT, including those created by the IDirect3DDevice8::CreateRenderTarget 
            // and IDirect3DDevice8::CreateDepthStencilSurface methods.", so this call should fail
            HRESULT status = m_device->Reset(&m_pp);
            if (FAILED(status)) {
                m_passedTests++;
                std::cout << "  + The D3DPOOL_DEFAULT allocation & Reset test has passed" << std::endl;

                m_totalTests++;
                // check to see if the device state is D3DERR_DEVICENOTRESET
                status = m_device->TestCooperativeLevel();
                if (status == D3DERR_DEVICENOTRESET) {
                    m_passedTests++;
                    std::cout << "  + The D3DERR_DEVICENOTRESET state test has passed" << std::endl;
                } else {
                    std::cout << "  - The D3DERR_DEVICENOTRESET state test has failed" << std::endl;
                }

            } else {
                std::cout << "  - The D3DPOOL_DEFAULT allocation & Reset test has failed" << std::endl;
                std::cout << "  ~ The D3DERR_DEVICENOTRESET state test did not run" << std::endl;
            }
        }

        // CreateStateBlock & Reset test
        void testCreateStateBlockAndReset() {
            resetOrRecreateDevice();

            // create a temporary state block
            DWORD stateBlockToken;
            m_device->CreateStateBlock(D3DSBT_ALL, &stateBlockToken);

            m_totalTests++;
            // D3D8 state blocks survive device Reset() calls and shouldn't be counted as losable resources
            HRESULT status = m_device->Reset(&m_pp);
            if (FAILED(status)) {
                std::cout << "  - The CreateStateBlock & Reset test has failed" << std::endl;
            } else {
                m_passedTests++;
                std::cout << "  + The CreateStateBlock & Reset test has passed" << std::endl;
            }

            m_device->DeleteStateBlock(stateBlockToken);
        }

        // CreateStateBlock monotonic tokens test
        void testCreateStateBlockMonotonicTokens(UINT stateBlocksCount) {
            resetOrRecreateDevice();

            std::vector<DWORD> stateBlockTokens;
            DWORD currentStateBlockToken;
            for (UINT i = 0; i < stateBlocksCount; i++) {
                // this shouldn't ever fail...
                m_device->CreateStateBlock(D3DSBT_ALL, &currentStateBlockToken);
                //std::cout << format("  * State block token:", currentStateBlockToken) << std::endl;
                stateBlockTokens.emplace_back(currentStateBlockToken);
            }

            m_totalTests++;

            bool monotonicStreak = true;
            // the initial value of the token should be 1, as in the previous test
            // we've also created a state block (with token 0), and device resets
            // in D3D8 don't affect state blocks/tokens in any meaningful way
            DWORD monotonicCounter = 1;
            for (auto& stateBlockToken : stateBlockTokens) {
                // subsequent tokens simply increment the initial value
                if (monotonicCounter == stateBlockToken) {
                    monotonicCounter += 1;
                    // release state blocks of checked tokens
                    m_device->DeleteStateBlock(stateBlockToken);
                }
                else {
                    monotonicStreak = false;
                    break;
                }
            }

            if (monotonicStreak) {
                m_passedTests++;
                std::cout << "  + The CreateStateBlock monotonic tokens test has passed" << std::endl;
            } else {
                std::cout << "  - The CreateStateBlock monotonic tokens test has failed" << std::endl;
            }
        }

        // CopyRects with depth stencil format test
        void testCopyRectsDepthStencilFormat() {
            resetOrRecreateDevice();

            Com<IDirect3DSurface8> sourceSurface;
            Com<IDirect3DSurface8> destinationSurface;

            m_device->CreateDepthStencilSurface(256, 256, D3DFMT_D16, D3DMULTISAMPLE_NONE, &sourceSurface);
            m_device->CreateDepthStencilSurface(256, 256, D3DFMT_D16, D3DMULTISAMPLE_NONE, &destinationSurface);

            m_totalTests++;
            // CopyRects does not handle surfaces with depth stencil formats, so this should fail
            HRESULT status = m_device->CopyRects(sourceSurface.ptr(), NULL, 0, destinationSurface.ptr(), NULL);
            if (FAILED(status)) {
                m_passedTests++;
                std::cout << "  + The CopyRects with depth stencil test has passed" << std::endl;
            } else {
                std::cout << "  - The CopyRects with depth stencil test has failed" << std::endl;
            }
        }

        // VCache query result test
        void testVCacheQueryResult() {
            resetOrRecreateDevice();

            D3DDEVINFO_VCACHE vCache;

            m_totalTests++;
            // shouldn't fail on any vendor (native AMD/Intel return S_FALSE, native Nvidia returns D3D_OK)
            HRESULT status = m_device->GetInfo(D3DDEVINFOID_VCACHE, &vCache, sizeof(D3DDEVINFO_VCACHE));
            if (FAILED(status)) {
                std::cout << "  - The VCache query response test has failed" << std::endl;
            } else {
                m_passedTests++;
                std::cout << "  + The VCache query response test has passed" << std::endl;
            }
        }

        // D3D Device capabilities tests
        void testDeviceCapabilities() {
            createDeviceWithFlags(&m_pp, D3DCREATE_SOFTWARE_VERTEXPROCESSING, true);

            D3DCAPS8 caps8SWVP;
            D3DCAPS8 caps8;

            m_device->GetDeviceCaps(&caps8SWVP);
            m_d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps8);

            std::cout << "Running device capabilities tests:" << std::endl;

            m_totalTests++;
            // Some D3D8 UE2.x games only enable character shadows if this capability is supported
            if (caps8.RasterCaps & D3DPRASTERCAPS_ZBIAS) {
                std::cout << "  + The D3DPRASTERCAPS_ZBIAS test has passed" << std::endl;
                m_passedTests++;
            } else {
                std::cout << "  - The D3DPRASTERCAPS_ZBIAS test has failed" << std::endl;
            }

            m_totalTests++;
            // MaxVertexBlendMatrixIndex should be 0 when queried from the D3D8 interface
            // and 255 when queried from the device in SWVP mode
            if (caps8.MaxVertexBlendMatrixIndex == 0u && caps8SWVP.MaxVertexBlendMatrixIndex == 255u) {
                std::cout << format("  + The MaxVertexBlendMatrixIndex INTF and SWVP test has passed (", caps8.MaxVertexBlendMatrixIndex, 
                                    ", ", caps8SWVP.MaxVertexBlendMatrixIndex, ")") << std::endl;
                m_passedTests++;
            } else {
                std::cout << format("  - The MaxVertexBlendMatrixIndex INTF and SWVP test has failed (", caps8.MaxVertexBlendMatrixIndex, 
                                    ", ", caps8SWVP.MaxVertexBlendMatrixIndex, ")") << std::endl;
            }

            m_totalTests++;
            // VS 1.1 is the latest version supported in D3D8
            UINT majorVSVersion = static_cast<UINT>((caps8.VertexShaderVersion & 0x0000FF00) >> 8);
            UINT minorVSVersion = static_cast<UINT>(caps8.VertexShaderVersion & 0x000000FF);
            if (majorVSVersion == 1u && minorVSVersion <= 1u) {
                std::cout << format("  + The VertexShaderVersion test has passed (", majorVSVersion, ".", minorVSVersion, ")") << std::endl;
                m_passedTests++;
            } else {
                std::cout << format("  - The VertexShaderVersion test has failed (", majorVSVersion, ".", minorVSVersion, ")") << std::endl;
            }

            m_totalTests++;
            // typically 256 and should not go above that in D3D8
            if (static_cast<UINT>(caps8.MaxVertexShaderConst) <= 256u) {
                std::cout << format("  + The MaxVertexShaderConst test has passed (", caps8.MaxVertexShaderConst, ")") << std::endl;
                m_passedTests++;
            } else {
                std::cout << format("  - The MaxVertexShaderConst test has failed (", caps8.MaxVertexShaderConst, ")") << std::endl;
            }
            
            m_totalTests++;
            // PS 1.4 is the latest version supported in D3D8
            UINT majorPSVersion = static_cast<UINT>((caps8.PixelShaderVersion & 0x0000FF00) >> 8);
            UINT minorPSVersion = static_cast<UINT>(caps8.PixelShaderVersion & 0x000000FF);
            if (majorVSVersion == 1u && minorVSVersion <= 4u) {
                std::cout << format("  + The PixelShaderVersion test has passed (", majorPSVersion, ".", minorPSVersion, ")") << std::endl;
                m_passedTests++;
            } else {
                std::cout << format("  - The PixelShaderVersion test has failed (", majorPSVersion, ".", minorPSVersion, ")") << std::endl;
            }
        }

        // Various surface format tests
        void testSurfaceFormats() {
            resetOrRecreateDevice();

            std::map<D3DFORMAT, char const*> sfFormats = { {D3DFMT_R8G8B8, "D3DFMT_R8G8B8"}, 
                                                           {D3DFMT_R3G3B2, "D3DFMT_R3G3B2"}, 
                                                           {D3DFMT_A8R3G3B2, "D3DFMT_A8R3G3B2"}, 
                                                           {D3DFMT_A8P8, "D3DFMT_A8P8"}, 
                                                           {D3DFMT_P8, "D3DFMT_P8"},
                                                           {D3DFMT_L6V5U5, "D3DFMT_L6V5U5"}, 
                                                           {D3DFMT_X8L8V8U8, "D3DFMT_X8L8V8U8"},
                                                           {D3DFMT_A2W10V10U10, "D3DFMT_A2W10V10U10"} };

            std::map<D3DFORMAT, char const*>::iterator sfFormatIter;

            Com<IDirect3DSurface8> surface;
            Com<IDirect3DTexture8> texture;

            // Note: CreateImageSurface calls should never fail, even with unsupported surface formats
            std::cout << "Running surface format tests:" << std::endl;

            for (sfFormatIter = sfFormats.begin(); sfFormatIter != sfFormats.end(); sfFormatIter++) {
                D3DFORMAT surfaceFormat = sfFormatIter->first;

                m_totalTests++;
                // D3DFMT_R8G8B8 is used in Hidden & Dangerous Deluxe;
                // D3DFMT_P8 (8-bit palleted textures) are required by some early d3d8 games
                HRESULT status = m_device->CreateImageSurface(256, 256, surfaceFormat, &surface);

                if (FAILED(status)) {
                    std::cout << format("  - The CreateImageSurface with ", sfFormatIter->second, " test has failed") << std::endl;
                } else {
                    m_passedTests++;
                    std::cout << format("  + The CreateImageSurface with ", sfFormatIter->second, " test has passed") << std::endl;
                }

                // D3DFMT_L6V5U5 is used in Star Wars: Republic Commando for bump mapping;
                // LotR: Fellowship of the Ring atempts to create a D3DFMT_P8 texture directly
                status = m_device->CreateTexture(256, 256, 1, 0, surfaceFormat, D3DPOOL_DEFAULT, &texture);

                if (FAILED(status)) {
                    std::cout << format("  ~ The ", sfFormatIter->second, " format is not supported by CreateTexture") << std::endl;
                } else {
                    m_totalTests++;
                    m_passedTests++;
                    std::cout << format("  + The CreateTexture with ", sfFormatIter->second, " test has passed") << std::endl;
                }
            }
        }

        // Depth Stencil format tests
        void testDepthStencilFormats() {
            resetOrRecreateDevice();

            HRESULT status;
            D3DPRESENT_PARAMETERS dsPP;

            memcpy(&dsPP, &m_pp, sizeof(m_pp));
            dsPP.EnableAutoDepthStencil = TRUE;

            std::map<D3DFORMAT, char const*> dsFormats = { {D3DFMT_D16_LOCKABLE, "D3DFMT_D16_LOCKABLE"}, 
                                                           {D3DFMT_D32, "D3DFMT_D32"}, 
                                                           {D3DFMT_D15S1, "D3DFMT_D15S1"}, 
                                                           {D3DFMT_D24S8, "D3DFMT_D24S8"}, 
                                                           {D3DFMT_D16, "D3DFMT_D16"},
                                                           {D3DFMT_D24X8, "D3DFMT_D24X8"}, 
                                                           {D3DFMT_D24X4S4, "D3DFMT_D24X4S4"} };

            std::map<D3DFORMAT, char const*>::iterator dsFormatIter;

            std::cout << "Running depth stencil format tests:" << std::endl;
            
            for (dsFormatIter = dsFormats.begin(); dsFormatIter != dsFormats.end(); dsFormatIter++) {
                dsPP.AutoDepthStencilFormat = dsFormatIter->first;

                status = m_d3d->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                                       dsPP.BackBufferFormat, dsPP.BackBufferFormat,
                                                       dsPP.AutoDepthStencilFormat);
                if (FAILED(status)) {
                    std::cout << format("  ~ The ", dsFormatIter->second, " format is not supported") << std::endl;
                } else {
                    m_totalTests++;

                    status = createDeviceWithFlags(&dsPP, D3DCREATE_HARDWARE_VERTEXPROCESSING, false);
                    if (FAILED(status)) {
                        std::cout << format("  - The ", dsFormatIter->second, " format test has failed") << std::endl;
                    } else {
                        m_passedTests++;
                        std::cout << format("  + The ", dsFormatIter->second, " format test has passed") << std::endl;
                    }
                }
            }
        }

        void printTestResults() {
            std::cout << format("Passed ", m_passedTests, "/", m_totalTests, " tests") << std::endl;
        }

        void prepare() {
            createDeviceWithFlags(&m_pp, D3DCREATE_HARDWARE_VERTEXPROCESSING, true);

            // don't need any of these for 2D rendering
            HRESULT status = m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
            if (FAILED(status))
                throw Error("Failed to set D3D8 render state for D3DRS_ZENABLE");
            status = m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            if (FAILED(status))
                throw Error("Failed to set D3D8 render state for D3DRS_CULLMODE");
            status = m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
            if (FAILED(status))
                throw Error("Failed to set D3D8 render state for D3DRS_LIGHTING");

            // Vertex Buffer
            void* vertices = nullptr;
            
            // tailored for 800 x 800 and the appearance of being centered
            std::array<RGBVERTEX, 3> rgbVertices = {{
                {100.0f, 675.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255, 0, 0),},
                {400.0f, 75.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 255, 0),},
                {700.0f, 675.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 0, 255),},
            }};
            const size_t rgbVerticesSize = rgbVertices.size() * sizeof(RGBVERTEX);

            status = m_device->CreateVertexBuffer(rgbVerticesSize, 0, RGBT_FVF_CODES,
                                                  D3DPOOL_DEFAULT, &m_vb);
            if (FAILED(status))
                throw Error("Failed to create D3D8 vertex buffer");

            status = m_vb->Lock(0, rgbVerticesSize, (BYTE**)&vertices, 0);
            if (FAILED(status))
                throw Error("Failed to lock D3D8 vertex buffer");
            memcpy(vertices, rgbVertices.data(), rgbVerticesSize);
            status = m_vb->Unlock();
            if (FAILED(status))
                throw Error("Failed to unlock D3D8 vertex buffer");
        }

        void render() {
            HRESULT status = m_device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
            if (FAILED(status))
                throw Error("Failed to clear D3D8 viewport");
            if (SUCCEEDED(m_device->BeginScene())) {
                status = m_device->SetStreamSource(0, m_vb.ptr(), sizeof(RGBVERTEX));
                if (FAILED(status))
                    throw Error("Failed to set D3D8 stream source");
                status = m_device->SetVertexShader(RGBT_FVF_CODES);
                if (FAILED(status))
                    throw Error("Failed to set D3D8 vertex shader");
                status = m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
                if (FAILED(status))
                    throw Error("Failed to draw D3D8 triangle list");
                if (SUCCEEDED(m_device->EndScene())) {
                    status = m_device->Present(NULL, NULL, NULL, NULL);
                    if (FAILED(status))
                        throw Error("Failed to present");
                } else {
                    throw Error("Failed to end D3D8 scene");
                }
            } else {
                throw Error("Failed to begin D3D8 scene");
            }
        }
    
    private:

        HRESULT createDeviceWithFlags(D3DPRESENT_PARAMETERS* presentParams, 
                                      DWORD behaviorFlags, 
                                      bool throwErrorOnFail) {
            if (m_d3d == nullptr)
                throw Error("The D3D8 interface hasn't been initialized");

            if (m_device != nullptr)
                m_device->Release();

            HRESULT status = m_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
                                                 behaviorFlags, presentParams, &m_device);
            if (throwErrorOnFail && FAILED(status))
                throw Error("Failed to create D3D8 device");

            return status;
        }

        HRESULT resetOrRecreateDevice() {
            if (m_d3d == nullptr)
                throw Error("The D3D8 interface hasn't been initialized");

            HRESULT status = D3D_OK;

            // return early if the call to Reset() works
            if (m_device != nullptr) {
                if(SUCCEEDED(m_device->Reset(&m_pp)))
                    return status;
                // prepare to clear the device otherwise
                m_device->Release();
            }

            status = m_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
                                         D3DCREATE_HARDWARE_VERTEXPROCESSING, 
                                         &m_pp, &m_device);
            if (FAILED(status))
                throw Error("Failed to create D3D8 device");

            return status;
        }

        HWND                          m_hWnd;

        Com<IDirect3D8>               m_d3d;
        Com<IDirect3DDevice8>         m_device;
        Com<IDirect3DVertexBuffer8>   m_vb;
        
        D3DPRESENT_PARAMETERS         m_pp;

        UINT                          m_totalTests;
        UINT                          m_passedTests;
};

LRESULT WINAPI WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch(message) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int main(int, char**) {
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L,
                     GetModuleHandle(NULL), NULL, LoadCursor(nullptr, IDC_ARROW), NULL, NULL,
                     "D3D8_Triangle", NULL};
    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow("D3D8_Triangle", "D3D8 Triangle - Blisto Retro Testing Edition", 
                              WS_OVERLAPPEDWINDOW, 50, 50, 
                              RGBTriangle::WINDOW_WIDTH, RGBTriangle::WINDOW_HEIGHT, 
                              GetDesktopWindow(), NULL, wc.hInstance, NULL);

    MSG msg;

    try {
        RGBTriangle rgbTriangle(hWnd);

        // list various D3D Device stats
        rgbTriangle.listAdapterDisplayModes();
        rgbTriangle.listBackBufferFormats(FALSE);
        rgbTriangle.listBackBufferFormats(TRUE);
        rgbTriangle.listDeviceCapabilities();
        rgbTriangle.listDeviceD3D9Capabilities();
        rgbTriangle.listVCacheQueryResult();
        
        // run D3D Device tests
        rgbTriangle.startTests();
        rgbTriangle.testZeroBackBufferCount();
        rgbTriangle.testBeginSceneReset();
        rgbTriangle.testPureDeviceSetSWVPRenderState();
        rgbTriangle.testDefaultPoolAllocationReset();
        rgbTriangle.testCreateStateBlockAndReset();
        rgbTriangle.testCreateStateBlockMonotonicTokens(100);
        rgbTriangle.testCopyRectsDepthStencilFormat();
        rgbTriangle.testVCacheQueryResult();
        rgbTriangle.testDeviceCapabilities();
        rgbTriangle.testSurfaceFormats();
        rgbTriangle.testDepthStencilFormats();
        rgbTriangle.printTestResults();

        // D3D8 triangle
        rgbTriangle.prepare();

        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);

        while (true) {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                
                if (msg.message == WM_QUIT) {
                    UnregisterClass("D3D8_Triangle", wc.hInstance);
                    return msg.wParam;
                }
            } else {
                rgbTriangle.render();
            }
        }

    } catch (const Error& e) {
        std::cerr << e.message() << std::endl;
        UnregisterClass("D3D8_Triangle", wc.hInstance);
        return msg.wParam;
    }
}
