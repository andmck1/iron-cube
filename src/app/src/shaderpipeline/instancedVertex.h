#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/timer.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include "shader/instancedVF/buffertypes.h"
#include <vulkan/vulkan_raii.hpp>

class InstancedVertexShaderPipelineUpdateData : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
    std::vector<Instance> *instances = nullptr;
    uint32_t *lodIndex = 0;
    bool *isAnimationOn = nullptr;
    float *rotationFrequency = nullptr;
    Timer *timer = nullptr;
    Texture *texture = nullptr;
    FragmentColourType *fragmentColourType = nullptr;
    bool *isLODOn = nullptr;
    float *maxLODDistance = nullptr;
    float *lodExponent = nullptr;
};

class InstancedVertexVulkanShaderPipeline : public VulkanShaderPipeline
{
    public:
    bool
    init(Geometry *geometry, ShaderPipelineUpdateData *updateData) override;
    bool initPipeline() override;
    bool update() override;
    bool bind() override;
    bool shutdown() override;
    bool recreateGraphicsPipeline() override;
    bool draw() override;
    PipelineStatistics getPipelineStatistics() override;
    bool updatePipelineStatistics() override;
    bool postRender() override;

    protected:
    bool initGraphicsPipeline();

    void initBufferDescriptions() override;
    void initBuffers() override;

    static uint calculateLOD(
        glm::mat4x4 modelMatrix, glm::vec4 minMeshBounds,
        glm::vec4 maxMeshBounds, glm::vec3 eyePosition, float maxLODDistance,
        float lodExponent, uint maxLODCount
    );
    static glm::mat4x4
    calculateModelMatrix(float4 transform, float4 rotation, float4 scale);

    void updateMVPBuffer();
    void updateAnimationBuffer();
    void updateInstancesBuffer();
    void updateDebugBuffer();
    void updateConstantsBuffer(uint lodLevel);

    VulkanBuffer vertexBuffer_;
    VulkanBuffer indexBuffer_;
    VulkanBuffer instanceBuffer_;

    Geometry *geometry_ = nullptr;

    std::array<uint64_t, 1> pipelineStatisticsRaw_ = {0};
    PipelineStatistics pipelineStatistics_{};
    InstancedVertexShaderPipelineUpdateData *updateData_ = nullptr;

    std::array<uint32_t, MAX_LOD_COUNT> lodCounts_;
    std::array<uint32_t, MAX_LOD_COUNT> lodCumulativeCounts_;

    uint32_t nDrawCalls_ = 0;

    std::vector<InstancedVF::DebugDataBuffer> debugDataBufferData_ = {
        InstancedVF::DebugDataBuffer()
    };
};
