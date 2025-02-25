#pragma once
#include "Service.h"
#include "EventLog.h"
#include <Windows.h>
#include <string>
#include <locale>
//#include <codecvt>

#include "FDWrapper.h"
//#include <FiscalsDLLMethods.h>

#define GET_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); })
#define GET_VALUE_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); }).first_child().value()

using std::string;

void Service::GetLastErrorMessage(int fiscalresult, json::value& result) {
	static constexpr size_t lbuf = 16384;
	auto buffer = std::make_unique<char[]>(lbuf);
	int iresult = 0;//FDWrapper::GetLastErrorMessage(buffer.get(), lbuf);
	if (iresult > 0) {
		result[L"message"] = json::value::string(decode_to_utf16(buffer.get(), CP_ACP));
	}
	else {
		if (fiscalresult == -45) { //niezainicjowana biblioteka
			result[L"message"] = json::value::string(L"Niezainicjowany sterownik, pod³¹cz urz¹dzenie i skonfiguruj po³¹czenie");
		}
		else if (fiscalresult == -999) {
			result[L"message"] = json::value::string(L"Przekroczony czas oczekiwania na wynik");
		}
		else {
			std::wstringstream ss;
			ss << L"Niezdefiniowana treœæ b³êdu dla: " << fiscalresult;
			result[L"message"] = json::value::string(ss.str().c_str());
		}
	}
	result[L"result"] = json::value::string(L"ERROR");
}

Service::Service(const pugi::xml_document& configuration) : BasicController() {

	pugi::xml_node api = GET_NODE(configuration, "WebService");

	std::wstring endpoint = StringToWString(api.attribute("endpoit").as_string("/api"));
	std::wstring host = StringToWString(api.attribute("host").as_string("*"));
	std::wstring port = StringToWString(api.attribute("port").as_string("8085"));
	bool ssl = api.attribute("ssl").as_bool(false);

	std::wstring schema{ ssl ? L"https" : L"http" };
	endpointBuilder.set_host(host);
	endpointBuilder.set_port(port);
	endpointBuilder.set_scheme(schema);

	SetUsers(configuration);
	setEndpoint(endpoint);

}

void Service::initRestOpHandlers() {
	_listener.support(methods::GET, std::bind(&Service::handleGet, this, std::placeholders::_1));
	_listener.support(methods::PUT, std::bind(&Service::handlePut, this, std::placeholders::_1));
}

float Service::calculateSum() {
	float sum = 0;
	for (unsigned int i = 0; i < numbers.size(); i++) {
		sum += numbers[i];
	}
	return sum;
}

void Service::GetFeatures(const http_request& message, const std::vector<std::wstring>& path) {
	static constexpr size_t lbuf = 1024;
	pplx::create_task(
		[=]() -> void { \
		json::value result; \
		auto buffer = std::make_unique<char[]>(lbuf); \
		int iresult = 0; /* FDWrapper::PobierzFunkcjeSterownikaDrukarki(buffer.get(), lbuf);*/ \
		if (iresult >= 0) {
			result[L"result"] = json::value::string(L"SUCCESS");
			result[L"message"] = json::value::string(CharToWString(buffer.get()));
			message.reply(status_codes::OK, result);
			return;
		} \
			GetLastErrorMessage(iresult, result); \
			message.reply(status_codes::InternalError, result);

		});
}

void Service::GetLastErrorMessage(const http_request& message, const std::vector<std::wstring>& path) {
	pplx::create_task(
		[=]() -> int {
			static constexpr size_t lbuf = 16384;
	auto buffer = std::make_unique<char[]>(lbuf);
	int iresult = 0; //FDWrapper::GetLastErrorMessage(buffer.get(), lbuf);
	json::value result;
	if (iresult > 0) {
		result[L"message"] = json::value::string(decode_to_utf16(buffer.get(), CP_ACP));
	}
	else {
		result[L"message"] = json::value::string(L"");
	}
	result[L"result"] = json::value::string(L"SUCCESS");

	message.reply(status_codes::OK, result);
	return iresult;
		});
}

