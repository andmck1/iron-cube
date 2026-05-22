#include "layer/DebugLayer.h"
#include "ecs/component/transformComponent.h"
#include <imgui.h>
#include <spdlog/spdlog.h>

void DebugLayer::setDebugData(DebugLayerData *debugData)
{
    debugData_ = debugData;
}

bool DebugLayer::initialise()
{
    showDemoWindow_ = false;

    spdlog::info("DebugLayer initialised");
    return true;
}

void DebugLayer::render()
{
    // Debug overlay window
    ImGui::Begin(
        "Debug Overlay", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar
    );

    // FPS display
    ImGui::Text("FPS: %.1f", debugData_->timer.getFps());
    ImGui::Text("Frame Time: %.2f ms", debugData_->timer.getFrameTime());

    // Frame time mini graph
    const auto &frameTimes = debugData_->timer.getFrameTimes();
    if (!frameTimes.empty())
    {
        float maxTime = 16.67f;
        ImGui::PlotLines(
            "##FrameTime", frameTimes.data(),
            static_cast<int>(frameTimes.size()), 0, nullptr, 0.0f, maxTime,
            ImVec2(0.0f, 60.0f)
        );
    }

    ImGui::Separator();

    // Transform controls
    ImGui::Text("Triangle Transform");
    ImGui::Separator();

    ImGui::SliderFloat3(
        "Position", &triangleTransform_.position.x, -5.0f, 5.0f
    );
    ImGui::SliderFloat3(
        "Rotation (deg)", &triangleTransform_.rotation.x, -180.0f, 180.0f
    );
    ImGui::SliderFloat3(
        "Scale", &triangleTransform_.scale.x, 0.1f, 5.0f, "%.2f"
    );
    ImGui::Separator();

    if (debugData_->scene)
    {
        debugData_->scene->getEntities()
            .back()
            ->getComponent<TransformComponent>()
            ->setTransform(triangleTransform_);
    }

    ImGui::End();

    // Optional ImGui demo window
    if (showDemoWindow_)
        ImGui::ShowDemoWindow(&showDemoWindow_);
}

void DebugLayer::shutdown()
{
    spdlog::info("DebugLayer shut down");
}
