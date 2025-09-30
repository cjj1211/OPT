/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
*  作者 : 宋志杰 zhijie.song@nueroxess.copm
*  修改记录:
*  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __LOGGER_H
#define __LOGGER_H

#include "LoggerGlobal.h"
#include <vector>
#include <memory>
#include <string>

#define NOMINMAX
#include <Windows.h>
#include <algorithm>

namespace log4cpp {
	class PatternLayout;
	class Category;
	class Appender;
}

BEGIN_NX_NAMESPACE

enum class LogPriority {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_FATAL
};

/**
 * 日志记录库.
 */
class NX_LOGGER_EXPORT CLogger
{
public:
	CLogger(const CLogger&) = delete;
	CLogger& operator=(const CLogger&) = delete;
	CLogger(const CLogger&&) = delete;
	CLogger& operator = (const CLogger&&) = delete;

	using CategoryRef = log4cpp::Category&;

	/** 
	* 获取日志类实例.
	*/
	static CLogger& Instance();

	/**
	 * 初始化日之类
	 * 
	 * @param[in] logPath 日志存储路径，默认存放到工作路径上一级下的Log文件夹下
	 * @param[in] priority 日志显示级别
	 */
	void Init(std::string logPath = "../Log/", LogPriority priority = LogPriority::LOG_INFO);

	/**
	 * 是否要开启调试控制台.
	 * 
	 * @param[in] enabled 是否开启
	 */
	void EnableDebug(const bool enabled);

	/**
	 * 设置日志保存级别.
	 * 
	 * @param priority[in] 日志保存级别
	 */
	void SetLogLevel(LogPriority priority);

	/**
	 * 获取当前设置的日志保存级别.
	 * 
	 * @return 日志保存级别
	 */
	LogPriority GetLogLevel() const;

	/**
	 * 记录日志.
	 * 
	 * @param[in] priority 日志级别
	 * @param[in] file 生成日志的所在文件名称
	 * @param[in] func 生成日志的所在函数名称
	 * @param[in] line 生成日志的所在行数
	 * @param[in] msg 日志内容
	 */
	void Log(LogPriority priority, const char* file, const char* func, const int line, const char* msg) const;

	/**
	 * 通过标准库字符串记录日志.
	 * 
	 * @param priority 日志级别
	 * @param file 生成日志的所在文件名称
	 * @param func 生成日志的所在函数名称
	 * @param line 生成日志的所在行数
	 * @param msg 日志内容
	 */
	void Log(LogPriority priority, const char* file, const char* func, const int line, const std::string& msg) const;

private:
	CLogger();
	~CLogger();

	template<typename ... Args>
	static std::string StringFormat(const char* format, Args ... args);

	std::string GetProcessName() const;

	CategoryRef					category;
	log4cpp::PatternLayout* patternLayout;
	LogPriority					priority;
	bool						enableDebugConsole;
};

template<typename ...Args>
std::string CLogger::StringFormat(char const* format, Args ...args)
{
	constexpr size_t oldLen = BUFSIZ;
	char buffer[oldLen]{};  // 默认栈上的缓冲区

	size_t newLen = snprintf(&buffer[0], oldLen, format, args...);
	++newLen;  // 算上终止符'\0'

	if (newLen > oldLen) {  // 默认缓冲区不够大，从堆上分配
		std::vector<char> newBuffer(newLen);
		snprintf(newBuffer.data(), newLen, format, args...);
		return std::string(newBuffer.data());
	}

	return buffer;
}

#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)

#ifdef _DEBUG
#define LOG_ACTIVATOR(msg, priority) \
	CLogger::Instance().Log(priority, __FILENAME__, __FUNCTION__, __LINE__, msg)
#else
#define LOG_ACTIVATOR(msg, priority) \
	CLogger::Instance().Log(priority, __FILENAME__, __FUNCTION__, __LINE__, msg)
#endif

/**
 * 记录日志，默认为DEBUG级别.
 */
#define LOG(msg) \
	LOG_ACTIVATOR(msg, NX::LogPriority::LOG_DEBUG)

/**
 * 记录DEBUG级别日志.
 */
#define LOG_DEBUG(msg) \
	LOG_ACTIVATOR(msg, NX::LogPriority::LOG_DEBUG)

/**
 * 记录INFO级别日志.
 */
#define LOG_INFO(msg) \
	LOG_ACTIVATOR(msg, NX::LogPriority::LOG_INFO)

/**
 * 记录WARNING级别日志.
 */
#define LOG_WARN(msg) \
	LOG_ACTIVATOR(msg, NX::LogPriority::LOG_WARNING)

/**
 * 记录ERROR级别日志.
 */
#define LOG_ERR(msg) \
	LOG_ACTIVATOR(msg, NX::LogPriority::LOG_ERROR)

/**
 * 记录FATAL级别日志.
 */
#define LOG_FAT(msg) \
	LOG_ACTIVATOR(msg, NX::LogPriority::LOG_FATAL)

END_NX_NAMESPACE

#endif
