#include "layers/DebugLayer.h"
#include "scene/IScene.h"
#include "service/ServiceLocator.h"
#include <imgui.h>
#include <spdlog/spdlog.h>

void DebugLayer::setDebugData(DebugLayerData *debugData)
{
    debugData_ = debugData;
}

bool DebugLayer::initialise()
{
    lastTime_ = std::chrono::steady_clock::now();
    fps_ = 0.0f;
    frameTime_ = 0.0f;
    frameTimes_.reserve(MaxFrameTimes);
    showDemoWindow_ = false;

    // Initialise transform from scene
    IScene *scene = ServiceLocator::instance().get<IScene>();
    if (scene)
    {
        transform_ = scene->getTriangleTransform();
    }

    spdlog::info("DebugLayer initialised");
    return true;
}

void DebugLayer::render()
{
    // Calculate FPS
    std::chrono::time_point currentTime = std::chrono::steady_clock::now();
    double delta =
        std::chrono::duration<double>(currentTime - lastTime_).count();
    lastTime_ = currentTime;

    frameTime_ = static_cast<float>(delta * 1000.0f); // ms
    fps_ = delta > 0 ? static_cast<float>(1.0f / delta) : 0.0f;

    frameTimes_.push_back(frameTime_);
    if (frameTimes_.size() > MaxFrameTimes)
        frameTimes_.erase(frameTimes_.begin());

    // Debug overlay window
    ImGui::Begin(
        "Debug Overlay", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar
    );

    // FPS display
    ImGui::Text("FPS: %.1f", fps_);
    ImGui::Text("Frame Time: %.2f ms", frameTime_);

    // Frame time mini graph
    if (!frameTimes_.empty())
    {
        float maxTime = 16.67f; // 60 fps cap
        ImGui::PlotLines(
            "##FrameTime", frameTimes_.data(),
            static_cast<int>(frameTimes_.size()), 0, nullptr, 0.0f, maxTime,
            ImVec2(0.0f, 60.0f)
        );
    }

    ImGui::Separator();

    // Transform controls
    ImGui::Text("Triangle Transform");
    ImGui::Separator();

    ImGui::SliderFloat3("Position", &transform_.position.x, -5.0f, 5.0f);
    ImGui::SliderFloat3(
        "Rotation (deg)", &transform_.rotation.x, -180.0f, 180.0f
    );
    ImGui::SliderFloat("Scale", &transform_.scale, 0.1f, 5.0f, "%.2f");

    ImGui::Separator();

    // Apply transform to scene
    if (debugData_->scene)
    {
        debugData_->scene->setTriangleTransform(transform_);
    }

    ImGui::End();

    // Optional ImGui demo window
    if (showDemoWindow_)
        ImGui::ShowDemoWindow(&showDemoWindow_);
}

void DebugLayer::shutdown()
{
    frameTimes_.clear();
    spdlog::info("DebugLayer shut down");
}
