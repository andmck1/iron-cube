#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

static const int VERTEX_LIMIT = 64;
static const int TRIANGLE_LIMIT = 126;

ConstantBuffer<MVPBuffer> mvp;
RWStructuredBuffer<VertexInputType> vertexBuffer;

[[vk::push_constant]]
cbuffer ModelData
{
    uint16_t nVertices;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    out indices uint3 triangles[TRIANGLE_LIMIT],
    out vertices VertexOutputType vertices[VERTEX_LIMIT]
)
{

    float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
    for (int i = 0; i < nVertices; i++)
    {
        VertexInputType vertex = vertexBuffer[i];
        vertices[i].position = mul(mvpMatrix, vertex.position);
        vertices[i].colour = (-1.f * vertexBuffer[i].normal + 1.f) / 2.f;
        vertices[i].colour[3] = 1.f;
    }

    // Ear Clipping
    // For each vertex
    // -> if the angle between the previous and next vertex is <= 90 degrees
    // -> -> if there are no vertices in the formed triangle between these 3
    // vertices
    // -> -> -> this is a valid triangle so add to list, remove vertex and start
    // loop again
    int earClippingVertexIndexList[VERTEX_LIMIT];
    for (int i = 0; i < 64; i++)
    {
        earClippingVertexIndexList[i] = i;
    }

    bool isFinished = false;
    int escapeCounter = 0;
    int loopLimit = 10_000;
    int triangleCounter = 0;
    int nClippedVerticesCounter = 0;
    while (!isFinished)
    {
        // vertex index = global list, clipped index = clipped list
        int currentVertexIndex = 0;
        int currentClippedIndex = 0;

        int previousVertexIndex = 0;
        int previousClippedIndex = 0;

        int nextVertexIndex = 0;
        int nextClippedIndex = 0;

        // For each vertex
        for (currentVertexIndex = 0; currentVertexIndex < nVertices;
             currentVertexIndex++)
        {
            // current clipped index
            currentClippedIndex =
                earClippingVertexIndexList[currentVertexIndex];
            if (currentClippedIndex < 0)
            {
                continue;
            }

            // previous clipped index
            previousVertexIndex = currentVertexIndex - 1;
            if (previousVertexIndex < 0)
            {
                previousVertexIndex = nVertices - 1;
            }
            previousClippedIndex =
                earClippingVertexIndexList[previousVertexIndex];
            while (previousClippedIndex < 0)
            {
                // In theory this can never go below 0
                previousVertexIndex -= 1;
                previousClippedIndex =
                    earClippingVertexIndexList[previousVertexIndex];
            }

            // next clipped index
            nextVertexIndex = currentVertexIndex + 1;
            if (nextVertexIndex > (nVertices - 1))
            {
                nextVertexIndex = 0;
            }
            nextClippedIndex = earClippingVertexIndexList[nextVertexIndex];
            while (nextClippedIndex < 0)
            {
                // In theory this can never go above nVertices-1
                nextVertexIndex += 1;
                nextClippedIndex = earClippingVertexIndexList[nextVertexIndex];
            }

            VertexInputType currentVertex = vertexBuffer[currentClippedIndex];
            VertexInputType previousVertex = vertexBuffer[previousClippedIndex];
            VertexInputType nextVertex = vertexBuffer[nextClippedIndex];

            // Check edges are convex
            if (!isConvex(previousVertex, currentVertex, nextVertex))
            {
                continue;
            }
            // TODO - check if any other vertices inside triangle

            // Add triangle
            triangles[triangleCounter] = uint3(
                previousClippedIndex, currentClippedIndex, nextClippedIndex
            );
            triangleCounter++;
            if (triangleCounter >= TRIANGLE_LIMIT)
            {
                isFinished = true;
            }

            // Remove vertex
            earClippingVertexIndexList[currentVertexIndex] = -1;
            nClippedVerticesCounter++;

            if (nVertices - nClippedVerticesCounter < 3)
            {
                isFinished = true;
            }
            break;
        }

        // Prevent infinite loop
        escapeCounter++;
        if (escapeCounter > loopLimit)
        {
            isFinished = true;
        }
    }

    SetMeshOutputCounts(nVertices, triangleCounter);
}

// angle formed by vertex1->vertex0 and vertex1->vertex2
bool isConvex(
    VertexInputType vertex0, VertexInputType vertex1, VertexInputType vertex2
)
{
    float triangleArea =
        vertex1.position.x * (vertex0.position.y - vertex2.position.y) +
        vertex0.position.x * (vertex2.position.y - vertex1.position.y) +
        vertex2.position.x * (vertex1.position.y - vertex0.position.y);
    // -ve sign for my winding!
    return triangleArea < 0.f;
}
