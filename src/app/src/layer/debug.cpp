#include "helpers/globals.h"
#include "layer/debug.h"
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
    ImGui::Checkbox("Collect Pipeline Stats?", debugData_->isDebugStatsOn);
    if (*debugData_->isDebugStatsOn)
    {
        for (const std::unique_ptr<Entity> &entity :
             debugData_->scene->getEntities())
        {
            MeshComponent *meshComponent =
                entity->getComponent<MeshComponent>();
            if (meshComponent)
            {
                PipelineStatistics pipelineStatistics =
                    meshComponent->getShaderPipeline()->getPipelineStatistics();

                // Pipeline statistics
                switch (pipelineStatistics.pipelineType)
                {
                case PipelineStatistics::PipelineType::TRADITIONAL:
                    ImGui::Text(
                        "%s VS: %.3e", entity->getName().c_str(),
                        (double) pipelineStatistics.nVertexShaderInvocations
                    );
                    break;
                case PipelineStatistics::PipelineType::MESH:
                    ImGui::Text(
                        "%s TS: %.3e", entity->getName().c_str(),
                        (double) pipelineStatistics.nTaskShaderInvocations
                    );
                    ImGui::Text(
                        "%s MS: %.3e", entity->getName().c_str(),
                        (double) pipelineStatistics.nMeshShaderInvocations
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
                uint nVertices = meshComponent->getMesh()
                                     ->getGeometry()
                                     ->meshletVertices.size();
                ImGui::Text("N Instances: %.3e", (double) nInstances);
                ImGui::Text(
                    "N Vertices per Instance: %.3e", (double) nVertices
                );
                ImGui::Text(
                    "N Total Vertices: %.3e", (double((nVertices * nInstances)))
                );

                if (pipelineStatistics.pipelineType ==
                    PipelineStatistics::PipelineType::MESH)
                {
                    ImGui::Text(
                        "N Meshlets per Instance: %.3e", (double) nMeshlets
                    );
                    ImGui::Text(
                        "N Total Meshlets: %.3e",
                        (double((nMeshlets * nInstances)))
                    );
                }
            }
        }
    }

    // Render
    ImGui::SeparatorText("Render");
    bool wireframe = renderer_ ? renderer_->isWireframeMode() : false;
    if (ImGui::Checkbox("Wireframe", &wireframe))
    {
        renderer_->setWireframeMode(wireframe);
        IShaderPipelineManager *pipelineManager =
            ServiceLocator::instance().get<IShaderPipelineManager>();
        if (pipelineManager)
            pipelineManager->recreatePipelines = true;
    }
    ImGui::SliderFloat(
        "Camera Far Plane", debugData_->cameraFarPlane, 1.0, 50.0
    );

    // Mesh
    ImGui::SeparatorText("Mesh");
    ImGui::Checkbox("Animate?", debugData_->isAnimationOn);
    ImGui::SliderInt(
        "N Instance Rows", debugData_->nInstanceRows, 1, MAX_N_INSTANCE_ROWS
    );
    ImGui::SliderInt(
        "N Instance Columns", debugData_->nInstanceColumns, 1,
        MAX_N_INSTANCE_COLS
    );
    ImGui::SliderInt(
        "N Instance Layers", debugData_->nInstanceLayers, 1,
        MAX_N_INSTANCE_LAYERS
    );

    // Shader type
    ImGui::SeparatorText("Shader");
    std::vector<const char *> shaderTypes = {
        "Default Vertex", "Instanced Vertex", "Mesh"
    };
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
