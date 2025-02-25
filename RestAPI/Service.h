#pragma once
#ifndef SERVICE_H_INCLUDED
#define SERVICE_H_INCLUDED

#include "BasicController.h"
#include <vector>
#include "UserManager.h"
#include "pugixml.hpp"

using std::vector;

class rest_exception : public std::exception {
private:
	http::status_code exit_code;
public:
	inline explicit rest_exception(const http::status_code _exit_code, const char* message) : std::exception(message) {
		exit_code = _exit_code;
	}
	const http::status_code get_code() {
		return exit_code;
	}
};

class Service : public BasicController
{
private:
	UserManager mgr_users;
	int64_t session = 0;
	std::vector<int> numbers;
	float calculateSum();
	void GetFeatures(const http_request& message, const std::vector<std::wstring>& path);
	void DailyReport(const http_request& message, const std::vector<std::wstring>& path);
	void PeriodicReport(const http_request& message, const std::vector<std::wstring>& path);
	void UserLogin(const http_request& message);
	void ConfigureFiscalsDriver(const http_request&, const json::value&);
	void UstawCzas(const http_request& message, const std::vector<std::wstring>& path);
	void TestDrukarki(const http_request& message, const std::vector<std::wstring>& path);
	void GetLastErrorMessage(int fiscalresult, json::value& value);
	void PrintXMLReceipt(const http_request& message, json::value& value);
	void PrintNoFiscalXML(const http_request& message, json::value& value);
	void SetUsers(const pugi::xml_document& configuration);
	void AutomaticlyDetectAndStart(const http_request& message, const std::vector<std::wstring>& path);
	void GetFiscalNames(const http_request& message, const std::vector<std::wstring>& path);
	void UserLogout(const http_request& message, int session);
	void GetLastErrorMessage(const http_request& message, const std::vector<std::wstring>& path);
	void DetectFiscals(web::http::http_request& message);
	void PobierzInformacjeKasowe(const http_request& message, const std::vector<std::wstring>& path);

public:
	Service(const std::wstring& address, const std::wstring& port) : BasicController(address, port) {}
	Service(const pugi::xml_document& configuration);
	~Service() {}

	void handleGet(http_request message);
	void handlePut(http_request message);
	void initRestOpHandlers() override;

};
#endif // SERVICE_H_INCLUDED
