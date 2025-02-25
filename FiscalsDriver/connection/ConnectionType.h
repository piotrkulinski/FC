#pragma once
/*! \file */

/**
 \ingroup Enumerations
 \brief Typ po³¹czenia z urz¹dzeniem.
 \author Piotr Kuliñski (&copy;) 2024-06
*/
enum class ConnectionType: int {
	/**
	 * \brief Wstêpnie nieokreœlone
	*/
	Unknow = -1,
	
	/**
	 * \brief (0) - Po³¹czenie RS232 \n
	 * Najczêœciej wykorzystywane po³¹czenie z urz¹dzeniami fiskalnymi.
	 * Jeœli nie znamy parametrów po³¹czenia, ale wiemy ¿e jest po RS, wówczas
	 * mo¿emy pos³u¿yæ siê metod¹ \ref CFiscalFactory::DetectFiscalPrinters "wykrywaj¹c¹ urz¹dzenia", która zwróci 
	 * nam \ref XmlApiDetectFiscalResponse "dane xml", gdzie bêdziemy mogli je odczytaæ.
	 * \include Examples\ConnectionRS232.xml
	*/
	RS232,
	
	/**
	 \brief (1) - Po³¹czenie TCP/IP
	 \include Examples\ConnectionTCP.xml
	*/
	TCP,

	/**
	 * @brief (3) - Wydruk do pliku
	*/
	FILE
};
