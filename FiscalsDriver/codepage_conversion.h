#pragma once
#include <sstream>
#include <string>

/**
 * @brief Zak³adamy ¿e podstawowym kodowaniem jest windows-1250
*/
class codepage_base {
protected:
	std::string code{
		"¥ÆÊ£ÑÓŒ¯" \
		"¹æê³ñóœŸ¿"
	};
	std::string translate{
		"¥ÆÊ£ÑÓŒ¯"\
		"¹æê³ñóœŸ¿"
	};
public:
	codepage_base() {}
	virtual ~codepage_base() {
		std::cout << "Destructor " << __FUNCTION__ << std::endl;
	}
	virtual std::string operator()(const std::string& cnv) {
		return encode(cnv);
	}

	virtual std::string encode(const std::string& cnv) {
		std::ostringstream newn;
		for (const auto& z : cnv) {
			const size_t p = code.find(z);
			newn << (p != std::string::npos ? translate.at(p) : z);
		}
		return newn.str();
	}
};

class codepage_mazovia : public codepage_base {
public:
	codepage_mazovia() {
		translate = {
			"\x8f\x95\x90\x9c\xa5\xa3\x98\xa0\xa1"\
			"\x86\x8d\x91\x92\xa4\xa2\x9e\xa6\xa7"
		};
	};
};

class codepage_cp852 : public codepage_base {
public:
	codepage_cp852() {
		code += "„”––";
		translate = {
			"\xa4\x8f\xa8\x9d\xe3\xe0\x97\x8d\xbd"\
			"\xa5\x86\xa9\x88\xe4\xa2\x98\xab\xbe\"\"-_"
		};
	};
};

//ISO 8859-2 (LatinII)		
class codepage_iso8859_2 : public codepage_base {
public:
	codepage_iso8859_2() {
		code += "„”––";
		translate = {
			"\xA1\xC6\xCA\xA3\xD1\xD3\xA6\xAC\xAF"\
			"\xB1\xE6\xEA\xB3\xF1\xF3\xB6\xBC\xBF\"\"-_"
		};
	};
};

class codepage_windows_1250 : public codepage_base {
public:
	codepage_windows_1250() {
		code += "„”––";
		translate = {
			"\xA5\xC6\xCA\xA3\xD1\xD3\x8C\x8F\xAF"\
			"\xB9\xE6\xEA\xB3\xF1\xF3\x9C\x9F\xBF\"\"-_"
		};
	};
};

class codepage_bezpolsk : public codepage_base {
public:
	codepage_bezpolsk() {
		translate = {
			"ACELNOSZZ"\
			"acelnoszz"
		};
	};
};

class codepage_noconvert : public codepage_base {
public:
	std::string encode(const std::string& cnv) override {
		return cnv;
	}
};
