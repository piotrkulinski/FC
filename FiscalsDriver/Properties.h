#pragma once
#include "FactoryResultConverter.h"
//#include "ConverterType.h"

#define PUT_ITEM(ss,value) ss << "<Property name=\"" #value "\" value=\"" << ##value << "\"/>";

/**
 * @brief W�a�ciwo�ci, mo�liwo�ci i ograniczenia wybranego sterownika
*/
struct Properties
{
	/**
	* Maksymalna d�ugo�� linii transakcyjnej, \n
	* po pod��czeniu urz�dzenia i otwarciu po��czenia, mo�e si� to zmieni�
	*/
	unsigned short trline_max_length = 38;

	/** 
	* Mo�liwo�� wydruku niefiskalnego ( \subpage NONFISCAL_XML )
	* \n np. metoda \ref WydrukXMLThermal
	*/
	unsigned short non_fiscal_xml = 0;

	/**
	 * Mo�liwo�� wydruku paragonu xml ( \subpage PARAGON_XML ).
	 * \n np. metoda \ref ParagonXML
	 */
	unsigned short receipt_xml = 0;

	/**
	 * Mo�liwo�� wydruku paragonu.
	 */
	unsigned short receipt_standard = 1;

	/**
	 * Raport dobowy / dzienny.
	 */
	unsigned short report_daily = 1;

	/**
	 * Raport okresowy.
	 */
	unsigned short report_periodic = 1;

	/**
	 * Raport zmiany.
	 */
	unsigned short report_shift = 1;

	/**
	 * Strona kodowa wydruku.
	 */
	std::string encoding{ "Mazovia" };

	std::string Serialize(ConverterType type) {
		std::map<std::string, std::string> tmpm{
			 {"trline_max_length",std::to_string(trline_max_length)}
			,{"non_fiscal_xml",std::to_string(non_fiscal_xml)}
			,{"receipt_xml",std::to_string(receipt_xml)}
			,{"receipt_standard",std::to_string(receipt_standard)}
			,{"report_daily",std::to_string(report_daily)}
			,{"report_periodic",std::to_string(report_periodic)}
			,{"report_shift",std::to_string(report_shift)}
			,{"encoding",encoding}
		};
		if (type == ConverterType::XML) {
			return toXml(tmpm);
		}
		else {
			auto conv = GetConverter::get(type);
			return conv->convert(tmpm);
		}
	}

private:
	std::string toXml(std::map<std::string, std::string>& stru) {
		std::stringstream sbuf;
		sbuf << "<Properties>";
		for (const auto& it : stru) {
			sbuf << "<Property name=\"" << it.first << "\" value=\"" << it.second << "\"/>";
		}
		sbuf << "</Properties>";
		return sbuf.str();
	}

};
