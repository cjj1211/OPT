/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
*  作者 : 宋志杰 zhijie.song@nueroxess.copm
*  修改记录:
*  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "CLogger.h"

#include <corecrt_io.h>
#include <direct.h>
#include <string>
#include <ranges>

#include <log4cpp/Category.hh>
#include "log4cpp/FileAppender.hh"
#include "log4cpp/RollingFileAppender.hh"
#include "log4cpp/Priority.hh"
#include "log4cpp/PatternLayout.hh"
#include "TimeStamp.h"

#define NOMINMAX
#include <regex>

#include "Windows.h"

BEGIN_NX_NAMESPACE

constexpr char g_kAppenderName[] = "OsAppender";

/*其中参数类型为std::string，类似于C语言中的printf，使用格式化字符串来描述输出格式，其具体含义如下：
*  u %c category；
*  u %d 日期；日期可以进一步的设置格式，用花括号包围，例如%d{%H:%M:%S,%l} 或者 %d{%d %m %Y%H:%M:%S,%l}。如果不设置具体日期格式，则如下默认格式被使用“Wed Jan 02 02:03:55 1980”。日期的格式符号与ANSI C函数strftime中的一致。但增加了一个格式符号%l，表示毫秒，占三个十进制位。
*  u %m 消息；
*  u %n 换行符，会根据平台的不同而不同，但对于用户透明；
*  u %p 优先级；
*  u %r 自从layout被创建后的毫秒数；
*  u %R 从1970年1月1日0时开始到目前为止的秒数；
*  u %u 进程开始到目前为止的时钟周期数；
*  u %x NDC。
*/
constexpr char g_kLogFormat[] = "%d:  %p  %m%n"; // 优先级 日期 NDC 消息

CLogger::CLogger()
	: category(log4cpp::Category::getRoot())
	, patternLayout(nullptr)
	, priority(LogPriority::LOG_DEBUG)
	, enableDebugConsole(false)
{
	std::ios::sync_with_stdio(false); // 关闭标准输入输出的同步功能，提升cout速度
	Init();
}

CLogger::~CLogger()
{
	log4cpp::Category::shutdown();
	// 不要主动delete appender和patternLayout, Category shutdown过程中会注销上述两个对象
	//	delete appender;
	//	delete patternLayout;
}

std::string CLogger::GetProcessName() const
{
	char    processFullName[_MAX_PATH] = { 0 };
	char    processName[0x40] = { 0 };

	DWORD dwpid = GetCurrentProcessId();
	GetModuleFileNameA(NULL, processFullName, _MAX_PATH); //进程完整路径

	const char* tmp1 = strrchr(processFullName, '\\');
	const char* tmp2 = strrchr(processFullName, '.');
	memcpy(processName, tmp1 + 1, std::min(static_cast<int>(tmp2 - tmp1 - 1), 0x40)); //截取得进程名
	return processName;
}

CLogger& CLogger::Instance()
{
	static CLogger instance;
	return instance;
}

void CLogger::Init(std::string sLogPath /*="../Log/"*/, LogPriority ePriority /*=LogPriority::LOG_INFO*/)
{
	patternLayout = new log4cpp::PatternLayout();
	patternLayout->setConversionPattern(g_kLogFormat);
	const auto processName = GetProcessName();

	if (-1 == _access(sLogPath.c_str(), 0))
	{
		if (const auto flag = _mkdir(sLogPath.c_str()); flag == 0)
		{
			// 创建成功
		}
		else
		{
			// 创建失败
		}
	}

	sLogPath += processName + "/";

	if (-1 == _access(sLogPath.c_str(), 0))
	{
		if (const auto flag = _mkdir(sLogPath.c_str()); flag == 0)
		{
			// 创建成功
		}
		else
		{
			// 创建失败
		}
	}

	const auto timestamp = GetCurrentTimeStamp();
	const auto filepath = sLogPath + timestamp + ".log";
	const auto appender = new log4cpp::FileAppender("NeuroXess", filepath);
	appender->setLayout(patternLayout);

	category.addAppender(appender);
	priority = ePriority;
	SetLogLevel(ePriority);
}

void CLogger::EnableDebug(const bool enabled)
{
	enableDebugConsole = enabled;
}

