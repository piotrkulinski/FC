#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <bitset>
#include "crc_calculate.h"

namespace stx {
	std::string GetErrorMessage(long error_code);

	class fiscal_stream : public std::ostringstream {
	public:

		/*
		FS - File Separator\n
		separator miêdzy blokami <header> i <data>
		*/
		static constexpr char FS{ 0x1C };

		/*
		GS - Group Separator\n
		separator wewn¹trz bloków <header> i <data>, dzieli bloki na  pola
		*/
		static constexpr char GS{ 0x1D };

		/*
		RS - Record Separator\n
		separator wewn¹trz pól danych, dzieli pola na subpola
		*/
		static constexpr char RS{ 0x1E };

		/*
		US - Unit Separator\n
		separator wewn¹trz subpól, dzieli subpola na elementy sk³adowe
		*/
		static constexpr char US{ 0x1F };

		/*
		DP - Decimal Point\n
		separator oddzielaj¹cy czêœæ ca³kowit¹ od czêœci u³amkowej liczby dziesiêtnej, jest to znak przecinka ','
		*/
		static constexpr char DP{ 0x2C };

		static constexpr char STX{ 0x02 };
		static constexpr char ETX{ 0x03 };
	public:
		void str(std::string b) {
			std::ostringstream::str(b);
		}
		std::string str() {
			std::string cmd = std::ostringstream::str();
			if (cmd.empty()) {
				return "";
			}
			std::ostringstream& p = *this;
			std::stringstream request;
			request
				<< STX
				<< cmd
				<< std::uppercase << std::hex << std::setfill('0') << std::setw(4) << crc(cmd)
				<< ETX;

			return request.str();
		}
	};
}
namespace posnet {
	constexpr char TAB{ 0x09 };
	constexpr char SEP{ 35 };
	constexpr char STX{ 0x02 };
	constexpr char ETX{ 0x03 };
	constexpr char LF{ 0x0A };

	std::string GetErrorMessage(long error_code);

	class fiscal_stream : public std::ostringstream {

	public:
		void str(std::string b) {
			std::ostringstream::str(b);
		}
		std::string str() {
			std::string cmd = std::ostringstream::str();
			if (cmd.empty()) {
				return "";
			}
			std::ostringstream& p = *this;
			std::stringstream request;
			request
				<< posnet::STX
				<< cmd
				<< '#'
				<< std::uppercase << std::hex << std::setfill('0') << std::setw(2) << posnet::crc(cmd)
				<< posnet::ETX;

			return request.str();
		}
	};
}
namespace thermal {
	constexpr char TAB{ 0x09 };
	constexpr char ESC{ 0x1B };
	constexpr char CR{ '\r' };     //Karetka powrotu CR 
	constexpr char FLD_SEP_1{ '/' }; //backslash /
	constexpr char FLD_SEP_2{ ';' }; //œrednik
	constexpr char POINT_DEC{ '.' }; //kropka dziesiêtna
	constexpr char END{ '\\' };    //END \

	constexpr char BEL{ 0x07 };
	constexpr char CAN{ 0x18 };
	constexpr char ENQ{ 0x05 };
	constexpr char DLE{ 0x10 };
	constexpr char DLE2{ 0x1A };
	constexpr char DLE_ERR{ 0x01 };
	constexpr char DLE_PE_AKK{ 0x02 };
	constexpr char DLE_ONL{ 0x04 };

	std::string GetErrorMessage(long error_code);
	/**
	 * @author Piotr Kuliñski
	 * @date 2022-09-29
	 * @brief Strumieñ dla requestów Thermal. \n
	 * Nie przekekazujemy do strumienia sekwencji rozpoczynaj¹cych ESC+P ani sekwencji koñcz¹cej CRC+ESC+SLASH \n
	 * Elementy te zostan¹ automatycznie wygenerowane przy pobraniu bufora (string) metod¹ str().
	*/
	class fiscal_stream : public std::ostringstream {
	public:
		/**
		 * @brief Wyczyszczenie strumienia
		 * @param b Ci¹g czyszcz¹cy, podajemy pusty ci¹g tekstowy "", aby wyczyœciæ bufor.
		*/
		void str(std::string b) {
			std::ostringstream::str(b);
		}
		/**
		 * @brief Pobranie bufora z automatycznym wygenerowaniem sekwencji rozpoczynaj¹cych, sumy kontrolnej i sekwencji koñcz¹cej.
		 * @return bufor bajtów gotowy do przes³ania do urz¹dzenia.
		*/
		std::string str() {
			std::ostringstream& p = *this;
			std::stringstream request; request << (char)27 << 'P' << std::ostringstream::str() << thermal::crc(p) << (char)27 << '\\';
			return request.str();
		}
	};

