#pragma once

#include <vector>
#include "DX9.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

struct Face;

class ConvexHull
{
public:

	ConvexHull(IDirect3DVertexBuffer9* vertexBuffer);
	~ConvexHull();


private:

	// fetch vertices from vertexBuffer
	bool GetVerticesFromBuffer(IDirect3DVertexBuffer9* vertexBuffer, std::vector<D3DXVECTOR3>* pOutVertices);

	// create convex hull from vertices
	void CreateConvexHull();


public:
	void Render();

private:

	// 
	std::vector<D3DXVECTOR3> vertices;
	std::vector<Face> faces;

	// use draw
	std::unique_ptr<LineSegment> line;
	std::unique_ptr<Point> point;

};

// tri
struct Face
{
	// a -> b -> c : clockwise
	D3DXVECTOR3 a, b, c;

	D3DXVECTOR3 CalcNormal()
	{
		D3DXVECTOR3 ab = b - a;
		D3DXVECTOR3 ca = c -a;
		D3DXVECTOR3 cross(0,0,0);
		D3DXVec3Cross(&cross, &ab, &ca);
		D3DXVec3Normalize(&cross, &cross);
		return cross;
	}
	
	// 戻り値
	// 1. 共有判定
	// 2. 共有する点1
	// 3. 共有する点2
	std::tuple<bool, D3DXVECTOR3, D3DXVECTOR3> IsShareEdge(const Face& face, bool temp) const
	{
		if (this->a == face.a)
		{
			if (this->b == face.b) return {true, this->a, this->b };
			else if (this->b == face.c) return {true, this->a, this->b };
			else if (this->c == face.b) return {true, this->c, this->a };
			else if (this->c == face.c) return {true, this->c, this->a };
		}
		else if (this->a == face.b)
		{
			if (this->b == face.a) return {true, this->a, this->b };
			else if (this->b == face.c) return {true, this->a, this->b };
			else if (this->c == face.a) return {true, this->c, this->a };
			else if (this->c == face.c) return {true, this->c, this->a };
		}
		else if (this->a == face.c)
		{
			if (this->b == face.b) return {true, this->a, this->b };
			else if (this->b == face.a) return {true, this->a, this->b };
			else if (this->c == face.b) return {true, this->c, this->a };
			else if (this->c == face.a) return {true, this->c, this->a };
		}
		else if(this->c == face.a || this->c == face.b || this->c == face.c)
		{
			if (this->b == face.a)      return {true, this->b, this->c };
			else if (this->b == face.b) return {true, this->b, this->c };
			else if (this->b == face.c) return {true, this->b, this->c };
		}

		return { false, {0,0,0}, {0,0,0} };
	}
};