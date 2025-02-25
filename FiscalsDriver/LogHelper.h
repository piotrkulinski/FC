#pragma warning(disable : 4273)
#pragma warning(disable : 4996)


#pragma once

#include <string>
#include <list>
#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <mutex>
#include <winver.h>
#include <iomanip>

#include "zip.h"
#include <filesystem>
#include <chrono>

enum class severity : unsigned long
{
	debug = 1,
	info = 4,
	warning = 8,
	connection = 16,
	exception = 64,
	error = 128
};


class LogHelper
{
private:
	std::mutex log_mutex;
	std::stringstream buffer;
	std::ofstream logFile;
	int direct = 0;
	static const size_t buflen = 56;
	char current_time[buflen];

	int StringToWString(std::wstring& ws, const std::string& s);

	char* GetVersionFromFile(char* lpszFilePath);
	inline static LogHelper* logger = nullptr;
	LogHelper(
		std::string _fileMask = "logs\\FiscalsDriver_%04d%02i%02i_%02i%02i%02i_%03i.log"
		, std::streampos _logSize = (10 * 1024 * 1024)
		, unsigned long int _logLevel = 255
		, USHORT _compress = 1
	);

	void freplaceAll(std::string subject, const std::string search, const std::string replace) {
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos) {
			subject = subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	}
public:
	~LogHelper(void);

	LogHelper& Direct(int _direct = 0) { direct = _direct; return *this; };
	LogHelper& PrintRAW(char raw);
	LogHelper& PrintRAW(const char* raw, size_t count);
	LogHelper& PrintRAW(const std::string& buffer);
	//LogHelper& PrintRAW(const std::string buffer);

	bool Close();
	LogHelper(LogHelper const&) = delete;
	//LogHelper& operator=(LogHelper const&) = delete;


	void CheckTime()
	{
		const auto now = std::chrono::system_clock::now();
		const std::time_t tt = std::chrono::system_clock::to_time_t(now);
		const struct tm* timeinfo = std::localtime(&tt);
		auto fine = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

		memset(&current_time[0], '\0', buflen);
		std::strftime(&current_time[0], buflen, "%F %T", timeinfo);
		sprintf(&current_time[0], "%s.%03lu :: ", &current_time[0], fine.time_since_epoch().count() % 1000);
	}

	const std::ofstream& GetOutputStream() {
		return logFile;
	}

	/**
	 * @brief Jedynie singleton zarzπdzany przez inteligentny wskaünik
	 * @return LogHelper
	*/
	static std::shared_ptr<LogHelper> instance() {
		static std::shared_ptr<LogHelper> singleton{
			new LogHelper()
		};
		return singleton;
	}

	void Version();
	void Write(std::string message, unsigned long nLog = (unsigned long)severity::info);

	template <class X> void WriteLine(X message, unsigned long nLog = (unsigned long)severity::info, bool logtime = true) {
		if (nLog > 0 && (config.LogLevel & nLog) <= 0)
			return;

		const std::lock_guard<std::mutex> lock(log_mutex);

		if (!logFile.is_open() || logFile.tellp() > config.LogSize) {
			CreateLogFile();
		}

		if (logtime) {
			CheckTime();
			logFile << std::endl << current_time;
		}

		if (nLog > 0) {
			auto LevelName = [](ULONG nLog) {
				std::stringstream ss;
				typedef severity lv[];
				for (const auto& level : lv{ severity::debug, severity::warning, severity::info, severity::exception, severity::error }) {
					if ((nLog & (ULONG)level) == (ULONG)level) {
						switch (level) {
							case severity::debug: return "DEBUG";
							case severity::warning: return "WARNING";
							case severity::info: return "INFO";
							case severity::exception: return "EXCEPTION";
							case severity::error: return "ERROR";
							default: return "UNKNOW";
						}
					}
				}
				return "UNDEFINED";
				};
			if (logtime)
				logFile << LevelName(nLog) << ":"; //"level(" << LevelName(nLog) << "): ";
		}

		std::stringstream ss; ss << message;
		ss.flush();
		#if defined(DEBUG)
		std::string s{ ss.str() };
		logFile << s;
		#else
		logFile << ss.str();
		#endif
		logFile.flush();
		#if defined(DEBUG)
		OutputDebugStringA("\r\n");
		OutputDebugStringA(s.c_str());
		#endif
	}

	bool CreateLogFile();
	void CompressLog(std::string _filelog);

	void Flush();
	friend LogHelper& operator<<(LogHelper* l, std::stringstream& ss);
	friend LogHelper& operator<<(LogHelper* l, const std::string ss);
	//friend LogHelper& operator<<(LogHelper* l, basic_string<const char*>& ss);

	template<class T>
	friend LogHelper& operator<<(LogHelper* out, const T& rhs);

	template <class T>
	friend std::shared_ptr<LogHelper>& operator<<(std::shared_ptr<LogHelper>& o, const T& thing) {
		std::stringstream ss; ss << thing;
		o->Write(ss.str());
		return o;
	}

	LogHelper& Put(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		char buf[4096];
		vsprintf(buf, fmt, args);
		va_end(args);
		buffer << buf;
		return *this;
	}

	LogHelper& Stream(std::ostringstream& _str) {
		WriteLine(_str.str().c_str());
		return *this;
	}

	struct
	{
		std::string FileName;
		std::string FileLogLastName{""};
		std::string FileNameMask{""};
		std::streampos LogSize = 0;  //4294967296
		ULONG LogLevel = 0;
		USHORT compress = 1;
	} config;

};