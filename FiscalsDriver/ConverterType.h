#pragma once
/*! \file */
/**
\brief Mo�liwe typy konwersji wyniku
\ingroup Enumerations
*/
enum ConverterType : size_t {

	/**
	0 - Konwersja na XML
	*/
	XML,

	/**
	1 - Konwersja na JSON
	*/
	JSON,

	/**
	2 - Konwersja na tekst, podzielony �rednikiem
	*/
	COMATEXT,

	/**
	3 - Surowy format tekstowy, pola wyeksportowane ci�giem, bez separator�w
	*/
	RAW,

	/**
	Brak formatera
	*/
	NONE
};