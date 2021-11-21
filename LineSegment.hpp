#pragma once
#include "DX9.hpp"


struct LineSegmentVertex
{
	float x, y, z;
	float nx, ny, nz;
	float color;

	static const DWORD FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
};

class LineSegment
{
public:
	// ctor dtor
	
	LineSegment();
	~LineSegment();

private:
	void Init();


public:
	void SetStartEnd(const D3DXVECTOR3* start, const D3DXVECTOR3* end);

	void Render();

	void SetMaterial(D3DMATERIAL9 mat);


private:

	D3DXVECTOR3 start, end;

	D3DXVECTOR3 offset, scaling;

	D3DXMATRIX  worldMatrix, rotationMatrix;

	D3DMATERIAL9 material;
	
	IDirect3DVertexBuffer9* vertexBuffer;

};
