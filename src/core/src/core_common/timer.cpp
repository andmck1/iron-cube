#include "core_common/timer.h"
#include <spdlog/spdlog.h>

Timer::Timer() : lastTime_(std::chrono::steady_clock::now())
{
    spdlog::debug("Timer initialized");
    lastFpsUpdateTime_ = lastTime_;
    frameTimes_.reserve(MAX_N_FRAMES);
}

void Timer::reset()
{
    spdlog::debug("Timer reset");
    lastTime_ = std::chrono::steady_clock::now();
    lastFpsUpdateTime_ = lastTime_;
    fps_ = 0.0f;
    frameTime_ = 0.0f;
    accumulatedDelta_ = 0.0f;
    frameCount_ = 0;
    frameTimes_.clear();
}

void Timer::tick()
{
    std::chrono::time_point currentTime = std::chrono::steady_clock::now();
    double delta =
        std::chrono::duration<double>(currentTime - lastTime_).count();
    lastTime_ = currentTime;

    frameTime_ = static_cast<float>(delta * 1000.0f);
    totalSecondsElapsed_ += static_cast<float>(delta);

    accumulatedDelta_ += static_cast<float>(delta);
    frameCount_++;
    totalFrames_++;

    if (accumulatedDelta_ >= FPS_UPDATE_INTERVAL)
    {
        fps_ = static_cast<float>(frameCount_ / accumulatedDelta_);
        accumulatedDelta_ = 0.0f;
        frameCount_ = 0;
        lastFpsUpdateTime_ = currentTime;
    }

    frameTimes_.push_back(frameTime_);
    if (frameTimes_.size() > MAX_N_FRAMES)
        frameTimes_.erase(frameTimes_.begin());
}

float Timer::getFps() const
{
    return fps_;
}

float Timer::getFrameTime() const
{
    return frameTime_;
}

float Timer::getDeltaSeconds() const
{
    return frameTime_ / 1000.0f;
}

const std::vector<float> &Timer::getFrameTimes() const
{
    return frameTimes_;
}

int Timer::getFramesSinceStart() const
{
    return totalFrames_;
}

float Timer::getTotalSecondsElapsed() const
{
    return totalSecondsElapsed_;
}
