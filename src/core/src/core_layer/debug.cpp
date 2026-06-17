#include "core_layer/debug.h"
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

    // Statistics
    ImGui::SeparatorText("Statistics");
    ImGui::Text("FPS: %.1f", debugData_->timer->getFps());
    ImGui::Text("Frame Time: %.2f ms", debugData_->timer->getFrameTime());
    for (const std::unique_ptr<Entity> &entity :
         debugData_->scene->getEntities())
    {
        MeshComponent *meshComponent = entity->getComponent<MeshComponent>();
        if (meshComponent)
        {
            PipelineStatistics pipelineStatistics =
                meshComponent->getShaderPipeline()->getPipelineStatistics();

            // Pipeline statistics
            switch (pipelineStatistics.pipelineType)
            {
            case PipelineStatistics::PipelineType::TRADITIONAL:
                ImGui::Text(
                    "%s VS: %d", entity->getName().c_str(),
                    pipelineStatistics.nVertexShaderInvocations
                );
                break;
            case PipelineStatistics::PipelineType::MESH:
                ImGui::Text(
                    "%s TS: %d", entity->getName().c_str(),
                    pipelineStatistics.nTaskShaderInvocations
                );
                ImGui::Text(
                    "%s MS: %d", entity->getName().c_str(),
                    pipelineStatistics.nMeshShaderInvocations
                );
                break;
            default:
                spdlog::warn("Pipeline type not supported for debug data");
            }
            ImGui::Text(
                "%s PS: %d", entity->getName().c_str(),
                pipelineStatistics.nFragmentShaderInvocations
            );

            uint nInstances = meshComponent->getMesh()->instances.size();
            uint nMeshlets =
                meshComponent->getMesh()->getGeometry()->meshlets.size();
            uint nVertices =
                meshComponent->getMesh()->getGeometry()->meshletVertices.size();
            ImGui::Text("N Instances: %d", nInstances);
            ImGui::Text("N Vertices per Instance: %d", nVertices);
            ImGui::Text("N Total Vertices: %d", nVertices * nInstances);

            if (pipelineStatistics.pipelineType ==
                PipelineStatistics::PipelineType::MESH)
            {
                ImGui::Text("N Meshlets per Instance: %d", nMeshlets);
                ImGui::Text("N Total Meshlets: %d", nMeshlets * nInstances);
            }
        }
    }

    // Render
    ImGui::SeparatorText("Render");
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
    ImGui::Checkbox("Are Animations On?", debugData_->isAnimationOn);

    // Shader type
    ImGui::SeparatorText("Shader");
    std::vector<const char *> shaderTypes = {"Traditional", "Mesh"};
    static int selectedShaderType = 0;
    if (ImGui::Combo(
            "Shader Type", &selectedShaderType, shaderTypes.data(),
            shaderTypes.size()
        ))
        shaderTypeUpdate = selectedShaderType;
    else
        shaderTypeUpdate = -1;

    // Culling controls
    ImGui::SeparatorText("Culling");
    std::vector<const char *> cullingFunctions = {
        "None", "Planes", "Sphere", "Cone", "Cone and Near Plane"
    };
    ImGui::Combo(
        "Culling Function", debugData_->visibilityFunc, cullingFunctions.data(),
        cullingFunctions.size()
    );

    ImGui::End();

    if (!renderer_->renderImGui())
        return false;

    return true;
}

bool DebugLayer::update()
{
    return true;
}

bool DebugLayer::postRender()
{
    return true;
}

bool DebugLayer::end()
{
    return true;
}
