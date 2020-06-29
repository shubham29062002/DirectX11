#pragma once
#include<chrono>
class ChiliTimer
{
public:
	ChiliTimer() noexcept;
	float Mark() noexcept;
	float peek() const noexcept;
private:
	std::chrono::steady_clock::time_point last;
};

