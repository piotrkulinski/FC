// resttest.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//
#include <iostream>
#include <Windows.h>

#include "EventLog.h"
#include "Service.h"
#include "FDWrapper.h"
#include <filesystem>

#include "RestAPI.h"

pugi::xml_document LoadConfiguration(const std::string& configuration) {

	pugi::xml_document doc;
	pugi::xml_parse_result confxml = doc.load_file(configuration.c_str()); //.load_string(configuration.c_str());
	if (confxml.status != pugi::xml_parse_status::status_ok) {
		std::stringstream ss; ss << "Błąd ładowania konfiguracji: " << configuration <<", status: " << confxml.status;
		LogEvent(ss.str().c_str(), EVENTLOG_ERROR_TYPE);
		throw std::exception(ss.str().c_str());
	}
	return doc;
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal.
		case CTRL_C_EVENT:
			printf("Ctrl-C event\n");
			SetEvent(g_ServiceStopEvent);
			Beep(750, 300);
			return TRUE;

			// CTRL-CLOSE: confirm that the user wants to exit.
		case CTRL_CLOSE_EVENT:
			printf("Ctrl-Close event\n");
			SetEvent(g_ServiceStopEvent);
			Beep(600, 200);
			return TRUE;

			// Pass other signals to the next handler.
		case CTRL_BREAK_EVENT:
			printf("Ctrl-Break event\n");
			SetEvent(g_ServiceStopEvent);
			Beep(900, 200);
			return FALSE;

		case CTRL_LOGOFF_EVENT:
			printf("Ctrl-Logoff event\n");
			Beep(1000, 200);
			return FALSE;

		case CTRL_SHUTDOWN_EVENT:
			printf("Ctrl-Shutdown event\n");
			Beep(750, 500);
			return FALSE;

		default:
			return FALSE;
	}
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	LogEvent(spath);
	SetCurrentDirectory(spath);
	DWORD fileAttr = GetFileAttributes(L"logs");
	if (!(fileAttr != INVALID_FILE_ATTRIBUTES && (fileAttr & FILE_ATTRIBUTE_DIRECTORY))) {
		CreateDirectory(L"logs", NULL);
	}

	FDWrapper::Init();
	if (FDWrapper::AutomaticlyDetectAndStart() < 0) {
		std::stringstream ss; ss << "Nie udało się automatycznie podłączyć urządzenia";
		LogEvent(ss.str().c_str(), EVENTLOG_ERROR_TYPE, 2000);
	}
	auto configuration = LoadConfiguration("wsconfiguration.xml");

	Service serv(configuration);
	serv.accept().wait();
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		Sleep(1000);
	}
	FDWrapper::Destroy();
	return ERROR_SUCCESS;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
		case SERVICE_CONTROL_STOP:
			LogEvent("Stop service");

			if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
				break;
			 /*
			  * Perform tasks necessary to stop the service here
			  */
			g_ServiceStatus.dwControlsAccepted = 0;
			g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			g_ServiceStatus.dwWin32ExitCode = 0;
			g_ServiceStatus.dwCheckPoint = 4;

			if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
			{
				LogEvent("My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error");
			}

			// This will signal the worker thread to start shutting down
			SetEvent(g_ServiceStopEvent);

			break;

		default:
			break;
	}
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	LogEvent(argv[1]);
	DWORD Status = E_FAIL;
	g_StatusHandle = RegisterServiceCtrlHandler((LPWSTR)SERVICE_NAME, ServiceCtrlHandler);
	if (g_StatusHandle == NULL)
	{
		CloseHandle(ghMutex);
		return;
	}

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		LogEvent("My Sample Service: ServiceMain: SetServiceStatus returned error");
		CloseHandle(ghMutex);
	}

	// Create a service stop event to wait on later
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		// Error creating event
		// Tell service controller we are stopped and exit
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(L"My Sample Service: ServiceMain: SetServiceStatus returned error");
		}
		return;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		LogEvent("My Sample Service: ServiceMain: SetServiceStatus returned error");
		CloseHandle(ghMutex);
		return;
	}

	try {
		HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, 0, 0, NULL);
		if (hThread != NULL) {
			std::cout << "thread working..." << std::endl;
			WaitForSingleObject(hThread, INFINITE);
		}
	}
	catch (std::exception& ex) {
		LogEvent(ex.what(), EVENTLOG_ERROR_TYPE);
	}
	ReleaseMutex(ghMutex);
	CloseHandle(ghMutex);
	CloseHandle(g_ServiceStopEvent);

