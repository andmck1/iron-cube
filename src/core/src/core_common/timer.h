#pragma once

#include <chrono>
#include <vector>

class Timer
{
    public:
    Timer();

    void reset();
    void tick();

    float getFps() const;
    float getFrameTime() const;
    float getDeltaSeconds() const;
    const std::vector<float> &getFrameTimes() const;

    private:
    static constexpr size_t MAX_N_FRAMES = 50;
    static constexpr double FPS_UPDATE_INTERVAL = 1.0;

    std::chrono::steady_clock::time_point lastTime_;
    std::chrono::steady_clock::time_point lastFpsUpdateTime_;
    float fps_ = 0.0f;
    float frameTime_ = 0.0f;
    float accumulatedDelta_ = 0.0f;
    int frameCount_ = 0;
    std::vector<float> frameTimes_;
};
