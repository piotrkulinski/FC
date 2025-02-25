#include "FDWrapper.h"
#include "pugixml.hpp"


#define GET_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); })
#define GET_VALUE_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); }).first_child().value()

int FDWrapper::PrintFiscal(XmlApiFiscalRequest xml_request) {
	typedef int(__stdcall* f_PrintFiscal)(XmlApiFiscalRequest);
	static f_PrintFiscal CALL_PrintFiscal{
		(f_PrintFiscal)GetProcAddress(FDWrapper::hDLL, "PrintFiscal")
	};

	int result = -1;
	result = CALL_PrintFiscal(xml_request);

	return result;
}
int FDWrapper::PrintNonFiscal(XmlApiNonFiscalRequest xml_request) {
	typedef int(__stdcall* f_PrintNonFiscal)(XmlApiNonFiscalRequest);
	static f_PrintNonFiscal CALL_PrintNonFiscal{
		(f_PrintNonFiscal)GetProcAddress(FDWrapper::hDLL, "PrintNonFiscal")
	};

	int result = CALL_PrintNonFiscal(xml_request);

	return result;
}


int FDWrapper::CheckFiscal(XmlApiDetectFiscalResponse outxml, size_t buffer_size) {
	typedef int(__stdcall* f_DetectFiscal)(XmlApiDetectFiscalResponse, size_t);
	static f_DetectFiscal CALL_DetectFiscal{
		(f_DetectFiscal)GetProcAddress(FDWrapper::hDLL, "DetectFiscal")
	};

	int result = -999;
	if (wait_fiscal.try_lock_until(std::chrono::steady_clock::now() + std::chrono::seconds(timeout_seconds_operation))) {
		result = CALL_DetectFiscal(outxml, buffer_size);
		wait_fiscal.unlock();
	}
	return result;
}


int FDWrapper::SetConnection(XmlApiConfigurationRequest conf) {
	typedef int(__stdcall* f_SetConnection)(XmlApiConfigurationRequest);
	static f_SetConnection CALL_SetConnection{
		(f_SetConnection)GetProcAddress(FDWrapper::hDLL, "SetConnection")
	};

	int result = -999;
	if (wait_fiscal.try_lock_until(std::chrono::steady_clock::now() + std::chrono::seconds(timeout_seconds_operation))) {
		result = CALL_SetConnection(conf);
		wait_fiscal.unlock();
	}
	return result;
}

int FDWrapper::Close() {
	if (FDWrapper::hDLL == NULL) {
		return 0;
	}
	typedef int(__stdcall* f_Close)();
	static f_Close CALL_Close{
		(f_Close)GetProcAddress(FDWrapper::hDLL, "Close")
	};
	int result = -999;
	if (wait_fiscal.try_lock_until(std::chrono::steady_clock::now() + std::chrono::seconds(timeout_seconds_operation))) {
		result = CALL_Close();
		wait_fiscal.unlock();
	}
	return result;
}

ConnectionState FDWrapper::OpenDefault() {
	typedef ConnectionState(__stdcall* f_OpenDefault)();
	static f_OpenDefault CALL_OpenDefault{
		(f_OpenDefault)GetProcAddress(FDWrapper::hDLL, "OpenDefault")
	};
	ConnectionState result = ConnectionState::UNKNOW;
	if (wait_fiscal.try_lock_until(std::chrono::steady_clock::now() + std::chrono::seconds(timeout_seconds_operation))) {
		result = CALL_OpenDefault();
		wait_fiscal.unlock();
	}
	return result;
}

int FDWrapper::AutomaticlyDetectAndStart() {
	constexpr size_t lbuf = 2048;
	int iresult = -999;
	//if (!wait_fiscal.try_lock_until(chrono::steady_clock::now() + chrono::seconds(timeout_seconds_operation))) {
	//	return iresult;
	//}
	//wait_fiscal.unlock();
	iresult = -10000;
	static std::shared_mutex auto_open;
	std::unique_lock lck(auto_open);

	try {
		Close();
		auto buffer = std::make_unique<char[]>(lbuf);
		CheckFiscal(buffer.get(), lbuf);

		pugi::xml_document doc;
		pugi::xml_parse_result confxml = doc.load_string(buffer.get());
		pugi::xml_node xport = GET_NODE(doc, "Settings");

		pugi::xml_node driver = GET_NODE(doc, "Driver");
		char* protocol = (char*)driver.attribute("recomended").as_string();

		std::stringstream ss;
		ss << "<Connection type=\"" << 0 << "\" ";
		if (xport != NULL && 0 == 0) {
			ss <<
				"name=\"COM" << xport.attribute("number").as_string() << "\" "
				"baudrate=\"" << xport.attribute("baudrate").as_llong(9600) << "\" "
				"databits=\"" << xport.attribute("databits").as_uint(8) << "\" "
				"stopbits=\"" << xport.attribute("stopbits").as_uint(1) << "\" "
				"parity=\"" << xport.attribute("parity").as_string("N") << "\" "
				"flowcontrol=\"" << xport.attribute("flowcontrol").as_string("N")[0] << "\" "
				"protocol=\"" << protocol;
		}
		else {
			ss <<
				"ip=\"" << xport.attribute("ip").as_string() << "\" "
				"port=\"" << xport.attribute("port").as_llong();
		}
		ss << "\"/>";



		//FDWrapper::InitFiscal();
		FDWrapper::SetConnection(ss.str().c_str());
		ConnectionState open_s = FDWrapper::OpenDefault();
		if (open_s == ConnectionState::OPEN) {
			iresult = 1;
		}
	}
	catch (std::exception& ex) {
		//LogEvent(ex.what(), EVENTLOG_WARNING_TYPE);
	}
	wait_fiscal.unlock();
	return iresult;
}

void FDWrapper::Destroy() {
	if (FDWrapper::hDLL != NULL) {
		static std::shared_mutex fdestroy;
		std::unique_lock lck(fdestroy);
		std::cout << "Destroy library, wait for lock..." << std::endl;
		FDWrapper::Close();
		FreeLibrary(FDWrapper::hDLL);
		FDWrapper::hDLL = NULL;
		std::cout << "Fiscals destroy" << std::endl;
		wait_fiscal.unlock();
	}
}