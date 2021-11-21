#include "DX9.hpp"

std::unique_ptr<DX9> DX9::instance;

DX9::DX9()
    : pDirect3D(nullptr), pDevice(nullptr)
{
    DX9::instance.reset(this);
}

DX9::~DX9()
{
    SAFE_RELEASE(this->pDevice);
    SAFE_RELEASE(this->pDirect3D);

    OUTPUT_DEBUG_FUNCNAME;
}

bool DX9::Init(const HWND* hWnd)
{
    ///////////////////////////////////////////////////////
    ///// initialize directx9

    this->pDirect3D = Direct3DCreate9(D3D_SDK_VERSION);

    if (!pDirect3D) return false;

    D3DPRESENT_PARAMETERS d3dpp =
    {
        .BackBufferWidth            = myApp::SCREEN_WIDTH,
        .BackBufferHeight           = myApp::SCREEN_HEIGHT,
        .BackBufferFormat           = D3DFMT_UNKNOWN,
        .BackBufferCount            = 1,
        .MultiSampleType            = D3DMULTISAMPLE_NONE,
        .MultiSampleQuality         = 0,
        .SwapEffect                 = D3DSWAPEFFECT_DISCARD,
        .hDeviceWindow              = *hWnd,
        .Windowed                   = true,
        .EnableAutoDepthStencil     = true,
        .AutoDepthStencilFormat     = D3DFMT_D24S8,
        .Flags                      = 0,
        .FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT,
        .PresentationInterval       = D3DPRESENT_INTERVAL_DEFAULT,
    };

    // hard x hard
    if (FAILED(this->pDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, *hWnd, 
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &this->pDevice)))
    {
        // hard x soft
        if (FAILED(this->pDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, *hWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &this->pDevice)))
        {
            // soft x hard
            if (FAILED(this->pDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, *hWnd,
                D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &this->pDevice)))
            {
                // soft x soft
                if (FAILED(this->pDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, *hWnd,
                    D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &this->pDevice)))
                {
                    // failed

                    OutputDebugString("Failed to Init DirectX\n");

                    return false;
                }
            }
        }
    }

    return true;
}