// Tell the service controller we are stopped
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		LogEvent("My Sample Service: ServiceMain: SetServiceStatus returned error");
	}
}

std::string GetAppFromPath()
{
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	std::wstring test(&szPath[0]); //convert to wstring
	std::string test2(test.begin(), test.end()); //and convert to string.

	size_t pos = test2.find_last_of("\\");
	if (pos != std::string::npos) {
		return test2.substr(pos + 1);
	}
	return test2;
}

void SvcInstall() //int argc, char** argv)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];
	TCHAR path[MAX_PATH];
	TCHAR qpath[MAX_PATH];

	std::stringstream ss; ss << "Installing the service..." << std::endl;
	LogEvent(ss.str().c_str());
	std::cout << ss.str();

	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		return;
	}
	wsprintf(qpath, L"\"%s\" --run", szPath);

	// Get a handle to the SCM database. 
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);  // full access rights 
//		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
		return;

	// Create the service
	schService = CreateService(
		schSCManager,              // SCM database 
		SERVICE_NAME,      // name of service 
		SERVICE_NAME,      // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, // service type 
		SERVICE_AUTO_START,        // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		qpath,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 

	if (schService == NULL)
	{
		if (DWORD error = GetLastError(); error != 0) //ERROR_SERVICE_EXISTS)
		{
			std::cout << "Error: " << error << std::endl;
		}
		CloseServiceHandle(schSCManager);
		std::stringstream ss; ss << "Install filed!" << std::endl;
		LogEvent(ss.str().c_str());
		return;
	}
	//CloseServiceHandle(schSCManager);
	//CloseServiceHandle(schService);

	schService = OpenService(schSCManager, SERVICE_NAME, GENERIC_ALL);
	if (!schService)
	{
		std::stringstream ss; ss << "open service failed: " << GetLastError() << std::endl;
		LogEvent(ss.str().c_str());
	}
	else {
		StartService(schService, 0, NULL);
	}
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	//}
}

