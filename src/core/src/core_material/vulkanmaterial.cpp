#include "core_material/vulkanmaterial.h"
#include "core_shader/vulkanshader.h"

VulkanMaterial::VulkanMaterial(BaseMaterialConfig materialConfig)
{
    config_ = materialConfig;
}

VulkanMaterialConfig *VulkanMaterial::getConfig()
{
    return dynamic_cast<VulkanMaterialConfig *>(&config_);
}

void VulkanMaterial::set()
{
}

void VulkanMaterial::addShader(IShader *shader)
{
    VulkanShader *vulkanShader = dynamic_cast<VulkanShader *>(shader);
    shaders.push_back(vulkanShader);
}
