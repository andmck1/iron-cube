#include "helpers/globals.h"
#include "layer/debug.h"
#include "core_ecs/component/mesh.h"
#include "core_ecs/ecs.h"
#include "core_render/irenderer.h"
#include "core_render/ishaderpipelinemanager.h"
#include "core_service/servicelocator.h"
#include "core_common/helpers.h"
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <vector>

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
    if (ImGui::CollapsingHeader("Statistics"))
    {
        ImGui::SeparatorText("Render Time");
        if (ImGui::BeginTable(
                "renderTime", 2,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_SizingFixedFit
            ))
        {
            ImGui::TableSetupColumn("Metric");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("FPS");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.1f", debugData_->timer->getFps());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Frame Time");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.2f ms", debugData_->timer->getFrameTime());

            ImGui::EndTable();
        }

        ImGui::Checkbox(
            "Collect GPU and Pipeline Stats?", debugData_->isDebugStatsOn
        );
        if (*debugData_->isDebugStatsOn)
        {
            ImGui::SeparatorText("GPU");
            if (ImGui::BeginTable(
                    "gpuStats", 4,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                        ImGuiTableFlags_SizingFixedFit
                ))
            {
                ImGui::TableSetupColumn("Device");
                ImGui::TableSetupColumn("Kernel Busy %%");
                ImGui::TableSetupColumn("Memory RW Busy %%");
                ImGui::TableSetupColumn("Memory Usage");
                ImGui::TableHeadersRow();

                for (GPUStats gpuDeviceStats : *debugData_->gpuStats)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", gpuDeviceStats.name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text(
                        "%d%%",
                        gpuDeviceStats.utilisation.percent_kernel_executing
                    );
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text(
                        "%d%%", gpuDeviceStats.utilisation.percent_memoryrw_busy
                    );
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text(
                        "%.2f/%.2f (%.2f)%%", gpuDeviceStats.memory.used,
                        (gpuDeviceStats.memory.free +
                         gpuDeviceStats.memory.used),
                        gpuDeviceStats.memory.percent_utilisation
                    );
                }

                ImGui::EndTable();
            }

            ImGui::SeparatorText("Pipeline");
            for (const std::unique_ptr<Entity> &entity :
                 debugData_->scene->getEntities())
            {
                MeshComponent *meshComponent =
                    entity->getComponent<MeshComponent>();
                if (meshComponent)
                {
                    PipelineStatistics pipelineStatistics =
                        meshComponent->getShaderPipeline()
                            ->getPipelineStatistics();

                    // Pipeline statistics
                    float pipelineUpdateTime =
                        meshComponent->getShaderPipeline()->updateTime * 1000.0;
                    float pipelineDrawTime =
                        meshComponent->getShaderPipeline()->drawTime * 1000.0;
                    float fenceWaitTime =
                        renderer_->getFrameWaitTime() * 1000.0;
                    float percentGPUTimeOfTotalFrameTime =
                        (pipelineUpdateTime + pipelineDrawTime +
                         fenceWaitTime) /
                        debugData_->timer->getFrameTime() * 100;

                    double nInstances =
                        meshComponent->getMesh()->instances.size();
                    double nMeshlets =
                        meshComponent->getMesh()
                            ->getGeometry()
                            ->meshletLODCounts[*debugData_->lodIndex];
                    double nVertices = meshComponent->getMesh()
                                           ->getGeometry()
                                           ->indexCounts[*debugData_->lodIndex];
                    double nTriangles = (nVertices / 3) * nInstances;

                    if (ImGui::BeginTable(
                            "pipelineStats", 2,
                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_SizingFixedFit
                        ))
                    {
                        ImGui::TableSetupColumn("Metric");
                        ImGui::TableSetupColumn("Value");
                        ImGui::TableHeadersRow();

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Pipeline Update Time (ms)");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.0f", pipelineUpdateTime);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Pipeline Draw Time (ms)");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.0f", pipelineDrawTime);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Fence Wait Time (ms)");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.0f", fenceWaitTime);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%% GPU Time of Frame");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.0f%%", percentGPUTimeOfTotalFrameTime);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("N Instances");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.3e", nInstances);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("N Maximum Vertices per Instance");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.3e", nVertices);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("N Total Maximum Vertices");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.3e", (nVertices * nInstances));

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("N Total Maximum Triangles");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.3e", nTriangles);

                        if (pipelineStatistics.pipelineType ==
                            PipelineStatistics::PipelineType::MESH)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("N Meshlets per Instance");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%.3e", (double) nMeshlets);

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("Total Maximum Meshlets");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%.3e", (double((nMeshlets * nInstances)))
                            );
                        }

                        ImGui::EndTable();
                    }

                    switch (pipelineStatistics.pipelineType)
                    {
                    case PipelineStatistics::PipelineType::TRADITIONAL:
                        if (ImGui::BeginTable(
                                "shaderInvocationsTraditional", 2,
                                ImGuiTableFlags_Borders |
                                    ImGuiTableFlags_RowBg |
                                    ImGuiTableFlags_SizingFixedFit
                            ))
                        {
                            ImGui::TableSetupColumn("Shader");
                            ImGui::TableSetupColumn("Invocations");
                            ImGui::TableHeadersRow();

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s VS", entity->getName().c_str());
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%.3e", (double) pipelineStatistics
                                            .nVertexShaderInvocations
                            );

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s PS", entity->getName().c_str());
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%.3e",
                                pipelineStatistics.nFragmentShaderInvocations
                            );
                        }
                        break;
                    case PipelineStatistics::PipelineType::MESH:
                        if (ImGui::BeginTable(
                                "shaderInvocationsMesh", 2,
                                ImGuiTableFlags_Borders |
                                    ImGuiTableFlags_RowBg |
                                    ImGuiTableFlags_SizingFixedFit
                            ))
                        {
                            ImGui::TableSetupColumn("Shader");
                            ImGui::TableSetupColumn("Invocations");
                            ImGui::TableHeadersRow();

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s TS", entity->getName().c_str());
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%.3e", (double) pipelineStatistics
                                            .nTaskShaderInvocations
                            );

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s MS", entity->getName().c_str());
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%.3e", (double) pipelineStatistics
                                            .nMeshShaderInvocations
                            );

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s PS", entity->getName().c_str());
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%.3e",
                                pipelineStatistics.nFragmentShaderInvocations
                            );
                        }
                        break;
                    default:
                        spdlog::warn(
                            "Pipeline type not supported for debug data"
                        );
                    }
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("N Drawn Triangles");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.3e", (double) pipelineStatistics.nTriangles);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("N Draw Calls");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", (int) pipelineStatistics.nDrawCalls);

                    ImGui::EndTable();
                }
            }
        }
    }

    // Render
    if (ImGui::CollapsingHeader("Render Options"))
    {
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
    }

    // Mesh
    if (ImGui::CollapsingHeader("Mesh Options"))
    {
        ImGui::SliderFloat("LOD Exponent", debugData_->lodExponent, 0.1f, 5.f);
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
    }

    // LOD
    if (ImGui::CollapsingHeader("LOD"))
    {
        ImGui::Checkbox("LOD?", debugData_->isLODOn);
        if (*debugData_->isLODOn)
        {
            ImGui::SliderInt("LOD Index", (int *) debugData_->lodIndex, 0, 4);
            ImGui::SliderFloat(
                "Max LOD Distance", debugData_->maxLODDistance, 1.f, 100.f
            );
        }
    }

    // Shader type
    if (ImGui::CollapsingHeader("Shader Options"))
    {
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
            "Culling Function", debugData_->visibilityFunc,
            cullingFunctions.data(), cullingFunctions.size()
        );
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

bool DebugLayer::postRender()
{
    return true;
}

bool DebugLayer::end()
{
    return true;
}