void Service::GetFiscalNames(const http_request& message, const std::vector<std::wstring>& path) {
	static constexpr size_t lbuf = 1024;
	pplx::create_task(
		[=]() -> int { \
		auto buffer = std::make_unique<char[]>(lbuf); \
		int iresult = 0; /*FDWrapper::GetFiscalNames(buffer.get(), lbuf)*/ \
		if (iresult >= 0) {
			json::value result;
			result[L"result"] = json::value::string(L"SUCCESS");
			result[L"message"] = json::value::string(CharToWString(buffer.get()));
			message.reply(status_codes::OK, result);
		} \
			return iresult;
		})
		.then(
			[=](pplx::task<int> resultTsk) { \
			int iresult = resultTsk.get(); \
			json::value result; \
			if (iresult < 0) {
				GetLastErrorMessage(resultTsk.get(), result);
				message.reply(status_codes::InternalError, result);
			}
			}
		);
}

void Service::PobierzInformacjeKasowe(const http_request& message, const std::vector<std::wstring>& path) {
	static constexpr size_t lbuf = 2048;
	pplx::create_task(
		[=]() -> int { \
		auto buffer = std::make_unique<char[]>(lbuf); \
		int iresult = 0 /*FDWrapper::PobierzInformacjeKasowe(buffer.get(), lbuf)*/; \
		if (iresult > 0) {
			pugi::xml_document doc;
			if (pugi::xml_parse_result ic_parse = doc.load_string(buffer.get()); ic_parse.status != pugi::xml_parse_status::status_ok) {
				std::stringstream ss; ss << "B³¹d ³adowania odpowiedzi z informacjami kasowymi: " << buffer.get() << ", status: " << ic_parse.status;
				LogEvent(ss.str().c_str(), EVENTLOG_ERROR_TYPE);
				throw std::exception(ss.str().c_str());
			}

			json::value st;
			for(pugi::xml_node item = doc.child("items").first_child(); item; item = item.next_sibling()) {			
				std::string name{item.attribute("name").as_string()};
				std::string value{item.attribute("value").as_string() };
				st[StringToWString(name)] = json::value::string(StringToWString(value));
			}
			json::value result;
			result[L"result"] = json::value::string(L"SUCCESS");
			//result[L"message"] = json::value::string(CharToWString(buffer.get()));
			result[L"message"] = st;
			message.reply(status_codes::OK, result);
		} \
			return iresult;
		})
		.then(
			[=](pplx::task<int> resultTsk) { \
			int iresult = resultTsk.get(); \
			json::value result; \
			if (iresult < 0) {
				GetLastErrorMessage(resultTsk.get(), result);
				message.reply(status_codes::InternalError, result);
			}
			}
		);
}

void Service::DailyReport(const http_request& message, const std::vector<std::wstring>& path) {
	pplx::create_task(
		[=]() -> int { \
		int iresult = -1; \
		if (path.size() == 4) {
			int year = _wtoi(path[1].c_str());
			int month = _wtoi(path[2].c_str());
			int day = _wtoi(path[3].c_str());
			iresult = 0; /*FDWrapper::RaportDobowy(year, month, day);*/
		}
		else {
			iresult = 0; /*FDWrapper::RaportDobowy();*/
		} \
			return iresult;
		})
		.then(
			[=](pplx::task<int> resultTsk) { \
			json::value result; \
			if (resultTsk.get() == 0) {
				result[L"result"] = json::value::string(L"SUCCESS");
				message.reply(status_codes::OK, result);
			}
			else {
				GetLastErrorMessage(resultTsk.get(), result);
				message.reply(status_codes::InternalError, result);
			}
			}
		);
}

void Service::UstawCzas(const http_request& message, const std::vector<std::wstring>& path) {
	pplx::create_task(
		[=]() -> int {
			int iresult = -1;
	if (path.size() == 7) {
		int year = _wtoi(path[1].c_str());
		int month = _wtoi(path[2].c_str());
		int date = _wtoi(path[3].c_str());
		int hour = _wtoi(path[4].c_str());
		int minute = _wtoi(path[5].c_str());
		int second = _wtoi(path[6].c_str());
		iresult = 0; /*FDWrapper::UstawCzas(year, month, date, hour, minute, second);*/
	}
	return iresult;
		})
		.then(
			[=](pplx::task<int> resultTsk) {
				json::value result;
		result[L"fiscalcode"] = json::value::number(resultTsk.get());
		if (resultTsk.get() == 0) {
			result[L"result"] = json::value::string(L"SUCCESS");
			message.reply(status_codes::OK, result);
		}
		else {
			json::value result;
			GetLastErrorMessage(resultTsk.get(), result);
			message.reply(status_codes::InternalError, result);
		}
			}
		);
}

