/**
 * @file
 * @brief Debug overlay layer with FPS counter and transform controls.
 */

#pragma once

#include "debug/ILayer.h"
#include "scene/SceneData.h"
#include <chrono>
#include <vector>

/**
 * @brief Debug overlay layer providing FPS counter and scene controls.
 *
 * Renders ImGui UI elements for debugging:
 * - Real-time FPS counter with frame time histogram
 * - Triangle transform controls (position, rotation, scale)
 * - Application info overlay
 */
class DebugLayer : public ILayer
{
    public:
    bool initialize() override;
    void render() override;
    void shutdown() override;

    private:
    // FPS tracking
    std::chrono::steady_clock::time_point lastTime_;
    float fps_;
    float frameTime_;
    std::vector<float> frameTimes_;
    static constexpr size_t MaxFrameTimes = 300;

    // Transform state (mirrors scene transform for UI binding)
    TriangleTransform transform_;
    bool showDemoWindow_;
};
