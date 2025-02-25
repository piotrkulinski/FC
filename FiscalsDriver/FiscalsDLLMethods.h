#include "StdAfx.h"
#include "LogHelper.h"
#include "FiscalTypes.h"
#include "FiscalError.h"
#include "FiscalStatus.h"
#include "FiscalResult.h"
#include <shared_mutex>
#include "FiscalFactory.h"
#include "xmlapi/XMLRequest.h"
#include "xmlapi/XMLDailyReport.h"
#include "xmlapi/XMLPeriodicReport.h"
#include "ConnectionTCP.h"
#include "Connection.h"
#include "ConnectionFile.h"
#include "IProtocol.h"
#include "ProtocolPosnet.h"
#include "ProtocolThermal.h"
#include "ProtocolElzabDLL.h"

#include "pro_ext.h"
#include "CallbackType.h"


/**
 \defgroup Enumerations Typy wyliczeniowe
 \defgroup FiscalTypesDefinition Typy danych wykorzystywane w komunikacji
 \mainpage Biblioteka FiscalsDriver
 \section global_sec FiscalsDriver - Sterownik wydruk�w fiskalnych.
 \copyright Piotr Kuli�ski (&copy;) 2024
 \author Piotr Kuli�ski

 <b>Biblioteka FiscalsDriver zawiera funkcje umo�liwiaj�ce komunikacj� z drukarkami fiskalnymi</b>\n
 \n
 \ref STANDARD_INTERFACE \n

 \subsection feature_sec Sekcja dla implementuj�cych bibliotek� w aplikacjach zewn�trznych
 \li \subpage fd_changelog
 \li \subpage impl_note

 <h1>Cechy biblioteki</h1>
 \li Jest <b>zwyk��</b> bibliotek� DLL, mo�liw� do pod��czenia w \em .net(C#), \em C++, \em VFP, \em Delphi \n
 \li Biblioteka jest jednow�tkowa
*/

#define SET_PROTOCOL(protocol,classproto)  \
	if (typeid(protocol) != typeid(classproto)) { \
		protocol = std::make_shared<classproto>(); \
	}

#if defined(FISCAL_IMPORT)
/** Deklaracja importu/eksportu meto */
#define FISCAL_API __declspec(dllimport)
#else
/** Deklaracja importu/eksportu meto */
#define FISCAL_API __declspec(dllexport)
#endif

#ifdef __cplusplus
/** Blok procedur extern */
#define BEGIN_CPP extern "C" {
#define END_CPP }
#else
#define BEGIN_CPP
#define END_CPP
#endif

#if defined(LOGGER)

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include "AnswerType.h"
#include "threadsafe_queue.h"

std::shared_ptr<LogHelper> fdlogger;
std::streambuf* coutbuf = std::cout.rdbuf();
std::streambuf* cerrbuf = std::cerr.rdbuf();
std::streambuf* clogbuf = std::clog.rdbuf();
const char* logPath = "logs";
#endif

using CurrentProtocol = std::shared_ptr<IProtocol>;
using ConnectionDevice = std::shared_ptr<Connection>;

using ConnectionLock = std::unique_lock<std::shared_mutex>;

class ConnectionFiscal {
public:
	std::string name;
	CurrentProtocol protocol;
	ConnectionDevice connection;
	std::shared_mutex connetion_mutex;
	ConnectionFiscal(std::string _name, CurrentProtocol _protocol, ConnectionDevice _connection) :
		name(_name),
		protocol(_protocol),
		connection(_connection) {
	}
	~ConnectionFiscal() {
		std::cout << "Zwolnienie zarejestrowanego po��czenia: " << name << std::endl;
		protocol.reset();
		connection.reset();
	}
};
using RegisterConnectionItem = std::unordered_map<std::string, std::unique_ptr<ConnectionFiscal>>;
RegisterConnectionItem connections;

BEGIN_CPP

/** Zmienna przechowuj�ca nazw� procedury callback dla foxpro */
std::shared_ptr<char[]> eventAnswer_CI = nullptr;

/** W�tek wydruku */
std::thread th_printing;

namespace fddll {
/**
 * \brief Opis interface dost�pu
 * \defgroup STANDARD_INTERFACE Standardowe API dost�pu do biblioteki
 * @{
 */

	int GetConnection();
 /**
  * \brief Oczekiwanie na zako�czenie zada� asynchronicznych.
  * \author Piotr Kuli�ski (&copy;) 2024-07
  */
	void WaitForFinishJob();

	/**
	 * \brief Inicjalizacja biblioteki. \n
	 * Ustawia logowania
	 * \note Metoda wewn�trzna biblioteki, nieeksportowana.
	 * \throw brak
	*/
	void Initialize();

	/**
	 * \brief Zwolnienie sterownika.
	 * \note Metoda wewn�trzna biblioteki, nieeksportowana.
	 * \throw brak
	 */
	void FiscalDestroy();

	/**
	 * \brief Otwarcie po��czenia z ustawieniami okre�lonymi metod� \ref setConnection.
	 * \return \n
	 * \ref ConnectionState::OPEN "OPEN" - brak b��du otwarcia
	 */
	FISCAL_API ConnectionState CALLBACK OpenDefault();

