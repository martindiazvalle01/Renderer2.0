# Details

Date : 2026-11-22 23:36:02

Directory /home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/src

Total : 50 files,  2753 codes, 331 comments, 879 blanks, all 3963 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [src/ResourceLayer/AssetManager.cpp](/src/ResourceLayer/AssetManager.cpp) | C++ | 76 | 8 | 26 | 110 |
| [src/ResourceLayer/AssetManager.hpp](/src/ResourceLayer/AssetManager.hpp) | C++ | 28 | 0 | 9 | 37 |
| [src/ResourceLayer/AssetPool.hpp](/src/ResourceLayer/AssetPool.hpp) | C++ | 56 | 10 | 16 | 82 |
| [src/ResourceLayer/GltfImporter.cpp](/src/ResourceLayer/GltfImporter.cpp) | C++ | 264 | 32 | 84 | 380 |
| [src/ResourceLayer/GltfImporter.hpp](/src/ResourceLayer/GltfImporter.hpp) | C++ | 71 | 4 | 20 | 95 |
| [src/ResourceLayer/ResourceManager.hpp](/src/ResourceLayer/ResourceManager.hpp) | C++ | 39 | 0 | 13 | 52 |
| [src/ResourceLayer/Storage.hpp](/src/ResourceLayer/Storage.hpp) | C++ | 8 | 0 | 5 | 13 |
| [src/ResourceLayer/structTypes.hpp](/src/ResourceLayer/structTypes.hpp) | C++ | 110 | 11 | 29 | 150 |
| [src/Tests/Cube.hpp](/src/Tests/Cube.hpp) | C++ | 97 | 16 | 28 | 141 |
| [src/application\_layer/App.cpp](/src/application_layer/App.cpp) | C++ | 62 | 1 | 23 | 86 |
| [src/application\_layer/App.hpp](/src/application_layer/App.hpp) | C++ | 21 | 0 | 8 | 29 |
| [src/core\_layers/Single\_GPU\_Core/Allocator.cpp](/src/core_layers/Single_GPU_Core/Allocator.cpp) | C++ | 20 | 2 | 8 | 30 |
| [src/core\_layers/Single\_GPU\_Core/Allocator.hpp](/src/core_layers/Single_GPU_Core/Allocator.hpp) | C++ | 14 | 0 | 4 | 18 |
| [src/core\_layers/Single\_GPU\_Core/DebugMessenger.cpp](/src/core_layers/Single_GPU_Core/DebugMessenger.cpp) | C++ | 26 | 0 | 3 | 29 |
| [src/core\_layers/Single\_GPU\_Core/DebugMessenger.hpp](/src/core_layers/Single_GPU_Core/DebugMessenger.hpp) | C++ | 15 | 0 | 4 | 19 |
| [src/core\_layers/Single\_GPU\_Core/Instance.cpp](/src/core_layers/Single_GPU_Core/Instance.cpp) | C++ | 53 | 5 | 14 | 72 |
| [src/core\_layers/Single\_GPU\_Core/Instance.hpp](/src/core_layers/Single_GPU_Core/Instance.hpp) | C++ | 11 | 0 | 3 | 14 |
| [src/core\_layers/Single\_GPU\_Core/LogicalDevice.cpp](/src/core_layers/Single_GPU_Core/LogicalDevice.cpp) | C++ | 80 | 12 | 25 | 117 |
| [src/core\_layers/Single\_GPU\_Core/LogicalDevice.hpp](/src/core_layers/Single_GPU_Core/LogicalDevice.hpp) | C++ | 19 | 0 | 6 | 25 |
| [src/core\_layers/Single\_GPU\_Core/PhysicalDeviceSelector.cpp](/src/core_layers/Single_GPU_Core/PhysicalDeviceSelector.cpp) | C++ | 75 | 6 | 19 | 100 |
| [src/core\_layers/Single\_GPU\_Core/PhysicalDeviceSelector.hpp](/src/core_layers/Single_GPU_Core/PhysicalDeviceSelector.hpp) | C++ | 11 | 0 | 2 | 13 |
| [src/core\_layers/Single\_GPU\_Core/RenderDevice.hpp](/src/core_layers/Single_GPU_Core/RenderDevice.hpp) | C++ | 57 | 0 | 6 | 63 |
| [src/core\_layers/Single\_GPU\_Core/Surface.cpp](/src/core_layers/Single_GPU_Core/Surface.cpp) | C++ | 18 | 1 | 4 | 23 |
| [src/core\_layers/Single\_GPU\_Core/Surface.hpp](/src/core_layers/Single_GPU_Core/Surface.hpp) | C++ | 13 | 0 | 7 | 20 |
| [src/main.cpp](/src/main.cpp) | C++ | 24 | 0 | 6 | 30 |
| [src/rendering\_layers/AssetTextureManager.cpp](/src/rendering_layers/AssetTextureManager.cpp) | C++ | 5 | 0 | 1 | 6 |
| [src/rendering\_layers/AssetTextureManager.hpp](/src/rendering_layers/AssetTextureManager.hpp) | C++ | 14 | 0 | 2 | 16 |
| [src/rendering\_layers/DescriptorManager.cpp](/src/rendering_layers/DescriptorManager.cpp) | C++ | 51 | 7 | 19 | 77 |
| [src/rendering\_layers/DescriptorManager.hpp](/src/rendering_layers/DescriptorManager.hpp) | C++ | 21 | 10 | 11 | 42 |
| [src/rendering\_layers/FrameManagement/FrameManager.cpp](/src/rendering_layers/FrameManagement/FrameManager.cpp) | C++ | 127 | 27 | 40 | 194 |
| [src/rendering\_layers/FrameManagement/FrameManager.hpp](/src/rendering_layers/FrameManagement/FrameManager.hpp) | C++ | 37 | 11 | 16 | 64 |
| [src/rendering\_layers/GpuImage.hpp](/src/rendering_layers/GpuImage.hpp) | C++ | 59 | 10 | 14 | 83 |
| [src/rendering\_layers/MeshBufferManager.cpp](/src/rendering_layers/MeshBufferManager.cpp) | C++ | 80 | 11 | 27 | 118 |
| [src/rendering\_layers/MeshBufferManager.hpp](/src/rendering_layers/MeshBufferManager.hpp) | C++ | 36 | 1 | 9 | 46 |
| [src/rendering\_layers/PipelineBuilder.cpp](/src/rendering_layers/PipelineBuilder.cpp) | C++ | 110 | 13 | 49 | 172 |
| [src/rendering\_layers/PipelineBuilder.hpp](/src/rendering_layers/PipelineBuilder.hpp) | C++ | 44 | 10 | 15 | 69 |
| [src/rendering\_layers/QueueSubmitter.hpp](/src/rendering_layers/QueueSubmitter.hpp) | C++ | 40 | 1 | 9 | 50 |
| [src/rendering\_layers/RenderGraph.cpp](/src/rendering_layers/RenderGraph.cpp) | C++ | 231 | 69 | 73 | 373 |
| [src/rendering\_layers/RenderGraph.hpp](/src/rendering_layers/RenderGraph.hpp) | C++ | 70 | 16 | 32 | 118 |
| [src/rendering\_layers/Renderer.cpp](/src/rendering_layers/Renderer.cpp) | C++ | 139 | 10 | 71 | 220 |
| [src/rendering\_layers/Renderer.hpp](/src/rendering_layers/Renderer.hpp) | C++ | 51 | 1 | 13 | 65 |
| [src/rendering\_layers/RendererConfig.hpp](/src/rendering_layers/RendererConfig.hpp) | C++ | 7 | 4 | 1 | 12 |
| [src/rendering\_layers/SceneDataManager.cpp](/src/rendering_layers/SceneDataManager.cpp) | C++ | 19 | 1 | 8 | 28 |
| [src/rendering\_layers/SceneDataManager.hpp](/src/rendering_layers/SceneDataManager.hpp) | C++ | 41 | 1 | 11 | 53 |
| [src/rendering\_layers/ShaderLoader.cpp](/src/rendering_layers/ShaderLoader.cpp) | C++ | 27 | 1 | 7 | 35 |
| [src/rendering\_layers/ShaderLoader.hpp](/src/rendering_layers/ShaderLoader.hpp) | C++ | 7 | 0 | 2 | 9 |
| [src/rendering\_layers/swapchain/Swapchain.cpp](/src/rendering_layers/swapchain/Swapchain.cpp) | C++ | 126 | 9 | 26 | 161 |
| [src/rendering\_layers/swapchain/Swapchain.hpp](/src/rendering_layers/swapchain/Swapchain.hpp) | C++ | 45 | 3 | 18 | 66 |
| [src/scene\_management\_layer/Camera.cpp](/src/scene_management_layer/Camera.cpp) | C++ | 55 | 1 | 15 | 71 |
| [src/scene\_management\_layer/Camera.hpp](/src/scene_management_layer/Camera.hpp) | C++ | 43 | 6 | 18 | 67 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)