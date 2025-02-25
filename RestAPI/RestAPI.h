#pragma once
#include <iostream>
#include <Windows.h>
#include <filesystem>
#include "pugixml.hpp"
#include "UserManager.h"

#define GET_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); })
#define GET_VALUE_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); }).first_child().value()

TCHAR spath[MAX_PATH];
std::filesystem::path path;
pugi::xml_document configuration;

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;
HANDLE ghMutex = NULL;
wchar_t* SERVICE_NAME = (wchar_t*)L"FiscalsDriverServices";

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
void SvcInstall();
VOID __stdcall DoDeleteSvc();
VOID __stdcall DoStopSvc();
BOOL __stdcall StopDependentServices(SC_HANDLE schSCManager, SC_HANDLE schService);