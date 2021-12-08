#pragma once

#include "utils.hpp"
#include "const.hpp"

// (small tetrahedron)
class Point
{
public:
	Point();

	~Point();

	void Init();

	void SetLocation(float x, float y, float z);

	D3DXVECTOR3 GetLocation() const
	{
		return D3DXVECTOR3
		(
			this->worldMatrix._41,
			this->worldMatrix._42,
			this->worldMatrix._43
		);
	};

	void Render();

private:

	D3DXMATRIX worldMatrix;
	D3DMATERIAL9 material;

	IDirect3DVertexBuffer9* vertexBuffer;
	IDirect3DIndexBuffer9* indexBuffer;
};