#pragma once
#include <Windows.h>

void replaceAll(std::string& s, const std::string& search, const std::string& replace);

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring& wstr);

// Convert an UTF8 string to a wide Unicode String
std::wstring decode_to_utf16(const std::string& str, unsigned long long codepage = CP_UTF8);
std::wstring decode_to_utf16(const char* str, unsigned long long codepage = CP_UTF8);

const std::wstring CharToWString(const char* c);
const std::string WStringToString(const std::wstring& c);
const std::wstring StringToWString(const std::string& c);
//const std::wstring GetWS(const char* c);
const wchar_t* GetWC(const char* c);
void LogEvent(const wchar_t* message, WORD wType = EVENTLOG_SUCCESS, DWORD dwEventID = 0);
void LogEvent(const char* message, WORD wType = EVENTLOG_SUCCESS, DWORD dwEventID=0 );
