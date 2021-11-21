#include "ConvexHull.hpp"

#include <algorithm>
#include <stack>
#include <queue>
#include <chrono>

ConvexHull::ConvexHull(IDirect3DVertexBuffer9* vertexBuffer)
	: vertices()
{
    line = std::make_unique<LineSegment>();

    this->GetVerticesFromBuffer(vertexBuffer, &this->vertices);
    
    this->QuickHull();
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

    return true;
}

//
//void ConvexHull::GiftWrapping()
//{
//    if (this->vertices.empty()) return;
//
//    this->vertices.clear();
//    //this->vertices.reserve(8);
//    //this->vertices.push_back({0,0,0});
//    //this->vertices.push_back({1,0,1});
//    //this->vertices.push_back({1,1,0});
//    //this->vertices.push_back({0,1,1});
//
//    //this->vertices.push_back({1,0,0});
//    //this->vertices.push_back({0,1,0});
//    //this->vertices.push_back({0,0,1});
//    //this->vertices.push_back({1,1,1});
//
//    //this->vertices.push_back({2,2,2});
//    //this->vertices.push_back({-2,-2,-2});
//
//    this->vertices.reserve(4);
//    this->vertices.push_back({0,0,0});
//    this->vertices.push_back({1,0,0});
//    this->vertices.push_back({0,1,0});
//    this->vertices.push_back({1,1,0});
//    this->vertices.push_back({0.5f,0.5f,1});
//
//    // 中心
//    D3DXVECTOR3 center(0, 0, 0);
//    for (auto& vertex : this->vertices)
//    {
//        center += vertex;
//    }
//    center /= this->vertices.size();
//
//    // 最初の辺を決める
//    Edge firstEdge =
//    {
//        .start = this->vertices.front()
//    };
//
//    for (size_t i = 1; i < this->vertices.size(); ++i)
//    {
//        if ((firstEdge.start.y > this->vertices[i].y))
//        {
//            firstEdge.start = this->vertices[i];
//        }
//        else if ((firstEdge.start.y == this->vertices[i].y))
//        {
//            if ((firstEdge.start.x > this->vertices[i].x))
//            {
//                firstEdge.start = this->vertices[i];
//            }
//            else if ((firstEdge.start.x == this->vertices[i].x))
//            {
//                if ((firstEdge.start.z > this->vertices[i].z))
//                {
//                    firstEdge.start = this->vertices[i];
//                }
//            }
//        }
//    }
//
//    float minDot = 1;
//    D3DXVECTOR3 toCenter = center - firstEdge.start;
//    D3DXVec3Normalize(&toCenter, &toCenter);
//    for (auto& vertex : this->vertices)
//    {
//        if (vertex != firstEdge.start)
//        {
//            D3DXVECTOR3 vec = vertex - firstEdge.start;
//            D3DXVec3Normalize(&vec, &vec);
//            float dot = abs(D3DXVec3Dot(&vec, &toCenter));
//            if (dot < minDot)
//            {
//                minDot = dot;
//                firstEdge.end = vertex;
//            }
//        }
//    }
//
//    OutputDebugFormat("\n\ncenter {}, {}, {}", center.x, center.y, center.z);
//    OutputDebugFormat("\nstart {}, {}, {}", firstEdge.start.x, firstEdge.start.y, firstEdge.start.z);
//    OutputDebugFormat("\nend   {}, {}, {}\n", firstEdge.end.x, firstEdge.end.y, firstEdge.end.z);
//
//    int loopnum = 0;
//
//    {
//
//
//        std::vector<Edge> processedEdge;
//        std::stack<Edge> stack;
//
//        stack.push(firstEdge);
//
//        while (!stack.empty())
//        {
//            Edge edge = stack.top();
//            stack.pop();
//
//            if (std::any_of(this->edges.begin(), this->edges.end(), [&](const Edge& edge_) { return edge_.EqualTo(edge); }))
//                continue;
//
//            D3DXVECTOR3 targetVertex(0,0,0);
//            for (auto& vertex : this->vertices)
//            {
//                if (vertex == edge.start || vertex == edge.end) continue;
//                targetVertex = vertex;
//                break;
//            }
//            D3DXPLANE targetPlane(0,0,0,0);
//            D3DXPlaneFromPoints(&targetPlane, &edge.end, &edge.start, &targetVertex);
//            D3DXPlaneNormalize(&targetPlane, &targetPlane);
//
//            for (size_t i = 1; i < this->vertices.size(); ++i)
//            {
//                if (this->vertices[i] == edge.start || this->vertices[i] == edge.end || this->vertices[i] == targetVertex) continue;
//
//                OutputDebugFormat("\np1 : {:.2f}, {:.2f}, {:.2f}", edge.start.x, edge.start.y, edge.start.z);
//                OutputDebugFormat("\np2 : {:.2f}, {:.2f}, {:.2f}", edge.end.x, edge.end.y, edge.end.z);
//                OutputDebugFormat("\np3 : {:.2f}, {:.2f}, {:.2f}", targetVertex.x, targetVertex.y, targetVertex.z);
//                OutputDebugFormat("\np4 : {:.2f}, {:.2f}, {:.2f}\n", this->vertices[i].x, this->vertices[i].y, this->vertices[i].z);
//
//                float dot = D3DXPlaneDotCoord(&targetPlane, &this->vertices[i]);
//                if (dot > 0)
//                {
//                    targetVertex = this->vertices[i];
//                    D3DXPlaneFromPoints(&targetPlane, &edge.end, &edge.start, &targetVertex);
//                    D3DXPlaneNormalize(&targetPlane, &targetPlane);
//                }
//                else if (dot == 0)
//                {
//                    //D3DXVECTOR3 vec1 = edge.end - edge.start;
//                    //D3DXVECTOR3 vec2 = targetVertex - edge.start;
//                    //D3DXVECTOR3 normalV1(0, 0, 0);
//                    //D3DXVec3Normalize(&normalV1, &vec1);
//
//                    //D3DXVECTOR3 h1 = vec1 * D3DXVec3Dot(&normalV1, &vec2) / D3DXVec3Length(&vec1) - vec2;
//
//                    //D3DXVECTOR3 vec3 = edge.end - edge.start;
//                    //D3DXVECTOR3 vec4 = this->vertices[i] - edge.start;
//                    //D3DXVECTOR3 normalV3(0, 0, 0);
//                    //D3DXVec3Normalize(&normalV3, &vec3);
//
//                    //D3DXVECTOR3 h2 = vec3 * D3DXVec3Dot(&normalV3, &vec4) / D3DXVec3Length(&vec3) - vec4;
//
//                    ////D3DXVECTOR3 vec1, vec2;
//                    ////D3DXVec3Cross(&vec1, &targetVertex, &edge.start);
//                    ////D3DXVec3Cross(&vec2, &targetVertex, &edge.end);
//                    ////D3DXVECTOR3 vec3, vec4;
//                    ////D3DXVec3Cross(&vec3, &this->vertices[i], &edge.start);
//                    ////D3DXVec3Cross(&vec4, &this->vertices[i], &edge.end);
//
//                    ////if (D3DXVec3Length(&vec1) + D3DXVec3Length(&vec2) < D3DXVec3Length(&vec3) + D3DXVec3Length(&vec4))
//                    //if (D3DXVec3LengthSq(&h1) > D3DXVec3LengthSq(&h2))
//                    //{
//                    //    targetVertex = this->vertices[i];
//                    //    D3DXPlaneFromPoints(&targetPlane, &edge.start, &edge.end, &targetVertex);
//                    //    D3DXPlaneNormalize(&targetPlane, &targetPlane);
//                    //}
//
//
//                    D3DXVECTOR3 vec  = edge.end - edge.start;
//                    D3DXVECTOR3 vec1 = targetVertex - edge.end;
//                    D3DXVECTOR3 vec2 = this->vertices[i] - edge.end;
//
//                    D3DXVec3Normalize(&vec, &vec);
//                    D3DXVec3Normalize(&vec1, &vec1);
//                    D3DXVec3Normalize(&vec2, &vec2);
//
//                    float decl1 = acos(D3DXVec3Dot(&vec, &vec1));
//                    float decl2 = acos(D3DXVec3Dot(&vec, &vec2));
//
//                    if (decl1 < decl2)
//                    {
//                        targetVertex = this->vertices[i];
//                        D3DXPlaneFromPoints(&targetPlane, &edge.end, &edge.start, &targetVertex);
//                        D3DXPlaneNormalize(&targetPlane, &targetPlane);
//                    }
//                    else if (decl1 == decl2)
//                    {
//                        if (D3DXVec3LengthSq(&vec1) < D3DXVec3LengthSq(&vec2))
//                        {
//                            targetVertex = this->vertices[i];
//                            D3DXPlaneFromPoints(&targetPlane, &edge.end, &edge.start, &targetVertex);
//                            D3DXPlaneNormalize(&targetPlane, &targetPlane);
//                        }
//                    }
//                }
//            }
//
//            OutputDebugFormat("for end\n\n");
//
//
//            //Edge newEdge =
//            //{
//            //    .start = edge.end,
//            //    .end = targetVertex
//            //}; 
//            Edge newEdge =
//            {
//                .start = targetVertex,
//                .end = edge.end
//            };
//            stack.push(newEdge);
//
//            //newEdge =
//            //{
//            //    .start = targetVertex,
//            //    .end = edge.start
//            //};
//            newEdge =
//            {
//                .start = edge.start,
//                .end = targetVertex
//            };
//            stack.push(newEdge);
//
//            this->edges.push_back(edge);
//        }
//
//    }
//
//    {
//        //auto findFace = [&, this](auto func, D3DXVECTOR3* start, D3DXVECTOR3* end, D3DXVECTOR3* ignoreVertex, D3DXVECTOR3* preFaceNormal)
//        //{
//        //    float maxDot = FLT_MIN;
//        //    D3DXVECTOR3 faceNormal(0, 0, 0);
//
//        //    Face newFace =
//        //    {
//        //        .a = *start,
//        //        .b = *end,
//        //        .c = {0,0,0},
//        //        .normal = {0,0,0}
//        //    };
//
//        //    for (auto& vertex : this->vertices)
//        //    {
//        //        if (ignoreVertex)
//        //        {
//        //            if (vertex == *ignoreVertex) continue;
//        //        }
//        //        if (vertex == *start || vertex == *end) continue;
//
//        //        D3DXVECTOR3 vec1 = -(vertex - *start);
//        //        D3DXVECTOR3 vec2 = -(vertex - *end);/*
//        //        D3DXVec3Normalize(&vec1, &vec1);
//        //        D3DXVec3Normalize(&vec2, &vec2);*/
//        //        D3DXVec3Cross(&faceNormal, &vec1, &vec2);
//        //        D3DXVec3Normalize(&faceNormal, &faceNormal);
//
//
//        //        float dot = 0;
//        //        if (preFaceNormal)
//        //        {
//        //            dot = (D3DXVec3Dot(preFaceNormal, &faceNormal));
//        //        }
//        //        else // first face
//        //        {
//        //            D3DXVECTOR3 outgoingVec = *start + *end - center * 2;
//        //            D3DXVec3Normalize(&outgoingVec, &outgoingVec);
//        //            dot = (D3DXVec3Dot(&outgoingVec, &faceNormal));
//        //        }
//
//        //        if (dot > maxDot)
//        //        {
//        //            maxDot = dot;
//        //            newFace.c = vertex;
//        //            newFace.normal = faceNormal;
//        //        }
//        //        else if (dot == maxDot)
//        //        {
//        //            D3DXVECTOR3 vec = 2 * vertex - (*start + *end);
//        //            D3DXVECTOR3 vec2 = 2 * newFace.c - (*start + *end);
//
//        //            if (D3DXVec3LengthSq(&vec) > D3DXVec3LengthSq(&vec2))
//        //            {
//        //                newFace.c = vertex;
//        //            }
//        //        }
//        //    }
//
//
//
//        //    if (std::any_of(this->faces.begin(), this->faces.end(), [&](const Face& face_) { return face_.EqualTo(newFace); }))
//        //        return false;
//
//        //    OutputDebugFormat("\nface normal : {:.2f}, {:.2f}, {:.2f}", newFace.normal.x, newFace.normal.y, newFace.normal.z);
//        //    this->faces.push_back(newFace);
//
//        //    func(func, &newFace.b, &newFace.a, &newFace.c, &newFace.normal);
//        //    func(func, &newFace.a, &newFace.c, &newFace.b, &newFace.normal);
//        //    func(func, &newFace.b, &newFace.c, &newFace.a, &newFace.normal);
//
//        //    return true;
//        //};
//
//        //findFace(findFace, &firstEdge.start, &firstEdge.end, nullptr, nullptr);
//    }
//
//    {
//        //auto findEdge = [&, this](auto func, Edge* edge, Edge* preEdge)
//        //{
//        //    ++loopnum;
//        //    if (std::any_of(this->edges.begin(), this->edges.end(), [&](const Edge& edge_) { return edge_.EqualTo(*edge); })) return false;
//
//        //    this->edges.push_back(*edge);
//
//        //    //D3DXVECTOR3 edgeDirection = edge->end - edge->start;
//        //    //D3DXVECTOR3 outgoingDirection = edge->start - center;
//
//        //    //D3DXVec3Normalize(&edgeDirection, &edgeDirection);
//        //    //D3DXVec3Normalize(&outgoingDirection, &outgoingDirection);
//
//        //    //D3DXVECTOR3 cross(0,0,0);
//        //    //D3DXVec3Cross(&cross, &edgeDirection, &outgoingDirection);
//        //    //D3DXVec3Normalize(&cross, &cross);
//
//        //    D3DXVECTOR3 candidateVertex(edge->start);
//        //    float maxDot = FLT_MIN;
//        //    for (auto& vertex : this->vertices)
//        //    {
//        //        if (vertex == edge->start || vertex == edge->end) continue;
//        //        if (preEdge)
//        //        {
//        //            if (vertex == preEdge->start || vertex == preEdge->end) continue;
//
//        //            D3DXVECTOR3 direction = vertex - edge->end;
//        //            D3DXVec3Normalize(&direction, &direction);
//
//        //            D3DXVECTOR3 preDirection = preEdge->end - preEdge->start;
//        //            D3DXVec3Normalize(&preDirection, &preDirection);
//
//        //            float dot = abs(D3DXVec3Dot(&direction, &preDirection));
//        //            if (dot > maxDot)
//        //            {
//        //                candidateVertex = vertex;
//        //                maxDot = dot;
//        //            }
//        //            else if (dot == maxDot)
//        //            {
//        //                D3DXVECTOR3 vec = 2 * vertex - (edge->start + edge->end);
//        //                D3DXVECTOR3 vec2 = 2 * candidateVertex - (edge->start + edge->end);
//
//        //                if (D3DXVec3LengthSq(&vec) > D3DXVec3LengthSq(&vec2))
//        //                {
//        //                    candidateVertex = vertex;
//        //                }
//        //            }
//        //        }
//        //    }
//
//        //    Edge edge1 =
//        //    {
//        //        .start = edge->start,
//        //        .end = candidateVertex
//        //    };
//        //    Edge edge2 =
//        //    {
//        //        .start = edge->end,
//        //        .end = candidateVertex
//        //    };
//
//        //    func(func, &edge1, edge);
//        //    func(func, &edge2, edge);
//
//        //    return true;
//        //};
//
//        //findEdge(findEdge, &firstEdge, &firstEdge);
//    }
//
//    OutputDebugFormat("\n num : {}", this->edges.size());
//    OutputDebugFormat("\nloop num : {}", loopnum);
//}

