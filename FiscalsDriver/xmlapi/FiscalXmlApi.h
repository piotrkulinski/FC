#pragma once
#include <atlstr.h>
#include "../codepage_conversion.h"
#include "../pugixml.hpp"
#include <iostream>

class FiscalXmlApi {
protected:
	int exit_code = 0L;
	std::unique_ptr<codepage_base> encoding_text;
	pugi::xml_document doc;

public:
	FiscalXmlApi() {};
	FiscalXmlApi(CString creceipt_xml) noexcept {
		CStringA s2(creceipt_xml);
		#ifdef LOGGER
		std::clog << s2.GetBuffer() << std::endl;
		#endif
		pugi::xml_parse_result result = doc.load_string(s2);
		//pugi::xml_parse_result result = doc.load_string(creceipt_xml, pugi::xml_encoding::encoding_utf8);
		if (!result) {
			exit_code = -1;
		}
	}	
	
	FiscalXmlApi(const char* creceipt_xml) noexcept {
		#ifdef LOGGER
		std::clog << creceipt_xml << std::endl;
		#endif
		pugi::xml_parse_result result = doc.load_string(creceipt_xml);
		//pugi::xml_parse_result result = doc.load_string(creceipt_xml, pugi::xml_encoding::encoding_utf8);
		if (!result) {
			exit_code = -1;
		}
	}

	pugi::xml_document& getXml() {
		return doc;
	}

	std::string encoding(const std::string& text) {
		return encoding_text->encode(text);
	}

	/**
	 * @brief indywidualna konwersja wg wskazanej strony kodowej
	 * @tparam T klasa strony kodowej \ref codepage_base
	*/
	template <class T>
	std::string encoding(const std::string& source, T newc) {
		return newc(source);
	}

	/**
	 * @brief Metoda szablonowa ustawiaj¹ca wybrany standard kodowania <br>
	 * Na ustawionym standardzie kodowania bazuje metoda [encoding] (\ref Paragon::encoding(const string& text))
	 * @param T nowy standard kodowania oparty o bazow¹ \ref codepage_base
	*/
	template <class T>
	void setEncoding() {
		//encoding_text.release();
		encoding_text.reset(nullptr);
		encoding_text = std::make_unique<T>();
	}

	long CheckCode() {
		return exit_code;
	}

};
