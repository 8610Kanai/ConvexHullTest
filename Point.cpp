#include "Point.hpp"

#include "DX9.hpp"
#include "CustomVertex.hpp"

Point::Point()
	: worldMatrix(), material({ .Emissive = {1,1,1} })
	, vertexBuffer(nullptr), indexBuffer(nullptr)
{
	D3DXMatrixIdentity(&this->worldMatrix);
	this->Init();
}

Point::~Point()
{
	SAFE_RELEASE(this->indexBuffer);
	SAFE_RELEASE(this->vertexBuffer);
}

void Point::Init()
{
	float a = 0.01f;
	CustomVertex_xyz_normal_diffuse vertices[] =
	{
		{-a,-a,-a,-a,-a,-a,0xffffff},
		{ a,-a, a, a,-a, a,0xffffff},
		{ a, a,-a, a, a,-a,0xffffff},
		{-a, a, a,-a, a, a,0xffffff}
	};
	WORD indices[] =
	{
		0,2,1,
		0,1,3,
		0,3,2,
		1,2,3
	};

	if (SUCCEEDED(DX9::instance->pDevice->CreateVertexBuffer(sizeof(vertices), 0, (CustomVertex_xyz_normal_diffuse::FVF), D3DPOOL_MANAGED, &this->vertexBuffer, 0)))
	{
		void* tempVB;
		if (SUCCEEDED(this->vertexBuffer->Lock(0, sizeof(vertices), &tempVB, 0)))
		{
			memcpy(tempVB, vertices, sizeof(vertices));
			this->vertexBuffer->Unlock();
		}


		if (SUCCEEDED(DX9::instance->pDevice->CreateIndexBuffer(sizeof(indices), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &this->indexBuffer, NULL)))
		{
			void* tempIB;
			if (SUCCEEDED(this->indexBuffer->Lock(0, sizeof(indices), &tempIB, 0)))
			{
				memcpy(tempIB, indices, sizeof(indices));
				this->indexBuffer->Unlock();
			}
		}
	}
}

void Point::SetLocation(float x, float y, float z)
{
	this->worldMatrix._41 = x;
	this->worldMatrix._42 = y;
	this->worldMatrix._43 = z;
}

void Point::Render()
{
	if (this->vertexBuffer && this->indexBuffer)
	{
		DX9::instance->pDevice->SetTransform(D3DTS_WORLD, &this->worldMatrix);
		DX9::instance->pDevice->SetTexture(0, nullptr);
		DX9::instance->pDevice->SetStreamSource(0, this->vertexBuffer, 0, sizeof(CustomVertex_xyz_normal_diffuse));
		DX9::instance->pDevice->SetFVF(CustomVertex_xyz_normal_diffuse::FVF);
		DX9::instance->pDevice->SetIndices(this->indexBuffer);
		DX9::instance->pDevice->SetMaterial(&this->material);
		DX9::instance->pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 4);
	}
}