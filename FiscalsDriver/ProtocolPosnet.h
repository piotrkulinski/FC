#pragma once

#include "IProtocol.h"
#include "protocol_fiscal.h"
#include <vector>
#include "NonFiscalLineType.h"
#include "CStateManage.h"


/**
 * @author Piotr Kuli�ski
 * @date 2024-09-28
 * @brief Klasa obs�uguj�ca zdeserializowany [paragon] (\ref Paragon) w oparciu o protok� posnet. \n
 * Wydruk nie korzysta z bibliotek posnetu, implementuje bezpo�rednio protok� posnet.
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
		char* w = (char*)strstr(response, "\tERR"); //b��d ramki
		if (w != NULL)
		{
			w = (char*)strstr(w, "\t?");
			w += 2;
			_err = atol(w);
		}
		else
		{
			w = (char*)strstr(response, "\t?"); //b��d sekwencji lub wykonania sekwencji
			if (w != NULL)
			{
				w += 2;
				_err = atol(w);
			}
		}
		return _err;
	}

	/*
	 * Sprawdzenie mechanizmu i interakcja z u�ytkownikiem
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
			state.set(std::string{ "Urz�dzenie nie zwr�ci�o �adnych danych" });
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
		Timeout<u_long, 1000> maxTime(60_sec); //minut� czekamy na wyja�nienie sprawy, a jak nie wysy�amy instrukcj� anulacji
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

		//CallAnswer("Rozpocz�cie wydruku");
		pugi::xml_document doc;
		if (const pugi::xml_parse_result xresult = doc.load_string(xml); !xresult) {
			result.set(FiscalResult::FAILED, "B��d parsowania wydruku");
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
				//<< "al" << "TERMINAL P�ATNICZY" << posnet::TAB;

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

			result.set(FiscalResult::OK, "Wydruk zako�czony poprawnie");

			if (false) {
				u_short pr = 0;
				ps.str("");
				ps << "hdrset" << posnet::TAB
					<< "tx&c&b&N&1" << "PIOTR KULI�SKI" << "&1&N&b&c" << '\n'
					<< "&c&5" << "Protok� " << (pr == 1 ? "THERMAL" : "POSNET") << "&5&c" << '\n'
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
					<< "cp" << 0 << posnet::TAB // 0 � WIN1250 1 � LATIN2,2 � MAZOVIA
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
			std::string message = std::string("B��d wydruku") + xex.what();
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message.c_str(), EventType::evn_exception);
		}
		catch (...) {
			std::string message = "B��d wydruku nefiskalnego";
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
			result.set(FiscalResult::FAILED, "B��d wczytywania paragonu");
			return FiscalError::PREPARE;
		}
		//Connection* connection = instance->getConnection().get();
		if (paragon.fconnection != nullptr && (paragon.fconnection->type != ConnectionType::RS232 || connection->isConnectionType(ConnectionType::TCP))) {
			result.set(FiscalResult::FAILED, "Protok� obs�uguje jedynie po��czenie RS232");
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
			throw std::exception("B��d mapowania stawki VAT");
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
			std::string response_vat; //na to b�dzie string_view
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

			std::stringstream ss; ss << "Wydruk zako�czony poprawnie, paragon: " << LastFiscalNumber;

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
			std::string message = std::string("B��d wydruku paragonu\n") + xex.what();
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message, EventType::evn_exception);
		}
		catch (...) {
			std::string message = "B��d wydruku paragonu\ntransakcja zostanie anulowana";
			result.set(FiscalResult::FAILED, message);
			CallAnswer(message, EventType::evn_exception);
		}

		if (result.check(FiscalResult::FAILED) && connection->IsOpened()) {
			try {
				req.str("");
				req << "trcancel" << posnet::TAB;
				sendread(connection, req.str(), std::string{ "" });
				CallAnswer("Wys�ano anulacj� transakcji", EventType::evn_exception);
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
		//b��dy ramki
		{ 1, "Nierozpoznana komenda" },
		{ 2, "Brak obowi�zkowego pola" },
		{ 3, "B��d konwersji pola (np.: przes�ana zosta�a warto�� z przecinkiem w polu kt�rego warto�� przesy�a si� w cz�ciach setnych np.: 12,34 zamiast 1234, lub przekroczony zakres danych)" },
		{ 4, "B��dny token" },
		{ 5, "Z�a suma kontrolna" },
		{ 6, "B��d budowy ramki, brak mnemonika parametru" },
		{ 10, "Niew�a�ciwa d�ugo�� pola danych" },
		{ 11, "Zape�niony bufor odbiorczy" },
		{ 12, "Nie mo�na wykona� rozkazu w trybie natychmiastowym" },
		{ 13, "Nie znaleziono rozkazu o podanym tokenie" },
		{ 14, "Zape�niona kolejka wej�ciowa" },
		{ 15, "B��d budowy ramki, brak sumy kontrolnej" },
		//B��dy polece�
		{ 30,"b��d nietypowy - rezygnacja, przerwanie funkcji" },
		{ 50,"B��d wykonywania operacji przez kas�" },
		{ 51,"B��d wykonywania operacji przez kas�" },
		{ 52,"B��d wykonywania operacji przez kas�" },
		{ 53,"B��d wykonywania operacji przez kas�" },
		{ 54,"B��d wykonywania operacji przez kas�" },
		{ 55,"B��d wykonywania operacji przez kas�" },
		{ 56,"B��d wykonywania operacji przez kas�" },
		{ 323,"Funkcja zablokowana w konfiguracji" },
		{ 360,"Znaleziono zwor� serwisow�" },
		{ 361,"Nie znaleziono zwory" },
		{ 364,"B��dne has�o w sensie niezgodno�ci z poprawno�ci� sk�adniow�" },
		{ 365,"B��dne has�o w sensie niezgodno�ci z tym z bazy" },
		{ 382,"Pr�ba wykonania raportu zerowego" },
		{ 383,"Brak raportu dobowego" },
		{ 384,"Brak rekordu w pami�ci" },
		{ 400,"B��dna warto��" },
		{ 404,"Wprowadzono nieprawid�owy kod kontrolny" },
		{ 460,"B��d zegara w trybie fiskalnym" },
		{ 461,"B��d zegara w trybie niefiskalnym" },
		{ 480,"Drukarka ju� autoryzowana, bezterminowo" },
		{ 481,"Nie rozpocz�to jeszcze autoryzacji" },
		{ 482,"Kod ju� wprowadzony" },
		{ 483,"Pr�ba wprowadzenia b��dnych warto�ci" },
		{ 484,"Min�� czas pracy kasy, sprzeda� zablokowana" },
		{ 485,"B��dny kod autoryzacji" },
		{ 486,"Blokada autoryzacji. Wprowad� kod z klawiatury" },
		{ 487,"U�yto ju� maksymalnej liczby kod�w" },
		{ 499,"Aktywny protok� Thermal. Prosz� zmieni� protok�" },
		{ 500,"Przepe�nienie statystyki minimalnej" },
		{ 501,"Przepe�nienie statystyki maksymalnej" },
		{ 502,"Przepe�nienie stanu kasy" },
		{ 503,"Warto�� stanu kasy po wyp�acie staje si� ujemna (przyjmuje si� stan zerowy kasy)" },
		{ 700,"B��dny adres IP" },
		{ 701,"B��d numeru tonu" },
		{ 702,"B��d d�ugo�ci impulsu szuflady" },
		{ 703,"B��d stawki VAT" },
		{ 705,"B��d czasu u�pienia" },
		{ 706,"B��d czasu wy��czenia" },
		{ 713,"B��dne parametry konfiguracji" },
		{ 714,"B��dna warto�� kontrastu wy�wietlacza" },
		{ 715,"B��dna warto�� pod�wietlenia wy�wietlacza" },
		{ 716,"B��dna warto�� czasu zaniku pod�wietlenia" },
		{ 717,"Za d�uga linia nag��wka albo stopki" },
		{ 718,"B��dna konfiguracja komunikacji" },
		{ 719,"B��dna konfiguracja protoko�u kom" },
		{ 720,"B��dny identyfikator portu" },
		{ 721,"B��dny numer tekstu reklamowego" },
		{ 722,"Podany czas wychodzi poza wymagany zakres" },
		{ 723,"Podana data/czas niepoprawne" },
		{ 724,"Inna godzina w r�nicach czasowych 0<=>23" },
		{ 726,"B��dna zawarto�� tekstu w linii wy�wietlacza" },
		{ 727,"B��dna warto�� dla przewijania na wy�wietlaczu" },
		{ 728,"B��dna konfiguracja portu" },
		{ 729,"B��dna konfiguracja monitora transakcji" },
		{ 730,"Port zaj�ty przez komputer" },
		{ 731,"Port zaj�ty przez TCP/IP" },
		{ 732,"Port zaj�ty przez monitor" },
		{ 733,"Port zaj�ty" },
		{ 734,"Port zaj�ty przez tunelowanie" },
		{ 738,"Nieprawid�owa konfiguracja Ethernetu" },
		{ 739,"Nieprawid�owy typ wy�wietlacza" },
		{ 740,"Dla tego typu wy�wietlacza nie mo�na ustawi� czasu zaniku pod�wietlenia" },
		{ 741,"Warto�� czasu spoza zakresu" },
		{ 745,"B��dna numer strony kodowej" },
		{ 746,"B��dna konfiguracja ramki monitora transakcji" },
		{ 747,"DHCP aktywne. Funkcja niedost�pna" },
		{ 748,"DHCP dozwolone tylko przy transmisji ethernet" },
		{ 752,"Protok� Thermal jest niedost�pny po interfejsie TCP/IP" },
		{ 820,"Negatywny wynik testu" },
		{ 821,"Brak testowanej opcji w konfiguracji" },
		{ 857,"Brak pami�ci na inicjalizacj� bazy drukarkowej" },
		{ 1000,"B��d fatalny modu�u fiskalnego" },
		{ 1001,"Wypi�ta pami�� fiskalna" },
		{ 1002,"B��d zapisu" },
		{ 1003,"B��d nie uj�ty w specyfikacji bios" },
		{ 1004,"B��dne sumy kontrolne" },
		{ 1005,"B��d w pierwszym bloku kontrolnym" },
		{ 1006,"B��d w drugim bloku kontrolnym" },
		{ 1007,"B��dny id rekordu" },
		{ 1008,"B��d inicjalizacji adresu startowego" },
		{ 1009,"Adres startowy zainicjalizowany" },
		{ 1010,"Numer unikatowy ju� zapisany" },
		{ 1011,"Brak numeru w trybie fiskalnym" },
		{ 1012,"B��d zapisu numeru unikatowego" },
		{ 1013,"Przepe�nienie numer�w unikatowych" },
		{ 1014,"B��dny j�zyk w numerze unikatowym" },
		{ 1015,"Wi�cej ni� jeden NIP" },
		{ 1016,"Drukarka w trybie do odczytu bez rekordu fiskalizacji" },
		{ 1017,"Przekroczono liczb� zerowa� RAM" },
		{ 1018,"Przekroczono liczb� raport�w dobowych" },
		{ 1019,"B��d weryfikacji numeru unikatowego" },
		{ 1020,"B��d weryfikacji statystyk z  RD" },
		{ 1021,"B��d odczytu danych z NVR do weryfikacji FM" },
		{ 1022,"B��d zapisu danych z NVR do weryfikacji FM" },
		{ 1023,"Pami�� fiskalna jest ma�a" },
		{ 1024,"Nie zainicjalizowany obszar danych w pami�ci fiskalnej" },
		{ 1025,"B��dny format numeru unikatowego" },
		{ 1026,"Za du�o b��dnych blok�w w FM" },
		{ 1027,"B��d oznaczenia b��dnego bloku" },
		{ 1028,"Rekord w pami�ci fiskalnej nie istnieje - obszar pusty" },
		{ 1029,"Rekord w pami�ci fiskalnej z dat� p�niejsz� od poprzedniego" },
		{ 1030,"B��d odczytu skr�tu raportu dobowego" },
		{ 1031,"B��d zapisu skr�tu raportu dobowego" },
		{ 1032,"B��d odczytu informacji o weryfikacji skr�tu raportu dobowego" },
		{ 1033,"B��d zapisu informacji o weryfikacji skr�tu raportu dobowego" },
		{ 1034,"B��d odczytu etykiety no�nika" },
		{ 1035,"B��d zapisu etykiety no�nika" },
		{ 1036,"Niezgodno�� danych kopii elektronicznej" },
		{ 1037,"B��dne dane w obszarze bit�w faktur, brak ci�g�o�ci, zapl�tany gdzie� bit lub podobne" },
		{ 1038,"B��d w obszarze faktur. Obszar nie jest pusty" },
		{ 1039,"Brak miejsca na nowe faktury" },
		{ 1040,"Suma faktur z raport�w dobowych jest wi�ksza od licznika faktur" },
		{ 1041,"B��d w obszarze ID modu�u kopii" },
		{ 1042,"B��d zapisu ID modu�u kopii" },
		{ 1043,"Obszar ID modu�u kopii zape�niony" },
		{ 1044,"nieudana fiskalizacja" },
		{ 1045,"Data wcze�niejsza od daty poprzedniego raportu dobowego" },
		{ 1950,"Przekroczony zakres totalizer�w paragonu" },
		{ 1951,"Wp�ata form� p�atno�ci przekracza max. wp�at�" },
		{ 1952,"Suma form p�atno�ci przekracza max. wp�at�" },
		{ 1953,"Formy p�atno�ci pokrywaj� ju� do zap�aty" },
		{ 1954,"Wp�ata reszty przekracza max. wp�at�" },
		{ 1955,"Suma form p�atno�ci przekracza max. wp�at�" },
		{ 1956,"Przekroczony zakres total" },
		{ 1957,"Przekroczony maksymalny zakres paragonu" },
		{ 1958,"Przekroczony zakres warto�ci opakowa�" },
		{ 1959,"Przekroczony zakres warto�ci opakowa� przy stornowaniu" },
		{ 1961,"Wp�ata reszty zbyt du�a" },
		{ 1962,"Wp�ata form� p�atno�ci warto�ci 0" },
		{ 1980,"Przekroczony zakres kwoty bazowej rabatu/narzutu" },
		{ 1981,"Przekroczony zakres kwoty po rabacie / narzucie" },
		{ 1982,"B��d obliczania rabatu/narzutu" },
		{ 1983,"Warto�� bazowa ujemna lub r�wna 0" },
		{ 1984,"Warto�� rabatu/narzutu zerowa" },
		{ 1985,"Warto�� po rabacie ujemna lub r�wna 0" },
		{ 1990,"Niedozwolone stornowanie towaru. B��dny stan transakcji" },
		{ 1991,"Niedozwolony rabat/narzut. B��dny stan transakcji" },
		{ 2000,"B��d pola VAT ( b��dny numer stawki lub nieaktywna stawka)" },
		{ 2002,"Brak nag��wka" },
		{ 2003,"Zaprogramowany nag��wek" },
		{ 2004,"Brak aktywnych stawek VAT" },
		{ 2005,"Brak trybu transakcji" },
		{ 2006,"B��d pola cena ( cena <= 0 )" },
		{ 2007,"B��d pola ilo�� ( ilo�� <= 0 )" },
		{ 2008,"B��d kwoty total" },
		{ 2009,"B��d kwoty total, r�wna zero" },
		{ 2010,"Przekroczony zakres totalizer�w dobowych" },
		{ 2020,"Zmiana daty niedozwolona" },
		{ 2021,"Pr�ba ponownego ustawienia zegara" },
		{ 2022,"Zbyt du�a r�nica dat" },
		{ 2023,"R�nica wi�ksza ni� 2 godziny w trybie u�ytkownika w trybie fiskalnym" },
		{ 2024,"Z�y format daty (np. 13 miesi�c )" },
		{ 2025,"Data wcze�niejsza od ostatniego zapisu do modu�u" },
		{ 2026,"B��d zegara" },
		{ 2027,"Przekroczono maksymaln� liczb� zmian stawek VAT" },
		{ 2028,"Pr�ba zdefiniowana identycznych stawek VAT" },
		{ 2029,"B��dne warto�ci stawek VAT" },
		{ 2030,"Pr�ba zdefiniowania stawek VAT wszystkich nieaktywnych" },
		{ 2031,"B��d pola NIP" },
		{ 2032,"B��d numeru unikatowego pami�ci fiskalnej" },
		{ 2033,"Urz�dzenie w trybie fiskalnym" },
		{ 2034,"Urz�dzenie w trybie niefiskalnym" },
		{ 2035,"Niezerowe totalizery" },
		{ 2036,"Urz�dzenie w stanie tylko do odczytu" },
		{ 2037,"Urz�dzenie nie jest w stanie tylko do odczytu" },
		{ 2038,"Urz�dzenie w trybie transakcji" },
		{ 2039,"Zerowe totalizery" },
		{ 2040,"B��d oblicze� walut, przepe�nienie przy mno�eniu lub dzieleniu" },
		{ 2041,"Pr�ba zako�czenia pozytywnego paragonu z warto�ci� 0" },
		{ 2042,"B��dy format daty pocz�tkowej" },
		{ 2043,"B��dy format daty ko�cowej" },
		{ 2044,"Pr�ba wykonania raportu miesi�cznego w danym miesi�cu" },
		{ 2045,"Data pocz�tkowa p�niejsza od bie��cej daty" },
		{ 2046,"Data ko�cowa wcze�niejsza od daty fiskalizacji" },
		{ 2047,"Numer pocz�tkowy lub ko�cowy r�wny zero" },
		{ 2048,"Numer pocz�tkowy wi�kszy od numeru ko�cowego" },
		{ 2049,"Numer raportu zbyt du�y" },
		{ 2050,"Data pocz�tkowa p�niejsza od daty ko�cowej" },
		{ 2051,"Brak pami�ci w buforze tekst�w" },
		{ 2052,"Brak pami�ci w buforze transakcji" },
		{ 2054,"Formy p�atno�ci nie pokrywaj� kwoty do zap�aty lub reszty" },
		{ 2055,"B��dna linia" },
		{ 2056,"Tekst pusty" },
		{ 2057,"Przekroczony rozmiar lub przekroczona liczba znak�w formatuj�cych" },
		{ 2058,"B��dna liczba linii" },
		{ 2059,"B��dny kod kreskowy" },
		{ 2060,"B��dny stan transakcji" },
		{ 2062,"Jest wydrukowana cz�� jakiego� dokumentu" },
		{ 2063,"B��d parametru" },
		{ 2064,"Brak rozpocz�cia wydruku lub transakcji" },
		{ 2067,"B��d ustawie� konfiguracyjnych wydruk�w / drukarki" },
		{ 2070,"Data przegl�du wcze�niejsza od systemowej" },
		{ 2080,"Nieparzysta liczba danych w formacie HEX" },
		{ 2081,"Niepoprawna warto�� dla formatu HEX" },
		{ 2101,"Zape�nienie bazy" },
		{ 2102,"Stawka nieaktywna" },
		{ 2103,"Nieprawid�owa stawka VAT" },
		{ 2104,"B��d nazwy" },
		{ 2105,"B��d przypisania stawki" },
		{ 2106,"Towar zablokowany" },
		{ 2107,"Nie znaleziono w bazie drukarkowej" },
		{ 2110,"B��d autoryzacji" },
		{ 2501,"B��dny identyfikator raportu" },
		{ 2502,"B��dny identyfikator linii raportu" },
		{ 2503,"B��dny identyfikator nag��wka raportu" },
		{ 2504,"Zbyt ma�o parametr�w raportu" },
		{ 2505,"Raport nie rozpocz�ty" },
		{ 2506,"Raport rozpocz�ty" },
		{ 2507,"B��dny identyfikator komendy" },
		{ 2508,"Pr�ba wydrukowania szerokiej formatki na papierze 57mm" },
		{ 2521,"Raport ju� rozpocz�ty" },
		{ 2522,"Raport nie rozpocz�ty" },
		{ 2523,"B��dna stawka VAT" },
		{ 2532,"B��dna liczba kopii faktur" },
		{ 2533,"Pusty numer faktury" },
		{ 2534,"B��dny format wydruku" },
		{ 2600,"B��dny typ rabatu/narzutu" },
		{ 2601,"Warto�� rabatu/narzutu spoza zakresu" },
		{ 2620,"B��dna nazwa wyceny" },
		{ 2621,"Tablica wycen zape�niona" },
		{ 2622,"Tablica wycen nie pusta" },
		{ 2623,"Tablica wycen zape�niona" },
		{ 2624,"Rozpocz�ta wycena innego typu" },
		{ 2625,"Za du�o linii wyceny" },
		{ 2701,"B��d identyfikatora stawki podatkowej" },
		{ 2702,"B��dny identyfikator dodatkowej stopki" },
		{ 2703,"Przekroczona liczba dodatkowych stopek" },
		{ 2704,"Zbyt s�aby akumulator" },
		{ 2705,"B��dny identyfikator typu formy p�atno�ci" },
		{ 2706,"Brak zasilacza" },
		{ 2710,"Us�uga o podanym identyfikatorze nie jest uruchomiona" },
		{ 2801,"B��d weryfikacji warto�ci rabatu/narzutu" },
		{ 2802,"B��d weryfikacji warto�ci linii sprzeda�y" },
		{ 2803,"B��d weryfikacji warto�ci opakowania" },
		{ 2804,"B��d weryfikacji warto�ci formy p�atno�ci" },
		{ 2805,"B��d weryfikacji warto�ci fiskalnej" },
		{ 2806,"B��d weryfikacji warto�ci opakowa� dodatnich" },
		{ 2807,"B��d weryfikacji warto�ci opakowa� ujemnych" },
		{ 2808,"B��d weryfikacji warto�ci wp�aconych form p�atno�ci" },
		{ 2809,"B��d weryfikacji warto�ci reszt" },
		{ 2851,"B��d stornowania, b��dna ilo��" },
		{ 2852,"B��d stornowania, b��dna warto��" },
		{ 2900,"Stan kopii elektronicznej nie pozwala na wydrukowanie tego dokumentu" },
		{ 2901,"Brak no�nika lub operacja na no�niku trwa" },
		{ 2902,"No�nik nie jest poprawnie zweryfikowany" },
		{ 2903,"Pami�� podr�czna kopii elektronicznej zawiera zbyt du�� ilo�� danych" },
		{ 2906,"Uszkodzony bufor kopii elektronicznej" },
		{ 2907,"Brak no�nika" },
		{ 2908,"No�nik nieprawid�owy - nieodpowiedni dla wybranej operacji" },
		{ 2911,"Brak pliku na no�niku" },
		{ 2913,"Nieprawid�owy wynik testu" },
		{ 2915,"Pusta pami�� podr�czna" },
		{ 2916,"Trwa weryfikacja no�nika" },
		{ 2917,"B��dny typ dokumentu" },
		{ 2918,"Dane niedost�pne (nieaktualne)" },
		{ 3051,"Nie mo�na zmieni� dwa razy waluty ewidencyjnej po RD" },
		{ 3052,"Pr�ba ustawienia ju� ustawionej waluty" },
		{ 3053,"B��dna nazwa waluty" },
		{ 3054,"Automatyczna zmiana waluty" },
		{ 3055,"B��dna warto�� przelicznika kursu" },
		{ 3056,"Przekroczono maksymaln� liczb� zmian walut" },
		{ 3080,"Pr�ba zdefiniowania stawek VAT ze star� dat�" },
		{ 3084,"Automatyczna zmiana stawek VAT" },
		{ 3085,"Brak pola daty" },
		{ 3090,"Nieprawid�owy kod autoryzacji formatki" },
		{ 3091,"Autoryzacja formatki zablokowana" },
		{ 3092,"Formatka zablokowana" },
		{ 3100,"Brak parametru autoryzacji fiskalizacji" },
		{ 3101,"Nieprawid�owy kod autoryzacji fiskalizacji" },
		{ 3102,"za krotka nazwa parametru destination" },
		{ 3103,"za krotka nazwa parametru transition" },
		{ 3104,"niedozwolone znaki w porcie docelowym" },
		{ 3105,"niedozwolone znaki w porcie tranzytowym" },
		{ 3106,"b��dne parametry w bloku 1" },
		{ 3107,"b��dne parametry w bloku 2" },
		{ 3110,"B��d napi�cia szuflady" },
		{ 3200,"Pr�ba wydruku pustego kodu" },
		{ 3201,"Kod przekracza obszar papieru lub jest zbyt du�y" },
		{ 3202,"Nieprawid�owa warto�� skali wydruku" },
		{ 3203,"Nieprawid�owa warto�� parametru Y2X ratio" },
		{ 3205,"Kod przekracza dopuszczalny obszar pami�ci" },
		{ 3206,"Strumie� wej�ciowy przekracza dopuszczaln� d�ugo��" },
		{ 3207,"Liczba kolumn poza zakresem" },
		{ 3208,"Liczba wierszy poza zakresem" },
		{ 3209,"Poziom korekcji b��d�w poza zakresem" },
		{ 3210,"Liczba pikseli na modu� poza zakresem" },
		{ 3220,"Nieobs�ugiwany typ kodu kreskowego" },
		{ 3221,"B��d zapisu kodu kreskowego" },
		{ 3222,"B��d odczytu kodu kreskowego" },
		{ 3250,"numer grafiki poza zakresem" },
		{ 3251,"brak grafiki w slocie" },
		{ 3252,"grafika tylko do odczytu" },
		{ 3253,"niepoprawny rozmiar grafiki" },
		{ 3254,"Przekroczony rozmiar pami�ci przeznaczony na grafik�" },
		{ 3255,"b��d zapisu grafiki na kopi� elektroniczn�" },
		{ 3256,"b��d zapisu grafiki" },
		{ 3257,"poziom drukowalno�ci grafik z kopii poza zakresem" },
		{ 3258,"niepoprawny rozmiar danych" },
		//Status mechanizmu (sprn) -> przesuni�te o 10000
		{ 10001,"Podniesiona d�wigania" },
		{ 10002,"Brak dost�pu do mechanizmu" },
		{ 10003,"Podniesiona pokrywa" },
		{ 10005,"Brak papieru - orygina�" },
		{ 10006,"Nieodpowiednia temperatura lub zasilanie" },
		{ 10007,"Chwilowy zanik zasilania" },
		{ 10008,"B��d obcinacza" },
		{ 10009,"B��d zasilacza" },
		{ 10010,"Podniesiona pokrywa przy obcinaniu" },
		//w�asne b��dy parsowania odpowiedzi
		{ 20001, "Brak znacznika STX" },
		{ 20002, "Brak znacznika ETX" },
		{ 20003, "Problem komunikacyjny z urz�dzeniem, brak odczytu" },
		{ 20004, "B��d zapisu do portu" },
		{ 20005, "Problem z wydrukiem paragonu\nprzekroczenie czasu obs�ugi\nanulowano" },
		{ 20006, "Wyj�tek w module fiskalnym" }
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