VOID __stdcall DoDeleteSvc()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS ssStatus;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		std::stringstream ss; ss << "OpenSCManager failed " << GetLastError() << std::endl;
		LogEvent(ss.str().c_str());
		std::cout << ss.str();
		return;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,       // SCM database 
		SERVICE_NAME,          // name of service 
		DELETE);            // need delete access 

	if (schService == NULL)
	{
		std::stringstream ss; ss << "OpenService failed " << GetLastError() << std::endl;
		LogEvent(ss.str().c_str());
		std::cout << ss.str();
		CloseServiceHandle(schSCManager);
		return;
	}

	// Delete the service.

	if (!DeleteService(schService))
	{
		std::stringstream ss; ss << "DeleteService failed " << GetLastError() << std::endl;
		LogEvent(ss.str().c_str());
		std::cout << ss.str();
	}
	else {
		std::stringstream ss; ss << "Service deleted successfully" << std::endl;
		LogEvent(ss.str().c_str());
		std::cout << ss.str();
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

VOID __stdcall DoStopSvc()
{

	SERVICE_STATUS_PROCESS ssp;
	ULONGLONG dwStartTime = GetTickCount64();
	DWORD dwBytesNeeded;
	DWORD dwTimeout = 30000; // 30-second time-out
	DWORD dwWaitTime;

	SC_HANDLE schService;
	SC_HANDLE schSCManager;

	auto stop_cleanup = [&]() {
		if (schService != NULL) {
			CloseServiceHandle(schService);
		}
		if (schSCManager != NULL) {
			CloseServiceHandle(schSCManager);
		}
	};

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (schSCManager == NULL)
	{
		std::stringstream ss; ss << "OpenSCManager failed " << GetLastError() << std::endl;
		LogEvent(ss.str().c_str());
		return;
	}

	// Get a handle to the service.
	schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS);
	if (schService == NULL)
	{
		std::stringstream ss; ss << "OpenService failed " << GetLastError() << std::endl;
		LogEvent(ss.str().c_str());
		stop_cleanup();
		return;
	}

	// Make sure the service is not already stopped.
	if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
	{
		std::stringstream ss; ss << "QueryServiceStatusEx failed " << GetLastError() << std::endl;
		LogEvent(ss.str().c_str());
		stop_cleanup();
		return;
	}

	if (ssp.dwCurrentState == SERVICE_STOPPED)
	{
		std::stringstream ss; ss << "Service is already stopped." << std::endl;
		LogEvent(ss.str().c_str());
		std::cout << ss.str();
		stop_cleanup();
		return;
	}

	// If a stop is pending, wait for it.
	while (ssp.dwCurrentState == SERVICE_STOP_PENDING)
	{
		std::stringstream ss; ss << "Service stop pending..." << std::endl;
		LogEvent(ss.str().c_str());
		std::cout << ss.str();
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssp.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
		{
			std::stringstream ss; ss << "QueryServiceStatusEx failed " << GetLastError() << std::endl;
			LogEvent(ss.str().c_str());
			stop_cleanup();
			return;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
		{
			std::stringstream ss; ss << "Service stopped successfully" << std::endl;
			LogEvent(ss.str().c_str());
			std::cout << ss.str();
			stop_cleanup();
			return;
		}

		if (GetTickCount64() - dwStartTime > dwTimeout)
		{
			std::stringstream ss; ss << "Service stop timed out!" << std::endl;
			LogEvent(ss.str().c_str());
			std::cout << ss.str();
			stop_cleanup();
			return;
		}
	}

	// If the service is running, dependencies must be stopped first.
	StopDependentServices(schSCManager, schService);

	// Send a stop code to the service.
	if (!ControlService(schService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp))
	{
		std::stringstream ss; ss << "ControlService failed " << GetLastError() << std::endl;
		LogEvent(ss.str().c_str());
		std::cout << ss.str();
		stop_cleanup();
		return;
	}

	// Wait for the service to stop.
	while (ssp.dwCurrentState != SERVICE_STOPPED)
	{
		Sleep(ssp.dwWaitHint);
		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
		{
			std::stringstream ss; ss << "QueryServiceStatusEx failed " << GetLastError() << std::endl;
			LogEvent(ss.str().c_str());
			std::cout << ss.str();
			stop_cleanup();
			return;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
			break;

		if (GetTickCount64() - dwStartTime > dwTimeout)
		{
			std::stringstream ss; ss << "Wait timed out" << std::endl;
			LogEvent(ss.str().c_str());
			std::cout << ss.str();
			stop_cleanup();
			return;
		}
	}

	std::stringstream ss; ss << "Service stopped successfully" << std::endl;
	LogEvent(ss.str().c_str());
	std::cout << ss.str();
}

BOOL __stdcall StopDependentServices(SC_HANDLE schSCManager, SC_HANDLE schService)
{
	DWORD i;
	DWORD dwBytesNeeded;
	DWORD dwCount;

	LPENUM_SERVICE_STATUS   lpDependencies = NULL;
	ENUM_SERVICE_STATUS     ess;
	SC_HANDLE               hDepService;
	SERVICE_STATUS_PROCESS  ssp;

	ULONGLONG dwStartTime = GetTickCount64();
	DWORD dwTimeout = 30000; // 30-second time-out

	// Pass a zero-length buffer to get the required buffer size.
	if (EnumDependentServices(schService, SERVICE_ACTIVE,
		lpDependencies, 0, &dwBytesNeeded, &dwCount))
	{
		 // If the Enum call succeeds, then there are no dependent
		 // services, so do nothing.
		return TRUE;
	}
	else
	{
		if (GetLastError() != ERROR_MORE_DATA)
			return FALSE; // Unexpected error

		// Allocate a buffer for the dependencies.
		lpDependencies = (LPENUM_SERVICE_STATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

		if (!lpDependencies)
			return FALSE;

		__try {
			// Enumerate the dependencies.
			if (!EnumDependentServices(schService, SERVICE_ACTIVE,
				lpDependencies, dwBytesNeeded, &dwBytesNeeded, &dwCount))
				return FALSE;

			for (i = 0; i < dwCount; i++)
			{
				ess = *(lpDependencies + i);
				// Open the service.
				hDepService = OpenService(schSCManager, ess.lpServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
				if (!hDepService)
					return FALSE;

				__try {
					// Send a stop code.
					if (!ControlService(hDepService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp))
						return FALSE;

						// Wait for the service to stop.
					while (ssp.dwCurrentState != SERVICE_STOPPED)
					{
						Sleep(ssp.dwWaitHint);
						if (!QueryServiceStatusEx(
							hDepService,
							SC_STATUS_PROCESS_INFO,
							(LPBYTE)&ssp,
							sizeof(SERVICE_STATUS_PROCESS),
							&dwBytesNeeded))
							return FALSE;

						if (ssp.dwCurrentState == SERVICE_STOPPED)
							break;

						if (GetTickCount64() - dwStartTime > dwTimeout)
							return FALSE;
					}
				}
				__finally
				{
					// Always release the service handle.
					CloseServiceHandle(hDepService);
				}
			}
		}
		__finally
		{
			// Always free the enumeration buffer.
			HeapFree(GetProcessHeap(), 0, lpDependencies);
		}
	}
	return TRUE;
}

void usage()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCP(1250);
	SetConsoleOutputCP(1250); //65001);
	std::cout << std::endl
		<< "FiscalsDriverService - rest'owy serwis do wydruków fiskalnych, silnik to FiscalsDriver.dll" << std::endl
		<< "Copyright (C) 2023 Piotr Kuliński" << std::endl
		<< std::endl;
	SetConsoleTextAttribute(hConsole, 10);
	std::string app = GetAppFromPath();
	std::cout
		<< "usage:" << std::endl
		<< "\t" << app << " [--install|-i] [--uninstall|-u] [--test|-t]" << std::endl
		<< std::endl
		<< "example:" << std::endl
		<< "\t" << app << " --install - instalacja i uruchomienie usługi" << std::endl
		<< "\t" << app << " --uninstall - deinstalacja usługi" << std::endl
		<< "\t" << app << " --test - test inline usługi" << std::endl
		<< std::endl;
	SetConsoleTextAttribute(hConsole, 7);
}

int main(int argc, char** argv)
{
	if (!GetModuleFileName(NULL, spath, MAX_PATH))
	{
		return -1;
	}
	std::wstring p(spath);
	size_t found = p.find_last_of(L"\\");
	spath[found] = '\0';

	LogEvent(spath);

	//path = std::filesystem::current_path(); //getting path
	//LogEvent(path.c_str());

	ghMutex = CreateMutex(NULL, FALSE, SERVICE_NAME);
	if (DWORD err = GetLastError(); ghMutex == NULL || err == ERROR_ALREADY_EXISTS)
	{
		std::wcout << SERVICE_NAME << L" instance arleady exist" << std::endl;
		Beep(750, 300);
		return 1;
	}

	auto cleanUp = [&]() {
		if (ghMutex != NULL) {
			ReleaseMutex(ghMutex);
			CloseHandle(ghMutex);
		}
	};

	if (argc > 1 && _strcmpi(argv[1], "--run") != 0) {
		SetConsoleOutputCP(1250);

		bool longarg = (strncmp(argv[1], "--", 2) == 0);
		if ((longarg && _strcmpi(argv[1], "--install") == 0) || strcmp(argv[1], "-i") == 0)
		{
			GetCurrentDirectory(MAX_PATH, spath);
			//DoStopSvc();
			//DoDeleteSvc();
			SvcInstall();
			std::cout << "finish." << std::endl;
			cleanUp();
			return 1;
		}

		if ((longarg && _strcmpi(argv[1], "--uninstall") == 0) || strcmp(argv[1], "-u") == 0)
		{
			std::stringstream ss; ss << "Uninstalling the service..." << std::endl;
			LogEvent(ss.str().c_str());
			std::cout << ss.str();
			DoStopSvc();
			DoDeleteSvc();
			std::cout << "finish." << std::endl;
			cleanUp();
			return 1;
		}

		if ((longarg && _strcmpi(argv[1], "--help") == 0) || _strcmpi(argv[1], "/?") == 0 || _strcmpi(argv[1], "?") == 0 || _strcmpi(argv[1], "-?") == 0) {
			usage();
			cleanUp();
			return 0;
		}
		if ((longarg && _strcmpi(argv[1], "--test") == 0) || strcmp(argv[1], "-t") == 0)
		{
			SetConsoleOutputCP(1250);
			usage();
			std::wcout << "Testing service" << std::endl;
			#ifdef DEBUG
			std::wcout << "Debug version, without authorization session" << std::endl;
			#endif
			SetConsoleCtrlHandler(CtrlHandler, TRUE);
			g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			ServiceWorkerThread(0);
			cleanUp();
			return 1;
		}
		return 2;
	}
	else if (argc == 1) {
		usage();
		cleanUp();
		return 0;
	}

	LogEvent(L"Start service");
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		std::stringstream ss; ss << "Start service error: " << GetLastError();
		LogEvent(ss.str().c_str(), EVENTLOG_ERROR_TYPE);
		cleanUp();
		return -1;
	}

	return 0;
		}