// TextureData.cpp
#include "TextureData.hpp"
#include <spdlog/spdlog.h>

bool TextureData::doLoad() {
    KTX_error_code result = ktxTexture2_CreateFromNamedFile(
        filePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTex);

    if (result != KTX_SUCCESS) {
        spdlog::warn("No se pudo cargar '{}', usando fallback blanco", filePath);
        result = ktxTexture2_CreateFromNamedFile("assets/textures/common/builtin_white.ktx2", 
                                              KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTex);
        if (result != KTX_SUCCESS) return false; // solo acá sí es fallo real total, ni si quiera se puede cargar la textura default
    }

    if (ktxTexture2_NeedsTranscoding(ktxTex)) {
        // BC7 es un buen default para GPU de escritorio
        // más adelante hay que consultar vk::PhysicalDevice::getFormatProperties()
        // en vez de hardcodeado
        result = ktxTexture2_TranscodeBasis(ktxTex, KTX_TTF_BC7_RGBA, 0);
        if (result != KTX_SUCCESS) {
            spdlog::error("Fallo al transcodificar '{}': {}", filePath, ktxErrorString(result));
            ktxTexture_Destroy(ktxTexture(ktxTex));
            ktxTex = nullptr;
            return false;
        }
    }

    return true;
}

bool TextureData::doUnload() {
    if (ktxTex) {
        ktxTexture_Destroy(ktxTexture(ktxTex));
        ktxTex = nullptr;
    }
    return true;
}