#include "Camera.hpp"
#include "DX9.hpp"

Camera::Camera(const HWND* hWnd)
    : eye(0, 3, -3), at(0, 0, 0), up(0, 1, 0), view(), proj()
    , mousePos(), preMousePos(), cameraSpeed(0.025f)
{
	D3DXMatrixLookAtLH(&this->view, &this->eye, &this->at, &this->up);
	DX9::instance->pDevice->SetTransform(D3DTS_VIEW, &this->view);

	D3DXMatrixPerspectiveFovLH(&this->proj, D3DX_PI * 0.25f, (float)myApp::SCREEN_WIDTH / (float)myApp::SCREEN_HEIGHT, 0.01f, 1000.0f);
	DX9::instance->pDevice->SetTransform(D3DTS_PROJECTION, &this->proj);


    GetCursorPos(&this->mousePos);
    ScreenToClient(*hWnd, &this->mousePos);
    this->preMousePos = this->mousePos;
}

Camera::~Camera()
{}

void Camera::MoveFPS(const HWND* hWnd)
{
    D3DXVECTOR3 mouseDirection =
    {
        static_cast<float>(this->mousePos.x - this->preMousePos.x),
        -static_cast<float>(this->mousePos.y - this->preMousePos.y),
        0.0f
    };

    D3DXVECTOR3 lookVec(this->view._13, this->view._23, this->view._33);

    this->at += D3DXVECTOR3(lookVec.z * mouseDirection.x, mouseDirection.y, -lookVec.x * mouseDirection.x) * 0.05f;

    auto MoveCameraBody = [&lookVec, this](float right, float up, float forward)
    {
        D3DXVECTOR3 moveRightVec(lookVec.z * right, 0, -lookVec.x * right);
        D3DXVECTOR3 moveUpVec(0, up, 0);

        D3DXVECTOR3 moveVec = forward * lookVec + moveRightVec + moveUpVec;
        this->eye += moveVec;
        this->at  += moveVec;
    };

    if(GetKeyState('W') < 0) MoveCameraBody(0, 0, this->cameraSpeed);
    if(GetKeyState('S') < 0) MoveCameraBody(0, 0, -this->cameraSpeed);
    if(GetKeyState('D') < 0) MoveCameraBody( this->cameraSpeed, 0, 0);
    if(GetKeyState('A') < 0) MoveCameraBody(-this->cameraSpeed, 0, 0);
    if(GetKeyState('E') < 0) MoveCameraBody(0,  this->cameraSpeed, 0);
    if(GetKeyState('Q') < 0) MoveCameraBody(0, -this->cameraSpeed, 0);

    D3DXMatrixLookAtLH(&this->view, &this->eye, &this->at, &this->up);
    DX9::instance->pDevice->SetTransform(D3DTS_VIEW, &view);

    this->preMousePos = this->mousePos;

    GetCursorPos(&this->mousePos);
    ScreenToClient(*hWnd, &this->mousePos);
}