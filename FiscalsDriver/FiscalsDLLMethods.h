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
 \section global_sec FiscalsDriver - Sterownik wydruków fiskalnych.
 \copyright Piotr Kuliñski (&copy;) 2024
 \author Piotr Kuliñski

 <b>Biblioteka FiscalsDriver zawiera funkcje umo¿liwiaj¹ce komunikacjê z drukarkami fiskalnymi</b>\n
 \n
 \ref STANDARD_INTERFACE \n

 \subsection feature_sec Sekcja dla implementuj¹cych bibliotekê w aplikacjach zewnêtrznych
 \li \subpage fd_changelog
 \li \subpage impl_note

 <h1>Cechy biblioteki</h1>
 \li Jest <b>zwyk³¹</b> bibliotek¹ DLL, mo¿liw¹ do pod³¹czenia w \em .net(C#), \em C++, \em VFP, \em Delphi \n
 \li Biblioteka jest jednow¹tkowa
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
		std::cout << "Zwolnienie zarejestrowanego po³¹czenia: " << name << std::endl;
		protocol.reset();
		connection.reset();
	}
};
using RegisterConnectionItem = std::unordered_map<std::string, std::unique_ptr<ConnectionFiscal>>;
RegisterConnectionItem connections;

BEGIN_CPP

/** Zmienna przechowuj¹ca nazwê procedury callback dla foxpro */
std::shared_ptr<char[]> eventAnswer_CI = nullptr;

/** W¹tek wydruku */
std::thread th_printing;

namespace fddll {
/**
 * \brief Opis interface dostêpu
 * \defgroup STANDARD_INTERFACE Standardowe API dostêpu do biblioteki
 * @{
 */

	int GetConnection();
 /**
  * \brief Oczekiwanie na zakoñczenie zadañ asynchronicznych.
  * \author Piotr Kuliñski (&copy;) 2024-07
  */
	void WaitForFinishJob();

	/**
	 * \brief Inicjalizacja biblioteki. \n
	 * Ustawia logowania
	 * \note Metoda wewnêtrzna biblioteki, nieeksportowana.
	 * \throw brak
	*/
	void Initialize();

	/**
	 * \brief Zwolnienie sterownika.
	 * \note Metoda wewnêtrzna biblioteki, nieeksportowana.
	 * \throw brak
	 */
	void FiscalDestroy();

	/**
	 * \brief Otwarcie po³¹czenia z ustawieniami okreœlonymi metod¹ \ref setConnection.
	 * \return \n
	 * \ref ConnectionState::OPEN "OPEN" - brak b³êdu otwarcia
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
	 * \remark Do kontrolowania przebiegu wydruku asynchronicznego u¿ywamy metod \ref GetState, \ref GetResult
	 * dodatkowo mo¿na u¿ywaæ \ref GetMessageResult, \ref GetMessageStatus \n
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
	 * \brief Wys³anie request XML, zlecaj¹cego zadanie.
	 * \n Metoda nieudokumentowana i nie u¿ywana
	 *
	 * \param XML
	 * \return
	 */
	FISCAL_API FiscalError SendRequest(XmlApiAllRequest xml_request);

	/**
	 * \brief Asynchronizacja dowolnego wydruku fiskalnego i niefiskalnego \ref SendRequest
	 * @param xml Definicja wydruku (lista wydruków)
	 * @return \ref FiscalStatus
	*/
	FISCAL_API FiscalError AsyncSendRequest(XmlApiAllRequest xml_request);

	/**
	 * \brief Przekazanie odpowiedzi co protoko³u (ECR->FISCAL)\n
	 * Protokó³ ¿¹da odpowiedzi z ECR poprzez ustawienie status ANSWER_xxxx
	 * \param answer numeryczny typ odpowiedzi, zale¿y od status ANSWER_xxxx
	 * \return FiscalError
	*/
	FISCAL_API FiscalError SetAnswer(AnswerType answer);

	/**
	 * \brief Pobranie rezultatu.
	 * \note Zaleca siê aby nie czêœciej ni¿ 500 ms
	 */
	FISCAL_API const FiscalResult GetResult();
	FISCAL_API const FiscalResult SetResult(FiscalResult newResult);

	/**
	 * \brief Pobranie statusu.
	 * \note Zaleca siê aby nie czêœciej ni¿ 100 ms
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
	 * \brief Metoda wykrywaj¹ca urz¹dzenia fiskalne.
	 * \note \ref XmlApiDetectFiscalResponse "Przyk³adowe rezultaty dzia³ania"
	 * \param[out] buffer XML ze struktur¹ informuj¹c¹ o stanie wykrycia urz¹dzenia.
	 * \param[in] buffer_size rozmiar bufora
	 * \return faktyczny rozmiar bufora (bufor jest uzupe³niany jeœli ma wymagany rozmiar)
	 */
	FISCAL_API size_t CALLBACK DetectFiscal(XmlApiDetectFiscalResponse buffer, size_t buffer_size = 8192);

	/**
	 \brief Zamkniêcie po³¹czenia z urz¹dzeniem.
	 \return \n
	 \ref FiscalError::STATUS_OK "STATUS_OK" \n
	 \ref FiscalError::CONFIGURATION_INVALID "CONFIGURATION_INVALID" np. w przypadku niezainicjowanego po³¹czenia, protoko³u
	 */
	FISCAL_API ConnectionState CALLBACK Close();

