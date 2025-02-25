#pragma once

#include "IProtocol.h"
#include "protocol_fiscal.h"
#include <vector>
#include "NonFiscalLineType.h"
#include "CStateManage.h"


/**
 * @author Piotr Kuliñski
 * @date 2024-09-28
 * @brief Klasa obs³uguj¹ca zdeserializowany [paragon] (\ref Paragon) w oparciu o protokó³ posnet. \n
 * Wydruk nie korzysta z bibliotek posnetu, implementuje bezpoœrednio protokó³ posnet.
 */
class ProtocolPosnet : public IProtocol {
private:
	const size_t ERROR_DEVICE_STATE{ 10000u };
	const size_t DEFAULT_WIDTH_NON_FISCAL{ 38u };

	auto checkresponse(const char* command, std::string& _response)
	{
		//if (_response.at(0)!= posnet::STX) {
		//	return 20001L;
		//}		
		//if (_response.at(_response.length() - 1)!= posnet::ETX) {
		//	return 20001L;
		//}
		//long result_code = 0l;
		//size_t p0 = _response.find("\tERR");
		//size_t p1 = _response.find("\t?");
		//if (p0 == std::string::npos || p1 == std::string::npos || (p1 - (p0 - 1)) <= 0) {
		//	return result_code;
		//}
		//p0--;
		//result_code = atol(_response.substr(p0, p1 - p0).c_str());
		//return result_code;

		const char* response = _response.c_str();
		if (*response != posnet::STX) {
			return 20001L;
		}

		if (response[_response.length() - 1] != posnet::ETX) {
			return 20002L;
		}

		long _err = 0l;
		char* w = (char*)strstr(response, "\tERR"); //b³¹d ramki
		if (w != NULL)
		{
			w = (char*)strstr(w, "\t?");
			w += 2;
			_err = atol(w);
		}
		else
		{
			w = (char*)strstr(response, "\t?"); //b³¹d sekwencji lub wykonania sekwencji
			if (w != NULL)
			{
				w += 2;
				_err = atol(w);
			}
		}
		return _err;
	}

	/*
	 * Sprawdzenie mechanizmu i interakcja z u¿ytkownikiem
	*/
	long sprn(Connection* connection, unsigned long remainingSeconds)
	{
		posnet::fiscal_stream request; request << "!sprn" << posnet::TAB;
		connection->WriteBytes(request.str());
		long result_code = -1L;
		std::stringstream ss;
		auto reads = connection->WaitReadData(ss, posnet::ETX);
		if (reads > 0) {
			std::string response = ss.str();
			if (response.length() <= 10) {
				return result_code;
			}
			size_t p0 = response.find_last_of("\tpr");
			size_t p1 = response.find("\t#");
			if (p0 == std::string::npos || p1 == std::string::npos || (p1 - (p0 - 1)) <= 0) {
				return result_code;
			}
			p0--;
			result_code = atol(response.substr(p0, p1 - p0).c_str());
			if (result_code != 0) {
				const auto description_error_code = (ERROR_DEVICE_STATE + result_code);
				std::string message = GetErrorMessage(description_error_code) +
					std::string("\nCzas na wykonanie ") +
					std::to_string(remainingSeconds) +
					std::string(" sek.");
				state.set(message);
				FiscalStatus ask;
				switch (result_code) {
					case 5: {
						ask = FiscalStatus::ANSWER_WAIT_PAPER;
						break;
					}
					default: {
						ask = FiscalStatus::ANSWER_WAIT_DEVICE;
						break;
					}
				}
				if (CallAnswer(message, EventType::evn_request_retry_cancel)) {
				}
				else if (!WaitForAnswer(ask, remainingSeconds * 1000)) {
					throw fiscal_exception(FiscalError::TIMEOUT_ANSWER);
				}

				if (!CheckAnswer(AnswerType::RETRY)) {
					throw fiscal_exception(FiscalError::USER_REJECT);
				}
			}
		}
		else {
			state.set(std::string{ "Urz¹dzenie nie zwróci³o ¿adnych danych" });
			if (!WaitForAnswer(FiscalStatus::ANSWER_WAIT_DEVICE, 60_sec)) {
				throw fiscal_exception(FiscalError::TIMEOUT_ANSWER);
			}
			if (!CheckAnswer(AnswerType::YES)) {
				throw fiscal_exception(FiscalError::USER_REJECT);
			}
		}
		return result_code;
	};

	const auto send(Connection* connection, const std::string& request)
	{
		size_t inBuff = request.length();
		inBuff -= connection->WriteBytes(request);
		if (!(inBuff == 0)) {
			throw fiscal_exception(FiscalError::COMMUNICATION_WRITE);
		}
		connection->ClearPort();
	};
	const auto sendread(Connection* connection, const std::string& request, std::string& response)
	{
		size_t inBuff = request.length();
		inBuff -= connection->WriteBytes(request);
		if (!(inBuff == 0)) {
			throw fiscal_exception(FiscalError::COMMUNICATION_WRITE);
		}

		std::stringstream ss;
		int nread = connection->WaitReadData(ss, posnet::ETX);
		if (nread > 0) {
			response.assign(ss.str());
			const auto last_error = checkresponse("", response);
			if (last_error != 0) {
				throw fiscal_exception(last_error);
			}
			return;
		}
		Timeout<u_long, 1000> maxTime(60_sec); //minutê czekamy na wyjaœnienie sprawy, a jak nie wysy³amy instrukcjê anulacji
		do {
			nread = sprn(connection, maxTime.elapsed() / 1000);
			if (maxTime.timeout()) {
				throw fiscal_exception(FiscalError::TIMEOUT_FISCAL);
			}
		} while (nread != 0);
	}

public:
	~ProtocolPosnet() {
		std::cout << "Destructor " << __FUNCTION__ << std::endl;
	}

