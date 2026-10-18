#pragma once
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

//Diseño de la pipeline builder sacado de VKGuide

class PipelineBuilder{

    public: 

        PipelineBuilder(){clear();}
        vk::Pipeline buildPipeline(vk::Device& device);
        void setShaders(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader);
        void setInputTopology(vk::PrimitiveTopology topology);
        void setPolygonMode(vk::PolygonMode mode);
        void setCullMode(vk::CullModeFlags cullMode, vk::FrontFace frontFace);
        //TODO: Cuando el Renderer ya sea usable hay que hacer que se pueda activar el multisampling
        void setMultisamplingNone();
        void disableBending();
        void setColorAttachmentFormat(vk::Format format);
        void setDepthFormat(vk::Format format);
        void disableDepthTest();

    private: 
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        vk::PipelineRasterizationStateCreateInfo rasterizer;
        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        vk::PipelineMultisampleStateCreateInfo multisampling;
        vk::PipelineLayout pipelineLayout;
        vk::PipelineDepthStencilStateCreateInfo depthStencil;
        vk::PipelineRenderingCreateInfo renderInfo;
        vk::Format colorAttachmentFormat;

        void clear();

};