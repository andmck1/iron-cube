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

    std::chrono::steady_clock::time_point lastTime_;
    float fps_ = 0.0f;
    float frameTime_ = 0.0f;
    std::vector<float> frameTimes_;
};
