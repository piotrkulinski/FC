#pragma once
#include "..\stdafx.h"
#include <string>
#include <vector>
#include <map>
#include "..\pugixml.hpp"
#include "..\codepage_conversion.h"
#include "..\protocol_fiscal.h"
#include "..\logger_definition.h"
#include "..\payment_conversion.h"
#include "..\connection\ConnectionType.h"
#include "PaymentType.h"
#include "DiscountType.h"
#include "DiscountTypeSummary.h"
#include "DiscountDescription.h"
#include "DescriptionType.h"
#include "Prepayment.h"
#include "Description.h"
#include "Payment.h"
#include "ReceiptLineType.h"
#include "ReceiptLine.h"
#include "ReceiptTotal.h"
#include "FiscalXmlApi.h"
#include "FiscalControl.h"
#include "connection\ConfigurationPort.h"
#include "connection\ConfigurationRS232.h"
#include "connection\ConfigurationTCP.h"

#define GET_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); })
#define GET_VALUE_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); }).first_child().value()

/**
* @author Piotr Kuliñski
* @brief Klasa reprezentuj¹ca pe³ny paragon po deserializacji<br>
* Deserializacja w oparciu o XML
*/
class Paragon : public FiscalXmlApi
{
private:
	std::vector<std::string_view> svv;

public:
	std::unique_ptr<ConfigurationPort> fconnection;
	//ConfigurationPort* fconnection;
	std::vector<ReceiptLine> receiptLines{};
	std::vector<Prepayment> prePayments{};
	std::vector<Payment> payments{};
	std::vector<Description> descriptions{};
	ReceiptTotal summary{};
	FiscalControl control;

	template <class TP>
	size_t PeymentID(size_t id, TP convertp) {
		return convertp(id);
	}

	template <class T, typename T2>
	auto VatID(T2 t2) {
		return T(t2);
	}

	Paragon(pugi::xml_document& xml) {
		load(xml);
	}

	//Paragon(CString creceipt_xml) : FiscalXmlApi(creceipt_xml) {
	//	if (!(exit_code == 0)) {
	//		return;
	//	}
	//	load(doc);
	//}
	
	Paragon(XmlApiFiscalRequest creceipt_xml) : FiscalXmlApi(creceipt_xml) {
		if (!(exit_code == 0)) {
			return;
		}
		load(doc);
	}

	void load(pugi::xml_document& doc) {

		pugi::xml_node receipt = doc.child("Paragon");

		if (pugi::xml_node lines = GET_NODE(receipt, "Linijki"); !lines.empty()) {
			#if defined(LOGGER)
			std::cout
				<< std::right
				<< std::setw(40) << "Nazwa towaru"
				<< std::setw(11) << "n.przed"
				<< std::setw(17) << "n.po rabacie"
				<< std::setw(10) << "iloœæ"
				<< std::setw(10) << "cena"
				<< std::setw(14) << "w.linijki"
				<< std::setw(17) << "w.zrabatowana"
				<< std::setw(10) << "rabat"
				<< std::endl;
			#endif
			for (pugi::xml_node line = lines.first_child(); line; line = line.next_sibling()) {
				ReceiptLine linijka(line);
				receiptLines.push_back(linijka);
				summary += linijka;
			}
		}
		else {
			exit_code = -2;
			return;
		}

		int nPort = -1;
		if (pugi::xml_node conn = GET_NODE(receipt, "Connection"); !conn.empty()) {
			pugi::xml_attribute port_type = conn.attribute("type");

			if (port_type.empty()) { //brak atrybutu, zak³adam, ¿e to stary format
				int nType = -1;
				pugi::xml_node xtype = GET_NODE(conn, "Type");
				if (!xtype.empty()) {
					nType = atoi(xtype.value());
				}

				std::stringstream ss;
					ss << "<Connection type=\"" << nType << "\" ";
					if (nType == (int)ConnectionType::RS232) {
						ss
							<< "name=\"" << GET_VALUE_NODE(conn, "Com") << "\" "
							<< "baudrate=\"" << GET_VALUE_NODE(conn, "BaudRate") << "\" "
							<< "databits=\"8\" "
							<< "parity=\"N\" "
							<< "flowcontrol=\"N\"";
					}
					else {
						ss 
						<< "name=\"" << GET_VALUE_NODE(conn, "TcpIp") << "\" "
						<< "ip=\"" << GET_VALUE_NODE(conn, "TcpIp") << "\" "
						<< "port=\"" << GET_VALUE_NODE(conn, "Port") << "\" ";
					}
				ss << "/>";

				pugi::xml_document cdoc;
				pugi::xml_parse_result result = cdoc.load_string(ss.str().c_str());
				pugi::xml_node xconnection = cdoc.first_child();
				fconnection = std::make_unique<ConfigurationRS232>(xconnection);
			}
			else {
				nPort = conn.attribute("type").as_int(nPort);
			}

			if (fconnection == nullptr) {
				ConnectionType type = static_cast<ConnectionType>(nPort);
				if (type == ConnectionType::RS232) {
					fconnection = std::make_unique<ConfigurationRS232>(conn);
				}
				else if (type == ConnectionType::TCP) {
					fconnection = std::make_unique<ConfigurationTCP>(conn);
				}
				else {
					fconnection = std::make_unique<ConfigurationPort>(conn);
				}
			}
		}

		if (pugi::xml_node header = GET_NODE(receipt, "Naglowek"); !header.empty()) {
			if (pugi::xml_node xml_opisy = GET_NODE(header, "Opisy"); !xml_opisy.empty()) {
				for (pugi::xml_node xml_opis = xml_opisy.first_child(); xml_opis; xml_opis = xml_opis.next_sibling()) {
					Description Opis(xml_opis);
					descriptions.push_back(Opis);
				}
			}
		}

		if (pugi::xml_node header = GET_NODE(receipt, "Sterowanie"); !header.empty()) {
			control = { header };
		}

		if (pugi::xml_node xml_prepayments = GET_NODE(receipt, "Zaliczki"); !xml_prepayments.empty()) {
			for (pugi::xml_node zaliczka = xml_prepayments.first_child(); zaliczka; zaliczka = zaliczka.next_sibling()) {
				Prepayment Zaliczka(zaliczka);
				prePayments.push_back(Zaliczka);
				summary += Zaliczka;
			}
		}

		if (pugi::xml_node xml_payments = GET_NODE(receipt, "Platnosci"); !xml_payments.empty()) {
			for (pugi::xml_node xml_payment = xml_payments.first_child(); xml_payment; xml_payment = xml_payment.next_sibling()) {
				Payment Platnosc(xml_payment);
				payments.push_back(Platnosc);
				summary += Platnosc;
			}
		}

		/**
		* @brief <b>Zabezpieczenie p³atnoœci paragonu</b> \n
		* Deserializacja paragonu zabezpiecza brakuj¹ce p³atnoœci i ewentualny brak ich pokrycia.\n
		* Potrafi dogenerowaæ brakuj¹c¹ p³atnoœæ lub jej czêœæ, jako Dop³ata, ew. Kaucja w przypadku rozliczeñ opakowaniami.
		*/
		if (long brakujace_platnosci = summary.wyrownaj(); brakujace_platnosci < 0) {
			Payment wyrownanie{
				std::to_string(payment_base::Gotowka),
				(summary.KaucjeWartosc() != 0 ? "Kaucja" : "Dop³ata"),
				abs(brakujace_platnosci)
			};
			payments.push_back(wyrownanie);
			summary += wyrownanie;
		}

		encoding_text = std::make_unique<codepage_noconvert>();
	}
};