	/**
	 * \brief Wydruk fiskalny ze zdefiniowanego XML-a
	 * \param xml
	 * \return \ref FiscalError
	*/
	FISCAL_API FiscalError CALLBACK PrintFiscal(XmlApiFiscalRequest xml_request);

	/**
	 * \brief Asynchronizacja wydruku fiskalnego \ref PrintFiscal
	 * \param xml Definicja wydruku
	 * \return \ref FiscalError
	 * \remark Do kontrolowania przebiegu wydruku asynchronicznego u�ywamy metod \ref GetState, \ref GetResult
	 * dodatkowo mo�na u�ywa� \ref GetMessageResult, \ref GetMessageStatus \n
	 * \note \ref impl_async
	*/
	FISCAL_API FiscalError CALLBACK AsyncPrintFiscal(XmlApiFiscalRequest xml_request);

	/**
	 * @brief Wydruk niefiskalny
	 * @param xml Definicja wydruku
	 * @return \rf FiscalError
	*/
	FISCAL_API FiscalError CALLBACK PrintNonFiscal(XmlApiNonFiscalRequest xml_request);

	/**
	 * @brief Asynchronizacja wydruku fiskalnego \ref PrintNonFiscal
	 * @param xml Definicja wydruku
	 * @return \rf FiscalError
	*/
	FISCAL_API FiscalError CALLBACK AsyncPrintNonFiscal(XmlApiNonFiscalRequest xml_request);

	/**
	 * \brief Wys�anie request XML, zlecaj�cego zadanie.
	 * \n Metoda nieudokumentowana i nie u�ywana
	 *
	 * \param XML
	 * \return
	 */
	FISCAL_API FiscalError SendRequest(XmlApiAllRequest xml_request);

	/**
	 * \brief Asynchronizacja dowolnego wydruku fiskalnego i niefiskalnego \ref SendRequest
	 * @param xml Definicja wydruku (lista wydruk�w)
	 * @return \ref FiscalStatus
	*/
	FISCAL_API FiscalError AsyncSendRequest(XmlApiAllRequest xml_request);

	/**
	 * \brief Przekazanie odpowiedzi co protoko�u (ECR->FISCAL)\n
	 * Protok� ��da odpowiedzi z ECR poprzez ustawienie status ANSWER_xxxx
	 * \param answer numeryczny typ odpowiedzi, zale�y od status ANSWER_xxxx
	 * \return FiscalError
	*/
	FISCAL_API FiscalError SetAnswer(AnswerType answer);

	/**
	 * \brief Pobranie rezultatu.
	 * \note Zaleca si� aby nie cz�ciej ni� 500 ms
	 */
	FISCAL_API const FiscalResult GetResult();
	FISCAL_API const FiscalResult SetResult(FiscalResult newResult);

	/**
	 * \brief Pobranie statusu.
	 * \note Zaleca si� aby nie cz�ciej ni� 100 ms
	 */
	FISCAL_API const FiscalStatus GetState();
	FISCAL_API const FiscalStatus SetState(FiscalStatus newStatus);

	/**
	 * \brief Pobranie statusu w formie opisowej.
	 */
	FISCAL_API int GetMessageState(char* buffer, size_t buffer_size = 512);

	/**
	 * \brief Pobranie rezultatu w formie opisowej.
	 */
	FISCAL_API int GetMessageResult(char* buffer, size_t buffer_size = 512);

	/**
	 * \brief Metoda wykrywaj�ca urz�dzenia fiskalne.
	 * \note \ref XmlApiDetectFiscalResponse "Przyk�adowe rezultaty dzia�ania"
	 * \param[out] buffer XML ze struktur� informuj�c� o stanie wykrycia urz�dzenia.
	 * \param[in] buffer_size rozmiar bufora
	 * \return faktyczny rozmiar bufora (bufor jest uzupe�niany je�li ma wymagany rozmiar)
	 */
	FISCAL_API size_t CALLBACK DetectFiscal(XmlApiDetectFiscalResponse buffer, size_t buffer_size = 8192);

	/**
	 \brief Zamkni�cie po��czenia z urz�dzeniem.
	 \return \n
	 \ref FiscalError::STATUS_OK "STATUS_OK" \n
	 \ref FiscalError::CONFIGURATION_INVALID "CONFIGURATION_INVALID" np. w przypadku niezainicjowanego po��czenia, protoko�u
	 */
	FISCAL_API ConnectionState CALLBACK Close();

