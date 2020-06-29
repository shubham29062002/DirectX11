#include "ChiliTimer.h"

ChiliTimer::ChiliTimer() noexcept
{
    last = std::chrono::steady_clock::now();
}

float ChiliTimer::Mark() noexcept
{
    const auto old = last;
    last = std::chrono::steady_clock::now();
    const std::chrono::duration<float> frameTime = last - old;

    return frameTime.count();
}

float ChiliTimer::peek() const noexcept
{
    return std::chrono::duration<float>(std::chrono::steady_clock::now() - last).count();
}