	FiscalError PrintNonFiscalTicket(Connection* connection, XmlApiFiscalRequest xml) override
	{
		LOGGER_START();
		lock_receipt_protocol lck3(set_receipt_mutex);

		CStateManage<FiscalStatus, FiscalResult> ms{ state,result }; //RAII

		//CallAnswer("Rozpoczêcie wydruku");
		pugi::xml_document doc;
		if (const pugi::xml_parse_result xresult = doc.load_string(xml); !xresult) {
			result.set(FiscalResult::FAILED, "B³¹d parsowania wydruku");
			return FiscalError::PREPARE;
		}

		size_t width{ DEFAULT_WIDTH_NON_FISCAL };
		constexpr int FormNumber{ 200 };

		try {
			const pugi::xml_node tools = doc.child("Lines");
			if (std::string MaxWidth = tools.attribute("MaxWidth").value(); !MaxWidth.empty()) {
				width = min(width, std::stoul(MaxWidth));
			}

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

			auto centerLine = [&](int width, const std::string& str) {
				int len = str.length();
				if (width < len) { return str; }

				int diff = width - len;
				int pad1 = diff / 2;
				int pad2 = diff - pad1;
				return std::string(pad1, ' ') + str + std::string(pad2, ' ');
				};

			auto PrintLine = [&](std::string params, size_t FormNumber, size_t _lineNumber, bool bold, bool upper) {
				posnet::fiscal_stream ps;
				ps
					<< "formline" << posnet::TAB
					<< "s1" << params << posnet::TAB
					<< "fn" << FormNumber << posnet::TAB
					<< "fl" << _lineNumber << posnet::TAB;
				if (bold) {
					ps << "sw1" << posnet::TAB;
				}
				if (upper) {
					ps << "sh1" << posnet::TAB;
				}
				//std::string res;
				//sendread(connection, ps.str(), res);
				send(connection, ps.str());
				};

			auto CreateLine = [this](auto& IloscZnakow, const std::vector<SValue>& values) {
				bool bold = false;
				for (const SValue& v : values) {
					if (v.FrmBits & FormatType::Bold) {
						bold = true;
						break;
					}
				}

				std::stringstream linia;
				unsigned int RemainingLength = IloscZnakow;

				for (auto const& item : values) {
					if (item.Value != "") {
						int len = item.Value.length();
						if (RemainingLength >= len) {
							int width = RemainingLength; //(RemainingLength / (bold ? 2 : 1)) + 1;					
							int diff = width - len;
							int pad1 = diff / 2;
							int pad2 = diff - pad1;
							if ((item.FrmBits & FormatType::Center) > 0) {
								linia << std::string(pad1, ' ') + item.Value + std::string(pad2, ' ');
							}
							else if ((item.FrmBits & FormatType::Right) > 0) {
								linia << std::string(diff, ' ') + item.Value;
							}
							else {
								linia << item.Value;
							}
							RemainingLength -= item.Value.length();
						}
						else {
							linia << item.Value;
						}
					}
				}
				std::string buff = linia.str();

				size_t len_line = buff.length();
				if (len_line > IloscZnakow) {
					buff = buff.substr(0, (len_line - RemainingLength));
				}
				return buff;
				};

			constexpr size_t BaseFormLine = 711;
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
				for (unsigned int I = 1; I < 4; I++) {
					std::string no = std::to_string(I);
					std::string els("Value" + no);
					const char* el = tool.child_value(els.c_str());
					SValue val{
						el,
						tool.attribute(std::string("Param" + no).c_str()).as_string(),
						GetBitsFormat(val.Attr)
					};
					values.push_back(val);
				}
				return values;
				};

			state.set(FiscalStatus::PRINTING);
			posnet::fiscal_stream ps; ps
				<< "formstart" << posnet::TAB
				<< "fn" << FormNumber << posnet::TAB
				<< "fh" << 52 << posnet::TAB;
				//<< "fh" << 84 << posnet::TAB
				//<< "al" << "TERMINAL P£ATNICZY" << posnet::TAB;

			//sendread(connection, ps.str(), std::string{ "" });
			send(connection, ps.str());

			for (pugi::xml_node tool = tools.first_child(); tool; tool = tool.next_sibling()) {
				bool bold = false;
				std::vector<SValue> values = FormatValue(tool);
				for (const SValue& v : values) {
					if (v.FrmBits & FormatType::Bold || v.FrmBits & FormatType::HeightOnly) {
						bold = true;
						break;
					}
				}
				size_t IloscZnakow = (bold ? width / 2 : width);

				NonFiscalLineType iTyp = static_cast<NonFiscalLineType>(atoi(tool.child_value("Typ")));

				switch (iTyp) {
					case NonFiscalLineType::L_Declined:
					case NonFiscalLineType::L_Receipt:
					{
						if (values[0].Value.length() > (width / 2)) {
							bold = false;
							IloscZnakow = width;
						}
						std::stringstream ss;
						ss << centerLine(IloscZnakow, values[0].Value) << posnet::LF << " ";
						//################################## ##### 707 alfanumeryczny, alfanumeryczny, +
						PrintLine(ss.str(), FormNumber, BaseFormLine, bold, false);
						break;
					}

					case NonFiscalLineType::L_TranAmountCurr:
					{
						IloscZnakow = width;
						std::string linia = CreateLine(IloscZnakow, values);
						linia += '\n';
						linia += ' ';
						//################################## ##### 707 alfanumeryczny, alfanumeryczny, +
						PrintLine(linia, FormNumber, BaseFormLine, false, false);
						break;
					}
					case NonFiscalLineType::L_CardOperation: {
						if (bold) {
							IloscZnakow *= 2;
						}
						std::string linia = CreateLine(IloscZnakow, values);
						PrintLine(linia, FormNumber, BaseFormLine, false, true);
						break;
					}
					case NonFiscalLineType::L_PID_RECNo:
					case NonFiscalLineType::L_MID:
					case NonFiscalLineType::L_CardNoDate:
					case NonFiscalLineType::L_AID:
					case NonFiscalLineType::L_CardName:
					case NonFiscalLineType::L_TranAmount:
					case NonFiscalLineType::L_ExRate:
					case NonFiscalLineType::L_MarkUp:
					case NonFiscalLineType::L_MarkUpInfo:
					case NonFiscalLineType::L_CurrencyConversionInfo:
					case NonFiscalLineType::L_VerificationMethod:
					case NonFiscalLineType::L_AuthCode:
					case NonFiscalLineType::L_TC:
					case NonFiscalLineType::L_ExRateInfo:
					case NonFiscalLineType::L_DateTime:
					case NonFiscalLineType::L_Void:
					case NonFiscalLineType::L_ForReceipt:
					case NonFiscalLineType::L_TrVoided:
					case NonFiscalLineType::L_AdditionalText:
					case NonFiscalLineType::L_Sign:
					{
						std::string linia = CreateLine(IloscZnakow, values);
						//707 dodajemy drugi pusty parametr
						// linia << "\\n ";
						//linia += '\n';
						//linia += ' ';
						//################################## ##### 707 alfanumeryczny, alfanumeryczny, +
						PrintLine(linia, FormNumber, BaseFormLine, bold, false);
						break;

					}

					case NonFiscalLineType::L_Empty:
					{
						posnet::fiscal_stream ps;
						//ps
						//	<< "formcmd" << posnet::TAB
						//	<< "fn" << FormNumber << posnet::TAB
						//	<< "cm0" << posnet::TAB;

						ps
							<< "formline" << posnet::TAB
							<< "s1" << " " << posnet::TAB
							<< "fn" << FormNumber << posnet::TAB
							<< "fl" << BaseFormLine << posnet::TAB;

						//std::string res;
						//sendread(connection, ps.str(), res);
						send(connection, ps.str());
						break;
					}

					default:
					{
						IloscZnakow = width;
						std::string linia = CreateLine(IloscZnakow, values);
						//linia += '\n';
						//linia += ' ';
						//|######################################| 711 alfanumeryczny, +
						PrintLine(linia, FormNumber, BaseFormLine, bold, false);
						//error = send_WydrukNiefiskalnyLiniaEx(FormNumber, 711, bold, bold, linia, L"", L"", L"", L"");
						break;
					}
				}
			}

			ps.str("");
			ps << "formend" << posnet::TAB << "fn" << FormNumber << posnet::TAB;
			//std::string res;
			//sendread(connection, ps.str(), res);
			send(connection, ps.str());

			result.set(FiscalResult::OK, "Wydruk zakoñczony poprawnie");

			if (false) {
				u_short pr = 0;
				ps.str("");
				ps << "hdrset" << posnet::TAB
					<< "tx&c&b&N&1" << "PIOTR KULIÑSKI" << "&1&N&b&c" << '\n'
					<< "&c&5" << "Protokó³ " << (pr == 1 ? "THERMAL" : "POSNET") << "&5&c" << '\n'
					<< "&c&3" << "POSNET Thermal HS FV EJ vr6.03" << "&3&c" << '\n'
					<< "&c&i&8" << "POSNET Thermal HS FV EJ 1.03" << "&8&i&c" << posnet::TAB
					<< "pr1" << posnet::TAB;
				//sendread(connection, ps.str(), res);
				send(connection, ps.str());
				ps.str("");	ps
					<< "pccfgset" << posnet::TAB
					<< "id" << 2 << posnet::TAB // 1-COM, 2 - USB
					<< "no" << 1 << posnet::TAB // 1-COM, >1 TCP
					<< "pr" << 0 << posnet::TAB // 0-posnet, 1 - thermal
					<< "cp" << 0 << posnet::TAB // 0 – WIN1250 1 – LATIN2,2 – MAZOVIA
					;
				//sendread(connection, ps.str(), res);
				send(connection, ps.str());

				ps.str("");
				ps << "pccfgget" << posnet::TAB;
				//sendread(connection, ps.str(), res);
				send(connection, ps.str());
			}

			return FiscalError::STATUS_OK;
		}
		catch (fiscal_exception<FiscalError>& ex) {
			std::string message = ex.what();
			if (message.empty()) {
				message = GetErrorMessage(ex.getErrorNo());
			}
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message.c_str(), EventType::evn_exception);
		}
		catch (fiscal_exception<long>& ex) {
			std::string message = GetErrorMessage(ex.getErrorNo());
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message.c_str(), EventType::evn_exception);
		}
		catch (std::exception& xex) {
			std::string message = std::string("B³¹d wydruku") + xex.what();
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message.c_str(), EventType::evn_exception);
		}
		catch (...) {
			std::string message = "B³¹d wydruku nefiskalnego";
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message.c_str(), EventType::evn_exception);
		}
		connection->ClearError();
		send(connection, "hardprncancel");
		return FiscalError::ANY_ERROR;
	}

	FiscalError PrintFiscalTicket(Connection* connection, XmlApiNonFiscalRequest xml)
	{
		LOGGER_START();
		lock_receipt_protocol lck3(set_receipt_mutex);

		CStateManage<FiscalStatus, FiscalResult> ms{ state,result }; //RAII

		long xresult = 0;
		Paragon paragon = DeserializeTicket(xml, xresult);
		if (xresult != 0L) {
			result.set(FiscalResult::FAILED, "B³¹d wczytywania paragonu");
			return FiscalError::PREPARE;
		}
		//Connection* connection = instance->getConnection().get();
		if (paragon.fconnection != nullptr && (paragon.fconnection->type != ConnectionType::RS232 || connection->isConnectionType(ConnectionType::TCP))) {
			result.set(FiscalResult::FAILED, "Protokó³ obs³uguje jedynie po³¹czenie RS232");
			return FiscalError::CONFIGURATION_INVALID;
		}

		//fake
		struct {
			size_t trline_max_length = 38;
		} properties;

		int LastFiscalNumber = 0;

		std::vector<std::string_view> vat;

		const auto MapVat = [&](const std::string& value) noexcept(false) {
			double nvat = 0.00;
			if (value.at(0) == 'Z' || value.find("99") != std::string::npos) {
				nvat = 100.00f;
			}
			else {
				nvat = fiscal::round(value);
			}
			for (u_short I = 1; I < 8; I++) {
				const double v = fiscal::round((const char*)&vat.at(I).substr(2, 10)[0]);
				if (v == nvat) {
					const char S = (const char)(vat.at(I).substr(1, 1)[0] - 32 - 65 + 48); //-65 numer stawki
					return S;
				}
			}
			throw std::exception("B³¹d mapowania stawki VAT");
			};

		auto fiscal_result = FiscalError::ANY_ERROR;

		posnet::fiscal_stream req;
		const bool foundOpened = connection->IsOpened();
		try {
			state.set(FiscalStatus::PROCESSING);
			if (!foundOpened) {
				if (connection->Open() != ConnectionState::OPEN) {
					throw fiscal_exception(FiscalError::COMMUNICATION);
				}
			}
			state.set(FiscalStatus::PRINTING);

			std::unique_ptr<payment_base> paymentConversion = std::make_unique<payment_to_posnet>();
			std::string response{};

			req.str(""); req << "sid" << posnet::TAB;
			sendread(connection, req.str(), response);

			for (auto& c : response) c = toupper(c);
			bool isOnline = (response.find("ONLINE") != std::string::npos);
			bool isHSFV = (response.find("HS FV") != std::string::npos);

			req << "dspcfg" << posnet::TAB << "id0" << posnet::TAB << "lu3" << posnet::TAB;
			sendread(connection, req.str(), response);

			req.str(""); req << "trcancel" << posnet::TAB;
			sendread(connection, req.str(), response);

			req.str(""); req << "vatget" << posnet::TAB;
			std::string response_vat; //na to bêdzie string_view
			sendread(connection, req.str(), response_vat);
			std::string_view sv{ response_vat };
			vat = splitSV(sv, posnet::TAB);
			if (vat.size() != 9) {
				throw fiscal_exception(FiscalError::PREPARE_VAT);
			}

			req.str(""); req << "trinit" << posnet::TAB << "bm0" << posnet::TAB;
			sendread(connection, req.str(), response);

			for (Description& desc : paragon.descriptions) {
				std::string request, response;
				req.str("");
				req << "ftrcfg" << posnet::TAB;
				switch (desc.typ) {
					case OPIS_NIP_NABYWCY: {
						try {
							req.str("");
							req << "trnipset" << posnet::TAB << "ni" + desc.nazwa << posnet::TAB;
							sendread(connection, req.str(), response);
							desc.isPrintHeader = true;
						}
						catch (std::exception& ex) {
							std::cerr << "Nip zostanie wydrukowany w stopce: " << ex.what() << std::endl;
						}
						continue;
					}
					case OPIS_KELNER: {
						req << "cc" << paragon.encoding(desc.nazwa).substr(0, 32) << posnet::TAB;
						break;
					}
					case OPIS_KASA: {
						req << "cn" << paragon.encoding(desc.nazwa).substr(0, 8) << posnet::TAB;
						break;
					}
					case OPIS_TRANSAKCJA0:
					case OPIS_TRANSAKCJA1: {
						req << "sn" << paragon.encoding(desc.nazwa) << posnet::TAB;
						break;
					}
					case OPIS_KOD_KRESKOWY: {
						req << "bc" << paragon.encoding(desc.nazwa) << posnet::TAB;
						break;
					}
					case OPIS_ZAPRASZAMY_PONOWNIE: {
						req << "ln" << paragon.encoding(desc.nazwa) << posnet::TAB;
						break;
					}
					case OPIS_NAZWA_PROGRAMU: {
						req.str("");
						req << "dsptxtline" << posnet::TAB << "id0" << posnet::TAB << "no0" << posnet::TAB << "ln" << paragon.encoding(desc.nazwa) << posnet::TAB;
						sendread(connection, req.str(), response);
						desc.isPrintHeader = true;
						continue;
					}
					default: {
						continue;
					}
				}
				desc.isPrintHeader = true;

				req << "fe0" << posnet::TAB;
				sendread(connection, req.str(), response);
			}

			for (ReceiptLine& line : paragon.receiptLines) {
				req.str("");
				req << "trline" << posnet::TAB
					<< "na" << paragon.encoding(line.nazwa.substr(0, properties.trline_max_length)) << posnet::TAB;
				if (!line.opis.empty()) {
					req << "op" << paragon.encoding(line.opis.substr(0, properties.trline_max_length)) << posnet::TAB;
				}
				req << "vt" << line.VatID<char>(MapVat) << posnet::TAB
					<< "pr" << line.cena_sprzedazy<Grosze>() << posnet::TAB
					<< "il" << line.Ilosc<DecimalStringTrim>() << posnet::TAB;

				if (line.czyRabatNarzut()) {
					req << "rd" << (line.czyRabat() ? "y" : "n") << posnet::TAB
						<< "rw" << line.wartosc_rabat_narzut<Grosze>() << posnet::TAB;
				}
				std::stringstream resp;
				sendread(connection, req.str(), response);
			}

			for (Prepayment zaliczka : paragon.prePayments) {
				req.str("");
				req << "trdiscntvat" << posnet::TAB
					<< "vt" << zaliczka.VatID<char>(MapVat) << posnet::TAB;

				switch (zaliczka.typRabatu) {
					case DiscountTypeSummary::P_RabatProcentowy:
					{
						req << "rd1" << posnet::TAB << "rp";
						break;
					}
					case DiscountTypeSummary::P_NarzutProcentowy:
					{
						req << "rd0" << posnet::TAB << "rp";
						break;
					}
					case DiscountTypeSummary::P_RabatKwotowy:
					{
						req << "rd1" << posnet::TAB << "rw";
						break;
					}
					case DiscountTypeSummary::P_NarzutKwotowy:
					{
						req << "rd0" << posnet::TAB << "rw";
						break;
					}
				}
				req << zaliczka.wartosc() << posnet::TAB;
				if (!zaliczka.nazwa.empty()) {
					req << "na" << paragon.encoding(zaliczka.nazwa).substr(0, 25) << posnet::TAB;
				}
				sendread(connection, req.str(), response);
			}

			for (Payment platnosc : paragon.payments) {
				req.str("");

				if (platnosc.isDcc()) {
					req
						<< "trpaymentcurr" << posnet::TAB
						<< "na" << posnet::TAB
						<< "wc" << platnosc.dcc.wartosc() << posnet::TAB
						<< "sb" << platnosc.dcc.symbol() << posnet::TAB
						<< "ra" << platnosc.dcc.kurs() << posnet::TAB;
				}
				else {
					req
						<< "trpayment" << posnet::TAB
						<< "ty" << paymentConversion->convert(platnosc.getPeymentID()) << posnet::TAB
						<< "wa" << platnosc.wartosc() << posnet::TAB;
					if (!platnosc.nazwa.empty()) {
						req << "na" << paragon.encoding(platnosc.nazwa).substr(0, 25) << posnet::TAB;
					}
					req << "re0" << posnet::TAB;
				}
				std::string response;
				sendread(connection, req.str(), response);
			}

			ReceiptTotal& summary = paragon.summary;

			req.str("");
			req
				<< "trend" << posnet::TAB
				<< "to" << summary.total() << posnet::TAB
				<< "fp" << summary.platnosc() << posnet::TAB
				<< "re" << 0l << posnet::TAB
				<< "fe0" << posnet::TAB;
			sendread(connection, req.str(), response);

			for (Description desc : paragon.descriptions) {
				if (!desc.isPrintHeader) {
					req.str("");
					req << "trftrln" << posnet::TAB
						<< "id" << (desc.typ > 1000 ? desc.typ % 1000 : desc.typ) << posnet::TAB
						<< "na";
					if (desc.typ == OPIS_NIP_NABYWCY) {
						req << "NIP:" << desc.nazwa << posnet::TAB << "sw1" << posnet::TAB << "sh0";
					}
					else {
						req << paragon.encoding(desc.nazwa);
					}
					req << posnet::TAB;
					sendread(connection, req.str(), response);
				}
			}
			req.str(""); req << "trftrend" << posnet::TAB;
			sendread(connection, req.str(), response);

			//pobranie ostatnio wydrukowanego numeru paragonu
			req.str(""); req << "scnt" << posnet::TAB;
			sendread(connection, req.str(), response);

			std::string_view scnt{ response };
			const auto vscnt = splitSV(scnt, posnet::TAB);

			if (vscnt.size() >= 4) {
				LastFiscalNumber = atoi((const char*)&vscnt[3].substr(2, 10)[0]);
			}

			std::stringstream ss; ss << "Wydruk zakoñczony poprawnie, paragon: " << LastFiscalNumber;

			result.set(FiscalResult::OK, ss.str());
			fiscal_result = FiscalError::STATUS_OK;
		}
		catch (fiscal_exception<FiscalError>& ex) {
			std::string message = ex.what();
			if (message.empty()) {
				message = GetErrorMessage(ex.getErrorNo());
				result.set(message);
			}
			result.set(FiscalResult::FAILED);
			CallAnswer(message, EventType::evn_exception);
		}
		catch (fiscal_exception<long>& ex) {
			std::string message = GetErrorMessage(ex.getErrorNo());
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message, EventType::evn_exception);
		}
		catch (std::exception& xex) {
			std::string message = std::string("B³¹d wydruku paragonu\n") + xex.what();
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message, EventType::evn_exception);
		}
		catch (...) {
			std::string message = "B³¹d wydruku paragonu\ntransakcja zostanie anulowana";
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message, EventType::evn_exception);
		}

		if (result.check(FiscalResult::FAILED) && connection->IsOpened()) {
			try {
				req.str("");
				req << "trcancel" << posnet::TAB;
				sendread(connection, req.str(), std::string{ "" });
				CallAnswer("Wys³ano anulacjê transakcji", EventType::evn_exception);
			}
			catch (...) {
				std::string message = "Problem z anulowaniem transakcji";
				result.set(FiscalResult::FAILED, message);
				CallAnswer(message, EventType::evn_exception);
			}
		}
		connection->ClearError();
		if (!foundOpened) {
			connection->Close();
		}

		return fiscal_result;
	}

	virtual std::string GetErrorMessage(FiscalError error_code) override {
		return IProtocol::GetErrorMessage(error_code);
	}

	virtual std::string GetErrorMessage(long error_code) override {
		static const std::map<long, std::string> errors{
		//b³êdy ramki
		{ 1, "Nierozpoznana komenda" },
		{ 2, "Brak obowi¹zkowego pola" },
		{ 3, "B³¹d konwersji pola (np.: przes³ana zosta³a wartoœæ z przecinkiem w polu którego wartoœæ przesy³a siê w czêœciach setnych np.: 12,34 zamiast 1234, lub przekroczony zakres danych)" },
		{ 4, "B³êdny token" },
		{ 5, "Z³a suma kontrolna" },
		{ 6, "B³¹d budowy ramki, brak mnemonika parametru" },
		{ 10, "Niew³aœciwa d³ugoœæ pola danych" },
		{ 11, "Zape³niony bufor odbiorczy" },
		{ 12, "Nie mo¿na wykonaæ rozkazu w trybie natychmiastowym" },
		{ 13, "Nie znaleziono rozkazu o podanym tokenie" },
		{ 14, "Zape³niona kolejka wejœciowa" },
		{ 15, "B³¹d budowy ramki, brak sumy kontrolnej" },
		//B³êdy poleceñ
		{ 30,"b³¹d nietypowy - rezygnacja, przerwanie funkcji" },
		{ 50,"B³¹d wykonywania operacji przez kasê" },
		{ 51,"B³¹d wykonywania operacji przez kasê" },
		{ 52,"B³¹d wykonywania operacji przez kasê" },
		{ 53,"B³¹d wykonywania operacji przez kasê" },
		{ 54,"B³¹d wykonywania operacji przez kasê" },
		{ 55,"B³¹d wykonywania operacji przez kasê" },
		{ 56,"B³¹d wykonywania operacji przez kasê" },
		{ 323,"Funkcja zablokowana w konfiguracji" },
		{ 360,"Znaleziono zworê serwisow¹" },
		{ 361,"Nie znaleziono zwory" },
		{ 364,"B³êdne has³o w sensie niezgodnoœci z poprawnoœci¹ sk³adniow¹" },
		{ 365,"B³êdne has³o w sensie niezgodnoœci z tym z bazy" },
		{ 382,"Próba wykonania raportu zerowego" },
		{ 383,"Brak raportu dobowego" },
		{ 384,"Brak rekordu w pamiêci" },
		{ 400,"B³êdna wartoœæ" },
		{ 404,"Wprowadzono nieprawid³owy kod kontrolny" },
		{ 460,"B³¹d zegara w trybie fiskalnym" },
		{ 461,"B³¹d zegara w trybie niefiskalnym" },
		{ 480,"Drukarka ju¿ autoryzowana, bezterminowo" },
		{ 481,"Nie rozpoczêto jeszcze autoryzacji" },
		{ 482,"Kod ju¿ wprowadzony" },
		{ 483,"Próba wprowadzenia b³êdnych wartoœci" },
		{ 484,"Min¹³ czas pracy kasy, sprzeda¿ zablokowana" },
		{ 485,"B³êdny kod autoryzacji" },
		{ 486,"Blokada autoryzacji. WprowadŸ kod z klawiatury" },
		{ 487,"U¿yto ju¿ maksymalnej liczby kodów" },
		{ 499,"Aktywny protokó³ Thermal. Proszê zmieniæ protokó³" },
		{ 500,"Przepe³nienie statystyki minimalnej" },
		{ 501,"Przepe³nienie statystyki maksymalnej" },
		{ 502,"Przepe³nienie stanu kasy" },
		{ 503,"Wartoœæ stanu kasy po wyp³acie staje siê ujemna (przyjmuje siê stan zerowy kasy)" },
		{ 700,"B³êdny adres IP" },
		{ 701,"B³¹d numeru tonu" },
		{ 702,"B³¹d d³ugoœci impulsu szuflady" },
		{ 703,"B³¹d stawki VAT" },
		{ 705,"B³¹d czasu uœpienia" },
		{ 706,"B³¹d czasu wy³¹czenia" },
		{ 713,"B³êdne parametry konfiguracji" },
		{ 714,"B³êdna wartoœæ kontrastu wyœwietlacza" },
		{ 715,"B³êdna wartoœæ podœwietlenia wyœwietlacza" },
		{ 716,"B³êdna wartoœæ czasu zaniku podœwietlenia" },
		{ 717,"Za d³uga linia nag³ówka albo stopki" },
		{ 718,"B³êdna konfiguracja komunikacji" },
		{ 719,"B³êdna konfiguracja protoko³u kom" },
		{ 720,"B³êdny identyfikator portu" },
		{ 721,"B³êdny numer tekstu reklamowego" },
		{ 722,"Podany czas wychodzi poza wymagany zakres" },
		{ 723,"Podana data/czas niepoprawne" },
		{ 724,"Inna godzina w ró¿nicach czasowych 0<=>23" },
		{ 726,"B³êdna zawartoœæ tekstu w linii wyœwietlacza" },
		{ 727,"B³êdna wartoœæ dla przewijania na wyœwietlaczu" },
		{ 728,"B³êdna konfiguracja portu" },
		{ 729,"B³êdna konfiguracja monitora transakcji" },
		{ 730,"Port zajêty przez komputer" },
		{ 731,"Port zajêty przez TCP/IP" },
		{ 732,"Port zajêty przez monitor" },
		{ 733,"Port zajêty" },
		{ 734,"Port zajêty przez tunelowanie" },
		{ 738,"Nieprawid³owa konfiguracja Ethernetu" },
		{ 739,"Nieprawid³owy typ wyœwietlacza" },
		{ 740,"Dla tego typu wyœwietlacza nie mo¿na ustawiæ czasu zaniku podœwietlenia" },
		{ 741,"Wartoœæ czasu spoza zakresu" },
		{ 745,"B³êdna numer strony kodowej" },
		{ 746,"B³êdna konfiguracja ramki monitora transakcji" },
		{ 747,"DHCP aktywne. Funkcja niedostêpna" },
		{ 748,"DHCP dozwolone tylko przy transmisji ethernet" },
		{ 752,"Protokó³ Thermal jest niedostêpny po interfejsie TCP/IP" },
		{ 820,"Negatywny wynik testu" },
		{ 821,"Brak testowanej opcji w konfiguracji" },
		{ 857,"Brak pamiêci na inicjalizacjê bazy drukarkowej" },
		{ 1000,"B³¹d fatalny modu³u fiskalnego" },
		{ 1001,"Wypiêta pamiêæ fiskalna" },
		{ 1002,"B³¹d zapisu" },
		{ 1003,"B³¹d nie ujêty w specyfikacji bios" },
		{ 1004,"B³êdne sumy kontrolne" },
		{ 1005,"B³¹d w pierwszym bloku kontrolnym" },
		{ 1006,"B³¹d w drugim bloku kontrolnym" },
		{ 1007,"B³êdny id rekordu" },
		{ 1008,"B³¹d inicjalizacji adresu startowego" },
		{ 1009,"Adres startowy zainicjalizowany" },
		{ 1010,"Numer unikatowy ju¿ zapisany" },
		{ 1011,"Brak numeru w trybie fiskalnym" },
		{ 1012,"B³¹d zapisu numeru unikatowego" },
		{ 1013,"Przepe³nienie numerów unikatowych" },
		{ 1014,"B³êdny jêzyk w numerze unikatowym" },
		{ 1015,"Wiêcej ni¿ jeden NIP" },
		{ 1016,"Drukarka w trybie do odczytu bez rekordu fiskalizacji" },
		{ 1017,"Przekroczono liczbê zerowañ RAM" },
		{ 1018,"Przekroczono liczbê raportów dobowych" },
		{ 1019,"B³¹d weryfikacji numeru unikatowego" },
		{ 1020,"B³¹d weryfikacji statystyk z  RD" },
		{ 1021,"B³¹d odczytu danych z NVR do weryfikacji FM" },
		{ 1022,"B³¹d zapisu danych z NVR do weryfikacji FM" },
		{ 1023,"Pamiêæ fiskalna jest ma³a" },
		{ 1024,"Nie zainicjalizowany obszar danych w pamiêci fiskalnej" },
		{ 1025,"B³êdny format numeru unikatowego" },
		{ 1026,"Za du¿o b³êdnych bloków w FM" },
		{ 1027,"B³¹d oznaczenia b³êdnego bloku" },
		{ 1028,"Rekord w pamiêci fiskalnej nie istnieje - obszar pusty" },
		{ 1029,"Rekord w pamiêci fiskalnej z dat¹ póŸniejsz¹ od poprzedniego" },
		{ 1030,"B³¹d odczytu skrótu raportu dobowego" },
		{ 1031,"B³¹d zapisu skrótu raportu dobowego" },
		{ 1032,"B³¹d odczytu informacji o weryfikacji skrótu raportu dobowego" },
		{ 1033,"B³¹d zapisu informacji o weryfikacji skrótu raportu dobowego" },
		{ 1034,"B³¹d odczytu etykiety noœnika" },
		{ 1035,"B³¹d zapisu etykiety noœnika" },
		{ 1036,"Niezgodnoœæ danych kopii elektronicznej" },
		{ 1037,"B³êdne dane w obszarze bitów faktur, brak ci¹g³oœci, zapl¹tany gdzieœ bit lub podobne" },
		{ 1038,"B³¹d w obszarze faktur. Obszar nie jest pusty" },
		{ 1039,"Brak miejsca na nowe faktury" },
		{ 1040,"Suma faktur z raportów dobowych jest wiêksza od licznika faktur" },
		{ 1041,"B³¹d w obszarze ID modu³u kopii" },
		{ 1042,"B³¹d zapisu ID modu³u kopii" },
		{ 1043,"Obszar ID modu³u kopii zape³niony" },
		{ 1044,"nieudana fiskalizacja" },
		{ 1045,"Data wczeœniejsza od daty poprzedniego raportu dobowego" },
		{ 1950,"Przekroczony zakres totalizerów paragonu" },
		{ 1951,"Wp³ata form¹ p³atnoœci przekracza max. wp³atê" },
		{ 1952,"Suma form p³atnoœci przekracza max. wp³atê" },
		{ 1953,"Formy p³atnoœci pokrywaj¹ ju¿ do zap³aty" },
		{ 1954,"Wp³ata reszty przekracza max. wp³atê" },
		{ 1955,"Suma form p³atnoœci przekracza max. wp³atê" },
		{ 1956,"Przekroczony zakres total" },
		{ 1957,"Przekroczony maksymalny zakres paragonu" },
		{ 1958,"Przekroczony zakres wartoœci opakowañ" },
		{ 1959,"Przekroczony zakres wartoœci opakowañ przy stornowaniu" },
		{ 1961,"Wp³ata reszty zbyt du¿a" },
		{ 1962,"Wp³ata form¹ p³atnoœci wartoœci 0" },
		{ 1980,"Przekroczony zakres kwoty bazowej rabatu/narzutu" },
		{ 1981,"Przekroczony zakres kwoty po rabacie / narzucie" },
		{ 1982,"B³¹d obliczania rabatu/narzutu" },
		{ 1983,"Wartoœæ bazowa ujemna lub równa 0" },
		{ 1984,"Wartoœæ rabatu/narzutu zerowa" },
		{ 1985,"Wartoœæ po rabacie ujemna lub równa 0" },
		{ 1990,"Niedozwolone stornowanie towaru. B³êdny stan transakcji" },
		{ 1991,"Niedozwolony rabat/narzut. B³êdny stan transakcji" },
		{ 2000,"B³¹d pola VAT ( b³êdny numer stawki lub nieaktywna stawka)" },
		{ 2002,"Brak nag³ówka" },
		{ 2003,"Zaprogramowany nag³ówek" },
		{ 2004,"Brak aktywnych stawek VAT" },
		{ 2005,"Brak trybu transakcji" },
		{ 2006,"B³¹d pola cena ( cena <= 0 )" },
		{ 2007,"B³¹d pola iloœæ ( iloœæ <= 0 )" },
		{ 2008,"B³¹d kwoty total" },
		{ 2009,"B³¹d kwoty total, równa zero" },
		{ 2010,"Przekroczony zakres totalizerów dobowych" },
		{ 2020,"Zmiana daty niedozwolona" },
		{ 2021,"Próba ponownego ustawienia zegara" },
		{ 2022,"Zbyt du¿a ró¿nica dat" },
		{ 2023,"Ró¿nica wiêksza ni¿ 2 godziny w trybie u¿ytkownika w trybie fiskalnym" },
		{ 2024,"Z³y format daty (np. 13 miesi¹c )" },
		{ 2025,"Data wczeœniejsza od ostatniego zapisu do modu³u" },
		{ 2026,"B³¹d zegara" },
		{ 2027,"Przekroczono maksymaln¹ liczbê zmian stawek VAT" },
		{ 2028,"Próba zdefiniowana identycznych stawek VAT" },
		{ 2029,"B³êdne wartoœci stawek VAT" },
		{ 2030,"Próba zdefiniowania stawek VAT wszystkich nieaktywnych" },
		{ 2031,"B³¹d pola NIP" },
		{ 2032,"B³¹d numeru unikatowego pamiêci fiskalnej" },
		{ 2033,"Urz¹dzenie w trybie fiskalnym" },
		{ 2034,"Urz¹dzenie w trybie niefiskalnym" },
		{ 2035,"Niezerowe totalizery" },
		{ 2036,"Urz¹dzenie w stanie tylko do odczytu" },
		{ 2037,"Urz¹dzenie nie jest w stanie tylko do odczytu" },
		{ 2038,"Urz¹dzenie w trybie transakcji" },
		{ 2039,"Zerowe totalizery" },
		{ 2040,"B³¹d obliczeñ walut, przepe³nienie przy mno¿eniu lub dzieleniu" },
		{ 2041,"Próba zakoñczenia pozytywnego paragonu z wartoœci¹ 0" },
		{ 2042,"B³êdy format daty pocz¹tkowej" },
		{ 2043,"B³êdy format daty koñcowej" },
		{ 2044,"Próba wykonania raportu miesiêcznego w danym miesi¹cu" },
		{ 2045,"Data pocz¹tkowa póŸniejsza od bie¿¹cej daty" },
		{ 2046,"Data koñcowa wczeœniejsza od daty fiskalizacji" },
		{ 2047,"Numer pocz¹tkowy lub koñcowy równy zero" },
		{ 2048,"Numer pocz¹tkowy wiêkszy od numeru koñcowego" },
		{ 2049,"Numer raportu zbyt du¿y" },
		{ 2050,"Data pocz¹tkowa póŸniejsza od daty koñcowej" },
		{ 2051,"Brak pamiêci w buforze tekstów" },
		{ 2052,"Brak pamiêci w buforze transakcji" },
		{ 2054,"Formy p³atnoœci nie pokrywaj¹ kwoty do zap³aty lub reszty" },
		{ 2055,"B³êdna linia" },
		{ 2056,"Tekst pusty" },
		{ 2057,"Przekroczony rozmiar lub przekroczona liczba znaków formatuj¹cych" },
		{ 2058,"B³êdna liczba linii" },
		{ 2059,"B³êdny kod kreskowy" },
		{ 2060,"B³êdny stan transakcji" },
		{ 2062,"Jest wydrukowana czêœæ jakiegoœ dokumentu" },
		{ 2063,"B³¹d parametru" },
		{ 2064,"Brak rozpoczêcia wydruku lub transakcji" },
		{ 2067,"B³¹d ustawieñ konfiguracyjnych wydruków / drukarki" },
		{ 2070,"Data przegl¹du wczeœniejsza od systemowej" },
		{ 2080,"Nieparzysta liczba danych w formacie HEX" },
		{ 2081,"Niepoprawna wartoœæ dla formatu HEX" },
		{ 2101,"Zape³nienie bazy" },
		{ 2102,"Stawka nieaktywna" },
		{ 2103,"Nieprawid³owa stawka VAT" },
		{ 2104,"B³¹d nazwy" },
		{ 2105,"B³¹d przypisania stawki" },
		{ 2106,"Towar zablokowany" },
		{ 2107,"Nie znaleziono w bazie drukarkowej" },
		{ 2110,"B³¹d autoryzacji" },
		{ 2501,"B³êdny identyfikator raportu" },
		{ 2502,"B³êdny identyfikator linii raportu" },
		{ 2503,"B³êdny identyfikator nag³ówka raportu" },
		{ 2504,"Zbyt ma³o parametrów raportu" },
		{ 2505,"Raport nie rozpoczêty" },
		{ 2506,"Raport rozpoczêty" },
		{ 2507,"B³êdny identyfikator komendy" },
		{ 2508,"Próba wydrukowania szerokiej formatki na papierze 57mm" },
		{ 2521,"Raport ju¿ rozpoczêty" },
		{ 2522,"Raport nie rozpoczêty" },
		{ 2523,"B³êdna stawka VAT" },
		{ 2532,"B³êdna liczba kopii faktur" },
		{ 2533,"Pusty numer faktury" },
		{ 2534,"B³êdny format wydruku" },
		{ 2600,"B³êdny typ rabatu/narzutu" },
		{ 2601,"Wartoœæ rabatu/narzutu spoza zakresu" },
		{ 2620,"B³êdna nazwa wyceny" },
		{ 2621,"Tablica wycen zape³niona" },
		{ 2622,"Tablica wycen nie pusta" },
		{ 2623,"Tablica wycen zape³niona" },
		{ 2624,"Rozpoczêta wycena innego typu" },
		{ 2625,"Za du¿o linii wyceny" },
		{ 2701,"B³¹d identyfikatora stawki podatkowej" },
		{ 2702,"B³êdny identyfikator dodatkowej stopki" },
		{ 2703,"Przekroczona liczba dodatkowych stopek" },
		{ 2704,"Zbyt s³aby akumulator" },
		{ 2705,"B³êdny identyfikator typu formy p³atnoœci" },
		{ 2706,"Brak zasilacza" },
		{ 2710,"Us³uga o podanym identyfikatorze nie jest uruchomiona" },
		{ 2801,"B³¹d weryfikacji wartoœci rabatu/narzutu" },
		{ 2802,"B³¹d weryfikacji wartoœci linii sprzeda¿y" },
		{ 2803,"B³¹d weryfikacji wartoœci opakowania" },
		{ 2804,"B³¹d weryfikacji wartoœci formy p³atnoœci" },
		{ 2805,"B³¹d weryfikacji wartoœci fiskalnej" },
		{ 2806,"B³¹d weryfikacji wartoœci opakowañ dodatnich" },
		{ 2807,"B³¹d weryfikacji wartoœci opakowañ ujemnych" },
		{ 2808,"B³¹d weryfikacji wartoœci wp³aconych form p³atnoœci" },
		{ 2809,"B³¹d weryfikacji wartoœci reszt" },
		{ 2851,"B³¹d stornowania, b³êdna iloœæ" },
		{ 2852,"B³¹d stornowania, b³êdna wartoœæ" },
		{ 2900,"Stan kopii elektronicznej nie pozwala na wydrukowanie tego dokumentu" },
		{ 2901,"Brak noœnika lub operacja na noœniku trwa" },
		{ 2902,"Noœnik nie jest poprawnie zweryfikowany" },
		{ 2903,"Pamiêæ podrêczna kopii elektronicznej zawiera zbyt du¿¹ iloœæ danych" },
		{ 2906,"Uszkodzony bufor kopii elektronicznej" },
		{ 2907,"Brak noœnika" },
		{ 2908,"Noœnik nieprawid³owy - nieodpowiedni dla wybranej operacji" },
		{ 2911,"Brak pliku na noœniku" },
		{ 2913,"Nieprawid³owy wynik testu" },
		{ 2915,"Pusta pamiêæ podrêczna" },
		{ 2916,"Trwa weryfikacja noœnika" },
		{ 2917,"B³êdny typ dokumentu" },
		{ 2918,"Dane niedostêpne (nieaktualne)" },
		{ 3051,"Nie mo¿na zmieniæ dwa razy waluty ewidencyjnej po RD" },
		{ 3052,"Próba ustawienia ju¿ ustawionej waluty" },
		{ 3053,"B³êdna nazwa waluty" },
		{ 3054,"Automatyczna zmiana waluty" },
		{ 3055,"B³êdna wartoœæ przelicznika kursu" },
		{ 3056,"Przekroczono maksymaln¹ liczbê zmian walut" },
		{ 3080,"Próba zdefiniowania stawek VAT ze star¹ dat¹" },
		{ 3084,"Automatyczna zmiana stawek VAT" },
		{ 3085,"Brak pola daty" },
		{ 3090,"Nieprawid³owy kod autoryzacji formatki" },
		{ 3091,"Autoryzacja formatki zablokowana" },
		{ 3092,"Formatka zablokowana" },
		{ 3100,"Brak parametru autoryzacji fiskalizacji" },
		{ 3101,"Nieprawid³owy kod autoryzacji fiskalizacji" },
		{ 3102,"za krotka nazwa parametru destination" },
		{ 3103,"za krotka nazwa parametru transition" },
		{ 3104,"niedozwolone znaki w porcie docelowym" },
		{ 3105,"niedozwolone znaki w porcie tranzytowym" },
		{ 3106,"b³êdne parametry w bloku 1" },
		{ 3107,"b³êdne parametry w bloku 2" },
		{ 3110,"B³¹d napiêcia szuflady" },
		{ 3200,"Próba wydruku pustego kodu" },
		{ 3201,"Kod przekracza obszar papieru lub jest zbyt du¿y" },
		{ 3202,"Nieprawid³owa wartoœæ skali wydruku" },
		{ 3203,"Nieprawid³owa wartoœæ parametru Y2X ratio" },
		{ 3205,"Kod przekracza dopuszczalny obszar pamiêci" },
		{ 3206,"Strumieñ wejœciowy przekracza dopuszczaln¹ d³ugoœæ" },
		{ 3207,"Liczba kolumn poza zakresem" },
		{ 3208,"Liczba wierszy poza zakresem" },
		{ 3209,"Poziom korekcji b³êdów poza zakresem" },
		{ 3210,"Liczba pikseli na modu³ poza zakresem" },
		{ 3220,"Nieobs³ugiwany typ kodu kreskowego" },
		{ 3221,"B³¹d zapisu kodu kreskowego" },
		{ 3222,"B³¹d odczytu kodu kreskowego" },
		{ 3250,"numer grafiki poza zakresem" },
		{ 3251,"brak grafiki w slocie" },
		{ 3252,"grafika tylko do odczytu" },
		{ 3253,"niepoprawny rozmiar grafiki" },
		{ 3254,"Przekroczony rozmiar pamiêci przeznaczony na grafikê" },
		{ 3255,"b³¹d zapisu grafiki na kopiê elektroniczn¹" },
		{ 3256,"b³¹d zapisu grafiki" },
		{ 3257,"poziom drukowalnoœci grafik z kopii poza zakresem" },
		{ 3258,"niepoprawny rozmiar danych" },
		//Status mechanizmu (sprn) -> przesuniête o 10000
		{ 10001,"Podniesiona dŸwigania" },
		{ 10002,"Brak dostêpu do mechanizmu" },
		{ 10003,"Podniesiona pokrywa" },
		{ 10005,"Brak papieru - orygina³" },
		{ 10006,"Nieodpowiednia temperatura lub zasilanie" },
		{ 10007,"Chwilowy zanik zasilania" },
		{ 10008,"B³¹d obcinacza" },
		{ 10009,"B³¹d zasilacza" },
		{ 10010,"Podniesiona pokrywa przy obcinaniu" },
		//w³asne b³êdy parsowania odpowiedzi
		{ 20001, "Brak znacznika STX" },
		{ 20002, "Brak znacznika ETX" },
		{ 20003, "Problem komunikacyjny z urz¹dzeniem, brak odczytu" },
		{ 20004, "B³¹d zapisu do portu" },
		{ 20005, "Problem z wydrukiem paragonu\nprzekroczenie czasu obs³ugi\nanulowano" },
		{ 20006, "Wyj¹tek w module fiskalnym" }
		};

		auto it = errors.find(error_code);
		if (it != errors.end()) {
			std::string error = errors.at(error_code);
			if (!error.empty()) {
				return error;
			}
		}
		return IProtocol::GetErrorMessage(error_code);
	}
};