	/**
	 * @author Piotr Kuliñski
	 * @date 2022-09-29
	 * @brief Strumieñ dla requestów Thermal, bez sumy kontrolnej \n
	 * Nie przekekazujemy do strumienia sekwencji rozpoczynaj¹cych ESC+P ani sekwencji koñcz¹cej CRC+ESC+SLASH \n
	 * Elementy te zostan¹ automatycznie wygenerowane przy pobraniu bufora (string) metod¹ str().
	*/
	class no_crc_stream : public std::ostringstream {
	public:
		/**
		* @brief Wyczyszczenie strumienia
		* @param b Ci¹g czyszcz¹cy, podajemy pusty ci¹g tekstowy "", aby wyczyœciæ bufor.
		*/
		void str(std::string b) {
			std::ostringstream::str(b);
		}
		/**
		 * @brief Pobranie bufora z automatycznym wygenerowaniem sekwencji rozpoczynaj¹cych i sekwencji koñcz¹cej, bez sumy kontrolnej.
		 * @return bufor bajtów gotowy do przes³ania do urz¹dzenia.
		*/
		std::string str() {
			std::ostringstream& p = *this;
			std::stringstream request; request << (char)27 << 'P' << std::ostringstream::str() << (char)27 << '\\';
			return request.str();
		}
	};
}
namespace fiscal {
	std::string to_string_not_decimalpoint(const double& str, unsigned short dec = 2);
	long to_long_not_decimalpoint(const double& str, unsigned short dec = 2);
	double round(const double& str, unsigned short dec = 2);
	double round(const char* str, unsigned short dec = 2);
	double round(const std::string& str, unsigned short dec = 2);
	std::string to_string(long value, unsigned short point = 2, std::string dec = ",.");
	std::string to_string(const double value, unsigned short point = 2, std::string dec = ",.");
	std::string obetnij_zera(std::string& v);
}

/**
* Opis ENQ
* 7 - 0 (zarezerwowany)
* 6 - 1 (zarezerwowany)
* 5 - 1 (zarezerwowany)
* 4 - 0 (zarezerwowany)
* 3 - FSK (0: drukarka jest w trybie szkoleniowym, 1: drukarka jest w trybie fiskalnym)
* 2 - CMD (1: ostatni rozkaz zosta³ wykonany poprawnie (bit jest kasowany po odebraniu ESC P, je¿eli
*             rozkaz nastêpuj¹cy po ESC P nie jest ¿¹daniem odes³ania informacji kasowych i ustawiony po
*             jego poprawnym wykonaniu),
* 1 - PAR (1: drukarka jest w trybie transakcji (po wykonaniu sekwencji pocz¹tek transakcji, przed
*             wykonaniem sekwencji anulowanie transakcji, standardowe zatwierdzenie transakcji,
*             zatwierdzenie transakcji z formami p³atnoœci (1) lub zatwierdzenie transakcji z
*			  formami p³atnoœci (2))
* 0 - TRF (1: ostatnia transakcja zosta³a sfinalizowana poprawnie (bit ustawiony po poprawnej realizacji
*             sekwencji standardowe zatwierdzenie transakcji, zatwierdzenie transakcji z formami
*             p³atnoœci (1) lub zatwierdzenie transakcji z formami p³atnoœci (2),
*             a kasowany po poprawnej realizacji pocz¹tek transakcji).
*/
struct ENQ
{
private:
	std::bitset<8> bity = 0;
public:
	const auto trf() { return bity.test(0); }
	const auto par() { return bity.test(1); }
	const auto cmd() { return bity.test(2); }
	const auto fiscal() { return bity.test(3); }
	const auto get() { return bity; }
	ENQ(unsigned char byte_enq) : bity(byte_enq)
	{
	}
	void display() {
		std::cout << "ENQ: \'" << bity << "\'" << std::endl;
		if (!cmd()) {
			std::cerr << "CMD: \'Blad polecenia\'" << std::endl;
			std::cerr << "FSK: \'" << (fiscal() ? "Fiskalna" : "Niefiskalna") << "\'" << std::endl;
			if (trf()) {
				std::cerr << "TRF: drukarka w trybie transakcji, anulacja transakcji" << std::endl;
			}
		}
	}
};

/**
* Opis DLE
* 7 - 0 (zarezerwowany)
* 6 - 1 (zarezerwowany)
* 5 - 1 (zarezerwowany)
* 4 - 1 (zarezerwowany)
* 3 - 0 (zarezerwowany)
* 2 - ONL ( = 1: stan "On-Line")
* 1 - PE/AKK ( = 1: stan "Brak Papieru" lub roz³adowana bateria akumulatorów)
* 0 - ERR (= 1: stan "B³¹d mechanizmu/sterownika")
*/
struct DLE
{
private:
	std::bitset<8> bity = 0;
public:
	DLE(unsigned char byte_dle) : bity(byte_dle)
	{
	}
	void display() {
		std::cout << "DLE: \'" << bity << "\'" << std::endl;
	}
	const auto err() { return bity.test(0); }
	/**
	 * @brief Sprawdzenie statusu
	 * @return
	 * 1-b³¹d (brak papieru lub roz³adowany akumulator), \n
	 * 0-ok
	*/
	const auto pe_akk() { return bity.test(1); }
	const auto no_onl() { return !bity.test(2); }
	const auto get() { return bity; }
};