void Service::TestDrukarki(const http_request& message, const std::vector<std::wstring>& path) {
	pplx::create_task(
		[=]() -> int {
			int iresult = 0; /*FDWrapper::TestDrukarki();*/
	return iresult;
		})
		.then(
			[=](pplx::task<int> resultTsk)
			{
				json::value result; \
				if (resultTsk.get() == 0) {
					result[L"result"] = json::value::string(L"SUCCESS");
					message.reply(status_codes::OK, result);
					return;
				} \
					GetLastErrorMessage(resultTsk.get(), result);
				message.reply(status_codes::InternalError, result);
			}
		);
}

void Service::PeriodicReport(const http_request& message, const std::vector<std::wstring>& path) {
	pplx::create_task(
		[=]() -> int {
			int iresult = -1;
	if (path.size() == 7) {
		int year1 = _wtoi(path[1].c_str());
		int month1 = _wtoi(path[2].c_str());
		int day1 = _wtoi(path[3].c_str());
		int year2 = _wtoi(path[4].c_str());
		int month2 = _wtoi(path[5].c_str());
		int day2 = _wtoi(path[6].c_str());
		iresult = 0; /*FDWrapper::RaportOkresowy(year1, month1, day1, year2, month2, day2);*/
	}
	return iresult;
		})
		.then(
			[=](pplx::task<int> resultTsk) {
				json::value result; \
		//result[L"fiscalcode"] = json::value::number(resultTsk.get());
			if (resultTsk.get() == 0) {
				result[L"result"] = json::value::string(L"SUCCESS");
				message.reply(status_codes::OK, result);
			}
			else {
				//json::value result;
				GetLastErrorMessage(resultTsk.get(), result);
				message.reply(status_codes::InternalError, result);
			}
			}
		);
}

void Service::UserLogin(const http_request& message) {
	pplx::create_task(
		[=]() -> std::tuple<bool, UserInformation> {
			try {
		auto headers = message.headers();
		if (headers.find(L"Authorization") == headers.end()) {
			throw std::exception("Bad request for authorization");
		}
		auto authHeader = headers[L"Authorization"];
		auto credsPos = authHeader.find(L"Basic");
		if (credsPos == std::string::npos) {
			throw std::exception("Invalid authorization");
		}
		auto base64 = authHeader.substr(credsPos + std::string("Basic").length() + 1);
		if (base64.empty()) {
			throw std::exception("Invalid basic authorization");
		}

		auto bytes = utility::conversions::from_base64(base64);

		std::string creds(bytes.begin(), bytes.end());
		auto colonPos = creds.find(":");
		if (colonPos == std::string::npos) {
			throw std::exception("Invalid basic authorization, empty user");
		}
		auto useremail = creds.substr(0, colonPos);
		auto password = creds.substr(colonPos + 1, creds.size() - colonPos - 1);

		//UserManager users;
		UserInformation userInfo;
		if (!mgr_users.signOn(useremail, password, userInfo)) {
			throw std::exception("Invalid authorization, user or password");
		}
		return std::make_tuple(true, userInfo);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return std::make_tuple(false, UserInformation{});
	}
		})
		.then(
			[=](pplx::task<std::tuple<bool, UserInformation>> resultTsk)
			{
				auto result = resultTsk.get(); \
				json::value response; \
			if (std::get<0>(result) == true) {
				std::string sname = std::get<1>(result).name + " " + std::get<1>(result).lastName;
				std::wstring name = decode_to_utf16(sname);

				int64_t privateSession = std::get<1>(result).session;
				response[L"result"] = json::value::string(L"SUCCESS");
				response[L"session"] = json::value::number(privateSession);
				response[L"message"] = json::value::string(L"Welcome " + name );
				message.reply(status_codes::OK, response);
				return;
			} \

				response[L"result"] = json::value::string(L"ERROR");
			try {
				message.reply(status_codes::Unauthorized, response);
			}
			catch (...) {
				message.reply(status_codes::BadRequest, response);
			}

			}
		);
}

