#include "core_layer/debug.h"
#include "core_ecs/component/transform.h"
#include "core_ecs/component/mesh.h"
#include "core_ecs/ecs.h"
#include "core_render/irenderer.h"
#include "core_render/ishaderpipelinemanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <imgui.h>
#include <spdlog/spdlog.h>

void DebugLayer::setDebugData(DebugLayerData *debugData)
{
    debugData_ = debugData;
}

bool DebugLayer::initialise()
{
    spdlog::debug("Initialising DebugLayer");
    spdlog::info("DebugLayer initialised");
    renderer_ = ServiceLocator::instance().get<IRenderer>();
    return true;
}

bool DebugLayer::frame()
{
    spdlog::debug("DebugLayer rendering");
    // Debug overlay window
    ImGui::Begin(
        "Debug Overlay", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar
    );

    // FPS display
    ImGui::Text("FPS: %.1f", debugData_->timer->getFps());
    ImGui::Text("Frame Time: %.2f ms", debugData_->timer->getFrameTime());

    // Frame time mini graph
    const std::vector<float> &frameTimes = debugData_->timer->getFrameTimes();
    if (!frameTimes.empty())
    {
        float maxTime = 33.33f;
        ImGui::PlotLines(
            "##FrameTime", frameTimes.data(),
            static_cast<int>(frameTimes.size()), 0, nullptr, 0.0f, maxTime,
            ImVec2(0.0f, 60.0f)
        );
    }

    ImGui::Separator();

    // Wireframe toggle
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    bool wireframe = renderer ? renderer->isWireframeMode() : false;
    if (ImGui::Checkbox("Wireframe", &wireframe) && renderer)
    {
        renderer->setWireframeMode(wireframe);
        IShaderPipelineManager *pipelineManager =
            ServiceLocator::instance().get<IShaderPipelineManager>();
        if (pipelineManager)
            pipelineManager->recreatePipelines = true;
    }

    // Shader type
    std::vector<const char *> shaderTypes = {"Traditional", "Mesh"};
    static int selectedShaderType = 0;
    if (ImGui::Combo(
            "Shader Type", &selectedShaderType, shaderTypes.data(),
            shaderTypes.size()
        ))
        shaderTypeUpdate = selectedShaderType;
    else
        shaderTypeUpdate = -1;

    // Pipeline statistics
    for (const std::unique_ptr<Entity> &entity :
         debugData_->scene->getEntities())
    {
        MeshComponent *meshComponent = entity->getComponent<MeshComponent>();
        if (meshComponent)
        {
            PipelineStatistics pipelineStatistics =
                meshComponent->getShaderPipeline()->getPipelineStatistics();
            ImGui::Text(
                "%s N Vertices: %d", entity->getName().c_str(),
                pipelineStatistics.nVertices
            );
        }
    }

    ImGui::End();

    if (!renderer_->renderImGui())
        return false;

    return true;
}

bool DebugLayer::update()
{
    return true;
}
