#pragma once
#include "DescriptionType.h"
#include <string>
#include "../pugixml.hpp"
/**
 * @author Piotr Kuliñski
 * @brief Opisy na paragonie
*/
struct Description
{
public:
	//PaymentType typ = PaymentType::Brak;
	bool isPrintHeader = false; //czy wydrukowano w nag³ówku
	DescriptionType typ = DescriptionType::OPIS_UNKNOW;
	std::string nazwa{ "" };

	std::string name() {
		return nazwa;
	}

	template <class TConvert>
	std::string name() {
		TConvert cnv;//{ nazwa };
		return cnv(nazwa);
	}

	template <class TConvertPredicat>
	std::string name(TConvertPredicat cnv) {
		return cnv(nazwa);
	}

	Description(const pugi::xml_node& desc) {
		std::string v = desc.attribute("typ").value();
		if (!v.empty()) {
			typ = (DescriptionType)atol(v.c_str());
			v = desc.attribute("wartosc").value();
			if (!v.empty()) {
				nazwa = v;
			}
		}
	}
};
