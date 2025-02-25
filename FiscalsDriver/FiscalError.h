#pragma once
/*! \file */
/**
 \ingroup Enumerations
 \author pkulinski (&copy;) 2024
 \brief Typy b³êdów z wykonania procedur wydruków
 */
enum class FiscalError : long
{
	/**
	 * @brief Nieokreœlono
	*/
	UNKONW = 0,
	
	/**
	 * @brief <b>Brak b³êdu</b>
	*/
	STATUS_OK = 1,

	/**
	 * @brief Ogólny b³¹d
	*/
	ANY_ERROR = -1,
	/**
	 * @brief Problem z przygotowaniem, np. przetworzeniem XML-a wejœciowego
	*/
	PREPARE = -100,

	/**
	 * @brief B³¹d przetworzenie odczytanych stawek VAT
	*/
	PREPARE_VAT = -101,

	/**
	 * @brief B³êdy zwi¹zane z konfiguracj¹.
	*/
	CONFIGURATION = -200,

	/**
	 * \brief Niepoprawna konfiguracja.
	 */
	CONFIGURATION_INVALID = -201,

	/**
	 * \brief B³êdy zwi¹zane z komunikacj¹.
	 */
	COMMUNICATION = -300,

	/**
	 * \brief B³¹d zapisu.
	 */
	COMMUNICATION_WRITE = -301,

	/**
	 * \brief B³¹d odczytu.
	 */
	COMMUNICATION_READ = -302,

	/**
	 * \brief B³êdy zwi¹zane z przekroczeniem czasu.
	 */
	TIMEOUT = -400,
	TIMEOUT_FISCAL = -401,	

	/**
	 * @brief Przekroczony czas na odpowiedŸ z ECR
	*/
	TIMEOUT_ANSWER = -402,	

	/**
	 * @brief B³êdy zwi¹zane z oprogramowaniem
	*/
	PROTOCOL = -1000,

	/**
	 * @brief Odpowiedzi u¿ytkownika
	*/
	USER = -2000,
	/**
	 * @brief U¿ytkownik odrzuci³ ostatnie pytanie
	*/
	USER_REJECT = -2001

};

