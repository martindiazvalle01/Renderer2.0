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
    Vertex*     vertices;    // vertexAddress
    SceneData*  scene;       // sceneAddress
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

struct VSOut {
    float4 pos    : SV_Position;
    float3 normal : NORMAL;
};

[shader("vertex")]
VSOut vertexMain(uint vid : SV_VertexID) {
    Vertex v = pc.vertices[vid];

    float4 world = mul(pc.model, float4(v.position, 1.0));

    VSOut o;
    o.pos    = mul(pc.scene->viewProj, world);
    o.normal = mul((float3x3)pc.model, v.normal);   // vale mientras no haya escala no uniforme
    return o;
}

[shader("fragment")]
float4 fragmentMain(VSOut i) : SV_Target {
    float3 n = normalize(i.normal);
    return float4(n * 0.5 + 0.5, 1.0);
}