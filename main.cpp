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
    // create window

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
    // init directx9

    (new DX9())->Init(&hWnd);


    ///////////////////////////////////////////////////////
    // init camera

    auto camera = std::make_unique<Camera>(&hWnd);

    ///////////////////////////////////////////////////////
    // init light

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

    // teapot mesh
    ID3DXMesh* teapotMesh;
    HRESULT hr = D3DXCreateTeapot(DX9::instance->pDevice, &teapotMesh, NULL);

    if (FAILED(hr))
    {
        OutputDebugString("D3DXCreateTeapot Error\n");
        return false;
    }

    // teapot mat
    D3DMATERIAL9 teapotMat =
    {
        .Diffuse = {0.5,0.5,0.5},
        .Ambient = {0.2,0.2,0.2}
    };

    // create teapot mesh's convexHull
    IDirect3DVertexBuffer9* vertexBuff;
    teapotMesh->GetVertexBuffer(&vertexBuff);
    auto convexHull = std::make_unique<ConvexHull>(vertexBuff);

    // point
    auto point = std::make_unique<Point>();
    point->SetLocation(1, 0, 0);

    // line
    auto line = std::make_unique<LineSegment>();

    // axes
    D3DXVECTOR3 o(0, 0, 0);
    D3DXVECTOR3 vx(100, 0, 0);
    D3DXVECTOR3 vy(0, 100, 0);
    D3DXVECTOR3 vz(0, 0, 100);

    ///////////////////////////////////////////////////////
    // main loop

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
            DX9::instance->pDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x192f60, 1.0f, 0);
            if (SUCCEEDED(DX9::instance->pDevice->BeginScene()))
            {
                // fps camera
                camera->MoveFPS(&hWnd);

                // axes
                line->SetMaterial({ .Emissive = {1,0,1,1} });
                line->SetStartEnd(&o, &vx);
                line->Render();
                line->SetStartEnd(&o, &vy);
                line->Render();
                line->SetStartEnd(&o, &vz);
                line->Render();
                line->SetMaterial({ .Emissive = {1,1,1,1} });


                convexHull->Render();

                // render teapot when spacebar is not pressed.
                D3DXMATRIX world;
                if (GetKeyState(VK_SPACE) >= 0)
                {
                    D3DXMatrixIdentity(&world);
                    DX9::instance->pDevice->SetTransform(D3DTS_WORLD, &world);
                    DX9::instance->pDevice->SetMaterial(&teapotMat);
                    teapotMesh->DrawSubset(0);
                }
                world._43 = 3;
                DX9::instance->pDevice->SetTransform(D3DTS_WORLD, &world);
                DX9::instance->pDevice->SetMaterial(&teapotMat);
                teapotMesh->DrawSubset(0);


                DX9::instance->pDevice->EndScene();
            }
            DX9::instance->pDevice->Present( NULL, NULL, NULL, NULL );
        }
    }

    SAFE_RELEASE(teapotMesh);

    OutputDebugFormat("\n{} Finished.\n\n", myApp::TITLE_NAME);

    return 0;
}