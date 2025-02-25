#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <wtypes.h>
#include <shared_mutex>
#include <FiscalTypes.h>
#include "ConnectionState.h"

using wait_for_compleat_fiscal_operation = std::unique_lock<std::shared_timed_mutex>;

class FDWrapper
{
public:
	static HMODULE hDLL;

public:
	static inline long long timeout_seconds_operation = 20;
	static inline std::shared_timed_mutex wait_fiscal;

	FDWrapper() {
		FDWrapper::Init("FiscalsDriver.dll");
	}
	~FDWrapper() {
		FDWrapper::Destroy();
	}
public:
	static int Init(std::string dllFile = "FiscalsDriver.dll") {
		wait_for_compleat_fiscal_operation lck(wait_fiscal);
		if (hDLL == NULL) {
			std::cout << "Wait for load fiscals..." << std::endl;
			hDLL = LoadLibraryA(dllFile.c_str());
			if (!hDLL) {
				std::cout << "Error load library: " << GetLastError() << std::endl;
				wait_fiscal.unlock();
				return -1;
			}
			std::cout << "Successfull" << std::endl;
		}
		wait_fiscal.unlock();
		return -1;
	}

	static int PrintFiscal(XmlApiFiscalRequest);
	static int PrintNonFiscal(XmlApiNonFiscalRequest);

	static int CheckFiscal(XmlApiDetectFiscalResponse, size_t len);
	static int SetConnection(XmlApiConfigurationRequest);
	static int Close();
	static ConnectionState OpenDefault();
	static int AutomaticlyDetectAndStart();
	static void Destroy();
};

inline HMODULE FDWrapper::hDLL = NULL; //LoadLibraryA("FiscalsDriver.dll");