void Service::UserLogout(const http_request& message, int session) {
	pplx::create_task(
		[=]() -> bool {
			try {
		return mgr_users.logout(session);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return false;
	}
		})
		.then(
			[=](bool resultTsk)
			{
				json::value response;
		response[L"result"] = json::value::string(resultTsk ? L"SUCCESS" : L"ERROR");
		message.reply(status_codes::OK, response);
			}
		);
}

void Service::ConfigureFiscalsDriver(const http_request& message, const json::value& value) {

	try {
		std::wstring p = value.at(U("protocol")).as_string();
		json::value con = value.at(U("connection"));
		int type = con.at(U("type")).as_integer();

		std::wstringstream ss;
		ss << "<Connection type=\"" << type << "\" ";

		if (type == 0) {
			std::wstring port = con.at(U("port")).as_string();
			int baudrate = con.at(U("baudrate")).as_integer();
			std::wstring parity = con.at(U("parity")).as_string();
			std::wstring flowcontrol = con.at(U("flowcontrol")).as_string();
			int databits = con.at(U("databits")).as_integer();
			std::wstring protocol = con.at(U("protocol")).as_string();
			ss <<
				"name=\"" << port << "\" "
				"baudrate=\"" << baudrate << "\" "
				"databits=\"" << databits << "\" "
				"parity=\"" << parity << "\" "
				"flowcontrol=\"" << flowcontrol << "\" " 
				"flowcontrol=\"" << flowcontrol << "\" "
				"protocol=\"" << protocol <<"\"/>";
		}
		std::wcout << ss.str() << std::endl;
		json::value result;

		std::wstring s{ ss.str() };
		FDWrapper::Close();
		FDWrapper::Init(); //(char*)(WStringToString(p).c_str()));
		FDWrapper::SetConnection(string(s.begin(), s.end()).c_str());
		ConnectionState iresult = FDWrapper::OpenDefault();
		if (iresult == ConnectionState::OPEN) {
			//result[L"fiscalcode"] = json::value::number(iresult);
			result[L"result"] = json::value::string(L"SUCCESS");
			message.reply(status_codes::OK, result);
			return;
		}
		GetLastErrorMessage((int)iresult, result);
		message.reply(status_codes::InternalError, result);
	}
	catch (std::exception& ex) {
		LogEvent(ex.what(), EVENTLOG_WARNING_TYPE);
		std::wcout << ex.what() << std::endl;
		json::value result;
		result[L"fiscalcode"] = json::value::number(-10000);
		result[L"result"] = json::value::string(L"EXCEPTION");
		message.reply(status_codes::InternalError, result);
	}
}

void Service::PrintXMLReceipt(const http_request& message, json::value& value) {

	try {
		static std::shared_mutex print_xml;
		std::unique_lock lck(print_xml);
		auto xml = value[U("xml")].as_string();
		auto bytes = utility::conversions::from_base64(xml);
		std::wstring sxml(bytes.begin(), bytes.end());
		int iresult = FDWrapper::PrintFiscal(WStringToString(sxml).c_str());
		json::value result;
		if (iresult >= 0) {
			result[L"fiscal_number"] = json::value::number(iresult);
			result[L"result"] = json::value::string(L"SUCCESS");
		}
		else {
			GetLastErrorMessage(iresult, result);
		}
		message.reply(status_codes::OK, result);
	}
	catch (std::exception& ex) {
		LogEvent(ex.what(), EVENTLOG_WARNING_TYPE);
		std::wcout << ex.what() << std::endl;
		json::value result;
		result[L"result"] = json::value::string(L"EXCEPTION");
		message.reply(status_codes::InternalError, result);
	}
}

void Service::PrintNoFiscalXML(const http_request& message, json::value& value) {

	try {
		static std::shared_mutex print_xml;
		std::unique_lock lck(print_xml);
		auto xml = value[U("xml")].as_string();
		auto bytes = utility::conversions::from_base64(xml);
		std::wstring sxml(bytes.begin(), bytes.end());
		int iresult = FDWrapper::PrintNonFiscal(WStringToString(sxml).c_str());
		json::value result;
		if (iresult >= 0) {
			result[L"result"] = json::value::string(L"SUCCESS");
		}
		else {
			GetLastErrorMessage(iresult, result);
		}
		message.reply(status_codes::OK, result);
	}
	catch (std::exception& ex) {
		LogEvent(ex.what(), EVENTLOG_WARNING_TYPE);
		std::wcout << ex.what() << std::endl;
		json::value result;
		result[L"result"] = json::value::string(L"EXCEPTION");
		message.reply(status_codes::InternalError, result);
	}
}

