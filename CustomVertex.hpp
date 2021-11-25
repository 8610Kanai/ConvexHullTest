#pragma once

#include "utils.hpp"

struct CustomVertex_xyz_normal_diffuse
{
	float x, y, z;
	float nx, ny, nz;
	float color;

	static const DWORD FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
};