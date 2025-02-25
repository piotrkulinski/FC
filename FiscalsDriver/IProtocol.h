#pragma once

#include <iostream>
#include <shared_mutex>
#include <vector>

#include "xmlapi/paragon.h"
#include "EventType.h"
#include "FiscalExceptions.h"
#include "connection\Connection.h"
#include "DeviceProtocol.h"

#include "AnswerType.h"

#include "FiscalError.h"
#include "FiscalStatus.h"
#include "FiscalResult.h"
#include "CallbackType.h"
#include "CState.h"
#include "xmlapi/XMLCashDocument.h"

//extern int FASTCALL _Execute(char FAR* stmt);
//extern int _fastcall _Execute(char* stmt);

#define GET_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); })
#define GET_VALUE_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); }).first_child().value()
#define COMPARE_TYPE(request,_class) typeid(request->getType()) == typeid(_class)
#define CAST_TYPE(request,_class) static_cast<_class*>(request.get())

constexpr unsigned long long operator "" _sec(unsigned long long t)
{
	return t * 1000;
}

using lock_receipt_protocol = std::unique_lock<std::shared_mutex>;

class IProtocol
{
private:
	std::thread::id current_thread;

	//Aktualnie zaimplementowane i obs�ugiwane protoko�y
	inline static std::map<DeviceProtocol, std::string> protocols{
			 {DeviceProtocol::Posnet,"POSNET"}
			,{DeviceProtocol::Thermal,"THERMAL"}
			,{DeviceProtocol::ElzabDLL,"ELZAB_DLL"}
	};

protected:
	#pragma region Metody protected	

	//EventHandler callbackEvent = nullptr;
	FoxEventHandler callAnswerCI = nullptr;

	CState<FiscalResult> result{ FiscalResult::UNKNOW };
	CState<FiscalStatus> state{ FiscalStatus::UNKNOW };

	std::shared_mutex set_receipt_mutex;

	#pragma region Zarz�dzanie komunikacj�ECR<->FISCAL

	std::mutex answer_mutex;
	std::condition_variable answer_condition;
	AnswerType answer = AnswerType::UNKNOW;

	#pragma endregion 

	//void RegisterEvent(const char* message, EventType type = EventType::evn_info) {
	//	std::ostream* ss;
	//	if (type == EventType::evn_error || type == EventType::evn_exception) {
	//		ss = &std::cerr;
	//	}
	//	else {
	//		ss = &std::cout;
	//	}
	//	(*ss) << "event type (" << (size_t)type << ") message: " << message << std::endl;
	//	TRIGGER_CALLBACK(message, type);
	//}
	//void RegisterEvent(std::string& message, EventType type = EventType::evn_info) {
	//	RegisterEvent(message.c_str(), type);
	//}

	int put_request(Connection* connection, std::shared_ptr<XMLRequest> request) {
		if (COMPARE_TYPE(request, XMLDailyReport)) {
			XMLDailyReport* r = CAST_TYPE(request, XMLDailyReport);
			return 0;
			//return send_RaportDobowy(r->year, r->month, r->day);
		}
		if (COMPARE_TYPE(request, XMLPeriodicReport)) {
			XMLPeriodicReport* r = CAST_TYPE(request, XMLPeriodicReport);
			return 0;
			//return send_RaportOkresowy(
			//	r->from.year, r->from.month, r->from.day,
			//	r->to.year, r->to.month, r->to.day
			//);
		}
		if (COMPARE_TYPE(request, XMLGetFeature)) {
			return 0; //getProperties(nullptr, 0); //odbi�r callback-iem
		}
		if (COMPARE_TYPE(request, XMLInfoCash)) {
			return 0; // getMappingVat(nullptr, 0); //odbi�r callback-iem
		}
		if (COMPARE_TYPE(request, XMLCashDocument)) {
			XMLCashDocument* r = CAST_TYPE(request, XMLCashDocument);
			//return send_WystawKPKW(r->type, r->paymentId, r->correct, r->value, CString(r->description.c_str()));
		}
		if (COMPARE_TYPE(request, XMLFiscalTicket)) {
			XMLFiscalTicket* r = CAST_TYPE(request, XMLFiscalTicket);
			return (int)PrintFiscalTicket(connection, r->getValue());
		}
		if (COMPARE_TYPE(request, XMLNonFiscalTicket)) {
			XMLNonFiscalTicket* r = CAST_TYPE(request, XMLNonFiscalTicket);
			return (int)PrintNonFiscalTicket(connection, r->getValue());
		}
		return 0;
	}


public:
	unsigned CALLBACK CallAnswer(const std::string& message, EventType ev = EventType::evn_request) {
		return CallAnswer(message.c_str(), ev);
	}

