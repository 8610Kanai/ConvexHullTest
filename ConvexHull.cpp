#include "ConvexHull.hpp"

#include <algorithm>
#include <stack>
#include <queue>
#include <chrono>

#include "CustomVertex.hpp"

ConvexHull::ConvexHull(IDirect3DVertexBuffer9* vertexBuffer)
	: vertices(), faces()
    , line(std::make_unique<LineSegment>())
    , point(std::make_unique<Point>())
{
    this->GetVerticesFromBuffer(vertexBuffer, &this->vertices);
    
    auto start = std::chrono::system_clock::now();
    this->CreateConvexHull();
    OutputDebugFormat("\n\n elapsed : {} ms.", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count());
}

ConvexHull::~ConvexHull()
{
    OUTPUT_DEBUG_FUNCNAME;
}

bool ConvexHull::GetVerticesFromBuffer(IDirect3DVertexBuffer9* pInVertexBuffer, std::vector<D3DXVECTOR3>* pOutVertices)
{
    if (!pInVertexBuffer) return false;

    // 頂点バッファーの情報を取得
    D3DVERTEXBUFFER_DESC desc = {};
    pInVertexBuffer->GetDesc(&desc);

    // 頂点データのレイアウト
    D3DVERTEXELEMENT9 elm[MAX_FVF_DECL_SIZE];
    D3DXDeclaratorFromFVF(desc.FVF, elm);

    // 座標情報(D3DFVF_XYZ)までのオフセット値 取得
    WORD offset = 0;
    for (int i = 0; i < D3DXGetDeclLength(elm); ++i)
    {
        if (elm[i].Type == D3DDECLTYPE_FLOAT3 && elm[i].Usage == D3DDECLUSAGE_POSITION)
        {
            offset = elm[i].Offset;
            break;
        }
    }
    // FVFからCustomVertexのサイズを取得
    unsigned vertexSize = D3DXGetFVFVertexSize(desc.FVF);
    unsigned verticesSize = desc.Size;
    unsigned vertexNum = verticesSize / vertexSize;

    void* ppbdata;
    if (SUCCEEDED(pInVertexBuffer->Lock(0, verticesSize, &ppbdata, 0)))
    {
        // 領域確保
        pOutVertices->reserve(vertexNum);

        D3DXVECTOR3 tempVertex;

        // ポインタ移動用
        BYTE* ppb = reinterpret_cast<BYTE*>(ppbdata);

        for (int i = 0; i < vertexNum; ++i)
        {
            memcpy(&tempVertex, ((ppb)+offset), sizeof(tempVertex));
            pOutVertices->push_back(tempVertex);

            // ポインタを移動
            ppb += vertexSize;
        }

        pInVertexBuffer->Unlock();
    }
    else return false;

    OutputDebugFormat("\n v num : {}", pOutVertices->size());

    return true;
}

