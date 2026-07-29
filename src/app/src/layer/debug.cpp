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

            float currentFrameTime = debugData_->timer->getFrameTime();
            frameTimes_.push_back(currentFrameTime);
            ImGui::TableNextRow();
            uint nFrameTimes = frameTimes_.size();
            if (nFrameTimes > MAX_N_FRAME_TIMES)
                frameTimes_.pop_front();

            timeSinceFrameTimeUpdate_ += currentFrameTime;
            if (timeSinceFrameTimeUpdate_ > FRAME_TIME_UPDATE_PERIOD * 1000.f)
            {
                timeSinceFrameTimeUpdate_ = 0.f;
                averageFrameTime_ = 0.f;
                stdFrameTime_ = 0.f;
                maxFrameTime_ = 0.f;
                for (float frameTime : frameTimes_)
                {
                    averageFrameTime_ += frameTime;
                    if (maxFrameTime_ < frameTime)
                        maxFrameTime_ = frameTime;
                }
                averageFrameTime_ /= nFrameTimes;

                for (float frameTime : frameTimes_)
                {
                    stdFrameTime_ += (frameTime - averageFrameTime_) *
                                     (frameTime - averageFrameTime_);
                }
                stdFrameTime_ /= nFrameTimes;
                stdFrameTime_ = std::pow(stdFrameTime_, 0.5);

                maxFrameTime_ =
                    std::max(std::round(maxFrameTime_ / 10.0f) * 10.0f, 1.f);
                maxFrameTime_ *= 2.f;
            }

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Average Frame Time");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(
                "%.2f ms (+-%.2f ms)", averageFrameTime_, stdFrameTime_
            );

            std::vector<float> plotValues(
                frameTimes_.begin(), frameTimes_.end()
            );
            ImVec2 plotSize = ImVec2(300.f, 60.f);
            ImGui::Text("%.2f", maxFrameTime_);
            ImGui::SameLine();
            ImGui::PlotLines(
                "Frame Time (ms)", plotValues.data(), MAX_N_FRAME_TIMES, 0,
                "Frame Time (ms)", 0.f, maxFrameTime_, plotSize
            );
            ImGui::Text("%.2f", 0.0);
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(0.f, 0.f));

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
                ImGui::TableSetupColumn("Kernel Busy %");
                ImGui::TableSetupColumn("Memory RW Busy %");
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

                        for (auto [statName, stat] : pipelineStatistics.stats)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%s", statName.c_str());
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%s", formatNumber(stat).c_str());
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
            "Texture", "Primitive", "LOD", "Normals"
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
            "Disc Outer Radius", debugData_->discOuterRadius, 10.f, 100000.f
        );
        ImGui::SliderFloat(
            "Disc Inner Radius", debugData_->discInnerRadius, 1.f, 1000.f
        );
        ImGui::SliderFloat("Disc Height", debugData_->discHeight, 1.f, 1000.f);

        if (*debugData_->discInnerRadius >= *debugData_->discOuterRadius)
        {
            *debugData_->discInnerRadius = 10.f;
            *debugData_->discOuterRadius = 100.f;
        }
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
            "Default Vertex",
            "Instanced Vertex",
            "Instanced Vertex With Compute",
            "Mesh",
            "Mesh With Compute",
            "Mesh DX Example"
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
