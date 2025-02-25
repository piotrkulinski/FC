#pragma once
#include "IProtocol.h"


#pragma region DEKLARACJA TYPÓW WSKAZNIKÓW NA FUNKCJE BIBLIOTECZNE

#define DECL_FUNCTION(_FUN_) typedef long(__stdcall* p_##_FUN_)
#define INIT_FUNCTION(_FUN_) p_##_FUN_ _FUN_
#define LOAD_FUNCTION(_FUN_) _FUN_ = (p_##_FUN_)GetProcAddress(dll,#_FUN_)

//#define DEFINE_FUNCTION(_FUN_) typedef long(__stdcall* p_##_FUN_) ;p_##_FUN_ _FUN_
//#define GET_FUNCTION(_FUN_) _FUN_ = (p_##_FUN_)GetProcAddress(dll,#_FUN_)

typedef unsigned char tBufferOfBytes[256];

DECL_FUNCTION(CommunicationInit)(long, long, long);
DECL_FUNCTION(CommunicationEnd)(void);
DECL_FUNCTION(ReceiptConditions)(void);
DECL_FUNCTION(ReceiptBegin)(void);
DECL_FUNCTION(pReceiptItem)(ReceiptLineType Sprzed, const char* Nazwa, long Stawka, long Komunikat, long Ilosc, long MP, const char* Jedn, long Cena, long* Wartosc);
DECL_FUNCTION(pReceiptItemEx)(ReceiptLineType Sprzed, const char* Nazwa, long Stawka, long Komunikat, long Ilosc, long MP, const char* Jedn, long Cena);
DECL_FUNCTION(ReceiptEnd)(long Disc);
DECL_FUNCTION(PrintSubtotal)(void);
DECL_FUNCTION(OpenDrawer)(long Number);
DECL_FUNCTION(ReceiptCancel)(void);
DECL_FUNCTION(PrintControl)(long BeforePrinting);
DECL_FUNCTION(PrintResume)(void);
DECL_FUNCTION(SetDebugMode)(void);
DECL_FUNCTION(ClearDebugMode)(void);
DECL_FUNCTION(DailyReport)(long Unconditionally);
DECL_FUNCTION(PeriodReport)(long Fiscal, long yy1, long mm1, long dd1, long yy2, long mm2, long dd2);
DECL_FUNCTION(LockedArticlesReport)(void);
DECL_FUNCTION(MonthlyReport)(long Fiscal, long Year, long Month);
DECL_FUNCTION(NumberReport)(long Fiscal, long FirstNumber, long LastNumber);
DECL_FUNCTION(ReceiptNumber)(long* Number);
DECL_FUNCTION(DailyReportNumber)(long* Number);
DECL_FUNCTION(WriteLineFeed)(long Number);
DECL_FUNCTION(PrinterStatusReport)(void);
DECL_FUNCTION(SetVAT)(long* Ile, long A, long B, long C, long D, long E, long F, long G);
DECL_FUNCTION(ReadVAT)(long* Ile, long* A, long* B, long* C, long* D, long* E, long* F, long* G);
DECL_FUNCTION(ErasePayments)(void);
DECL_FUNCTION(EraseLines)(void);
DECL_FUNCTION(pDeviceName)(char* Name);
DECL_FUNCTION(pReceiptPurchaserNIP)(const char* NIP);
DECL_FUNCTION(pFillPayment)(long Number, const char* Name, long Total, long Rest);
DECL_FUNCTION(pFillLines)(long Number, const char* Line, long* FreeLines);
DECL_FUNCTION(pDllVersion)(char* Ver);
DECL_FUNCTION(pDllAuthor)(char* Auth);
DECL_FUNCTION(pErrMessage)(long Number, char* Message);
DECL_FUNCTION(pCharsInArticleName)(char* Name);
DECL_FUNCTION(pReadClock)(char* Time);
DECL_FUNCTION(pReadUniqueNumber)(char* Number);
DECL_FUNCTION(pReadTotal)(char* Total);
DECL_FUNCTION(pReadSelTotal)(char* Rate, char* Total);
DECL_FUNCTION(pDisplayFP600)(long Number, char* Caption);
DECL_FUNCTION(ChangeTime)(long Hour, long Minute);
DECL_FUNCTION(PackageItem)(long Param, int Number, int Quantity, long Price);
DECL_FUNCTION(pControlPrintout)(long  Oper, char* Name, long TaxRate);
DECL_FUNCTION(OpenPort)(long PortNo, long Speed, long WriteTimeout, long ReadTimeout);
DECL_FUNCTION(ClosePort)(void);
DECL_FUNCTION(RSSequence)(long ControlCode, long QuantityOfBytesToReceive, long QuantityOfBytesToSend, tBufferOfBytes* InputBuffer, tBufferOfBytes OutputBuffer);
DECL_FUNCTION(pInvoiceBegin)(char* Numer, char* Nazwa1, char* Nazwa2, char* Nazwa3, char* Nazwa4, char* Nazwa5, char* Nazwa6, char* NIP);
DECL_FUNCTION(IsCodePage1250)(long* is);
#pragma endregion
//template <typename... Args>
//long __stdcall* loadFun(Args&& a) {
//
//};

