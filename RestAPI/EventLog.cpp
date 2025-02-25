#include <string>
#include "EventLog.h"

void replaceAll(std::string& s, const std::string& search, const std::string& replace) {
	for (size_t pos = 0; ; pos += replace.length()) {
		// Locate the substring to replace
		pos = s.find(search, pos);
		if (pos == std::string::npos) break;
		// Replace by erasing and inserting
		s.erase(pos, search.length());
		s.insert(pos, replace);
	}
}

std::string utf8_encode(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring decode_to_utf16(const std::string& str, unsigned long long codepage)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
std::wstring decode_to_utf16(const char* str, unsigned long long codepage)
{
	int slen = strlen(str);
	int size_needed = MultiByteToWideChar(codepage, 0, &str[0], slen, NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(codepage, 0, &str[0], slen, &wstrTo[0], size_needed);
	return wstrTo;
}

const std::string WStringToString(const std::wstring& c) {
	std::string st{c.begin(),c.end()};
	return st;
}
const std::wstring StringToWString(const std::string& c) {
	return (std::wstring(c.begin(),c.end()));
}

const std::wstring CharToWString(const char* c) {
	std::string s(c);
	std::wstring ws(s.begin(), s.end());
	return ws;
}

//const std::wstring GetWS(const char* c) {
//	std::string st{c};
//	std::wstring cnv(st.begin(),st.end());
//	return cnv;
//}

const wchar_t* GetWC(const char* c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

void LogEvent(const wchar_t* message, WORD wType, DWORD dwEventID) {
	static const wchar_t* custom_log_name = L"FiscalsDriverService";
	static HANDLE event_log = RegisterEventSource(NULL, custom_log_name);
	if (event_log) {
		ReportEvent(event_log, wType, 0, dwEventID, NULL, 1, 0, &message, NULL);
	}
}
void LogEvent(const char* message, WORD wType, DWORD dwEventID) {
	static const wchar_t* custom_log_name = L"FiscalsDriverService";
	static HANDLE event_log = RegisterEventSource(NULL, custom_log_name);
	if (event_log) {
		ReportEventA(event_log, wType, 0, dwEventID, NULL, 1, 0, &message, NULL);
	}
}