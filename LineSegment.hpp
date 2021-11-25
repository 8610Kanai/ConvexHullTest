#pragma once
#include "DX9.hpp"


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
