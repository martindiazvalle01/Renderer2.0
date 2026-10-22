// colored_triangle.slang
// Versión Slang de colored_triangle.vert + colored_triangle.frag
// (ambas etapas en un solo archivo, con dos entry points)

// Lo que el vertex shader le pasa al fragment shader
struct VSOutput
{
    float4 position : SV_Position;                 // equivale a gl_Position
    [[vk::location(0)]] float3 color : COLOR0;     // equivale a layout(location = 0) out/in vec3
};

// Posiciones de los 3 vértices del triángulo
static const float3 positions[3] = {
    float3( 1.0,  1.0, 0.0),
    float3(-1.0,  1.0, 0.0),
    float3( 0.0, -1.0, 0.0)
};

// Colores de los 3 vértices
static const float3 colors[3] = {
    float3(1.0, 0.0, 0.0), // rojo
    float3(0.0, 1.0, 0.0), // verde
    float3(0.0, 0.0, 1.0)  // azul
};

[shader("vertex")]
VSOutput vertexMain(uint vertexIndex : SV_VulkanVertexID) // equivale a gl_VertexIndex
{
    VSOutput output;
    output.position = float4(positions[vertexIndex], 1.0);
    output.color    = colors[vertexIndex];
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target // equivale a layout(location = 0) out vec4
{
    return float4(input.color, 1.0);
}
