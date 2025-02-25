#pragma once
/*! \file */

/**
 \ingroup Enumerations
 \brief Typ po��czenia z urz�dzeniem.
 \author Piotr Kuli�ski (&copy;) 2024-06
*/
enum class ConnectionType: int {
	/**
	 * \brief Wst�pnie nieokre�lone
	*/
	Unknow = -1,
	
	/**
	 * \brief (0) - Po��czenie RS232 \n
	 * Najcz�ciej wykorzystywane po��czenie z urz�dzeniami fiskalnymi.
	 * Je�li nie znamy parametr�w po��czenia, ale wiemy �e jest po RS, w�wczas
	 * mo�emy pos�u�y� si� metod� \ref CFiscalFactory::DetectFiscalPrinters "wykrywaj�c� urz�dzenia", kt�ra zwr�ci 
	 * nam \ref XmlApiDetectFiscalResponse "dane xml", gdzie b�dziemy mogli je odczyta�.
	 * \include Examples\ConnectionRS232.xml
	*/
	RS232,
	
	/**
	 \brief (1) - Po��czenie TCP/IP
	 \include Examples\ConnectionTCP.xml
	*/
	TCP,

	/**
	 * @brief (3) - Wydruk do pliku
	*/
	FILE
};
