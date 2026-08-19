module rasterizer;

struct Vertex
{
    float3 position;
    float3 color;
};

struct Camera {
    float4x4 view;
    float4x4 projection;
}

struct Transform {
    float4x4 model;
    float4x4 viewport;
}

struct RenderData {
    StructuredBuffer<Vertex> vertices;
    StructuredBuffer<uint> indices;
    RWTexture2D<float4> colorOutput;
    RWTexture2D<uint> depthOutput;
}

layout(set = 0) ConstantBuffer<Camera> camera;
layout(set = 1) RenderData renderData;
layout(push_constant) Transform transform;

uint packDepth(float ndcDepth)
{
    return ~asuint(ndcDepth * 0.5 + 0.5);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void draw(uint3 id : SV_DispatchThreadID) {
    uint2 size;
    renderData.colorOutput.GetDimensions(size.x, size.y);

    if (id.x >= size.x || id.y >= size.y)
        return;

    float2 pixelCenter = float2(id.xy) + 0.5;
    uint triangle = id.z;

    uint i0 = renderData.indices[triangle * 3 + 0];
    uint i1 = renderData.indices[triangle * 3 + 1];
    uint i2 = renderData.indices[triangle * 3 + 2];

    Vertex v0 = renderData.vertices[i0];
    Vertex v1 = renderData.vertices[i1];
    Vertex v2 = renderData.vertices[i2];

    float4x4 viewProjection = mul(camera.view, camera.projection);
    float4x4 MVP = mul(transform.model, viewProjection);

    float4 clipSpaceP0 = mul(float4(v0.position, 1.0), MVP);
    float4 clipSpaceP1 = mul(float4(v1.position, 1.0), MVP);
    float4 clipSpaceP2 = mul(float4(v2.position, 1.0), MVP);

    if (clipSpaceP0.w <= 0.0 || clipSpaceP1.w <= 0.0 || clipSpaceP2.w <= 0.0)
        return;

    float4 ndcP0 = clipSpaceP0 / clipSpaceP0.w;
    float4 ndcP1 = clipSpaceP1 / clipSpaceP1.w;
    float4 ndcP2 = clipSpaceP2 / clipSpaceP2.w;

    float4 p0 = mul(ndcP0, transform.viewport);
    float4 p1 = mul(ndcP1, transform.viewport);
    float4 p2 = mul(ndcP2, transform.viewport);

    float area = ((p1.x - p0.x) * (p2.y - p0.y)) - ((p2.x - p0.x) * (p1.y - p0.y));
    float w0 = ((p1.x - pixelCenter.x) * (p2.y - pixelCenter.y)) - ((p2.x - pixelCenter.x) * (p1.y - pixelCenter.y));
    float w1 = ((p2.x - pixelCenter.x) * (p0.y - pixelCenter.y)) - ((p0.x - pixelCenter.x) * (p2.y - pixelCenter.y));
    float w2 = ((p0.x - pixelCenter.x) * (p1.y - pixelCenter.y)) - ((p1.x - pixelCenter.x) * (p0.y - pixelCenter.y));

    if (area == 0.0)
        return;

    w0 /= area;
    w1 /= area;
    w2 /= area;

    if (w0 < 0.0 || w1 < 0.0 || w2 < 0.0)
        return;

    float depth = w0 * p0.z + w1 * p1.z + w2 * p2.z;
    uint packed = packDepth(depth);

    float3 color = w0 * v0.color + w1 * v1.color + w2 * v2.color;

    uint previous;
    InterlockedMax(renderData.depthOutput[id.xy], packed, previous);
    if (packed > previous)
    {
        renderData.colorOutput[id.xy] = float4(color, 1.0);
    }
}