void Service::SetUsers(const pugi::xml_document& configuration)
{
	pugi::xml_node conf = configuration.child("Configuration");
	if (pugi::xml_node users = GET_NODE(conf, "Users"); !users.empty()) {
		auto lst = users.children("User");
		std::for_each(lst.begin(), lst.end(),
		[&](auto user) {
			UserInformation ui{
				user.attribute("email").as_string(),
				user.attribute("password").as_string(),
				user.attribute("name").as_string(),
				user.attribute("lastname").as_string(),
				0
			};
			mgr_users.signUp(ui);
		});
	}
}

void Service::DetectFiscals(web::http::http_request& message)
{
	json::value result;
	constexpr size_t lbuf = 2048;
	try {
		FDWrapper::Init();
		FDWrapper::Close();
		auto buffer = std::make_unique<char[]>(lbuf);
		FDWrapper::CheckFiscal(buffer.get(), lbuf);

		string fd{ buffer.get() };
		replaceAll(fd, "\r", "");
		replaceAll(fd, "\n", "");
		replaceAll(fd, "\t", "");

		std::wstring ws = decode_to_utf16(fd.c_str(), CP_ACP);
		
		if (ConnectionState iresult = FDWrapper::OpenDefault(); iresult == ConnectionState ::OPEN) {
			GetLastErrorMessage((int)iresult, result);
			result[L"message"] = json::value::string(ws);
		}
		else {
			result[L"result"] = json::value::string(L"SUCCESS");
			result[L"message"] = json::value::string(ws);
		}
		message.reply(status_codes::OK, result);
	}
	catch (std::exception& ex) {
		LogEvent(ex.what(), EVENTLOG_WARNING_TYPE);
		result[L"result"] = json::value::string(L"ERROR");
		result[L"message"] = json::value::string(L"");
		message.reply(status_codes::InternalError, result);
	}
}

void Service::AutomaticlyDetectAndStart(const http_request& message, const vector<std::wstring>& path) {
	pplx::create_task(
		[=]() -> int {
			return FDWrapper::AutomaticlyDetectAndStart();
		})
		.then(
			[=](pplx::task<int> resultTsk) {
				int iresult = resultTsk.get();
		json::value result;
		//result[L"fiscalcode"] = json::value::number(iresult);
		if (resultTsk.get() >= 0) {
			result[L"result"] = json::value::string(L"SUCCESS");
			message.reply(status_codes::OK, result);
			return;
		}
		GetLastErrorMessage(iresult, result);
		message.reply(status_codes::InternalError, result);
			}
		);
}

