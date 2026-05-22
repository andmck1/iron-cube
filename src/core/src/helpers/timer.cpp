#include "helpers/timer.h"

Timer::Timer() : lastTime_(std::chrono::steady_clock::now())
{
    frameTimes_.reserve(MAX_N_FRAMES);
}

void Timer::reset()
{
    lastTime_ = std::chrono::steady_clock::now();
    fps_ = 0.0f;
    frameTime_ = 0.0f;
    frameTimes_.clear();
}

void Timer::tick()
{
    std::chrono::time_point currentTime = std::chrono::steady_clock::now();
    double delta =
        std::chrono::duration<double>(currentTime - lastTime_).count();
    lastTime_ = currentTime;

    frameTime_ = static_cast<float>(delta * 1000.0f);
    fps_ = delta > 0 ? static_cast<float>(1.0f / delta) : 0.0f;

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

const std::vector<float> &Timer::getFrameTimes() const
{
    return frameTimes_;
}
