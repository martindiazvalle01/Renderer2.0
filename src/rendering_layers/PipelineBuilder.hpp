#pragma once
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

//Diseño de la pipeline builder sacado de VKGuide

class PipelineBuilder{

    public: 

        PipelineBuilder(){clear();}
        vk::raii::Pipeline buildPipeline(const vk::raii::Device& device);
        void setShaders(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader,
                                 const char* vertexEntry, const char* fragmentEntry);
        void setInputTopology(vk::PrimitiveTopology topology);
        void setPolygonMode(vk::PolygonMode mode);
        void setCullMode(vk::CullModeFlags cullMode, vk::FrontFace frontFace);
        //TODO: Cuando el Renderer ya sea usable hay que hacer que se pueda activar el multisampling
        void setMultisamplingNone();
        void disableBending();
        void setColorAttachmentFormat(vk::Format format);
        void setDepthFormat(vk::Format format);
        void disableDepthTest();

        //Parte hecha con IA (Me dio flojera hacer los setters)
        // --- Layout (necesario) ---
        void setPipelineLayout(vk::PipelineLayout layout) { pipelineLayout = layout; }
        vk::PipelineLayout getPipelineLayout() const { return pipelineLayout; }

        // --- Shader stages ---
        void setShaderStages(const std::vector<vk::PipelineShaderStageCreateInfo>& stages) { shaderStages = stages; }
        const std::vector<vk::PipelineShaderStageCreateInfo>& getShaderStages() const { return shaderStages; }

        // --- Input assembly ---
        void setInputAssembly(const vk::PipelineInputAssemblyStateCreateInfo& v) { inputAssembly = v; }
        const vk::PipelineInputAssemblyStateCreateInfo& getInputAssembly() const { return inputAssembly; }

        // --- Rasterizer ---
        void setRasterizer(const vk::PipelineRasterizationStateCreateInfo& v) { rasterizer = v; }
        const vk::PipelineRasterizationStateCreateInfo& getRasterizer() const { return rasterizer; }

        // --- Color blend attachment ---
        void setColorBlendAttachment(const vk::PipelineColorBlendAttachmentState& v) { colorBlendAttachment = v; }
        const vk::PipelineColorBlendAttachmentState& getColorBlendAttachment() const { return colorBlendAttachment; }

        // --- Multisampling ---
        void setMultisampling(const vk::PipelineMultisampleStateCreateInfo& v) { multisampling = v; }
        const vk::PipelineMultisampleStateCreateInfo& getMultisampling() const { return multisampling; }

        // --- Depth/stencil ---
        void setDepthStencil(const vk::PipelineDepthStencilStateCreateInfo& v) { depthStencil = v; }
        const vk::PipelineDepthStencilStateCreateInfo& getDepthStencil() const { return depthStencil; }

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