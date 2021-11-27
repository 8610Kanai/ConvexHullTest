#pragma once

#include <compare>
#include <vector>
#include <thread>
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
	bool GetVerticesFromBuffer(IDirect3DVertexBuffer9* vertexBuffer);

	// create convex hull from vertices
	bool CreateConvexHull();


public:
	void Render();

private:

	// 
	std::vector<D3DXVECTOR3> origineVertices;
	std::vector<Face> faces;
	std::vector<Face> temps;

	// use draw
	std::unique_ptr<LineSegment> line;
	std::unique_ptr<Point> point;

	std::thread createTask;

	// creation completed?
	bool isCompleted;

};

// tri
struct Face
{
	// a -> b -> c : clockwise
	D3DXVECTOR3 a, b, c;

	// ñ@ê¸
	D3DXVECTOR3 CalcNormal()
	{
		D3DXVECTOR3 ab = b - a;
		D3DXVECTOR3 ca = c -a;
		D3DXVECTOR3 cross(0,0,0);
		D3DXVec3Cross(&cross, &ab, &ca);
		D3DXVec3Normalize(&cross, &cross);
		return cross;
	}

	//std::strong_ordering operator <=> (const Face&) const = default;

	bool operator == (const Face& face_) const
	{
		if (this->a == face_.a && this->b == face_.b && this->c == face_.c) return true;
		if (this->a == face_.b && this->b == face_.c && this->c == face_.a) return true;
		if (this->a == face_.c && this->b == face_.a && this->c == face_.b) return true;
		return false;
	}

	bool operator != (const Face& face_) const
	{
		return !(*this == face_);
	}
	
	// return :
	// 1. sharing?
	// 2. sharing point 1
	// 3. sharing point 2
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