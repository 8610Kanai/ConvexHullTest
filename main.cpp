#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <Windows.h>

#include "utils.hpp"
#include "const.hpp"
#include "DX9.hpp"
#include "LineSegment.hpp"
#include "ConvexHull.hpp"
#include "Camera.hpp"
#include "Point.hpp"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_DESTROY:
        PostQuitMessage(0);

        break;

    case WM_KEYDOWN:
        if(wParam == VK_ESCAPE) PostQuitMessage(0);

        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    ///////////////////////////////////////////////////////
    ///// create window

    WNDCLASSEX wcex =
    {
        .cbSize        = sizeof(WNDCLASSEX),
        .style         = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc   = WndProc,
        .cbClsExtra    = 0,
        .cbWndExtra    = 0,
        .hInstance     = hInstance,
        .hIcon         = nullptr,
        .hCursor       = nullptr,
        .hbrBackground = nullptr,
        .lpszMenuName  = nullptr,
        .lpszClassName = myApp::WINDOW_NAME,
        .hIconSm       = nullptr
    };

    if (!RegisterClassEx(&wcex)) return 0;

    HWND hWnd = {};

    hWnd = CreateWindow
    (
        myApp::WINDOW_NAME,
        myApp::TITLE_NAME,
        WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME),
        320, 180,
        myApp::SCREEN_WIDTH, myApp::SCREEN_HEIGHT,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hWnd) return 0;

    ShowWindow(hWnd, nCmdShow);


    ///////////////////////////////////////////////////////
    ///// init directx9

    (new DX9())->Init(&hWnd);


    ///////////////////////////////////////////////////////
    ///// init camera

    auto camera = std::make_unique<Camera>(&hWnd);

    ///////////////////////////////////////////////////////
    ///// init light

    D3DLIGHT9 light =
    {
        .Type      = D3DLIGHT_DIRECTIONAL,
        .Diffuse   = { 1,1,1 },
        .Ambient   = { 0.35f,0.35f,0.35f },
        .Direction = { -1, -1, -1 },
    };
    DX9::instance->pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    DX9::instance->pDevice->SetLight(0, &light);
    DX9::instance->pDevice->LightEnable(0, TRUE);

    ///////////////////////////////////////////////////////
    // test

    ID3DXBuffer* materials;
    DWORD numMaterials;
    ID3DXMesh* mesh;
    HRESULT hr = D3DXLoadMeshFromX
    (
        "res/tree.x",
        D3DXMESH_MANAGED,
        DX9::instance->pDevice,
        NULL,
        &materials,
        NULL,
        &numMaterials,
        &mesh
    );

    if (FAILED(hr))
    {
        OutputDebugString("XModel::Init() Error\n");
        return false;
    }

    IDirect3DVertexBuffer9* vertexBuff;
    mesh->GetVertexBuffer(&vertexBuff);
    auto convexHull = std::make_unique<ConvexHull>(vertexBuff);


    ///////////////////////////////////////////////////////
    ///// main loop

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            camera->MoveFPS(&hWnd);

            DX9::instance->pDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x192f60, 1.0f, 0);
            if (SUCCEEDED(DX9::instance->pDevice->BeginScene()))
            {
                convexHull->Render();

                if (GetKeyState(VK_SPACE) < 0)
                {
                    D3DXMATRIX world;
                    D3DXMatrixIdentity(&world);
                    world._41 = 0;
                    DX9::instance->pDevice->SetTransform(D3DTS_WORLD, &world);
                    for (int i = 0; i < numMaterials; ++i)
                    {
                        D3DXMATERIAL mat = ((D3DXMATERIAL*)(materials->GetBufferPointer()))[i];
                        DX9::instance->pDevice->SetMaterial(&(mat.MatD3D));
                        mesh->DrawSubset(i);
                    }
                }

                DX9::instance->pDevice->EndScene();
            }
            DX9::instance->pDevice->Present( NULL, NULL, NULL, NULL );
        }
    }

    SAFE_RELEASE(vertexBuff);
    SAFE_RELEASE(mesh);
    SAFE_RELEASE(materials);

    OutputDebugFormat("\n{} Finished.\n\n", myApp::TITLE_NAME);

    return 0;
}