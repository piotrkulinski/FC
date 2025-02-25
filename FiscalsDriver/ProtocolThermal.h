#pragma once
#include <tuple>
#include "IProtocol.h"
#include "protocol_fiscal.h"
#include "NonFiscalLineType.h"
#include "logger_definition.h"
#include "codepage_conversion.h"

/**
 @author Piotr Kuli�ski (&copy;) 2024
 @brief Klasa obs�uguj�ca wydruki fiskalne i niefiskalne na protokole \b thermal, przekazane
 w postaci sformalizowanego XML-a.

*/

class ProtocolThermal : public IProtocol {
public:
	~ProtocolThermal() {
		std::cout << "Destructor " << __FUNCTION__ << std::endl;
	}

	FiscalError PrintNonFiscalTicket(Connection* connection, XmlApiNonFiscalRequest xml) override
	{
		LOGGER_START();
		lock_receipt_protocol lck3(set_receipt_mutex);

		CStateManage<FiscalStatus, FiscalResult> ms{ state,result }; //RAII

		const auto sendWithoutCheck = [&](const std::string& request) {
			size_t inBuff = request.length();
			inBuff -= connection->WriteBytes(request);
			const bool issend = (inBuff == 0);
			if (!issend) {
				if (DWORD lastError = GetLastError(); lastError == ERROR_INVALID_HANDLE) {
					throw fiscal_exception(lastError, "B��d dost�pu do portu komunikacyjnego");
				}
			}
			return issend;
			};

		const auto sendreadWithoutCheck = [&](const std::string& req, std::string& response) {
			if (sendWithoutCheck(req)) {
				std::stringstream ss;
				int nread = connection->WaitReadData(ss, '\\');
				if (nread > 0) {
					response.assign(ss.str());
				}
			}
			};

		const bool isOpened = connection->IsOpened();

		try {
			if (!isOpened) {
				if (connection->Open() != ConnectionState::OPEN) {
					throw fiscal_exception(FiscalError::COMMUNICATION, "B��d otwarcia portu COM");
				}
			}
			state.set(FiscalStatus::PREPARE);
			pugi::xml_document doc;
			pugi::xml_parse_result xresult = doc.load_string(xml);
			if (!xresult) {
				throw fiscal_exception(FiscalError::PREPARE, "B��d parsowania XML");
			}

			pugi::xml_node lines = doc.child("Lines");
			size_t width = 60; //59;
			std::string swidth = lines.attribute("MaxWidth").value();
			if (!swidth.empty()) {
				width = min(std::stoul(swidth), width);
			}
			state.set(FiscalStatus::PRINTING);
			connection->ClearPort();
			thermal::fiscal_stream req;
			req.str(""); req << "#v";

			std::string response;
			sendreadWithoutCheck(req.str(), response);
			for (auto& c : response) c = toupper(c);
			//Surowy thermal, pierwotny dla drukarek posnetowych, reszta to jakie� odmiany
			bool IsRawThermal = (response.find("THERMAL") != std::string::npos);

			req.str(""); req << "1#e";
			connection->WriteBytes(req.str());

			std::unique_ptr<codepage_base> encoder;
			pugi::xml_attribute xRaportNo = lines.attribute("Report");
			pugi::xml_attribute xRaportHeader = lines.attribute("Header");

			unsigned int RaportNo = xRaportNo.as_int(-1);;
			unsigned int RaportHeader = xRaportHeader.as_int(0);
			unsigned int ReportLine = 0;
			unsigned int DefaultType = 0;

			req.str("");

			if (IsRawThermal) {
				//width = 34;
				encoder = std::make_unique<codepage_mazovia>();
				if (RaportNo == -1) {
					RaportNo = 22;
				}
				req << "0;" << RaportNo << ";" << RaportHeader << "$w";
			}
			else {
				encoder = std::make_unique<codepage_noconvert>();
				if (RaportNo == -1) {
					RaportNo = 200;
				}
				DefaultType = 0;
				req << "0;" << RaportNo << ";" << RaportHeader << ";12$w";
			}
			connection->WriteBytes(req.str());

			/*
			ESC P 0; P2[; P3] $w <check> ESC \
				Gdzie:
			P2:  numer raportu, kt�rego nag��wek ma by� drukowany,
			P3 : numer nag��wka, kt�ry ma by� wydrukowany(niekt�re raporty maj� kilka
				zdefiniowanych nag��wk�w).Je�li wydruk ma tylko jeden rodzaj nag��wka, to
			P3  powinno mie� warto�� 0, lub nie jest wymagane.
			*/

			enum FormatType : unsigned long
			{
				EmptyFormat = 0,
				Center = 1,
				Left = 2,
				Right = 4,
				Bold = 8,
				HeightOnly = 16
			};

			struct SValue
			{
				std::string Value;
				const char* Attr;
				unsigned long FrmBits;
			};

			auto CreateLine = [this](auto& IloscZnakow, const std::vector<SValue>& values) {
				std::string linia = "";
				unsigned int RemainingLength = IloscZnakow;
				for (auto const& value : values) {
					if (!value.Value.empty()) {
						size_t vlen = value.Value.length();
						if (RemainingLength >= vlen) {
							if ((value.FrmBits & FormatType::Right) > 0) {
								linia += std::string(IloscZnakow - linia.length() - vlen, ' ') + value.Value;
							}
							else {
								linia += value.Value;
							}
							RemainingLength -= vlen;
						}
						else {
							linia += value.Value;
						}
					}
				}
				if (linia.length() > IloscZnakow) {
					return linia.substr(0, IloscZnakow);
				}
				return linia;
				};

			auto FormatValue = [this](pugi::xml_node tool) {
				auto GetBitsFormat = [](const char* frmtxt) {
					unsigned long frm = FormatType::EmptyFormat;
					std::string inp(frmtxt);
					frm += (inp.find("Bold") != std::string::npos ? FormatType::Bold : FormatType::EmptyFormat);
					frm += (inp.find("Height") != std::string::npos ? FormatType::HeightOnly : FormatType::EmptyFormat);
					frm += (inp.find("Left") != std::string::npos ? FormatType::Left : FormatType::EmptyFormat);
					frm += (inp.find("Right") != std::string::npos ? FormatType::Right : FormatType::EmptyFormat);
					frm += (inp.find("Center") != std::string::npos ? FormatType::Center : FormatType::EmptyFormat);
					return frm;
					};

				std::vector<SValue> values;
				for (unsigned int I = 1; I < 4; I++)
				{
					std::string no = std::to_string(I);
					std::string els("Value" + no);
					const char* el = tool.child_value(els.c_str());
					SValue val;
					val.Value = el;
					val.Attr = tool.attribute(std::string("Param" + no).c_str()).as_string();
					val.FrmBits = GetBitsFormat(val.Attr);
					values.push_back(val);
				}
				return values;
				};

			for (pugi::xml_node line = lines.first_child(); line; line = line.next_sibling())
			{
				//dla termala nie ma co za bardzo rozpatrywa� typu
				//np. prot. thermal na Novitus i Posnetach nie jest zgodny
				//pomimo ustawienia zgodno�ci na novitusie, 
				//formatka 22 i linia 0

				int typ = 37;
				if (auto type = line.child_value("Typ"); type != nullptr && strcmp(type, "") > 0) {
					typ = atoi(type);
				}
				typ = (IsRawThermal ? 37 : 0);
				if (typ > -1) {
					ReportLine = typ;
				}
				else {
					ReportLine = DefaultType;
				}

				bool doubleWidth = false;
				bool doubleHeight = false;
				unsigned short invers = 0;
				std::vector<SValue> values = FormatValue(line);
				if (!IsRawThermal) {
					for (const SValue& v : values)
					{
						if (v.FrmBits & FormatType::Bold)
						{
							doubleWidth = true;
						}
						if (v.FrmBits & FormatType::HeightOnly) {
							doubleHeight = true;
						}
					}
				}
				size_t IloscZnakow = (doubleWidth ? width / 2 : width);

				req.str("");

				NonFiscalLineType iTyp = static_cast<NonFiscalLineType>(atoi(line.child_value("Typ")));
				switch (iTyp)
				{
					case NonFiscalLineType::L_Empty:
					{
						req << RaportNo << ";" << (u_short)255 << "$w\r";
						break;
					}
					case NonFiscalLineType::L_Declined:
					{
						invers = 1;
					}
					default:
					{
						std::string linia = CreateLine(IloscZnakow, values);
						req
							<< RaportNo << ";"
							<< ReportLine
							<< ";"
							<< (doubleWidth ? '1' : '0') << ";"
							<< invers << ";" //inwers
							<< "0" << ";" //czcionka podstawowa
							<< (values[0].FrmBits & FormatType::Center ? '1' : '0') << ";"
							<< (doubleHeight ? "2" : "0")
							<< "$w"
							<< encoder->encode(linia)
							<< "\r";
					}
				}
				connection->WriteBytes(req.str());
			}

			req.str("");
			req << "1;" << RaportNo << ";0;0$w";
			connection->WriteBytes(req.str());
			result.set(FiscalResult::OK, "Wydruk niefiskalny zako�czony poprawnie");
		}
		catch (std::exception& ex) {
			std::stringstream ss; ss << "Problem z wydrukiem XML: " << ex.what() << std::endl;
			result.set(FiscalResult::FAILED, ss.str());
		}
		catch (...) {
			std::stringstream ss; ss << "Problem z wydrukiem XML " << std::endl;
			result.set(FiscalResult::FAILED, ss.str());
		}

		connection->ClearPort();
		if (!isOpened) {
			connection->Close();
		}
		return FiscalError::STATUS_OK;
	}