void CLogger::SetLogLevel(const LogPriority ePriority)
{
	priority = ePriority;
	switch (priority)
	{
	case LogPriority::LOG_DEBUG:
		category.setPriority(log4cpp::Priority::DEBUG);
		break;
	case LogPriority::LOG_INFO:
		category.setPriority(log4cpp::Priority::INFO);
		break;
	case LogPriority::LOG_WARNING:
		category.setPriority(log4cpp::Priority::WARN);
		break;
	case LogPriority::LOG_ERROR:
		category.setPriority(log4cpp::Priority::ERROR);
		break;
	case LogPriority::LOG_FATAL:
		category.setPriority(log4cpp::Priority::FATAL);
		break;
	}
}

LogPriority CLogger::GetLogLevel() const
{
	return priority;
}

void CLogger::Log(const LogPriority ePriority, const char* sFile, const char* sFunc, const int iLine, const char* sMsg) const
{
	const auto line = StringFormat("%s, %d, %s, %s", sFile, iLine, sFunc, sMsg);

	switch (ePriority)
	{
	case LogPriority::LOG_DEBUG:
#ifndef _DEBUG
		if (enableDebugConsole)
		{
			category.debug(line);
		}
#else
		category.debug(line);
#endif
		break;
	case LogPriority::LOG_INFO:
		category.info("  " + line);
		break;
	case LogPriority::LOG_WARNING:
		category.warn(line);
		break;
	case LogPriority::LOG_ERROR:
		category.error(line);
		break;
	case LogPriority::LOG_FATAL:
		category.fatal(" " + line);
		break;
	}
#ifndef _DEBUG
	if (enableDebugConsole)
	{
		switch (ePriority)
		{
		case LogPriority::LOG_DEBUG:
			std::cout << "\033[0m";
			break;
		case LogPriority::LOG_INFO:
			std::cout << "\033[32m";
			break;
		case LogPriority::LOG_WARNING:
			std::cout << "\033[33m";
			break;
		case LogPriority::LOG_ERROR:
			std::cout << "\033[31m";
			break;
		case LogPriority::LOG_FATAL:
			std::cout << "\033[31m";
			break;
		}
		std::cout << line << "\033[0m" << std::endl;
	}
#else
	switch (ePriority)
	{
	case LogPriority::LOG_DEBUG:
		std::cout << "\033[0m";
		break;
	case LogPriority::LOG_INFO:
		std::cout << "\033[32m";
		break;
	case LogPriority::LOG_WARNING:
		std::cout << "\033[33m";
		break;
	case LogPriority::LOG_ERROR:
		std::cout << "\033[31m";
		break;
	case LogPriority::LOG_FATAL:
		std::cout << "\033[31m";
		break;
	}
	std::cout << line << "\033[0m" << std::endl;
#endif
}

void CLogger::Log(LogPriority ePriority, const char* sFile, const char* sFunc, const int iLine, const std::string& sMsg) const
{
	const auto line = StringFormat("%s, %d, %s, %s", sFile, iLine, sFunc, sMsg.c_str());

	switch (ePriority)
	{
	case LogPriority::LOG_DEBUG:
#ifndef _DEBUG
		if (enableDebugConsole)
		{
			category.debug(line);
		}
#else
		category.debug(line);
#endif
		break;
	case LogPriority::LOG_INFO:
		category.info("  " + line);
		break;
	case LogPriority::LOG_WARNING:
		category.warn(line);
		break;
	case LogPriority::LOG_ERROR:
		category.error(line);
		break;
	case LogPriority::LOG_FATAL:
		category.fatal(" " + line);
		break;
	}

#ifndef _DEBUG
	if (enableDebugConsole)
	{
		switch (ePriority)
		{
		case LogPriority::LOG_DEBUG:
			std::cout << "\033[32m";
			break;
		case LogPriority::LOG_INFO:
			std::cout << "\033[32m";
			break;
		case LogPriority::LOG_WARNING:
			std::cout << "\033[33m";
			break;
		case LogPriority::LOG_ERROR:
			std::cout << "\033[31m";
			break;
		case LogPriority::LOG_FATAL:
			std::cout << "\033[31m";
			break;
		}

		std::cout << line << "\033[0m" << std::endl;
	}
#else
	switch (ePriority)
	{
	case LogPriority::LOG_DEBUG:
		std::cout << "\033[32m";
		break;
	case LogPriority::LOG_INFO:
		std::cout << "\033[32m";
		break;
	case LogPriority::LOG_WARNING:
		std::cout << "\033[33m";
		break;
	case LogPriority::LOG_ERROR:
		std::cout << "\033[31m";
		break;
	case LogPriority::LOG_FATAL:
		std::cout << "\033[31m";
		break;
	}

	std::cout << line << "\033[0m" << std::endl;
#endif
}

END_NX_NAMESPACE