void ConvexHull::CreateConvexHull()
{
    if (this->vertices.size() < 4) return;

    ///////////////////////////////////////////////////////////
    //// function objects

    // 四面体の符号付き体積
    auto CalcSignedTetrahedronVolume = [](const D3DXVECTOR3& a, const D3DXVECTOR3& b, const D3DXVECTOR3& c, const D3DXVECTOR3& d)
    {
        D3DXVECTOR3 ab = b - a;
        D3DXVECTOR3 ca = c - a;
        D3DXVECTOR3 ad = d - a;
        D3DXVECTOR3 cross(0, 0, 0);
        D3DXVec3Cross(&cross, &ab, &ca);

        return (/*abs*/(D3DXVec3Dot(&cross, &ad)) / 6.0f);
    };

    // 点郡のうち四面体内の点を除いて返す
    auto RemovePointInsideTetrahedron = [&CalcSignedTetrahedronVolume](const std::vector<D3DXVECTOR3>& points, const D3DXVECTOR3& a, const D3DXVECTOR3& b, const D3DXVECTOR3& c, const D3DXVECTOR3& d)
    {
        std::vector<D3DXVECTOR3> newPoints;
        for (auto& point : points)
        {
            if (point == a || point == b || point == c || point == d) continue;
            float v  = abs( CalcSignedTetrahedronVolume(a, b, c, d) );
            float v1 = abs( CalcSignedTetrahedronVolume(a, b, c, point) );
            float v2 = abs( CalcSignedTetrahedronVolume(d, b, a, point) );
            float v3 = abs( CalcSignedTetrahedronVolume(d, c, b, point) );
            float v4 = abs( CalcSignedTetrahedronVolume(d, a, c, point) );

            if ((v1 + v2 + v3 + v4) - v > 0)
            {
                newPoints.push_back(point);
            }
        }
        return newPoints;
    };

    // 点郡のうち三角形面の上側で最も遠い点
    // 戻り値 : 最遠点 (上側にない場合 { FLT_MIN, FLT_MIN, FLT_MIN } )
    auto CalcFurthestPoint = [&CalcSignedTetrahedronVolume](const std::vector<D3DXVECTOR3>& points, const Face& face)
    {
        //OutputDebugFormat("\n pointnum : {}", points.size());
        float maxSignedVolume = -FLT_EPSILON;
        D3DXVECTOR3 farPoint(0, 0, 0);
        for (auto& point : points)
        {
            if (point == face.a || point == face.b || point == face.c) continue;
            float signedVolume = CalcSignedTetrahedronVolume(face.a, face.b, face.c, point);

            if (signedVolume > maxSignedVolume)
            {
                maxSignedVolume = signedVolume;
                farPoint = point;
            }
        }

        if (maxSignedVolume < 0)
        {
            farPoint = { FLT_MAX, FLT_MAX, FLT_MAX };
        }

        return farPoint;
    };

    // 点郡のうち面の上側の点を返す
    auto GetPointsUpperSideOfFace = [&CalcSignedTetrahedronVolume](const std::vector<D3DXVECTOR3>& points, const Face& face)
    {
        std::vector<D3DXVECTOR3> newPoints;
        for (auto& point : points)
        {
            if (point == face.a || point == face.b || point == face.c) continue;

            if (0 <= CalcSignedTetrahedronVolume(face.a, face.b, face.c, point))
                newPoints.push_back(point);
        }

        return newPoints;
    };


    ///////////////////////////////////////////////////////////
    //// first tetrahedron

    D3DXVECTOR3 min = this->vertices.front();
    D3DXVECTOR3 max = min;

    for (size_t i = 1; i < this->vertices.size(); ++i)
    {
        if ((this->vertices[i].x < min.x)
            && this->vertices[i].y < min.y 
            && this->vertices[i].z < min.z)
        {
            min = this->vertices[i];
        }

        if ((this->vertices[i].x > max.x)
            && this->vertices[i].y > max.y
            && this->vertices[i].z > max.z)
        {
            max = this->vertices[i];
        }
    }

    float maxLenSq = FLT_MIN;
    D3DXVECTOR3 far1(0, 0, 0);
    for (auto& vertex : this->vertices)
    {
        D3DXVECTOR3 vec1 = min - max;
        D3DXVec3Normalize(&vec1, &vec1);
        D3DXVECTOR3 vec2 = vertex - max;
        float vec1Len = D3DXVec3Dot(&vec1, &vec2);
        float vec2LenSq = D3DXVec3LengthSq(&vec2);
        float lenSq = vec2LenSq - vec1Len * vec1Len;
        if (lenSq > maxLenSq)
        {
            maxLenSq = lenSq;
            far1 = vertex;
        }
    }

    float maxSignedVolume = FLT_MIN;
    D3DXVECTOR3 far2(0,0,0);
    for (auto& vertex : this->vertices)
    {
        float signedVolume = CalcSignedTetrahedronVolume(min, max, far1, vertex);
        if (abs(signedVolume) > abs(maxSignedVolume))
        {
            maxSignedVolume = signedVolume;
            far2 = vertex;
        }
    }
    if (maxSignedVolume > 0)
    {
        std::swap(min, max);
    }

    OutputDebugFormat("\n     min : {:.2f}, {:.2f}, {:.2f}", min.x, min.y, min.z);
    OutputDebugFormat("\n     max : {:.2f}, {:.2f}, {:.2f}", max.x, max.y, max.z);
    OutputDebugFormat("\n     far1 : {:.2f}, {:.2f}, {:.2f}", far1.x, far1.y, far1.z);
    OutputDebugFormat("\n     far2 : {:.2f}, {:.2f}, {:.2f}", far2.x, far2.y, far2.z);

    // 四面体内の点の除去
    this->vertices = RemovePointInsideTetrahedron(this->vertices, min, max, far1, far2);


    this->faces.push_back({ min, max, far1 } );
    this->faces.push_back({ max, min, far2 } );
    this->faces.push_back({ far1, max, far2 });
    this->faces.push_back({ far1, far2, min });

    std::queue<Face> queueFaces;
    queueFaces.push({ min, max, far1 } );
    queueFaces.push({ max, min, far2 } );
    queueFaces.push({ far1, max, far2 });
    queueFaces.push({ far1, far2, min });

    ///////////////////////////////////////////////////////////
    //// loop

    while (!this->vertices.empty() && !queueFaces.empty())
    {
        Face face = queueFaces.front();
        queueFaces.pop();

        std::vector<D3DXVECTOR3> upperSidePoints = GetPointsUpperSideOfFace(this->vertices, face);
        auto furthest = CalcFurthestPoint(upperSidePoints, face);
        if (furthest == D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX)) continue;

        std::vector<Face> invisibleFaces;
        std::vector<Face> visibleFaces;

        for (auto& face : this->faces)
        {
            float signedVolume = CalcSignedTetrahedronVolume(face.a, face.b, face.c, furthest);
            if (signedVolume <= 0)
            {
                invisibleFaces.push_back(face);
            }
            else
            {
                visibleFaces.push_back(face);
            }
        }

        this->faces = invisibleFaces;

        for (auto& visibleFace : visibleFaces)
        {
            // 不要な頂点の除外
            this->vertices = RemovePointInsideTetrahedron(this->vertices, visibleFace.a, visibleFace.b, visibleFace.c, furthest);


            for (auto& invisibleFace : invisibleFaces)
            {
                auto [isShared, shareP1, shareP2] = visibleFace.IsShareEdge(invisibleFace, true);
                if (isShared)
                {
                    Face newFace = { shareP1, shareP2, furthest };
                    this->faces.push_back(newFace);
                    queueFaces.push(newFace);
                }
            }
        }
    }


    OutputDebugFormat("\n     vnun :  {}", vertices.size());
}


void ConvexHull::Render()
{
    for (size_t i = 0; i < this->faces.size(); ++i)
    {
        Face face = this->faces[i];

        //if (i == this->faces.size() - 1)
        //{
        //    line->SetMaterial({ .Emissive = {1,1,0} });
        //}
        //else line->SetMaterial({ .Emissive = {1,1,1} });

        line->SetStartEnd(&face.a, &face.b);
        line->Render();
        line->SetStartEnd(&face.b, &face.c);
        line->Render();
        line->SetStartEnd(&face.c, &face.a);
        line->Render();

        //D3DXVECTOR3 center = (face.a + face.b + face.c) / 3.0f;
        //D3DXVECTOR3 end = center + face.CalcNormal() * 0.05f;
        //line->SetStartEnd(&center, &end);
        //line->Render();
        //D3DXVECTOR3 ab = face.b - face.a;
        //D3DXVec3Normalize(&ab, &ab);
        //ab *= 0.01f;
        //ab += end;
        //line->SetStartEnd(&end, &ab);
        //line->Render();
    }

    for (auto& vertex : this->vertices)
    {
        point->SetLocation(vertex.x, vertex.y, vertex.z);
        point->Render();
    }
}