	/**
	\brief Ustawienie po��czenia przed otwarciem \n
	\version 1.1
	\param konfiguracja po��czenia
	\return
	\ref FiscalError::STATUS_OK "STATUS_OK" - ustawiono po��czenie \n
	\ref FiscalError::CONFIGURATION_INVALID "CONFIGURATION_INVALID" - niepoprawna konfiguracja po��czenia
	*/
	FISCAL_API FiscalError CALLBACK SetConnection(XmlApiConfigurationRequest xml_configuration);
	/**
	 * \brief Opcja przysz�o�ciowa.\n
	 * Jej zadaniem jest rejestracja dowolnej liczby po��cze� i przepinanie si� pomi�dzy istniej�cymi.\n
	 * Pozwala�o by to wykonywa� jednocze�nie wiele r�wnoleg�ych operacji na wielu urz�dzeniach.\n
	 * Blokad� by�oby zarejestrowane po��czenie (mo�e wykonywa� jedn� operacj� na urz�dzeniu).
	 * \remarks Kluczem po��czenia mo�e by� element FiscalID (<b>numer fiskalny</b>) dodany do atrybut�w po��czenia.
	 * \author Piotr Kuli�ski (&copy;) 2024-07-04
	 */
	FISCAL_API FiscalError CALLBACK RegisterConnection(XmlApiConfigurationRequest xml_configuration);

	/**
	\brief  Pobranie w�a�ciwo�ci sterownika protoko�u
	\version 1.1
	\note Bufor wynikowy podlega formatowaniu ustawionym przez \ref setFormater
	\n domy�lnym konwerterem jest \ref ConverterType::XML "XML", nie jest obs�ugiwany \ref ConverterType::COMATEXT "COMATEXT"
	\see Properties
	\param buffer bufor wyj�ciowy
	\param buffer_size rozmiar bufora
	\return rzeczywisty rozmiar danych
	\n \ref CFiscal::FISCAL_NOT_INIT "FISCAL_NOT_INIT"
	*/
	FISCAL_API int CALLBACK GetProperties(char* buffer, size_t buffer_size = 8192);

	/**
	 * \brief Zarejestrowanie metody callback dla Fox.
	 * \author Piotr Kuli�ski (&copy;) 2024-07
	 */
	FISCAL_API FiscalError CALLBACK RegisterEventAnswerFox(VFPFunctionSI);

	/**
	 * \brief Rejestracja funkcji callback dla �rodowisk innych ni� FoxPro.
	 * \author Piotr Kuli�ski (&copy;) 2024-07-03
	 */
	FISCAL_API FiscalError CALLBACK RegisterEventAnswer(FoxEventHandler);

/**
 * @}
 */
}

END_CPP



/**
\page fd_changelog Ostatnie zmiany w wersjach
<ol type="A">
	<li><b>1.0.0.0</b>
		<ol type="1">
			<li>Podej�cie typu Protok� + connection (a nie konkretny sterownik)</li>
		</ol>
	</li>
</ol>
*/

/**
\page impl_note Interface standardowy biblioteki
\brief Interface standardowy biblioteki \n
 dla aplikacji np. typu \b .net, \b C++, \b VFP, \b delphi \n
Przyk�adowe flow \n
\li Ustawienie po��czenia \ref SetConnection
\li Otwarcie po��czenia z urz�dzeniem \ref OpenDefault
\li Operacje na urz�dzeniu \ref AsyncPrintFiscal, \ref AsyncPrintNonFiscal, \ref AsyncSendRequest, \ref PrintFiscal, \ref PrintNonFiscal, \ref SendRequest
\li Zamkni�cie portu \ref Close
*/

/**
\page impl_async Obs�uga wywo�ania metod asynchronicznych
\brief W przypadku u�ywania metod asynchronicznych powinno si� kontrolowa� proces wykonania,
chocia�by z uwagi na obs�ug� sytuacji krytycznych typu: <i>b��d mechanizmu</i>, <i>brak papieru</i>.
Protok� w takich przypadkach wymaga interakcji z u�ytkownikiem. Do kontroli procesu nale�y u�y� p�tli. \n
Przyk�ad \n\n
\code{cpp}
if (FiscalError run_state = AsyncXXX(); run_state==FiscalError::STATUS_OK) {
	FiscalStatus state = FiscalStatus::UNKNOW;
	do {
		sleep(200);
		state = GetState();
		switch (state) {
			case FiscalStatus::ANSWER_WAIT_PAPER: {
				MessageBox(0, L"Brak papieru, wymie� i kliknij OK", L"Uwaga!", 0);
				SetAnswer(AnswerType::YES);
				break;
			}
		}
	} while (state!=FiscalStatus::FINISH);
}
FiscalResult result = GetResult();
\endcode
*/

/**
\page PARAGON_XML Definicja wydruku paragonu w formacie XML
\n Wydruk mo�e zosta� wykonany za pomoc� metody \ref ParagonXML pod warunkiem,
\n �e \ref getProperties ma ustawion� w�a�ciwo�� \b receipt_xml=1
\n

Przyk�ad konfiguracji \n
\include "Full.xml"

Schemat do weryfikacji paragonu \n
\include "full.xsd"
*/

/**
\page NONFISCAL_XML Definicja wydruku niefiskalnego w formacie XML
Wydruk mo�e zosta� wykonany za pomoc� metody \ref WydrukXMLThermal pod warunkiem,
�e \ref getProperties ma ustawion� w�a�ciwo�� \b non_fiscal_xml=1
\n
\n \subpage NONFISCAL_TYPES
\n
\n Przyk�ad konfiguracji \n
\include "NonFiscalTerminal.xml"
*/