void ConvexHull::QuickHull()
{
    if (this->vertices.empty()) return;

    //this->vertices.clear();
    //this->vertices.reserve(8);
    //this->vertices.push_back({ 0,0,0 });
    //this->vertices.push_back({ 1,0,1 });
    //this->vertices.push_back({ 1,1,0 });
    //this->vertices.push_back({ 0,1,1 });

    //this->vertices.push_back({ 1,0,0 });
    //this->vertices.push_back({ 0,1,0 });
    //this->vertices.push_back({ 0,0,1 });
    //this->vertices.push_back({ 1,1,1 });

    //this->vertices.push_back({2,2,2});
    //this->vertices.push_back({-2,-2,-2});

    //this->vertices.reserve(4);
    //this->vertices.push_back({0,0,0});
    //this->vertices.push_back({1,0,0});
    //this->vertices.push_back({0,1,0});
    //this->vertices.push_back({1,1,0});
    //this->vertices.push_back({0.5f,0.5f,1});

    //this->vertices.clear();
    //this->vertices.reserve(5);
    //this->vertices.push_back({0,0,0});
    //this->vertices.push_back({1,0,1});
    //this->vertices.push_back({1,0,-1});
    //this->vertices.push_back({0.5f,1,0});
    //this->vertices.push_back({10.5f,-1,0});
    //this->vertices.push_back({0.5f,-2,0});

    //////////////////////////////////////////////
    // 六面体(双三角錐)を決める
    // 赤道面の三角形 -> XminZmin - XmaxZmin - Zmax

    D3DXVECTOR3 xmin_zmin = this->vertices.front();
    D3DXVECTOR3 xmax_zmin = this->vertices.front();
    D3DXVECTOR3 zmax = this->vertices.front();

    for (size_t i = 1; i < this->vertices.size(); ++i)
    {
        if ((this->vertices[i].x < xmin_zmin.x)
            || (this->vertices[i].x == xmin_zmin.x && this->vertices[i].z < xmin_zmin.z))
        {
            xmin_zmin = this->vertices[i];
        }

        if ((this->vertices[i].x > xmax_zmin.x)
            || (this->vertices[i].x == xmax_zmin.x && this->vertices[i].z < xmax_zmin.z))
        {
            xmax_zmin = this->vertices[i];
        }
    }
    for (size_t i = 1; i < this->vertices.size(); ++i)
    {
        if (this->vertices[i] == xmin_zmin || this->vertices[i] == xmax_zmin) continue;
        if (this->vertices[i].z > zmax.z)
        {
            zmax = this->vertices[i];
        }
    }

    //OutputDebugFormat("\n first");
    //OutputDebugFormat("\n {:.2f}, {:.2f}, {:.2f}", xmin_zmin.x, xmin_zmin.y, xmin_zmin.z);
    //OutputDebugFormat("\n {:.2f}, {:.2f}, {:.2f}", xmax_zmin.x, xmax_zmin.y, xmax_zmin.z);
    //OutputDebugFormat("\n {:.2f}, {:.2f}, {:.2f}", zmax.x, zmax.y, zmax.z);


    auto CalcTetrahedronVolume = [](const D3DXVECTOR3 &a, const D3DXVECTOR3 &b, const D3DXVECTOR3 &c, const D3DXVECTOR3 &d)
    {
        D3DXVECTOR3 ab = b - a;
        D3DXVECTOR3 ac = c - a;
        D3DXVECTOR3 ad = d - a;
        D3DXVECTOR3 cross(0, 0, 0);
        D3DXVec3Cross(&cross, &ab, &ac);

        return ( abs( D3DXVec3Dot(&cross, &ad) ) / 6.0f );
    };

    std::vector<Face> processedFaces;
    std::queue<Face> stack_face;
    stack_face.push({ xmin_zmin, xmax_zmin, zmax });
    stack_face.push({ xmax_zmin, xmin_zmin, zmax });
    int loopNum = 0;

    auto start = std::chrono::system_clock::now();

    while (!stack_face.empty())
    {
        ++loopNum;

        if (20 < static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count()))
        {
            OutputDebugFormat("\n\n aaaaaaaaaaaaaaaaaaa\n FAILED.\n aaaaaaaaaaaaaaaaaaaa\n");
            break;
        }

        //auto face = stack_face.top();
        auto face = stack_face.front();
        stack_face.pop();

        OutputDebugFormat("\nfacenum {}", this->faces.size());

        if (std::any_of(processedFaces.begin(), processedFaces.end(), [&](const Face& face_) { return face_.EqualTo(face); }))
        //if (std::any_of(this->faces.begin(), this->faces.end(), [&](const Face& face_) { return face_.EqualTo(face); }))
        {
            OutputDebugFormat("\ncancel");
            OutputDebugFormat("\n     facea : {:.2f}, {:.2f}, {:.2f}", face.a.x, face.a.y, face.a.z);
            OutputDebugFormat("\n     faceb : {:.2f}, {:.2f}, {:.2f}", face.b.x, face.b.y, face.b.z);
            OutputDebugFormat("\n     facec: {:.2f}, {:.2f}, {:.2f}", face.c.x, face.c.y, face.c.z);
            OutputDebugFormat("\n");
            continue;
        }

        processedFaces.push_back(face);

        D3DXPLANE plane(0,0,0,0);
        D3DXVECTOR3 normal(0, 0, 0);
        D3DXVECTOR3 ab = face.b - face.a;
        D3DXVECTOR3 ca = face.a - face.c;
        D3DXVec3Cross(&normal, &ab, &ca);
        D3DXVec3Normalize(&normal, &normal);
        D3DXPlaneFromPointNormal(&plane, &face.a, &normal);
        D3DXPlaneNormalize(&plane, &plane);


        OutputDebugFormat("\n         facea : {:.2f}, {:.2f}, {:.2f}", face.a.x, face.a.y, face.a.z);
        OutputDebugFormat("\n         faceb : {:.2f}, {:.2f}, {:.2f}", face.b.x, face.b.y, face.b.z);
        OutputDebugFormat("\n         facec: {:.2f}, {:.2f}, {:.2f}", face.c.x, face.c.y, face.c.z);
        OutputDebugFormat("\n      normal: {:.2f}, {:.2f}, {:.2f}", normal.x, normal.y, normal.z);

        D3DXVECTOR3 farthest(FLT_MAX, FLT_MAX, FLT_MAX);
        float maxDot = 0;

        for (size_t i = 0; i < this->vertices.size(); ++i)
        {
            if (this->vertices[i] == face.a || this->vertices[i] == face.b || this->vertices[i] == face.c)  continue;
            float dot = D3DXPlaneDotCoord(&plane, &this->vertices[i]);
            OutputDebugFormat("\ndot : {}", dot);
            if (dot > maxDot)
            {
                maxDot = dot;
                farthest = this->vertices[i];
            }
            ++loopNum;
        }
        //OutputDebugFormat("\n");
        OutputDebugFormat("\n         farth : {:.2f}, {:.2f}, {:.2f}", farthest.x, farthest.y, farthest.z);

        if (farthest == D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX))
        {
            OutputDebugFormat("\n  add face");
            this->faces.push_back(face);
        }
        else
        {
            // 点の除外
            std::vector<D3DXVECTOR3> newVertices;

            for (auto& vertex : this->vertices)
            {
                if (vertex == face.a || vertex == face.b || vertex == face.c)
                {
                    newVertices.push_back(vertex);
                    continue;
                }
                if (vertex == farthest)
                {
                    newVertices.push_back(vertex);
                    continue;
                }
                float v  = CalcTetrahedronVolume(farthest, face.a, face.b, face.c);
                float v1 = CalcTetrahedronVolume(face.a, face.b, face.c, vertex);
                float v2 = CalcTetrahedronVolume(farthest, face.a, face.b, vertex);
                float v3 = CalcTetrahedronVolume(farthest, face.b, face.c, vertex);
                float v4 = CalcTetrahedronVolume(farthest, face.c, face.a, vertex);

                OutputDebugFormat("\n v {}", (v1 + v2 + v3 + v4) - v);
                OutputDebugFormat("\n         targe : {:.2f}, {:.2f}, {:.2f}", vertex.x, vertex.y, vertex.z);

                if ((v1 + v2 + v3 + v4) - v > FLT_EPSILON)
                {
                    OutputDebugFormat("\n aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
                    newVertices.push_back(vertex);
                }
                ++loopNum;
            }
            this->vertices.swap(newVertices);

            stack_face.push({face.a, face.b, farthest});
            stack_face.push({face.b, face.c, farthest});
            stack_face.push({face.c, face.a, farthest});
        }
    }

    OutputDebugFormat("\n\nloopnum {}", loopNum);
    OutputDebugFormat("\n\nelapsed {}", static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count()));
    OutputDebugFormat("\nfacenum {}", this->faces.size());
    OutputDebugFormat("\n nokori  {}", this->vertices.size());

    for (auto& vertex : this->vertices)
    {
        OutputDebugFormat("\n nokori : {:.2f}, {:.2f}, {:.2f}", vertex.x, vertex.y, vertex.z);
    }

    this->faces;

    this->temps.reserve(this->faces.size());
    for (auto& face : this->faces)
    {
        D3DXVECTOR3 ab = face.b - face.a;
        D3DXVECTOR3 ca = face.a - face.c;
        D3DXVECTOR3 cross(0, 0, 0);
        D3DXVec3Cross(&cross, &ab, &ca);
        D3DXVec3Normalize(&cross, &cross);
        cross *= 0.25f;
        this->temps.push_back(cross);
    }

}

void ConvexHull::Render()
{
    for (size_t i = 0; i < this->faces.size(); ++i)
    {
        Face face = this->faces[i];

        line->SetStartEnd(&face.a, &face.b);
        line->Render();
        line->SetStartEnd(&face.b, &face.c);
        line->Render();
        line->SetStartEnd(&face.c, &face.a);
        line->Render();

        //D3DXVECTOR3 center = (face.a + face.b + face.c) / 3.0f;
        //D3DXVECTOR3 end = center + this->temps[i];
        //line->SetStartEnd(&center, &end);
        //line->Render();
    }
}