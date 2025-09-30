#pragma once
#include <string>
#include <chrono>

/**
 * \brief 获取秒级时间戳
 * \return 时间戳字符串
 */
static std::string GetCurrentTimeStamp() {
	const std::time_t timeStamp = std::time(nullptr);

	char buffer[80];
	const tm* timeinfo = localtime(&timeStamp);
    strftime(buffer, 80, "%Y-%m-%d %H-%M-%S", timeinfo);
    printf("%s\n", buffer);
    return std::basic_string(buffer);
}