	/**
	\brief Ustawienie po³¹czenia przed otwarciem \n
	\version 1.1
	\param konfiguracja po³¹czenia
	\return
	\ref FiscalError::STATUS_OK "STATUS_OK" - ustawiono po³¹czenie \n
	\ref FiscalError::CONFIGURATION_INVALID "CONFIGURATION_INVALID" - niepoprawna konfiguracja po³¹czenia
	*/
	FISCAL_API FiscalError CALLBACK SetConnection(XmlApiConfigurationRequest xml_configuration);
	/**
	 * \brief Opcja przysz³oœciowa.\n
	 * Jej zadaniem jest rejestracja dowolnej liczby po³¹czeñ i przepinanie siê pomiêdzy istniej¹cymi.\n
	 * Pozwala³o by to wykonywaæ jednoczeœnie wiele równoleg³ych operacji na wielu urz¹dzeniach.\n
	 * Blokad¹ by³oby zarejestrowane po³¹czenie (mo¿e wykonywaæ jedn¹ operacjê na urz¹dzeniu).
	 * \remarks Kluczem po³¹czenia mo¿e byæ element FiscalID (<b>numer fiskalny</b>) dodany do atrybutów po³¹czenia.
	 * \author Piotr Kuliñski (&copy;) 2024-07-04
	 */
	FISCAL_API FiscalError CALLBACK RegisterConnection(XmlApiConfigurationRequest xml_configuration);

	/**
	\brief  Pobranie w³aœciwoœci sterownika protoko³u
	\version 1.1
	\note Bufor wynikowy podlega formatowaniu ustawionym przez \ref setFormater
	\n domyœlnym konwerterem jest \ref ConverterType::XML "XML", nie jest obs³ugiwany \ref ConverterType::COMATEXT "COMATEXT"
	\see Properties
	\param buffer bufor wyjœciowy
	\param buffer_size rozmiar bufora
	\return rzeczywisty rozmiar danych
	\n \ref CFiscal::FISCAL_NOT_INIT "FISCAL_NOT_INIT"
	*/
	FISCAL_API int CALLBACK GetProperties(char* buffer, size_t buffer_size = 8192);

	/**
	 * \brief Zarejestrowanie metody callback dla Fox.
	 * \author Piotr Kuliñski (&copy;) 2024-07
	 */
	FISCAL_API FiscalError CALLBACK RegisterEventAnswerFox(VFPFunctionSI);

	/**
	 * \brief Rejestracja funkcji callback dla œrodowisk innych ni¿ FoxPro.
	 * \author Piotr Kuliñski (&copy;) 2024-07-03
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
			<li>Podejœcie typu Protokó³ + connection (a nie konkretny sterownik)</li>
		</ol>
	</li>
</ol>
*/

/**
\page impl_note Interface standardowy biblioteki
\brief Interface standardowy biblioteki \n
 dla aplikacji np. typu \b .net, \b C++, \b VFP, \b delphi \n
Przyk³adowe flow \n
\li Ustawienie po³¹czenia \ref SetConnection
\li Otwarcie po³¹czenia z urz¹dzeniem \ref OpenDefault
\li Operacje na urz¹dzeniu \ref AsyncPrintFiscal, \ref AsyncPrintNonFiscal, \ref AsyncSendRequest, \ref PrintFiscal, \ref PrintNonFiscal, \ref SendRequest
\li Zamkniêcie portu \ref Close
*/

/**
\page impl_async Obs³uga wywo³ania metod asynchronicznych
\brief W przypadku u¿ywania metod asynchronicznych powinno siê kontrolowaæ proces wykonania,
chocia¿by z uwagi na obs³ugê sytuacji krytycznych typu: <i>b³¹d mechanizmu</i>, <i>brak papieru</i>.
Protokó³ w takich przypadkach wymaga interakcji z u¿ytkownikiem. Do kontroli procesu nale¿y u¿yæ pêtli. \n
Przyk³ad \n\n
\code{cpp}
if (FiscalError run_state = AsyncXXX(); run_state==FiscalError::STATUS_OK) {
	FiscalStatus state = FiscalStatus::UNKNOW;
	do {
		sleep(200);
		state = GetState();
		switch (state) {
			case FiscalStatus::ANSWER_WAIT_PAPER: {
				MessageBox(0, L"Brak papieru, wymieñ i kliknij OK", L"Uwaga!", 0);
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
\n Wydruk mo¿e zostaæ wykonany za pomoc¹ metody \ref ParagonXML pod warunkiem,
\n ¿e \ref getProperties ma ustawion¹ w³aœciwoœæ \b receipt_xml=1
\n

Przyk³ad konfiguracji \n
\include "Full.xml"

Schemat do weryfikacji paragonu \n
\include "full.xsd"
*/

/**
\page NONFISCAL_XML Definicja wydruku niefiskalnego w formacie XML
Wydruk mo¿e zostaæ wykonany za pomoc¹ metody \ref WydrukXMLThermal pod warunkiem,
¿e \ref getProperties ma ustawion¹ w³aœciwoœæ \b non_fiscal_xml=1
\n
\n \subpage NONFISCAL_TYPES
\n
\n Przyk³ad konfiguracji \n
\include "NonFiscalTerminal.xml"
*/
