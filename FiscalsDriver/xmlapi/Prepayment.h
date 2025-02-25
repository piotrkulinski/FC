#pragma once

#include "DiscountTypeSummary.h"
#include "DiscountDescription.h"
#include <string>
#include "../MyUtils.h"
#include "../pugixml.hpp"
#include "../protocol_fiscal.h"

/**
 * @author Piotr Kuliñski
 * @date 2022-09-20
 * @brief Zaliczki, przedp³aty
 */
struct Prepayment
{
private:
	long value{ 0 };

public:
	DiscountTypeSummary typRabatu = DiscountTypeSummary::P_BrakRabatu;
	DiscountDescription opisRabatu = DiscountDescription::BrakOpisu;
	std::string napisRabatu{ "" };
	std::string nazwa{ "" };
	std::string vat{ "" };

	std::string name() {
		return napisRabatu;
	}

	template <typename R, class predicat>
	R VatID(predicat fun) {
		return fun(vat);
	}

	template <class ConversionPredicat>
	std::string name(ConversionPredicat cnv) {
		return cnv(napisRabatu);
	}

	Prepayment(const pugi::xml_node& line) {
		vat = line.attribute("ptu").value();
		std::string typ = line.attribute("typ").value();
		double w = std::stof(line.attribute("kwota").value());
		Grosze g;
		value = abs(g(w)); //fiscal::round(line.attribute("kwota").value());
		pugi::xml_attribute sys_opis = line.attribute("sys_opis");
		pugi::xml_attribute usr_opis = line.attribute("usr_opis");
		if (!usr_opis.empty()) {
			napisRabatu = usr_opis.value();
		}
		if (!sys_opis.empty()) {
			opisRabatu = (DiscountDescription)atol(sys_opis.value());
		}
		typRabatu = (DiscountTypeSummary)atol(typ.c_str());
	}

	long wartosc() {
		return value;
	}

	template <class ConversionPredicat>
	const auto wartosc() {
		ConversionPredicat convert;
		return convert(wartosc());
	}
};