	unsigned CALLBACK CallAnswer(const char* message, EventType ev = EventType::evn_request) {
		std::ostream* ss;
		if (ev == EventType::evn_error || ev == EventType::evn_exception) {
			ss = &std::cerr;
		}
		else {
			ss = &std::cout;
		}
		(*ss) << "event type (" << (size_t)ev << ") message: " << message << std::endl;

		if (callAnswerCI != nullptr && current_thread == std::this_thread::get_id()) {
			callAnswerCI(message, ev);
			return 1;
		}
		return 0;
	}

	IProtocol() :current_thread(std::this_thread::get_id()) {

	}

	virtual ~IProtocol() {
		std::cout << "Destructor " << __FUNCTION__ << std::endl;
	}

	#pragma region Zarz�dzanie statusami i odpowiedziami

	/**
	 * @brief
	 * W pierwszej kolejno�ci zerujemy odpowied� \p answer i ustawiamy status \p requestState, nast�pnie \n
	 * czekamy na odpowied� z ECR, kt�ra przyjdzie metod� SetAnswer.\n
	 * Po otrzymaniu odpowiedzi lub timeout-u, status jest przywracany do poprzedniego stanu.\n
	 * Przywr�cenie statusu jest potrzebne jedynie po to, aby ECR nie ponawia� tej samej akcji.\n
	 * Ma�o prawdopodobne, �e po udzieleniu odpowiedzi przez ECR, a przed przywr�ceniem status
	 * nast�pi kolejne jego ponowne pobranie. Dlatego w p�tli ECR odbioru statusu i rezultatu
	 * powinno stosowa� si� minimalny sleep na poziome 100 ms.
	 * @param requestState status pytania, na kt�re ma zareagowa� ECR
	 * @param timeout czas oczekiwania w milisekundach
	 * @return true/false je�li doczekali�my si� odpowiedzi (je�li timeout==0 w�wczas zawsze true)
	*/
	bool WaitForAnswer(FiscalStatus requestState, long long timeout = 60_sec) {
		std::unique_lock<std::mutex> lock(answer_mutex);
		answer = AnswerType::UNKNOW;
		auto last_state = state.get();
		state.set(requestState);
		bool status = true;
		if (timeout == 0) {
			answer_condition.wait(lock, [this]() -> bool {return (answer != AnswerType::UNKNOW); });
		}
		else {
			std::chrono::milliseconds period(timeout);
			status = answer_condition.wait_for(lock, period, [this]() -> bool {return (answer != 0); });
		}
		state.set(last_state);
		return status;
	}

	void SetAnswer(AnswerType answer_user) {
		std::lock_guard<std::mutex> lock(answer_mutex);
		answer = answer_user;
		answer_condition.notify_all();
	}
	const auto GetAnswer() {
		std::lock_guard<std::mutex> lock(answer_mutex);
		return answer;
	}
	inline const auto CheckAnswer(AnswerType check_answer) {
		std::lock_guard<std::mutex> lock(answer_mutex);
		return (answer == check_answer);
	}

	FiscalStatus SetStatus(FiscalStatus newStatus) {
		return state.set(newStatus);
	}

	FiscalStatus GetStatus() {
		return state.get();
	}
	std::string GetMessageState() {
		return state.description();
	}

	std::string GetMessageResult() {
		return result.description();
	}

	FiscalResult SetResult(FiscalResult newResult) {
		return result.set(newResult);
	}

	template <typename T>
	T SetResponse(CState<T> sr, T newSr) {
		return sr.set(newSr);
	}

	FiscalResult GetResult() {
		return result.get();
	}

	bool CheckState(FiscalStatus checkState) {
		return state.check(checkState);
	}

	bool CheckResult(FiscalResult checkResult) {
		return result.check(checkResult);
	}

	#pragma endregion 

	static auto GetProtocols() {
		return protocols;
	}

	/**
	 * \brief Meoda callback \n
	   Zostanie zarejestrowana jedynie pod warunkiem, �e zadanie fiskalne jest wykonywane asynchronicznie.
	 * \remarks Podtowane jest to problemami z dost�pem do pami�ci, dla �rodowiska FoxPro, przy wywo�ywaniu procedury. Problemem jest inny w�tek uruchomieniowy.
	 * \author Piotr Kuli�ski (&copy;) 2024
	 * \date 2024-07-02
	 * \param fun
	 */
	unsigned RegisterAnswerCallback(FoxEventHandler fun) {
		if (current_thread == std::this_thread::get_id()) {
			callAnswerCI = fun;
			return 1;
		}
		return 0;
	}

	/**
	 * @brief Deserializacja paragonu XML do obiektu \ref Paragon
	 * @param xml
	 * @param check_code
	 * @return \ref Paragon
	*/
	Paragon DeserializeTicket(XmlApiFiscalRequest xml, long& check_code) {
		LOGGER_START();
		//CallAnswer("DeserializeTicket");
		state.set(FiscalStatus::PREPARE);

		Paragon paragon(xml);

		check_code = paragon.CheckCode();
		if (check_code == -1L || check_code == -2L) {
			LOGGER_SS("B��d parsowania XML" << check_code);
			return nullptr;
		}
		return paragon;
	}

