#pragma once
#include "ReceiptLineType.h"
#include "DiscountType.h"
#include "DiscountDescription.h"
#include <string>
#include "../pugixml.hpp"
#include "../protocol_fiscal.h"

#define GET_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); })
#define GET_VALUE_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); }).first_child().value()

/**
 * @author Piotr Kuliñski
 * @brief Linijka sprzeda¿owe
*/
struct ReceiptLine
{
public:
	/**
	 * @brief Typ linii domyœlnie \ref LineType::Sale
	*/
	ReceiptLineType typ{ ReceiptLineType::Sale };
	DiscountType typRabatu{ DiscountType::BrakRabatu };
	DiscountDescription opisRabatu{ DiscountDescription::BrakOpisu };
	std::string napisRabatu{ "" };
	std::string nazwa{ "" };
	std::string opis{ "" };
	std::string vat{ "" };
	std::string jm{ "" };
	double rabat = 0.00f;
	double ilosc = 0.00f;
	u_short decimal_point = 2;

	struct {
		long cena{ 0 };
		long wartosc_przed_rabatem{ 0 };
		long rabat{ 0 };
	} grosze;

	struct {
		double po_rabacie = 0.00f;
		/** Cena brutto przed rabatem */
		double przed_rabatem = 0.00f;
	} cena;

	ReceiptLine(const pugi::xml_node& line) {

		nazwa = line.attribute("nazwa").value();
		opis = line.attribute("opis").as_string();
		jm = line.attribute("jm").as_string("szt");
		std::string _Ilosc = line.attribute("ilosc").value();
		vat = line.attribute("ptu").value();
		std::string _Cena = line.attribute("cena").value();
		pugi::xml_node RabatNarzut = GET_NODE(line, "RabatNarzut");
		pugi::xml_attribute xml_type = line.attribute("typ");
		if (!xml_type.empty()) {
			typ = static_cast<ReceiptLineType>(atoi(xml_type.value()));
		}
		ilosc = fiscal::round(_Ilosc, 6);
		cena.przed_rabatem = fiscal::round(_Cena);
		cena.po_rabacie = cena.przed_rabatem;

		if (!RabatNarzut.empty()) {
			/*
				"<RabatNarzut type=\"K\" value=\"-3.00\" sys_opis=\"16\" usr_opis=\"Zaliczka nocleg\"/>" +
				"<PTU>23</PTU>"+
			*/
			std::string typ = RabatNarzut.attribute("type").value();
			rabat = fiscal::round(RabatNarzut.attribute("value").value());

			if (typ.compare("K") == 0) { //kwotowy
				typRabatu = (rabat <= 0.00f ? DiscountType::RabatKwotowy : DiscountType::NarzutKwotowy);
			}
			else { //(P) procentowy
				typRabatu = (rabat <= 0.00f ? DiscountType::RabatProcentowy : DiscountType::NarzutProcentowy);
			}
			opisRabatu = (DiscountDescription)atol(RabatNarzut.attribute("sys_opis").value());
			pugi::xml_attribute user_opis = RabatNarzut.attribute("usr_opis");
			if (!user_opis.empty()) {
				napisRabatu = user_opis.value();
			}
			if (typRabatu == DiscountType::RabatKwotowy || typRabatu == DiscountType::NarzutKwotowy) {
				cena.po_rabacie = (((ilosc * cena.przed_rabatem * 100.0f + 0.005f) / 100.0f) + rabat) / ilosc;
			}
			else if (typRabatu == DiscountType::RabatProcentowy || typRabatu == DiscountType::RabatProcentowy) {
				cena.po_rabacie = cena.przed_rabatem + (cena.przed_rabatem * (rabat / 100.00f));
			}
		}
		DoubleToLong conv;
	    long cena_long = conv(cena.przed_rabatem);
		double wartosc_pozycji = (ilosc * cena_long);
		grosze.wartosc_przed_rabatem = conv(wartosc_pozycji,0);
		grosze.rabat = conv(rabat);
		grosze.cena = cena_long;
	}

	#pragma region Iloœæ
	/**
	 * @brief Pobranie iloœci bez zer koñcowych
	 * @return string
	*/
	const auto str_ilosc() {
		std::string v = fiscal::to_string(ilosc, 6, ",.");
		return fiscal::obetnij_zera(v);
	}
	template <class Pred>
	const auto Ilosc() {
		Pred conv;
		return conv(ilosc);
	}	

	#pragma endregion

	const bool czyRabatNarzut() {
		return (cena.po_rabacie != cena.przed_rabatem);
	}
	const bool czyRabat() {
		return (cena.po_rabacie < cena.przed_rabatem);
	}
	const bool czyNarzut() {
		return (cena.po_rabacie > cena.przed_rabatem);
	}

	#pragma region Wartoœæ [Metody dostêpowe i konwertuj¹ce]
	/** Wartoœæ pozycji przed rabatem (iloœæ * cena_brutto) dok³adnoœæ 0,01 */
	const double wartosc() {
		return (grosze.wartosc_przed_rabatem / 100.0f);
	}

	template <class Pred> const auto wartosc() {
		Pred conv;
		return conv(wartosc());
	}
	#pragma endregion

	/** Wartoœæ pozycji po zrabatowaniu (iloœæ * cena_brutto - rabat) dok³adnoœæ 0,01 */
	const double wartosc_zrabatowana() {
		return (wartosc() + rabat);
	}

	/** Wartoœæ pozycji po zrabatowaniu (iloœæ * cena_brutto - rabat) dok³adnoœæ 0,01 */
	template <class Pred> const auto wartosc_zrabatowana() {
		Pred conv;
		return conv(wartosc_zrabatowana());
	}

	#pragma region CenaSprzedazy [Metody dostêpowe i konwertuj¹ce]
	/** Cena pozycji przed uwzglêdnieniem rabatu, cena brutto */
	const double cena_sprzedazy() {
		return (cena.przed_rabatem);
	}
	/** Cena pozycji przed uwzglêdnieniem rabatu, cena brutto */
	template <class Pred>
	const auto cena_sprzedazy() {
		Pred conv;
		return conv(cena.przed_rabatem);
	}
	#pragma endregion

	const double wartosc_rabat_narzut() {
		double rn = (abs(wartosc() - wartosc_zrabatowana()));
		return (rn);
	}
	template <class Pred>
	const double wartosc_rabat_narzut() {
		Pred conv;
		return conv(wartosc_rabat_narzut());
	}

	/**
	 * @brief Wartoœæ rabatu, mo¿e byæ to kwota albo procent
	 * @return watroœæ zawsze dodatnia, czy rabat/narzu rozró¿niane parametrem \ref typRabatu
	*/
	const double wartosc_rabatu() {
		double rn = abs(rabat);
		return (rn);
	}

	std::string name() {
		return nazwa;
	}

	template <class TConvert>
	std::string name() {
		TConvert cnv;
		return cnv(nazwa);
	}

	template <class TConvertPredicat>
	std::string name(TConvertPredicat cnv) {
		return cnv(nazwa);
	}

	template <typename R, class predicat>
	R VatID(predicat fun) {
		return fun(vat);
	}
};