void Service::handleGet(http_request message) {

	try {
		vector<std::wstring> path = requestPath(message);
		if (path.size() == 0) {
			std::stringstream wss; wss << "Niepoprawne wywo³anie path";
			LogEvent(wss.str().c_str(), EVENTLOG_WARNING_TYPE, 2002);
			throw rest_exception(status_codes::Unauthorized, wss.str().c_str());
		}
		std::wcout << "GET request: " << path[0] << std::endl;

		if (!(path[0] == L"users" && path[1] == L"signon")) {
			auto headers = message.headers();
			if (headers.find(L"session") == headers.end()) {
				LogEvent("Empty session", EVENTLOG_WARNING_TYPE, 2001);
				#ifndef DEBUG
				throw rest_exception(status_codes::Unauthorized, "Empty session");
				#endif
			}
			int requestSession = _wtoi(headers[L"session"].c_str());
			if (!mgr_users.checkSession(requestSession)) {
				std::stringstream wss; wss << "Unauthorized session: " << requestSession;
				LogEvent(wss.str().c_str(), EVENTLOG_WARNING_TYPE, 2001);
				#ifndef DEBUG
				throw rest_exception(status_codes::Unauthorized, wss.str().c_str());
				#endif
			}
		}
		//if (path.empty()) {
		//}
		//else {
		//if (path[0].compare(L"features") == 0) {
		//	GetFeatures(message, path);
		//}
		//else 
		//if (path[0].compare(L"fiscalnames") == 0) {
		//	GetFiscalNames(message, path);
		//}		
		//else 
		if (path[0].compare(L"infocash") == 0) {
			//PobierzInformacjeKasowe(message, path);
		}
		else if (path[0].compare(L"dailyreport") == 0) {
			//DailyReport(message, path);
		}
		else if (path[0].compare(L"periodicreport") == 0) {
			//PeriodicReport(message, path);
		}
		else if (path[0].compare(L"errormessage") == 0) {
			GetLastErrorMessage(message, path);
		}
		else if (path[0].compare(L"detect") == 0) {
			DetectFiscals(message);
		}
		else if (path[0] == L"users" && path[1] == L"signon") {
			UserLogin(message);
		}
		else {
			json::value result;
			result[L"result"] = json::value::string(L"ERROR");
			message.reply(status_codes::BadRequest, result);
		}

	}
	catch (rest_exception& e) {
		LogEvent(e.what(), EVENTLOG_WARNING_TYPE);
		json::value result;
		result[L"result"] = json::value::string(L"ERROR");
		result[L"message"] = json::value::string(StringToWString(e.what()));
		message.reply(e.get_code(), result);
	}
	catch (std::exception& e) {
		LogEvent(e.what(), EVENTLOG_WARNING_TYPE);
		message.reply(status_codes::BadRequest);
	}
}

void Service::handlePut(http_request message) {
	std::wcout << "PUT request: ";
	message
		.extract_json()
		.then([=](pplx::task<json::value> task) { \
			try {
		\
			auto headers = message.headers(); \
			if (headers.find(L"session") == headers.end()) {
				LogEvent("Empty session", EVENTLOG_WARNING_TYPE, 2001);
				#ifndef DEBUG
				throw rest_exception(status_codes::Unauthorized, "Empty session");
				#endif
			} \
				int requestSession = _wtoi(headers[L"session"].c_str());
			if (!mgr_users.checkSession(requestSession)) {
				std::stringstream wss; wss << "Unauthorized session: " << requestSession;
				LogEvent(wss.str().c_str(), EVENTLOG_WARNING_TYPE, 2001);
				#ifndef DEBUG
				throw rest_exception(status_codes::Unauthorized, wss.str().c_str());
				#endif
			}
			std::vector<std::wstring> path = requestPath(message);
			std::wcout << path[0] << std::endl;
			json::value val = task.get();

			if (path[0].compare(L"receiptxml") == 0) {
				PrintXMLReceipt(message, val);
				return;
			}
			else if (path[0].compare(L"nofiscalxml") == 0) {
				PrintNoFiscalXML(message, val);
				return;
			}
			else if (path[0].compare(L"config") == 0) {
				//ConfigureFiscalsDriver(message, val);
				return;
			}
			else if (path[0].compare(L"settime") == 0) {
				//UstawCzas(message, path);
				return;
			}
			else if (path[0].compare(L"test") == 0) {
				//TestDrukarki(message, path);
				return;
			}
			else if (path[0].compare(L"autoconfiguration") == 0) {
				AutomaticlyDetectAndStart(message, path);
				return;
			}
			else if (path[0] == L"users" && path[1] == L"logout") {
				UserLogout(message, requestSession);
				return;
			}
			else {
				int number = val[U("number")].as_number().to_int32();
				numbers.push_back(number);
			}
			json::value result;
			result[L"result"] = json::value::string(L"SUCCESS");
			message.reply(status_codes::OK, result);
	}
	catch (rest_exception& e) {
		LogEvent(e.what(), EVENTLOG_WARNING_TYPE);
		json::value result;
		result[L"result"] = json::value::string(L"ERROR");
		result[L"message"] = json::value::string(StringToWString(e.what()));
		message.reply(e.get_code(), result);
	}
	catch (std::exception& e) {
		LogEvent(e.what(), EVENTLOG_WARNING_TYPE);
		json::value result;
		result[L"result"] = json::value::string(L"ERROR");
		message.reply(status_codes::BadRequest, result);
	}
			});
}
