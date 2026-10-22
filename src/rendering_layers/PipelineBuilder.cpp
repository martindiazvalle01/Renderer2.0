#include "PipelineBuilder.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_core.h>

void PipelineBuilder::clear()
{
    // Restablece a 0 todas las estructuras que necesitamos con su tipo correcto de dato

    inputAssembly.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;

    rasterizer.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;

    colorBlendAttachment = vk::PipelineColorBlendAttachmentState{};

    multisampling.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;

    pipelineLayout = vk::PipelineLayout{};

    depthStencil.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;

    renderInfo.sType = vk::StructureType::ePipelineRenderingCreateInfo;

    shaderStages.clear();
}

vk::raii::Pipeline PipelineBuilder::buildPipeline(const vk::raii::Device& device){
    //Creamos el estado del viewport de nuestro viewport y de las tijeras
    // Por el momento solo se permitira un viewport y una tijera

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
    viewportState.pNext = nullptr;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // setup del color bending, no estamos usando objetos transparentes aun

    vk::PipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
    colorBlending.pNext = nullptr;

    colorBlending.logicOpEnable = vk::False;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // borramos completamente VertexInputStateCreateInfo ya que no lo necesitamos
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;

    // Construimos la pipeline real
    
    vk::GraphicsPipelineCreateInfo pipelineInfo;

    pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
    pipelineInfo.pNext = &renderInfo;
    pipelineInfo.stageCount = (uint32_t)shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = pipelineLayout;

    //Seteamos el estado dinamico
    vk::DynamicState state[] = {vk::DynamicState::eViewport,vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamicInfo;
    dynamicInfo.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
    dynamicInfo.pDynamicStates = &state[0];
    dynamicInfo.dynamicStateCount = 2;

    pipelineInfo.pDynamicState = &dynamicInfo;


    auto newPipeline = device.createGraphicsPipeline(VK_NULL_HANDLE,pipelineInfo);

    


    return newPipeline;

    
}

// TODO: Creo que esto se podria mejorar teniendo un struct y una funcion que rellene el struct con el archivo de ambos shader,
//  para que asi se les pueda colocar nombres diferentes a cada shader, servira para debuggear
// PipelineBuilder.cpp
void PipelineBuilder::setShaders(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader,
                                 const char* vertexEntry, const char* fragmentEntry)
{
    shaderStages.clear();

    vk::PipelineShaderStageCreateInfo vertStage{};
    vertStage.setStage(vk::ShaderStageFlagBits::eVertex);
    vertStage.setModule(vertexShader);
    vertStage.setPName(vertexEntry);

    vk::PipelineShaderStageCreateInfo fragStage{};
    fragStage.setStage(vk::ShaderStageFlagBits::eFragment);
    fragStage.setModule(fragmentShader);
    fragStage.setPName(fragmentEntry);

    shaderStages.push_back(vertStage);
    shaderStages.push_back(fragStage);
}


void PipelineBuilder::setInputTopology(vk::PrimitiveTopology topology){
    inputAssembly.topology = topology;
    //TODO: Revisar esto de primitive restart, no lo explican en el tutorial

    inputAssembly.primitiveRestartEnable = vk::False;
}


void PipelineBuilder::setPolygonMode(vk::PolygonMode mode){
    rasterizer.polygonMode = mode;
    rasterizer.lineWidth = 1.f;
}

void PipelineBuilder::setCullMode(vk::CullModeFlags cullMode, vk::FrontFace frontFace){
    rasterizer.cullMode = cullMode;
    rasterizer.frontFace = frontFace;
}

void PipelineBuilder::setMultisamplingNone(){

    multisampling.sampleShadingEnable = vk::False;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = vk::False;
    multisampling.alphaToOneEnable = vk::False;
}

void PipelineBuilder::disableBending(){
    //write mask en default
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB 
                                        | vk::ColorComponentFlagBits::eA;
  
    colorBlendAttachment.blendEnable = vk::False;
}

void PipelineBuilder::setColorAttachmentFormat(vk::Format format){
    colorAttachmentFormat = format;
    // Conecta el formato a la estructura renderInfo

    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachmentFormats = &colorAttachmentFormat;
}

void PipelineBuilder::setDepthFormat(vk::Format format){
    renderInfo.depthAttachmentFormat = format;
}

void PipelineBuilder::disableDepthTest(){
    depthStencil.depthBoundsTestEnable = vk::False;
    depthStencil.depthWriteEnable = vk::False;
    depthStencil.depthCompareOp = vk::CompareOp::eNever;
    depthStencil.depthBoundsTestEnable = vk::False;
    depthStencil.stencilTestEnable = vk::False;

    depthStencil.front = vk::StencilOpState{};
    depthStencil.back = vk::StencilOpState{};
    depthStencil.minDepthBounds = 0.f;
    depthStencil.maxDepthBounds = 1.f;

}