	/**
	 * @brief Wydruk niefiskalny na podstawie przekazanego XML-a
	 * @param connection Po��czenie z urzadzeniem
	 * @param xml predefiniowany ml
	 * @return \ref FiscalError
	*/
	virtual FiscalError PrintNonFiscalTicket(Connection* connection, XmlApiNonFiscalRequest xml) = 0;

	/**
	 * @brief Wydruk fiskalny na podstawie przekazanego XML-a
	 * @details W pierwszej kolejno�ci XML jest deserializowany i ew. uzupe�niany o brakuj�ce a wymagane wyliczenia.
	 * @author Piotr Kuli�ski (&copy;) 2024
	 * @param connection Po��czenie z urzadzeniem
	 * @param xml predefiniowany ml
	 * @return \ref FiscalError
	*/
	virtual FiscalError PrintFiscalTicket(Connection* connection, XmlApiFiscalRequest xml) = 0;

	virtual std::string GetErrorMessage(long error_code) {
		std::stringstream ss; ss << "(" << error_code << ") Unknow error";
		return ss.str();
	}

	virtual std::string GetErrorMessage(FiscalError error_code) {
		std::stringstream ss; ss << "(" << static_cast<long>(error_code) << ") ";
		switch (error_code) {
			case FiscalError::PREPARE: {ss << "B��d parsowania odczytanych danych"; break; }
			case FiscalError::PREPARE_VAT: {ss << "B��d parsowania odczytanych stawek VAT"; break; }
			case FiscalError::COMMUNICATION: {ss << "Og�lny b��d komunikacji z urz�dzeniem"; break; }
			case FiscalError::COMMUNICATION_WRITE: {ss << "B��d zapisu do urz�dzenia"; break; }
			case FiscalError::COMMUNICATION_READ: {ss << "B��d odczytu z urz�dzenia"; break; }
			case FiscalError::CONFIGURATION: {ss << "Og�lny b��d konfiguracji"; break; }
			case FiscalError::CONFIGURATION_INVALID: {ss << "B��d ustawienie konfiguracji"; break; }
			case FiscalError::PROTOCOL: {ss << "B��d zwi�zany z protoko�em"; break; }
			case FiscalError::TIMEOUT: {ss << "Przekroczenie czasu obs�ugi"; break; }
			case FiscalError::TIMEOUT_FISCAL: {ss << "Problem z wydrukiem paragonu\nprzekroczenie czasu obs�ugi\nanulowano"; break; }
			case FiscalError::USER_REJECT: {ss << "U�ytkownik odpowiedzia� przecz�co na pytanie FISCAL"; break; }
			default: {ss << "Niezdefiniowany b��d"; break; }
		}
		return ss.str();
	}

	/**
	 * \brief Wys�anie request definiuj�cego dowolny typ wydruku.
	 * \author Piotr Kuli�ski (&copy;) 2024
	 * \date 2024-06-29
	 * \param connection Po��czenie
	 * \param xml_request definicja wydruku
	 * \return FiscalError
	 */
	FiscalError SendRequest(Connection* connection, XmlApiAllRequest xml_request) {
		auto oxml = std::make_unique<FiscalXmlApi>(xml_request);
		pugi::xml_document& doc = oxml.get()->getXml();
		pugi::xml_node requests = doc.child("Requests");
		if (requests.empty()) {
			return FiscalError::PREPARE;
		}
		auto req = requests.children("Request");
		std::for_each(req.begin(), req.end(),
			[&](auto request) { \
			std::string ctype = request.attribute("type").as_string(); \
			std::shared_ptr<XMLRequest> oxml; \
			if (ctype.compare("daily_report") == 0) {
				oxml = std::make_shared<XMLDailyReport>(request);
			}
			else if (ctype.compare("daily_report_by_day") == 0) {
				oxml = std::make_shared<XMLDailyReport>(request);
			}
			else if (ctype.compare("periodic_report") == 0) {
				oxml = std::make_shared<XMLPeriodicReport>(request);
			}
			else if (ctype.compare("get_feature") == 0) {
				oxml = std::make_shared<XMLGetFeature>(request);
			}
			else if (ctype.compare("print_kpkw") == 0) {
				oxml = std::make_shared<XMLCashDocument>(request);
			}
			else if (ctype.compare("get_infocash") == 0) {
				oxml = std::make_shared<XMLInfoCash>(request);
			}
			else if (ctype.compare("fiscal_ticket") == 0) {
				oxml = std::make_shared<XMLFiscalTicket>(request);
			}
			else if (ctype.compare("non_fiscal_ticket") == 0) {
				oxml = std::make_shared<XMLNonFiscalTicket>(request);
			}
			else {
				return FiscalError::CONFIGURATION;
			} \
				put_request(connection, oxml);
			}
		);
		return FiscalError::STATUS_OK;
	}
};
