#include "LineSegment.hpp"

#include "CustomVertex.hpp"

LineSegment::LineSegment()
	: start({ 0,0,0 }), end({ 0,0,0 })
	, offset({ 0,0,0 }), scaling({ 0,0,0 })
	, worldMatrix(), rotationMatrix()
	, material({ .Emissive = {1,1,1} }), vertexBuffer(nullptr)
{
	D3DXMatrixIdentity(&this->worldMatrix);
	D3DXMatrixIdentity(&this->rotationMatrix);

	this->Init();
}

LineSegment::~LineSegment()
{
	SAFE_RELEASE(this->vertexBuffer);

	OUTPUT_DEBUG_FUNCNAME;
}

void LineSegment::Init()
{
	CustomVertex_xyz_normal_diffuse vertices[2] =
	{
		{0, 0, 0,  0, 1, 0,  0xffffff},
		{1, 0, 0,  0, 1, 0,  0xffffff}
	};

	if (SUCCEEDED(DX9::instance->pDevice->CreateVertexBuffer(sizeof(vertices), 0, (CustomVertex_xyz_normal_diffuse::FVF), D3DPOOL_MANAGED, &this->vertexBuffer, 0)))
	{
		void* tempVB;
		if (SUCCEEDED(this->vertexBuffer->Lock(0, sizeof(vertices), &tempVB, 0)))
		{
			memcpy(tempVB, vertices, sizeof(vertices));
			this->vertexBuffer->Unlock();

			return;
		}
	}
}

void LineSegment::SetStartEnd(const D3DXVECTOR3* start, const D3DXVECTOR3* end)
{
	D3DXVECTOR3 direction = *end - *start;

	this->offset = *start;
	this->scaling = { D3DXVec3Length(&direction),1,1 };

	D3DXVECTOR3 normalizedDirection = {};
	D3DXVec3Normalize(&normalizedDirection, &direction);

	D3DXVECTOR3 baseDirection(1,0,0);
	D3DXVECTOR3 rotationAxis(0,0,0);
	D3DXVec3Cross(&rotationAxis, &baseDirection, &normalizedDirection);
	if (D3DXVec3LengthSq(&rotationAxis) <= FLT_EPSILON) rotationAxis = { 0,0,1 };

	float radian = acosf(D3DXVec3Dot(&baseDirection, &normalizedDirection));

	D3DXQUATERNION tempQuat(0,0,0,1);
	D3DXQuaternionRotationAxis(&tempQuat, &rotationAxis, radian);
	D3DXMatrixRotationQuaternion(&this->rotationMatrix, &tempQuat);

	D3DXMATRIX scaleMatrix;
	D3DXMatrixScaling(&scaleMatrix, this->scaling.x, this->scaling.y, this->scaling.z);
	this->worldMatrix = scaleMatrix * rotationMatrix;
	this->worldMatrix._41 = this->offset.x;
	this->worldMatrix._42 = this->offset.y;
	this->worldMatrix._43 = this->offset.z;
}

void LineSegment::Render()
{
	DX9::instance->pDevice->SetTransform(D3DTS_WORLD, &this->worldMatrix);
	DX9::instance->pDevice->SetTexture(0, nullptr);
	DX9::instance->pDevice->SetStreamSource(0, this->vertexBuffer, 0, sizeof(CustomVertex_xyz_normal_diffuse));
	DX9::instance->pDevice->SetFVF(CustomVertex_xyz_normal_diffuse::FVF);
	DX9::instance->pDevice->SetMaterial(&this->material);
	DX9::instance->pDevice->DrawPrimitive(D3DPT_LINELIST, 0, 2);
}

void LineSegment::SetMaterial(D3DMATERIAL9 mat)
{
	this->material = mat;
}