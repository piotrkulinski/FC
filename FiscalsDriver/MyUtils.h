#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <string>
#include <vector>
#include <bitset>
#include <map>

#include <sstream>
#include <iomanip>
#include <iostream>

#include "crc_calculate.h"
#include "protocol_fiscal.h"
#include "connection\cport.h"

#include <comutil.h>

bool isBitSet(int liczba, int nrBitu);
boolean VariantBoolToBool(VARIANT_BOOL varFlag);

std::string getParity(cport::Parity parity);
cport::Parity getParity(std::string parity);

std::string getFlowControl(cport::FlowControl flowControl);
cport::FlowControl getFlowControl(std::string flowControl);

std::string getStopBits(cport::StopBits flowControl);
cport::StopBits getStopBits(std::string flowControl);

std::string utf8_encode(const std::wstring& wstr);
std::string utf8_encode(const std::wstring& wstr, unsigned long long codepage);
std::wstring decode_to_utf16(const char* str, unsigned long long codepage);

void replaceAll(std::string& subject, const std::string& search, const std::string& replace);
std::string StripXml(const std::string& buffer);

template <class T>
std::vector<T> splitSV(T strv, char delims)
{
	std::vector<T> output;
	size_t first = 0;
	while (first < strv.size())
	{
		const auto second = strv.find_first_of(delims, first);

		if (first != second)
		{
			output.emplace_back(strv.substr(first, second - first));
		}
		else {
			output.emplace_back("");
		}

		if (second == T::npos)
			break;

		first = second + 1;
	}
	return output;
}


/**
 * @author Piotr Kuliñski
 * @date 2022-09-17
 * @brief Konwersja double na long, z przesuniêciem kropki dziesiêtnej o dwa miejsca w prawo.
 * Domyœlnie przekazana wartoœæ ma znacz¹ce dwa miejsca po kropce dziesiêtnej. \n
 * Wartoœæ zostanie zaokr¹glona zgodnie z parametrem dec_point.
*/
//template <const u_short DC=2>
struct DoubleToLong {
	long operator()(const double& to_convert, const u_short& dec_point = 2) {
		const short sign = to_convert >= 0 ? 1 : -1;
		double c;
		const double r = modf(to_convert * pow(10, dec_point), &c);
		long long_value = (long)c + (abs(r) >= 0.5f ? sign : 0);
		return long_value;
	}
};

/**
 * @author Piotr Kuliñski
 * @date 2022-09-17
 * @brief Konwersja float na long, z przesuniêciem kropki dziesiêtnej o dwa miejsca w prawo
*/
struct Grosze {
	long operator()(const double& to_convert) {
		return (long)floor(to_convert * 100.0f + (to_convert >= 0 ? 0.5f : -0.5f));
	}
};

/**
 * @author Piotr Kuliñski
 * @date 2022-09-17
 * @brief Konwersja float na long, z przesuniêciem kropki dziesiêtnej o dwa miejsca w prawo
*/
struct GroszeToDecimal {
	double operator()(const long& to_convert) {
		return (double)(to_convert / 100.0f);
	}
};

/**
 * @author Piotr Kuliñski
 * @date 2022-10-06
 * @brief Konwersja groszy (long) na double i na string: 599 -> 5.99
*/
struct GroszeToDecimalString {
	std::string operator()(const long& to_convert) {
		double wartosc = fiscal::round((double)(to_convert / 100.0f));
		return fiscal::to_string(wartosc);
	}
};

/**
 * @author Piotr Kuliñski
 * @date 2022-09-17
 * @brief Konwersja na tekst z ewentualn¹ zamian¹ przecinaka dziesiênego na kropkê.
*/
struct DecimalString {
	std::string operator()(const double& to_convert) {
		double r = fiscal::round(to_convert);
		return fiscal::to_string(r);
	}
};

/**
 * @author Piotr Kuliñski
 * @date 2022-09-26
 * @brief Konwersja na tekst / wartoœæ bezwzglêdna / z ewentualn¹ zamian¹ przecinaka dziesiênego na kropkê.
*/
struct DecimalStringAbs {
	std::string operator()(const double& to_convert) {
		double r = fiscal::round(abs(to_convert));
		return fiscal::to_string(r);
	}
};

/**
 * @author Piotr Kuliñski
 * @date 2022-09-17
 * @brief Konwersja na tekst z ewentualn¹ zamian¹ przecinka dziesiêtnego na kropkê
 * z jednoczesnym obciêciem zer koñcowych (pack)
*/
struct DecimalStringTrim {
	std::string operator()(const double& to_convert) {
		std::string v = fiscal::to_string(to_convert, 6, ",.");
		return fiscal::obetnij_zera(v);
	}
};
