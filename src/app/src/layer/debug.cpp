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

                    // Grow time-history vector to cover this entity
                    while (static_cast<int>(debugData_->timeHistories.size()) <
                           debugData_->scene->getEntities().size())
                    {
                        debugData_->timeHistories.emplace_back();
                    }
                    int entityIndex = 0;
                    int i = 0;
                    for (const std::unique_ptr<Entity> &e :
                         debugData_->scene->getEntities())
                    {
                        if (e.get() == entity.get())
                        {
                            entityIndex = i;
                            break;
                        }
                        i++;
                    }
                    PipelineTimeHistory &history =
                        debugData_->timeHistories[entityIndex];
                    history.push(
                        pipelineUpdateTime, pipelineDrawTime, fenceWaitTime,
                        percentGPUTimeOfTotalFrameTime
                    );

                    double nInstances =
                        meshComponent->getMesh()->instances.size();
                    double nVertices = meshComponent->getMesh()
                                           ->getGeometry()
                                           ->indexCounts[*debugData_->lodIndex];
                    double nTriangles = (nVertices / 3) * nInstances;

                    // Per-frame time line charts
                    {
                        ImVec2 plotSize = ImVec2(300.f, 60.f);
                        ImGui::Text("100");
                        ImGui::SameLine();
                        ImGui::PlotLines(
                            "GPU %", history.gpuPercent, DEBUG_HISTORY_FRAMES,
                            history.head_, "GPU %", 0.f, 100.f, plotSize
                        );
                        ImGui::Text("0");
                        ImGui::SameLine();
                        ImGui::Dummy(ImVec2(0.f, 0.f));
                    }

                    // Stats table — only relevant rows per pipeline type
                    std::string tableName = "Pipeline";
                    if (ImGui::BeginTable(
                            tableName.c_str(), 2,
                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_SizingFixedFit
                        ))
                    {
                        ImGui::TableSetupColumn("Metric");
                        ImGui::TableSetupColumn("Value");
                        ImGui::TableHeadersRow();

                        // Common rows
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("N Instances");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", formatNumber(nInstances).c_str());

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("N Total Maximum Vertices");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(
                            "%s", formatNumber(nVertices * nInstances).c_str()
                        );

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("N Total Maximum Triangles");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", formatNumber(nTriangles).c_str());

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("N Drawn Triangles");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(
                            "%s",
                            formatNumber((double) pipelineStatistics.nTriangles)
                                .c_str()
                        );

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("N Draw Calls");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", (int) pipelineStatistics.nDrawCalls);

                        // Pipeline-specific invocation rows
                        if (pipelineStatistics.pipelineType ==
                            PipelineStatistics::PipelineType::TRADITIONAL)
                        {
                            // Only show CS invocations for compute shader
                            if (selectedShaderType == 2)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text("CS Invocations");
                                ImGui::TableSetColumnIndex(1);
                                ImGui::Text(
                                    "%s",
                                    formatNumber((double) pipelineStatistics
                                                     .nComputeShaderInvocations)
                                        .c_str()
                                );
                            }

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("VS Invocations");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%s",
                                formatNumber((double) pipelineStatistics
                                                 .nVertexShaderInvocations)
                                    .c_str()
                            );

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("PS Invocations");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%s",
                                formatNumber((double) pipelineStatistics
                                                 .nFragmentShaderInvocations)
                                    .c_str()
                            );
                        }
                        else if (
                            pipelineStatistics.pipelineType ==
                            PipelineStatistics::PipelineType::MESH
                        )
                        {
                            double nMeshlets =
                                meshComponent->getMesh()
                                    ->getGeometry()
                                    ->meshletLODCounts[*debugData_->lodIndex];

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("TS Invocations");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%s", formatNumber((double) pipelineStatistics
                                                       .nTaskShaderInvocations)
                                          .c_str()
                            );

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("MS Invocations");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%s", formatNumber((double) pipelineStatistics
                                                       .nMeshShaderInvocations)
                                          .c_str()
                            );

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("PS Invocations");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%s",
                                formatNumber((double) pipelineStatistics
                                                 .nFragmentShaderInvocations)
                                    .c_str()
                            );

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("N Meshlets per Instance");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%s", formatNumber(nMeshlets).c_str());

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("Total Maximum Meshlets");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text(
                                "%s",
                                formatNumber(nMeshlets * nInstances).c_str()
                            );
                        }

                        ImGui::EndTable();
                    }
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
            "Camera Far Plane", debugData_->cameraFarPlane, 1.0, 1500.0
        );

        std::vector<const char *> fragmentColourTypes = {
            "Texture", "Primitive", "LOD"
        };
        ImGui::Combo(
            "Fragment Colour Type",
            reinterpret_cast<int *>(debugData_->fragmentColourType),
            fragmentColourTypes.data(), fragmentColourTypes.size()
        );

        std::vector<const char *> cullingFunctions = {
            "None", "Planes", "Sphere", "Cone", "Cone and Near Plane"
        };
        ImGui::Combo(
            "Culling Function",
            reinterpret_cast<int *>(debugData_->visibilityFunc),
            cullingFunctions.data(), cullingFunctions.size()
        );
    }

    // Mesh
    if (ImGui::CollapsingHeader("Mesh Options"))
    {
        ImGui::Checkbox("Animate?", debugData_->isAnimationOn);

        ImGui::SliderInt(
            "N Instances", debugData_->nInstances, 1, MAX_N_INSTANCES
        );
        ImGui::SliderFloat(
            "Disc Outer Radius", debugData_->discOuterRadius, 1.f, 10000.f
        );
        ImGui::SliderFloat(
            "Disc Inner Radius", debugData_->discInnerRadius, 1.f, 100.f
        );
        ImGui::SliderFloat("Disc Height", debugData_->discHeight, 1.f, 100.f);
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
            ImGui::SliderFloat(
                "LOD Exponent", debugData_->lodExponent, 0.1f, 5.f
            );
        }
    }

    // Shader type
    if (ImGui::CollapsingHeader("Shader Options"))
    {
        ImGui::SeparatorText("Shader");
        std::vector<const char *> shaderTypes = {
            "Default Vertex", "Instanced Vertex",
            "Instanced Vertex With Compute", "Mesh"
        };
        if (ImGui::Combo(
                "Shader Type", &selectedShaderType, shaderTypes.data(),
                shaderTypes.size()
            ))
            shaderTypeUpdate = selectedShaderType;
        else
            shaderTypeUpdate = -1;
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