	FiscalError PrintFiscalTicket(Connection* connection, XmlApiFiscalRequest xml) override
	{
		LOGGER_START();
		lock_receipt_protocol lck3(set_receipt_mutex);

		CStateManage<FiscalStatus, FiscalResult> ms{ state,result }; //RAII

		long xresult = 0;
		Paragon paragon = DeserializeTicket(xml, xresult);
		if (xresult != 0L) {
			result.set(FiscalResult::FAILED, "Problem odczytem XML z paragonem");
			return FiscalError::PREPARE;
		}

		state.set(FiscalStatus::WORKING);
		int LastFiscalNumber = 0;

		//Connection* connection = instance->getConnection().get();

		if (paragon.fconnection != nullptr && (paragon.fconnection->type != ConnectionType::RS232 || connection->isConnectionType(ConnectionType::TCP))) {
			std::string error_message{ "Protok� obs�uguje jedynie po��czenie RS232" };
			result.set(FiscalResult::FAILED, error_message);
			return FiscalError::CONFIGURATION_INVALID;
		}

		FiscalError last_error{ FiscalError::UNKONW };
		std::vector<std::string_view> vat;

		#pragma region STA�E
		constexpr auto ERR_DLE = 1000L;
		constexpr auto ERR_THERMAL = 10000L;
		constexpr auto ERR_EXCEPTION = 20000L;

		#pragma endregion

		#pragma region LAMBDA KOMUNIKACYJE

		const auto MapVat = [&](const std::string& value) noexcept(false) {
			const double nvat = (
				(value.at(0) == 'Z' || value.find("99") != std::string::npos)
				? 100.00f
				: fiscal::round(value)
				);

			for (u_short I = 1; I < 8; I++) {
				const double v = fiscal::round(vat.at(I).data());
				if (v == nvat) {
					const char S = (const char)(64 + I);
					return S;
				}
			}
			throw fiscal_exception(FiscalError::PREPARE_VAT, "B��d mapowania stawki VAT");
			};

		const auto readdata = [&](std::string& response) {
			std::stringstream ss;
			const int nread = connection->WaitReadData(ss, thermal::END);
			if (nread > 0) {
				response.assign(ss.str());
			}
			return (nread > 0);
			};

		const auto sendWithoutCheck = [&](const std::string& request) {
			size_t inBuff = request.length();
			inBuff -= connection->WriteBytes(request);
			const bool issend = (inBuff == 0);
			if (!issend) {
				if (const DWORD lastError = GetLastError(); lastError == ERROR_INVALID_HANDLE) {
					std::cerr << (long)lastError, "B��d dost�pu do portu komunikacyjnego";
					if (callAnswerCI != nullptr) {
						callAnswerCI("B��d dost�pu do portu komunikacyjnego", EventType::evn_warrning);
					}					
					throw fiscal_exception(FiscalError::COMMUNICATION, "B��d dost�pu do portu komunikacyjnego");
				}
			}
			return issend;
			};

		/*
		 * Pobranie DLE, ale jednoczesna komunikacja z u�ytkownikiem w przypadku np. braku papieru, podniesionej pokrywy itp
		 * remainingSeconds - Informacyjny czas w sekundach ile jeszcze u�ytkownik ma czasu na usuni�cie "awarii"
		 * return DLE
		*/
		const auto check_dle = [&](unsigned long remainingSeconds) {
			if (connection->WriteSingleByte(thermal::DLE) == 1) {
				if (unsigned char byte_dle = 0; connection->XReadData(&byte_dle, 1) == 1) {
					DLE dle(byte_dle);
					if (dle.err() || dle.pe_akk() || dle.no_onl()) {
						#if defined(LOGGER)
						dle.display();
						#endif
						auto last_error = (long)((dle.err() ? thermal::DLE_ERR : (dle.pe_akk() ? thermal::DLE_PE_AKK : thermal::DLE_ONL)) + ERR_DLE);
						if (CallAnswer("Problem z urz�dzeniem, sprawd� je")) {
						} else if (!WaitForAnswer(FiscalStatus::ANSWER_WAIT_DEVICE)) {
							throw fiscal_exception(FiscalError::COMMUNICATION);
						}
					}
					return dle;
				}
			}
			return DLE(0);
			};

		auto check_enq = [&]() {
			if (connection->WriteSingleByte(thermal::ENQ) == 1) {
				if (unsigned char byte_enq = 0; connection->XReadData(&byte_enq, 1) == 1) {
					ENQ enq(byte_enq);
					if (!enq.cmd()) {
						#if defined(LOGGER)
						enq.display();
						#endif
						if (enq.trf()) {
							Sleep(100);
							connection->WriteByte(thermal::CAN);
						}
					}
					return enq;
				}
			}
			return ENQ(1);
			};

		auto check_error = [&](unsigned long remainingSeconds) {
			auto last_error = 0L;

			//ostatni b��d
			std::string commad{ "0#n" };
			thermal::no_crc_stream request; request << commad;

			if (sendWithoutCheck(request.str())) {
				std::string response;
				if (readdata(response)) {
					//informacje kasowe
					std::string cmd{ "#E" };
					const size_t _od = response.find(cmd);
					const size_t _do = response.find(thermal::ESC, _od);
					if (_od < _do) {
						long result_code = atol(response.substr(_od + cmd.length(), _do - cmd.length() - _od).c_str());
						if (result_code != 0) {
							auto const last_error = result_code + ERR_THERMAL;
							std::string message = GetErrorMessage(last_error);
							throw fiscal_exception(FiscalError::PROTOCOL, message.c_str());
						}
					}
				}
			}
			return last_error;
			};

		/*
		 * Wys�anie request do drukarki
		 * Po wysy�ce jest automatycznie sprawdzany ENQ, je�li jest b��d, wchodzi w 60sek p�tl� testuj�c�
		 * param creceipt_xml
		 * return
		*/
		const auto sendbuffer = [&](const std::string& request) {
			bool issend = sendWithoutCheck(request);
			if (ENQ enq = check_enq(); !enq.cmd()) {
				constexpr u_long wait_ms = 10_sec; //minut� czekamy na wyja�nienie sprawy, a jak nie wysy�amy instrukcj� anulacji
				const Timeout<u_long, 1000> maxTime(wait_ms);
				do {
					if (DLE dle = check_dle(maxTime.elapsed() / 1000); dle.err() || dle.pe_akk() || dle.no_onl()) {
						enq = check_enq();
						if (enq.cmd()) {
							return true;
						}
						continue;
					}
					//enq i dle zwraca b��dy, pobieramy ostatni rozkaz i przerywamy
					Sleep(500);

					if (const long dle = check_error(maxTime.elapsed() / 1000); dle != 0) {
						auto last_error = FiscalError::TIMEOUT_FISCAL;
						std::string message = GetErrorMessage(last_error);
						throw fiscal_exception(last_error, message.c_str());
					}
					issend = true;
					break;
				} while (!maxTime.timeout());
			}
			if (!issend) {
				throw fiscal_exception(FiscalError::COMMUNICATION_WRITE,"B��d komunikacji z urz�dzeniem");
			}
			return (issend);
			};

		const auto sendreadWithoutCheck = [&](const std::string& req, std::string& response) {
			if (!sendWithoutCheck(req)) {
				throw fiscal_exception(FiscalError::COMMUNICATION_WRITE, GetErrorMessage(last_error).c_str());
			}
			std::stringstream ss;
			const int nread = connection->WaitReadData(ss, thermal::END);
			if (nread > 0) {
				response.assign(ss.str());
				return;
			}
			};

		const auto GetLastReceipt = [&](const DeviceProtocol& type) {
			std::string response{ "" };

			if (type == DeviceProtocol::Novitus) {
				//tu zwracany jest numer paragonu, nie wydruku
				const std::string dle{ thermal::DLE2 };
				sendreadWithoutCheck(dle, response);
				auto dle2 = splitSV(response, thermal::FLD_SEP_2);
				if (dle2.size() == 5) {
					return atoi(dle2[2].c_str());
				}
			}

			if (type == DeviceProtocol::Thermal) {
				//tu zwracany jest numer wydruku
				thermal::fiscal_stream reqcrc;
				reqcrc << "50#s";
				sendreadWithoutCheck(reqcrc.str(), response);

				const size_t s = response.find("50#X");
				const size_t f = response.find(thermal::FLD_SEP_1, s);
				if (f > s) {
					return atoi(response.substr(s + 4, f - 4 - s).c_str());
				}
			}
			return -1;
			};

		#pragma endregion
		ErrorMode error_mode = ErrorMode::DEVICE_NO_BREAK;
		const bool isOpened = connection->IsOpened();
		try {
			state.set(FiscalStatus::PRINTING);
			if (!isOpened) {
				connection->setSettings(*(paragon.fconnection.get()));
				if (connection->Open() != ConnectionState::OPEN) {
					throw fiscal_exception(FiscalError::COMMUNICATION, "B��d otwarcia portu COM");
				}
			}

			connection->ClearPort();
			connection->WriteByte(thermal::CAN);

			std::string response{};
			thermal::fiscal_stream reqcrc;

			/* (1)
			wyst�pienie b��du nie daje komunikatu i nie zawiesza przetwarzania.
			Rodzaj b��du mo�e by� testowany przy u�yciu sekwencji ��danie odes�ania informacji kasowych
			(pole Ostatni b��d odpowiedzi), lub sekwencj� ��danie odes�ania kodu b��du ostatniego rozkazu
			Po ustawieniu sekwencji nie oczekujemy na odpowied� w przypadku wielu rozkaz�w.
			*/
			reqcrc << ErrorMode::DEVICE_NO_BREAK << "#e"; //zmiana obs�ugi b��d�w;
			sendWithoutCheck(reqcrc.str());

			reqcrc.str("0$e"); //cancel transaction
			sendWithoutCheck(reqcrc.str());

			response.clear();
			reqcrc.str("#v"); //pobranie wersji i nazwy urz�dzenia
			sendreadWithoutCheck(reqcrc.str(), response);
			if (response.empty()) {				
				throw fiscal_exception(FiscalError::COMMUNICATION_READ, "B��d komunikacji z urz�dzeniem - nieudane pobranie wersji oprogramowania");
			}

			for (auto& c : response) c = toupper(c);
			const bool isThermal = (
				response.find("THERMAL") != std::string::npos
				);
			const bool isNovitus = !isThermal && (
				response.find("NOVITUS HD") != std::string::npos ||
				response.find("HD E") != std::string::npos ||
				response.find("DELIO E") != std::string::npos ||
				response.find("BONO") != std::string::npos
				);

			//konwersje identyfikator�w p�atno�ci DB na p�atno�ci drukarki
			std::unique_ptr<payment_base> paymentConversion;
			if (isNovitus) {
				paymentConversion = std::make_unique<payment_to_novitus>();
			}
			else {
				paymentConversion = std::make_unique<payment_to_thermal>();
			}

			//Wymiana strony kodowej dla thermala
			if (isThermal || isNovitus) {
				paragon.setEncoding<codepage_mazovia>();
			}
			else {
				paragon.setEncoding<codepage_noconvert>(); //w zasadzie martwy kod
			}

			#pragma region INFORMACJE KASOWE

			std::string response_vat; //na to b�dzie string_view
			thermal::no_crc_stream nreq; nreq << "23#s";
			sendreadWithoutCheck(nreq.str(), response_vat);

			const std::string_view sv{ response_vat };
			vat = splitSV(sv, thermal::FLD_SEP_1);
			if (vat.size() != 18) {
				throw fiscal_exception(FiscalError::PREPARE_VAT, "B��d parsowania stawek VAT");
			}

			bool isThermalFiscalNip = false;
			if (isThermal) {
				const std::string prefixes{ "/BFJ/BFK/BFG/BFP/BCY/BFL/BEX/BEW/BEV" };
				if (size_t last_sep = response_vat.find_last_of(thermal::FLD_SEP_1); last_sep != std::string::npos) {
					std::string prefix = response_vat.substr(last_sep, 4);
					isThermalFiscalNip = (prefixes.find(prefix) != std::string::npos);
				}
			}

			#pragma endregion

			std::string
				nrkasy{ "" },
				zgloszony{ "" },
				nrsys{ "" },
				zapraszamy{ "" },
				dsp_info{ "" },
				nip{ "" };

			#pragma region OPERACJE DODATKOWE
			/*
			 * Dodatkowe operacje wykonywane na urz�dzeniu fiskalnym
			 * Otwarcie szuflady, kontrola przed wydrukiem itp.
			 * np. sprawdzenie ilo�ci raport�w dobowych
			*/
			for (const auto& [element, type, value] : paragon.control.action) {
				if (element == FiscalControlType::OpenDrawer && (type== FiscalControlActionType::Before || type == FiscalControlActionType::BeforeAndAfter)) {
					//val.type() == typeid(bool) && 
					if (std::any_cast<bool>(value)) {
						nreq.str("");
						nreq << "1$d";
						sendWithoutCheck(nreq.str());
					}
				}
			}
			#pragma endregion

			for (Description& desc : paragon.descriptions) {
				switch (desc.typ) {
					case OPIS_NIP_NABYWCY:
					{
						nip = paragon.encoding(desc.nazwa).substr(0, 16);
						if (isThermal && !isThermalFiscalNip) {
							desc.nazwa = "NIP: " + desc.nazwa.substr(0, 16);
							continue;
						}
						break;
					}
					case OPIS_KELNER:
					{
						zgloszony = paragon.encoding(desc.nazwa);
						break;
					}
					case OPIS_KASA:
					{
						nrkasy = paragon.encoding(desc.nazwa);
						break;
					}
					case OPIS_TRANSAKCJA0:
					case OPIS_TRANSAKCJA1:
					{
						nrsys = paragon.encoding(desc.nazwa);
						break;
					}
					case OPIS_KOD_KRESKOWY:
					{
						// request += "bc" + WartoscOpisu + thermal::TAB;
						break;
					}
					case OPIS_ZAPRASZAMY_PONOWNIE:
					{
						zapraszamy = paragon.encoding(desc.nazwa);
						break;
					}
					case OPIS_NAZWA_PROGRAMU:
					{
						dsp_info = paragon.encoding(desc.nazwa);
						break;
					}
					default:
					{
						continue;
					}
				}
				desc.isPrintHeader = true;
			}

			/*
			* Prze��czenie wy�wietlacza LCD operatora z powrotem w tryb kasowy
			* (dopuszczalne po up�ywie 45 sekund od zako�czenia paragonu!),
			*/
			nreq.str("");
			nreq << "4$d";
			sendWithoutCheck(nreq.str());

			/*
			* wys�anie napisu do wy�wietlacza, napis <string> nie mo�e zawiera� sekwencji ESC....
			*/
			nreq.str("");
			nreq << "2$d" << dsp_info;
			sendWithoutCheck(nreq.str());

			#pragma region ROZPOCZECIE_TRANSAKCJI

			unsigned short liniDodatkowych = 0;
			liniDodatkowych += !dsp_info.empty() ? 1 : 0;
			liniDodatkowych += !zapraszamy.empty() ? 1 : 0;

			reqcrc.str("");
			reqcrc << "0" << thermal::FLD_SEP_2 << liniDodatkowych << "$h";

			if (liniDodatkowych > 0) {
				if (!dsp_info.empty()) {
					reqcrc << paragon.encoding(dsp_info) << thermal::CR;
				}
				if (!zapraszamy.empty()) {
					reqcrc << paragon.encoding(zapraszamy) << thermal::CR;
				}
			}
			sendWithoutCheck(reqcrc.str());

			#pragma endregion 

			if (!zapraszamy.empty()) {
				reqcrc.str("");
				reqcrc << "24" << thermal::FLD_SEP_2 << "1$z" << paragon.encoding(zapraszamy) << thermal::CR;
				sendWithoutCheck(reqcrc.str());
			}

			/**
			* @brief <b>Uwagi co do drukowania NIP w cz�ci fiskalnej.</b>\n
			* NIP dla urz�dze� POSNET w protokole thermal nie jest obs�ugiwany. \n
			* W przypadku urz�dze� NOVITUS jest drukowany w cz�ci fiskalnej ("NOVITUS zgod." - rozszerzony thermal dla novitusa) \n
			* Nazwa drukarki odczytana z urz�dzenia np. NOVITUS HD(e) jako propagowana jako "HD E" i niewiele m�wi o producencie. \n
			* Dlatego kluczowym znacznikiem jest fraza POSNET w nazwie urz�dzenia. \n
			* Je�li jej nie ma zak�adamy, �e jest to urz�dzenie Novitus-a.
			*/
			if (!nip.empty()) {
				bool isPrint = false;
				if (isNovitus) {
					reqcrc.str("");
					reqcrc << "100;1$z" << nip << thermal::CR;
					try {
						sendWithoutCheck(reqcrc.str());
						isPrint = true;
					}
					catch (...) {
					}
				}

				if (isThermal && isThermalFiscalNip) {
					reqcrc.str("");
					reqcrc << "1$N" << nip << thermal::CR;
					try {
						sendWithoutCheck(reqcrc.str());
						isPrint = true;
					}
					catch (...) {
					}
				}
				std::for_each(paragon.descriptions.begin(), paragon.descriptions.end(), [&](Description& desc) {
					if (desc.typ == DescriptionType::OPIS_NIP_NABYWCY)
						desc.isPrintHeader = isPrint;
					});
			}

			unsigned int numberLine = 0;
			//for (ReceiptLine& line : paragon.receiptLines) { //sortuje elementy, wg x klucza
			for (size_t I = 0; I < paragon.receiptLines.size(); I++) {
				ReceiptLine& line = paragon.receiptLines[I];
				if (line.typ != ReceiptLineType::Sale) {
					continue;
				}

				reqcrc.str("");
				reqcrc
					<< ++numberLine;
				if (line.czyRabatNarzut()) {
					reqcrc
						<< thermal::FLD_SEP_2 << line.typRabatu
						<< thermal::FLD_SEP_2 << line.opisRabatu;
				}
				reqcrc
					<< "$l"
					<< paragon.encoding(line.nazwa) << thermal::CR
					<< line.Ilosc<DecimalStringTrim>() << thermal::CR
					<< line.VatID<char>(MapVat) << thermal::FLD_SEP_1
					<< line.cena_sprzedazy<DecimalString>() << thermal::FLD_SEP_1
					<< line.wartosc<DecimalString>();

				if (line.czyRabatNarzut()) {
					reqcrc
						<< thermal::FLD_SEP_1
						<< fiscal::to_string(abs(line.wartosc_rabatu()));
					if (line.opisRabatu == DiscountDescription::DefiniowanyPrzezUzytkownika) {
						reqcrc
							<< thermal::FLD_SEP_1
							<< line.napisRabatu.substr(0, (isThermal ? 20 : 40));
					}
				}
				reqcrc << thermal::FLD_SEP_1;
				sendbuffer(reqcrc.str());
			}

			ReceiptTotal& podsumowanie = paragon.summary;
			const bool pkb = podsumowanie.KaucjePobrane() > 0;
			const bool pkz = podsumowanie.KaucjeZwrocone() > 0;
			if (pkb || pkz) {
				//Wysy�ka opakowa�(tak naprawd� to deklaracja dla drukarki, rozliczona w podsumowaniu)
				std::for_each(
					paragon.receiptLines.begin(),
					paragon.receiptLines.end(),
					[&](ReceiptLine& line) {
						if (line.typ >= ReceiptLineType::KaucjaPobrana && line.typ <= ReceiptLineType::KaucjaZwroconaStorno) {
							reqcrc.str("");
							reqcrc
								<< (
									line.typ == ReceiptLineType::KaucjaPobrana ? 6 :
									line.typ == ReceiptLineType::KaucjaPobranaStorno ? 7 :
									line.typ == ReceiptLineType::KaucjaZwrocona ? 10 :
									line.typ == ReceiptLineType::KaucjaZwroconaStorno ? 11 : 0
									)
								<< "$d"
								<< line.cena_sprzedazy<DecimalString>() << thermal::FLD_SEP_1
								<< thermal::CR //numer kaucji, na obecn� chwil� przyjmujemy 1
								<< line.Ilosc<DecimalString>() << thermal::CR;
							sendbuffer(reqcrc.str());
						}
					}
				);
			}

			for (Prepayment zaliczka : paragon.prePayments) {
				reqcrc.str("");
				reqcrc
					<< (zaliczka.VatID<char>(MapVat) - 65) << thermal::FLD_SEP_2
					<< zaliczka.typRabatu << thermal::FLD_SEP_2
					<< zaliczka.opisRabatu
					<< "$L"
					<< podsumowanie.sprzedaz<GroszeToDecimalString>(zaliczka.vat) << thermal::FLD_SEP_1
					<< zaliczka.wartosc<GroszeToDecimalString>();

				if (zaliczka.opisRabatu == DiscountDescription::DefiniowanyPrzezUzytkownika) {
					reqcrc
						<< thermal::FLD_SEP_1 << paragon.encoding(zaliczka.napisRabatu).substr(0, 20) << thermal::CR;
				}
				sendbuffer(reqcrc.str());
			}

			for (Payment platnosc : paragon.payments) {
				reqcrc.str("");
				reqcrc
					<< "1" << thermal::FLD_SEP_2 //zg�oszenie formy p�atno�ci
					<< paymentConversion->convert(platnosc.getPeymentID());

				if (!isThermal && platnosc.isDcc()) {
					reqcrc
						<< thermal::FLD_SEP_2
						<< thermal::FLD_SEP_2 << "0" // p�atno��
						<< thermal::FLD_SEP_2 << "4"; // max precyzja
				}
				reqcrc
					<< "$b"
					<< platnosc.wartosc<GroszeToDecimalString>() << thermal::FLD_SEP_1
					<< paragon.encoding(platnosc.nazwa).substr(0, 16) << thermal::CR;

				if (platnosc.isDcc()) {
					reqcrc
						<< platnosc.dcc.kurs() << thermal::CR
						<< platnosc.dcc.wartosc<GroszeToDecimalString>() << thermal::FLD_SEP_1;
				}
				sendbuffer(reqcrc.str());
			}

			#pragma region ZAKONCZENIE PARAGONU

			reqcrc.str("");
			reqcrc
				/* pole: 2 */ << 0 << thermal::FLD_SEP_2 // Pn - ilo�� dodatkowych linii
				/* pole: 3 */ << 3 << thermal::FLD_SEP_2 // Pc - Zako�czenie, 3-skr�cona forma zako�czenie z $y+$z
				/* pole: 4 */ << 0 << thermal::FLD_SEP_2 // Py - drukownaie skr�conego podsumownaia w grupie
				/* pole: 5 */ << (podsumowanie.dsp() > 0.00f ? 0 : 1) << thermal::FLD_SEP_2 // Pdsp - 0 - kwota DSP dodatnia, 1 - ujemna
				/* pole: 6 */ << 0 << thermal::FLD_SEP_2 // Px - 0 - opis rabatu nie wyst�puje
				/* pole: 7 */ << (pkb ? 1 : 0) << thermal::FLD_SEP_2 // Pkb
				/* pole: 8 */ << (pkz ? 1 : 0) << thermal::FLD_SEP_2 // Pkz
				/* pole: 9 */ << (!nrsys.empty() ? 1 : 0) << thermal::FLD_SEP_2 // Pns && czy numer systemowy
				/* pole:10 */ << 0 << thermal::FLD_SEP_2 // Pfn - p�atno�ci rozliczane w innej transakcji (0-nie ma)
				/* pole:11 */ << 0 << thermal::FLD_SEP_2 // Pr - p�atno�ci rozliczane w innej transakcji, reszta wyliczana automatycznie
				/* pole:12 */ << (isThermal ? 1 : 0) << thermal::FLD_SEP_2 // Pg - 0 -  kwota WPLATY jest ignorowana (wplata got�wki nie wyst�puje)
				/* pole:13 */ << 0  // Psx - p�atno�ci rozliczane w innej transakcji
				/* pole:14 */ << "$y" // zamkni�cie z formami p�atno�ci
				/* pole:15 */ << nrkasy << thermal::CR //numer kasy
				/* pole:16 */ << zgloszony << thermal::CR // Operator
				/* pole:17 */ << nrsys << thermal::CR; // numer systemowy

			if (pkb) {
				reqcrc << 1 << thermal::CR << 1 << thermal::CR;// Pkb>0 - kaucje pobrane
			}
			if (pkz) {
				reqcrc << 1 << thermal::CR << 1 << thermal::CR;// Pkz>0 - kaucej zwracana
			}

			reqcrc
				/* pole:24 */ << podsumowanie.total<GroszeToDecimalString>() << thermal::FLD_SEP_1
				/* pole:25 */ << podsumowanie.dsp<GroszeToDecimalString>() << thermal::FLD_SEP_1
				/* pole:26 */ << 0 << thermal::FLD_SEP_1 // rabat
				/* pole:27 */ << podsumowanie.wplata<GroszeToDecimalString>() << thermal::FLD_SEP_1
				/* pole:28 */ << podsumowanie.reszta<GroszeToDecimalString>() << thermal::FLD_SEP_1;

			if (pkb) {
				//reqcrc << fiscal::to_string(fiscal::round(podsumowanie.KaucjePobrane<GroszeToDecimal>())) << FLD_SEP_1;
				reqcrc << podsumowanie.KaucjePobrane<GroszeToDecimalString>() << thermal::FLD_SEP_1;
			}
			if (pkz) {
				//reqcrc << fiscal::to_string(fiscal::round(podsumowanie.KaucjeZwrocone<GroszeToDecimal>())) << FLD_SEP_1;
				reqcrc << podsumowanie.KaucjeZwrocone<GroszeToDecimalString>() << thermal::FLD_SEP_1;
			}
			sendbuffer(reqcrc.str());

			#pragma endregion

			#pragma region WYDRUK OPAKOWA�
			/**
			 * Wydruk opakowa� - jedynie numery opakowa�
			 * Linia ta pozwala wydrukowa� tyle opakowa� ile wys�ano z dok�adn� ilo�ci� i cen�
			*/
			long nr_opak = 0;
			std::for_each(
				paragon.receiptLines.begin(),
				paragon.receiptLines.end(),
				[&](ReceiptLine& line) {
					if (line.typ >= ReceiptLineType::KaucjaPobrana && line.typ <= ReceiptLineType::KaucjaZwroconaStorno) {
						reqcrc.str("");
						reqcrc
							<< (
								line.typ == ReceiptLineType::KaucjaPobrana ||
								line.typ == ReceiptLineType::KaucjaPobranaStorno ? 4 : 8
								)
							<< "$z"
							<< nr_opak++ << thermal::CR
							<< line.Ilosc<DecimalString>() << thermal::CR
							<< line.cena_sprzedazy<DecimalString>() << thermal::FLD_SEP_1;
						sendbuffer(reqcrc.str());
					}
				}
			);
			#pragma endregion 

			for (Description desc : paragon.descriptions) {
				if (!desc.isPrintHeader) {
					reqcrc.str("");
					reqcrc
						<< "20" << thermal::FLD_SEP_2
						<< std::to_string(desc.typ > 1000 ? desc.typ % 1000 : desc.typ)
						<< "$z"
						<< paragon.encoding(desc.nazwa).substr(0, 25) << thermal::CR;
					sendbuffer(reqcrc.str());
				}
			}

			if (isNovitus) {
				if (nrsys.empty()) {
					reqcrc.str("");
					reqcrc << "20;0$z" << nrsys << thermal::CR;
					sendWithoutCheck(reqcrc.str());
				}
				if (!zapraszamy.empty()) {
					reqcrc.str("");
					reqcrc << "24;1$z" << zapraszamy << thermal::CR;
					sendWithoutCheck(reqcrc.str());
				}
			}

			if (isThermal) {
				//cztery puste linie po wydruku
				reqcrc.str("");
				reqcrc << "20;25$z" << thermal::CR;
				for (unsigned short I = 0; I < 4; I++) {
					sendWithoutCheck(reqcrc.str());
				}
			}

			reqcrc.str("");
			reqcrc << "28;1$z";
			sendbuffer(reqcrc.str());

			LastFiscalNumber = GetLastReceipt(isNovitus ? DeviceProtocol::Novitus : DeviceProtocol::Thermal);

			std::stringstream ss; ss << "Wydruk zako�czony poprawnie, paragon: " << LastFiscalNumber;
			result.set(FiscalResult::OK, ss.str());
		}

		catch (fiscal_exception<FiscalError>& ex) {
			std::string message = ex.what();
			if (message.empty()) {
				message = GetErrorMessage(ex.getErrorNo());
			}
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message,EventType::evn_exception);
		}
		catch (fiscal_exception<long>& ex) {
			std::string message = GetErrorMessage(ex.getErrorNo());
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message, EventType::evn_exception);
		}
		catch (const std::exception& xex)
		{
			std::string message{ std::string("B��d wydruku paragonu\n") + xex.what() };
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message, EventType::evn_exception);
		}
		catch (...)
		{
			std::string message{ "B��d wydruku paragonu\ntransakcja zostanie anulowana" };
			LOGGER_SS(message);
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message, EventType::evn_exception);
		}

		//przywr�cenie obs�ugi b��d�w
		if (error_mode != ErrorMode::DEVICE_NO_BREAK && connection->IsOpened()) {
			thermal::fiscal_stream req_error_mode;
			req_error_mode << error_mode << "#e";
			sendWithoutCheck(req_error_mode.str());
		}

		if (result.check(FiscalResult::FAILED) && connection->IsOpened()) {
			try {
				//RegisterEvent("Transakcja anluowana");
				thermal::fiscal_stream reqcrc; reqcrc << "0$e";
				sendWithoutCheck(reqcrc.str());
			}
			catch (...) {
				std::string message{ "Problem z anulowaniem transakcji" };
				LOGGER_SS(message);
				result.set(FiscalResult::FAILED, message);
				CallAnswer(message, EventType::evn_exception);
			}
		}

		connection->ClearError();
		if (!isOpened) {
			connection->Close();
		}

		return FiscalError::STATUS_OK;
	}

	virtual std::string GetErrorMessage(FiscalError error_code) override {
		return IProtocol::GetErrorMessage(error_code);
	}

	virtual std::string GetErrorMessage(long error_code) override {
		static const std::map<long, std::string> errors{
					//B��d DLE
					{1001, "Drukarka w trybie off-line"},
					{1002, "Brak Papieru lub roz�adowana bateria akumulator�w"},
					{1004, "B��d mechanizmu/sterownika"},
					//thermal error
					{10001,"Nie zainicjalizowany zegar RTC"},
					{10002,"B��d bajtu kontrolnego"},
					{10003,"Z�a ilo�� parametr�w"},
					{10004,"B��d danych"},
					{10005,"B��d wykonania (zapisu) do zegara RTC lub b��d odczytu zegara RTC"},
					{10006,"B��d odczytu totalizer�w, b��d operacji z pami�ci� fiskaln�"},
					{10007,"Data wcze�niejsza od daty ostatniego zapisu w pami�ci fiskalnej (wykonanie raportu dobowego lub programowanie stawek PTU niemo�liwe !)"},
					{10008,"B��d operacji - niezerowe totalizery !"},
					{10009,"B��d operacji I/O (np. nie usuni�ta zwora serwisowa) -B��d operacji I/O (nie przes�ana baza towarowa z aplikacji)"},
					{10011,"B��d programowania stawek PTU (z�a liczba stawek, lub niepoprawne warto�ci stawek). "},
					{10012,"B��dny nag��wek, zbyt d�ugi lub pusty (zawiera np. same spacje). -Brak nag��wka przy programowaniu stawek PTU"},
					{10013,"Pr�ba fiskalizacji  urz�dzenia w trybie fiskalnym"},
					{10016,"B��dna nazwa (pusta lub za d�uga)"},
					{10017,"B��dne oznaczenie ilo�ci (puste lub za d�ugie)"},
					{10018,"B��dne oznaczenie stawki PTU (lub brak),  pr�ba sprzeda�y w stawce nieaktywnej lub pr�ba sprzeda�y towaru zablokowanego"},
					{10019,"B��d warto�ci CENA (syntaktyka, zakres, brak lub zako�czenie transakcji z rabatem/ narzutem przekraczaj�cym sprzeda� minimaln�/ maksymaln�)"},
					{10020,"B��d warto�ci BRUTTO lub RABAT (syntaktyka, zakres lub brak), -B��d niespe�nienia warunku  ilo�� x cena  = brutto, -Przy rabacie kwotowym uwzgl�dnienie rabatu nie mo�e prowadzi� do ujemnego wyniku, niespe�nienie powy�szego daje b��d #20"},
					{10021,"Sekwencja odebrana przez drukark� przy wy��czonym trybie transakcji, przy obrocie opakowaniami lub transakcji aptecznej."},
					{10022,"B��d operacji STORNO (w wyniku wykonania tej operacji suma w danej grupie podatkowej wychodzi ujemna) lub b��d operacji z rabatem np. warto�� towaru po uwzgl�dnieniu rabatu wychodzi ujemna. Wyst�puje r�wnie� przy storno opakowania"},
					{10023,"zako�czenie transakcji bez sprzeda�y"},
					{10025,"B��dny kod terminala/ kasjera (z�a d�ugo�� lub format) lub b��dna tre�� dodatkowych linii"},
					{10026,"B��d kwoty 'WP�ATA' (syntaktyka; je�eli r�nica WP�ATA-TOTAL <0 to napisy 'got�wka','reszta' nie b�d� drukowane !), 'PRZYJ�CIE' przes�ana w LBTRXEND nie jest zgodna z sum� warto�ci otrzymanych w sekwencjach LBDSPDEP, -B��d pola kwota_PLN w obs�udze form p�atno�ci"},
					{10027,"B��dna suma ca�kowita TOTAL lub b��dna kwota RABAT"},
					{10028,"Przepe�nienie totalizera (max. 99 999 999,99  dla jednej grupy podatkowej)"},
					{10029,"��danie zako�czenia (pozytywnego !) trybu transakcji, w momencie kiedy nie zosta� on jeszcze w��czony"},
					{10030,"B��d kwoty WP�ATA (syntaktyka)"},
					{10031,"Nadmiar dodawania (przekroczenie zakresu got�wki w kasie)"},
					{10032,"Warto�� po odj�ciu staje si� ujemna (przyjmuje si� w�wczas stan zerowy kasy !)"},
					{10033,"B��d napisu <zmiana> lub <kasjer> lub <numer> lub <kaucja> (np. za d�ugi lub zawieraj�cy b��dne znaki)"},
					{10034,"B��d jednej z kwot lub pozosta�ych napis�w"},
					{10035,"Zerowy stan totalizer�w"},
					{10036,"Ju� istnieje zapis o tej dacie"},
					{10037,"Operacja przerwana z klawiatury (przed rozpocz�ciem drukowania)"},
					{10038,"B��d nazwy"},
					{10039,"B��d oznaczenia PTU"},
					{10040,"Brak nag��wka w pami�ci RAM -Ten b��d pojawia si� tak�e w przypadku wyst�pienia b��du blokuj�cego tryb fiskalny "},
					{10041,"B��d napisu <numer_kasy> (za d�ugi lub zawieraj�cy b��dne znaki) -B��d napisu <recepta> (A)"},
					{10042,"B��d napisu <numer_kasjera>, -B��d kwoty OPLATA (A)"},
					{10043,"B��d napisu <numer_par>, -B��d kwoty OPLATA (A)"},
					{10044,"B��d napisu <kontrahent>, -Brak w paragonie leku dla kt�rego by�a drukowana wcze�niej wycena (A), -Pr�ba wydruku zwyk�ego paragonu przy niepustej tablicy wycen (A)"},
					{10045,"B��d napisu <terminal>, -Przepe�nienie tablicy wycen (zbyt du�a liczba wycen wydrukowana) (A)"},
					{10046,"B��d napisu <nazwa_karty> -B��d pola <nazwa> (A)"},
					{10047,"B��d napisu <numer_karty> -B��d pola <postac_dawka> (A)"},
					{10048,"B��d napisu <data_m> -B��d pola <opakowanie> (A)"},
					{10049,"B��d napisu <data_r> -B��d pola <ptu> (A)"},
					{10050,"B��d napisu <kod_autoryz> -B��d pola <ilo��> (A)"},
					{10051,"B��d warto�ci <kwota> -B��d pola <kasa_nr> (A)"},
					{10052,"B��d pola <platne> (A)"},
					{10053,"B��d pola <recepta> (A)"},
					{10054,"B��d pola <refundacja> (A)"},
					{10055,"B��d pola <wydal> (A)"},
					{10056,"B��d pola <data> (A)"},
					{10057,"B��d pola <ilosc_lekospis> (A)"},
					{10058,"B��d pola <pacjent> (A)"},
					{10059,"B��d pola <lekarz> (A)"},
					{10060,"B��d pola <refundator> (A)"},
					{10061,"B��d pola <wojew�dztwo> (A)"},
					{10062,"B��d jednej z warto�ci numeryczn. (A)"},
					{10063,"Pr�ba usuni�cia nieistniej�cej wyceny (A)"},
					{10064,"B��d pola <kasa> (A)"},
					{10065,"B��d pola <osoba> (A)"},
					{10066,"B��d pola <par1> (A)"},
					{10067,"B��d pola <par2> (A)"},
					{10068,"B��d pola <znak_z> (A)"},
					{10069,"B��d jednej z warto�ci numerycznych (A)"},
					{10070,"B��d pola <numer> (A)"},
					{10071,"B��d pola <osoba> (A)"},
					{10072,"B��d pola <pacjent> (A)"},
					{10073,"B��d pola <lekarz> (A)"},
					{10077,"B��d pola <recepta> (A)"},
					{10078,"B��d pola <nazwa> (A)"},
					{10079,"B��d pola <postac_dawka> (A)"},
					{10080,"B��d pola <opakowanie> (A)"},
					{10081,"B��d pola <ilosc> (A)"},
					{10082,"Przekroczona liczba programowania kod�w autoryzacyjnych przez RS -Niedozwolony rozkaz w bie��cym stanie urz�dzenia -B��d pola <cena> (A)"},
					{10083,"Z�a warto�� kaucji przes�anej w $z -B��d pola <wartosc> (A)"},
					{10084,"Przekroczona liczba wys�anych napis�w na wy�wietlacz -B��d pola <limit> (A)"},
					{10085,"Nie zaprogramowano stawek VAT -B��d pola <razem_wartosc> (A)"},
					{10086,"B��d pola <refundacja> (A)"},
					{10087,"B��d pola <doplata> (A)"},
					{10088,"B��d pola <oplata> (A)"},
					{10089,"B��d pola <marza> (A)"},
					{10090,"Operacja tylko z kaucjami - nie mo�na wys�a� towar�w -B��d pola <tax> (A)"},
					{10091,"By�a wys�ana forma p�atno�ci - nie mo�na wys�a� towar�w -B��d w zako�czeniu transakcji zwi�zany z formami p�atno�ci -B��d pola <kasa_nr> (A)"},
					{10092,"Przepe�nienie bazy towarowej -B��d pola <wydal> (A)"},
					{10093,"B��d anulowania formy p�atno�ci -B��d pola <pacjent> (A)"},
					{10094,"Przekroczenie maksymalnej kwoty sprzeda�y -B��d pola <lekarz> (A)"},
					{10095,"Pr�ba ponownego rozpocz�cia transakcji (drukarka w trybie transakcji) -B��d pola <refundator>"},
					{10096,"Przekroczony limit czasu na wydruk paragonu (20 minut) -B��d pola <wojewodztwo> (A)"},
					{10097,"Blokada sprzeda�y z powodu s�abego akumulatora"},
					{10098,"Blokada sprzeda�y z powodu za�o�onej zwory serwisowej"},
					{10100,"B��d pola <nazwa> (A)"},
					{10101,"B��d pola <recepta> (A)"},
					{10102,"B��d pola <ilo��> (A)"},
					{10103,"B��d pola <przyj��> (A)"},
					{10104,"B��d pola <zestawi�> (A)"},
					{10105,"B��d pola <wyda�> (A)"},
					{10106,"B��d pola <nazwaX> (A)"},
					{10107,"B��d pola <iloscX> (A)  "},
					{10108,"B��d pola <cenaX> (A)"},
					{10109,"B��d pola <razem_wartosc> (A)"},
					{10110,"Brak miejsca w pami�ci podr�cznej kopii elektronicznej -B��d pola <nazwa VAT>,<VAT> lub <podatek VAT> (A)"},
					{10111,"Dane z pami�ci podr�cznej kopii nieprzeniesione na no�nik -B��d pola <taxa> (A)"},
					{10112,"Drukarka nie jest gotowa, spr�buj ponownie -B��d pola <op�ata> (A)"},
					{10113,"Waluta ewidencyjna by�a ju� zmieniona po ostatnim raporcie dobowym, -Blokada sprzeda�y z powodu nieudanej pr�by automatycznej zmiany waluty,"},
					{10114,"B��d pola <pacjent> (A)"},
					{10115,"B��d pola <lekarz> (A)"},
					{10116,"B��d pola <refundator> (A)"},
					{10117,"B��d pola <wojew�dztwo> (A)"},
					{10118,"B��d pola <opis> (A)"},
					{10255,"Nierozpoznana komenda"},
					//w�asne b��dy parsowania odpowiedzi
					{20003, "Problem komunikacyjny z urz�dzeniem, brak odczytu" },
					{20004, "B��d zapisu do portu" },
					{20005, "Problem z wydrukiem paragonu\nprzekroczenie czasu obs�ugi\nanulowano" },
					{20006, "Wyj�tek w module fiskalnym" }
		};

		auto it = errors.find(error_code);
		if (it != errors.end()) {
			std::string error = errors.at(error_code);
			if (!error.empty()) {
				return error;
			}
		}
		IProtocol::GetErrorMessage(error_code);
	}

};

