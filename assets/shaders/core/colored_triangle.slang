struct Vertex {
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv0;
    float2 uv1;
    float4 color;
    uint   boneIndices[2];   // 4 x uint16 = 8 bytes (padding por ahora)
    float  boneWeights[4];
};                           // 96 bytes: static_assert(sizeof(Vertex) == 96) en C++

struct SceneData {
    float4x4 viewProj;
    float4   camPos;
};

struct PushConstants {
    float4x4    model;
    Vertex*     vertices;
    SceneData*  scene;
    uint baseColorTex;
    uint normalTex;
    uint metallicRoughnessTex;
    uint emissiveTex;
    uint occlusionTex;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

struct VSOut {
    float4 pos    : SV_Position;
    float3 normal : NORMAL;
    float2 uv     : TEXCOORD0;
};
[[vk::binding(0, 0)]] Sampler2D textures[];


[shader("vertex")]
VSOut vertexMain(uint vid : SV_VertexID) {
    Vertex v = pc.vertices[vid];

    float4 world = mul(pc.model, float4(v.position, 1.0));

    VSOut o;
    o.uv = v.uv0;
    o.pos    = mul(pc.scene->viewProj, world);
    o.normal = mul((float3x3)pc.model, v.normal);   // vale mientras no haya escala no uniforme
    return o;
}

[shader("fragment")]
float4 fragmentMain(VSOut i) : SV_Target {
    uint idx = pc.baseColorTex;
    if      (i.uv.x < 0.2) idx = pc.baseColorTex;
    else if (i.uv.x < 0.4) idx = pc.normalTex;
    else if (i.uv.x < 0.6) idx = pc.metallicRoughnessTex;
    else if (i.uv.x < 0.8) idx = pc.emissiveTex;
    else                   idx = pc.occlusionTex;
    return textures[NonUniformResourceIndex(idx)].Sample(i.uv);
}