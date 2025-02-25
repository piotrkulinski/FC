#pragma once

#include <tuple>
#include <utility>
#include <algorithm>
#include <winioctl.h>
#include "atlstr.h"
#include <vector>
#include <SetupAPI.h>
#include <iomanip>
#include <string_view>
#include "crc_calculate.h"
#include <iostream>
#include <shared_mutex>
#include "MyUtils.h"
#include <Connection.h>
#include <ConnectionRS232.h>
#include "logger_definition.h"
#include "ErrorMode.h"
#include "FiscalTypes.h"

using DriverClass = std::unique_ptr<std::string>;

#if defined(TEST_LOGGER)
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#endif

#pragma comment(lib, "setupapi.lib") 

class CFiscalFactory
{
private:
	static int RegQueryValueString(_In_ ATL::CRegKey& key, _In_ LPCTSTR lpValueName, _Out_ LPTSTR& pszValue) {
		//Initialize the output parameter
		pszValue = NULL;

		//First query for the size of the registry value
		ULONG nChars = 0;
		LSTATUS nStatus = key.QueryStringValue(lpValueName, NULL, &nChars);
		if (nStatus != ERROR_SUCCESS) {
			SetLastError(nStatus);
			return 1;
		}

		//Allocate enough bytes for the return value
		DWORD dwAllocatedSize = ((nChars + 1) * sizeof(TCHAR)); //+1 is to allow us to NULL terminate the data if required
		pszValue = reinterpret_cast<LPTSTR>(LocalAlloc(LMEM_FIXED, dwAllocatedSize));
		if (pszValue == NULL)
			return 1;

		//We will use RegQueryValueEx directly here because ATL::CRegKey::QueryStringValue does not handle non-Null terminated data
		DWORD dwType = 0;
		ULONG nBytes = dwAllocatedSize;
		pszValue[0] = _T('\0');
		nStatus = RegQueryValueEx(key, lpValueName, NULL, &dwType, reinterpret_cast<LPBYTE>(pszValue), &nBytes);
		if (nStatus != ERROR_SUCCESS) {
			LocalFree(pszValue);
			pszValue = NULL;
			SetLastError(nStatus);
			return 1;
		}
		if (((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ)) || ((nBytes % sizeof(TCHAR)) != 0)) {
			LocalFree(pszValue);
			pszValue = NULL;
			SetLastError(ERROR_INVALID_DATA);
			return 1;
		}
		if (pszValue[(nBytes / sizeof(TCHAR)) - 1] != _T('\0')) {
			//Forcibly NULL terminate the data ourselves
			pszValue[(nBytes / sizeof(TCHAR))] = _T('\0');
		}

		return 0;
	}

	#if defined(TEST_LOGGER)

	std::shared_ptr<LogHelper> fdlogger;
	std::streambuf* coutbuf = std::cout.rdbuf();
	std::streambuf* cerrbuf = std::cerr.rdbuf();
	std::streambuf* clogbuf = std::clog.rdbuf();
	const char* logPath = "logs";
	#endif


public:
	CFiscalFactory(void) 
	{
		#ifdef TEST_LOGGER
		DWORD fileAttr = GetFileAttributesA(logPath);
		if (!(fileAttr != INVALID_FILE_ATTRIBUTES && (fileAttr & FILE_ATTRIBUTE_DIRECTORY))) {
			CreateDirectoryA(logPath, NULL);
		}

		fdlogger = LogHelper::instance();

		// musi byæ pierwsze logowanie, poniewa¿
		// fdlogger->GetOutputStream().rdbuf() pobiera wskaŸnik do ofstream pliku, a plik jest tworzony przy pierwszym logowanym tekœcie
		fdlogger->Version();
		fdlogger << "\n";
		std::cout.rdbuf(fdlogger->GetOutputStream().rdbuf());
		std::cerr.rdbuf(fdlogger->GetOutputStream().rdbuf());
		std::clog.rdbuf(fdlogger->GetOutputStream().rdbuf());
		std::cout.flush();
		std::cerr.flush();
		std::clog.flush();
		std::cout << "Constructor: " << __FUNCTION__ << std::endl;
		#endif
	}

	~CFiscalFactory(void) 
	{
		std::cout << __FUNCTION__ << ": zwolnienie sterownika " << std::endl;
		#if defined(TEST_LOGGER)
		try {

			//dll_connection.reset();
			//dll_protocol.reset();

			std::cout.flush();
			std::clog.flush();
			std::cout.rdbuf(coutbuf);
			std::clog.rdbuf(cerrbuf);
			std::cerr.rdbuf(clogbuf);
			fdlogger->Close();

		}
		catch (std::exception& ex) {
			std::cerr << __FUNCTION__ << " exception!: " << ex.what();
		}
		#endif
	}

	static size_t DetectFiscalPrinters(XmlApiDetectFiscalResponse response, size_t len = 8192);
};
