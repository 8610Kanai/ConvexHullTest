#pragma once

#include <vector>
#include "DX9.hpp"
#include "LineSegment.hpp"

struct Face;
struct Edge;

class ConvexHull
{
public:

	ConvexHull(IDirect3DVertexBuffer9* vertexBuffer);
	~ConvexHull();


private:


	bool GetVerticesFromBuffer(IDirect3DVertexBuffer9* vertexBuffer, std::vector<D3DXVECTOR3>* pOutVertices);

	// ‚Ú‚Â
	//void GiftWrapping();

	void QuickHull();

public:
	void Render();

private:

	std::vector<D3DXVECTOR3> vertices;

	std::vector<Face> faces;
	std::unique_ptr<LineSegment> line;

	std::vector<D3DXVECTOR3> temps;

};

struct Face
{
	// a -> b -> c : ccw
	D3DXVECTOR3 a, b, c;

	bool EqualTo(const Face& face) const
	{
		if (this->a == face.a && this->b == face.b && this->c == face.c) return true;
		else if (this->a == face.b && this->b == face.c && this->c == face.a) return true;
		else if (this->a == face.c && this->b == face.a && this->c == face.b) return true;

		return false;
	}
};

struct Edge
{
	D3DXVECTOR3 start, end;

	bool EqualTo(const Edge& edge) const
	{
		return
		(
			(this->start == edge.start && this->end == edge.end)
			||
			(this->start == edge.end && this->end == edge.start)
		);
	}
};