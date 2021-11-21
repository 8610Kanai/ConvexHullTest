#pragma once

#include "utils.hpp"
#include "const.hpp"

class DX9
{
public:
	static std::unique_ptr<DX9> instance;

public:

	DX9();
	~DX9();

	bool Init(const HWND* hWnd);

public:

	IDirect3D9* pDirect3D;
	IDirect3DDevice9* pDevice;

};