class ProtocolElzabDLL : public IProtocol
{
private:
	HMODULE dll{ nullptr };

	#pragma region DEKLARACJA WSKAZNIKÓW FUNKCJI
	INIT_FUNCTION(CommunicationInit);
	INIT_FUNCTION(CommunicationEnd);
	INIT_FUNCTION(ReceiptConditions);
	INIT_FUNCTION(ReceiptBegin);
	/** Wydruk lini paragonu: <1: Typ linijki>, <2: Nazwa produktu>, <3: ID Vat>, <4: Numer komunikatu do wydrukowania> */
	INIT_FUNCTION(pReceiptItem);
	INIT_FUNCTION(pReceiptItemEx);
	INIT_FUNCTION(ReceiptEnd);
	INIT_FUNCTION(PrintSubtotal);
	INIT_FUNCTION(OpenDrawer);
	INIT_FUNCTION(ReceiptCancel);
	INIT_FUNCTION(PrintControl);
	INIT_FUNCTION(PrintResume);
	INIT_FUNCTION(DailyReport);
	//INIT_FUNCTION(PeriodReport);
	INIT_FUNCTION(LockedArticlesReport);
	//INIT_FUNCTION(MonthlyReport);
	INIT_FUNCTION(NumberReport);
	INIT_FUNCTION(ReceiptNumber);
	//INIT_FUNCTION(DailyReportNumber);
	//INIT_FUNCTION(WriteLineFeed);
	//INIT_FUNCTION(PrinterStatusReport);
	INIT_FUNCTION(SetVAT);
	INIT_FUNCTION(ReadVAT);
	//INIT_FUNCTION(ErasePayments);
	//INIT_FUNCTION(EraseLines);
	INIT_FUNCTION(pDeviceName);
	INIT_FUNCTION(pFillPayment);
	INIT_FUNCTION(pFillLines);
	//INIT_FUNCTION(pDllVersion);
	//INIT_FUNCTION(pDllAuthor);
	//INIT_FUNCTION(pErrMessage);
	//INIT_FUNCTION(pCharsInArticleName);
	//INIT_FUNCTION(pReadClock);
	INIT_FUNCTION(pReadUniqueNumber);
	//INIT_FUNCTION(pReadTotal);
	//INIT_FUNCTION(pReadSelTotal);
	//INIT_FUNCTION(pDisplayFP600);
	//INIT_FUNCTION(ChangeTime);
	//INIT_FUNCTION(PackageItem);
	//INIT_FUNCTION(pControlPrintout);
	INIT_FUNCTION(OpenPort);
	INIT_FUNCTION(ClosePort);
	//INIT_FUNCTION(RSSequence);
	INIT_FUNCTION(pInvoiceBegin);
	INIT_FUNCTION(SetDebugMode);
	INIT_FUNCTION(ClearDebugMode);
	INIT_FUNCTION(pReceiptPurchaserNIP);
	INIT_FUNCTION(IsCodePage1250);
	#pragma endregion 

public:
	FiscalError PrintFiscalTicket(Connection* connection, const char* xml) override {
		lock_receipt_protocol lck3(set_receipt_mutex);
		CStateManage<FiscalStatus, FiscalResult> ms{ state,result }; //RAII

		if (!connection->isConnectionType(ConnectionType::RS232)) {
			result.set(FiscalResult::FAILED, "Protokó³ obs³uguje jedynie po³¹czenie RS232");
			return FiscalError::CONFIGURATION_INVALID;
		}

		FiscalError response_procedure = FiscalError::ANY_ERROR;
		long xresult = 0;
		Paragon paragon = DeserializeTicket(xml, xresult);
		if (xresult != 0L) {
			return FiscalError::PREPARE;
		}

		int LastFiscalNumber = 0;
		long last_error = 0;
		long elzabOpen = -1;
		long A, B, C, D, E, F, G, ilV;

		#pragma region STA£E
		auto SeekElzabVAT = [&](const std::string& value) noexcept(false) {
			long nvat = (
				(value.at(0) == 'Z' || value.find("99") != std::string::npos)
				? 10000l
				: (long)(fiscal::round(value) * 100)
				);
			switch (nvat) {
				case 2300: return 1l; //A
				case  800: return 2l; //B
				case  500: return 3l; //C
				case 2500: return 4l; //D
				case 2540: return 5l; //E
				case 2550: return 6l; //F
				case 1000: return 7l; //G
				default: return 1l;
			}
			};

		#pragma endregion

		LOAD_FUNCTION(ReceiptCancel);

		const bool isOpened = connection->IsOpened();
		try {
			if (isOpened) {
				connection->ClearPort();
				connection->Close();
			}
			LOAD_FUNCTION(pReadUniqueNumber);
			LOAD_FUNCTION(CommunicationInit);
			LOAD_FUNCTION(CommunicationEnd);
			LOAD_FUNCTION(PrintSubtotal);
			LOAD_FUNCTION(pFillLines);
			LOAD_FUNCTION(pFillPayment);
			LOAD_FUNCTION(pReceiptPurchaserNIP);
			LOAD_FUNCTION(pInvoiceBegin);
			LOAD_FUNCTION(pReceiptItem);
			LOAD_FUNCTION(OpenDrawer);
			LOAD_FUNCTION(ReceiptBegin);
			LOAD_FUNCTION(ReceiptEnd);
			LOAD_FUNCTION(ReceiptNumber);
			LOAD_FUNCTION(pDeviceName);
			LOAD_FUNCTION(ReadVAT);
			LOAD_FUNCTION(SetDebugMode);
			LOAD_FUNCTION(ClearDebugMode);
			LOAD_FUNCTION(IsCodePage1250);

			//SetDebugMode();
			//tylko RS
			const ConfigurationRS232 cfg = static_cast<ConfigurationRS232>(((ConnectionRS232*)connection)->getSettings());
			long port = atol(cfg.port.substr(3, 3).c_str());
			elzabOpen = CommunicationInit(port, cfg.baudrate, 5l);
			long active_codepage = 0;
			IsCodePage1250(&active_codepage);
			if (active_codepage == 0) {
				paragon.setEncoding<codepage_cp852>();
			}
			else if (active_codepage == 1) {
				paragon.setEncoding<codepage_base>();
			}
			else {
				paragon.setEncoding<codepage_mazovia>();
			}
			std::unique_ptr<payment_base> paymentConversion = std::make_unique<payment_elzab>();

			//char uniq[250];
			//pReadUniqueNumber(uniq);

			#pragma region INFORMACJE KASOWE
			ReadVAT(&ilV, &A, &B, &C, &D, &E, &F, &G);
			#pragma endregion

			long res, wynik;
			//wynik = pFillLines(2, (char*)"gotówka", &res);
			//wynik = pFillPayment(1, (char*)"gotówka", 29372, 0); //29209
			wynik = ReceiptBegin();

			std::string nrkasy = "";
			std::string zgloszony = "";
			std::string nrsys = "";
			std::string zapraszamy = "";
			std::string dsp_info = "";

			#pragma region OPERACJE DODATKOWE
			/*
			 * Dodatkowe operacje wykonywane na urz¹dzeniu fiskalnym
			 * Otwarcie szuflady, kontrola przed wydrukiem itp.
			 * np. sprawdzenie iloœci raportów dobowych
			*/
			for (const auto& [element, type, value] : paragon.control.action) {
				if (element == FiscalControlType::OpenDrawer && (type == FiscalControlActionType::Before || type == FiscalControlActionType::BeforeAndAfter)) {
					if (std::any_cast<bool>(value)) {
						OpenDrawer(1);
					}
				}
			}
			#pragma endregion

			for (Description& desc : paragon.descriptions) {
				std::string encode_name{ paragon.encoding(desc.nazwa) };
				switch (desc.typ) {
					case OPIS_NIP_NABYWCY:
					{
						pReceiptPurchaserNIP(encode_name.substr(0, 16).c_str());
						break;
					}
					case OPIS_KELNER:
					{
						zgloszony = encode_name;
						break;
					}
					case OPIS_KASA:
					{
						nrkasy = encode_name;
						break;
					}
					case OPIS_TRANSAKCJA0:
					case OPIS_TRANSAKCJA1: {
						nrsys = encode_name;
						break;
					}
					case OPIS_KOD_KRESKOWY:
					{
						//request += "bc" + WartoscOpisu + CP_TAB;
						break;
					}
					case OPIS_ZAPRASZAMY_PONOWNIE:
					{
						zapraszamy = encode_name;
						break;
					}
					case OPIS_NAZWA_PROGRAMU:
					{
						dsp_info = encode_name;
						break;
					}
					default:
					{
						continue;
					}
				}
				desc.isPrintHeader = true;
			}
			long summary_receipt = 0;
			DoubleToLong ilosc_bez_decimal;
			for (ReceiptLine& line : paragon.receiptLines) {
				if (line.typ == ReceiptLineType::Sale) {
					long wartosc = 0;
					wynik = pReceiptItem(
						  line.typ
						, paragon.encoding(line.nazwa).c_str()
						, line.VatID<long>(SeekElzabVAT)
						, 0
						, ilosc_bez_decimal(line.ilosc, line.decimal_point)
						, line.decimal_point
						, line.jm.c_str()
						, line.cena_sprzedazy<Grosze>()
						, &wartosc
					);
					summary_receipt += wynik;
				}
			}

			for (Prepayment zaliczka : paragon.prePayments) {
			}

			//pFillLines(paragon.payments.size(), "", &res);
			long nr = 0;
			for (Payment platnosc : paragon.payments) {
				wynik = pFillPayment(
					++nr /*platnosc.PeymentID(cv_peyment_id)*/,
					paragon.encoding(platnosc.nazwa.empty() ? std::string{ "Gotówka" } : platnosc.nazwa).substr(0, 16).c_str(),
					platnosc.wartosc(),
					0);
			}

			for (Description desc : paragon.descriptions) {
				if (!desc.isPrintHeader) {
				}
			}
			ReceiptEnd(0);

			//ReceiptEnd(paragon.summary.rabat());
			long lastReceipt = 0;
			ReceiptNumber(&lastReceipt);
			LastFiscalNumber = static_cast<int>(lastReceipt);

			LOGGER_SS("Paragon: " << LastFiscalNumber);
			//RegisterEvent("Wydruk zakoñczony poprawnie");
			result.set(FiscalResult::OK);
			response_procedure = FiscalError::STATUS_OK;
		}
		catch (std::exception& xex)
		{
			std::string message = std::string("B³¹d wydruku paragonu\n") + xex.what();
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message, EventType::evn_exception);
		}
		catch (...)
		{
			std::string message = "B³¹d wydruku paragonu\ntransakcja zostanie anulowana";
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message, EventType::evn_exception);
		}

		if (!result.check(FiscalResult::OK)) {
			try {
				ReceiptCancel();
			}
			catch (...) {
				std::string message = "Problem z anulowaniem transakcji";
				LOGGER_SS(message);
				result.set(FiscalResult::FAILED, message);
				CallAnswer(message, EventType::evn_exception);
			}
		}

		if (elzabOpen != -1) {
			elzabOpen = CommunicationEnd();
			ClearDebugMode();
		}

		return response_procedure;
	}

	FiscalError PrintNonFiscalTicket(Connection* connection, const char* xml) override {
		return FiscalError::ANY_ERROR;
	}

	~ProtocolElzabDLL() {
		std::cout << "Destructor " << __FUNCTION__ << std::endl;
		if (dll != nullptr) {
			FreeLibrary(dll);
			dll = nullptr;
		}
	}

	ProtocolElzabDLL() {
	//todo: Istniej mo¿liwoœæ wielokrotnego przyspieszenia druku i unikniêcia b³êdów, poprzez implementacjê dll
		std::string dllfile{ "elzabdr.dll" };
		if (FILE* file = fopen(dllfile.c_str(), "r")) {
			fclose(file);
			dll = LoadLibraryA(dllfile.c_str());
			return;
		}
		throw fiscal_exception(FiscalError::ANY_ERROR, "B³¹d inicjacji DLL dla ELZAB-a");
	};
};

