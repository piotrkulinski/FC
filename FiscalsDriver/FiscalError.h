#pragma once
/*! \file */
/**
 \ingroup Enumerations
 \author pkulinski (&copy;) 2024
 \brief Typy b��d�w z wykonania procedur wydruk�w
 */
enum class FiscalError : long
{
	/**
	 * @brief Nieokre�lono
	*/
	UNKONW = 0,
	
	/**
	 * @brief <b>Brak b��du</b>
	*/
	STATUS_OK = 1,

	/**
	 * @brief Og�lny b��d
	*/
	ANY_ERROR = -1,
	/**
	 * @brief Problem z przygotowaniem, np. przetworzeniem XML-a wej�ciowego
	*/
	PREPARE = -100,

	/**
	 * @brief B��d przetworzenie odczytanych stawek VAT
	*/
	PREPARE_VAT = -101,

	/**
	 * @brief B��dy zwi�zane z konfiguracj�.
	*/
	CONFIGURATION = -200,

	/**
	 * \brief Niepoprawna konfiguracja.
	 */
	CONFIGURATION_INVALID = -201,

	/**
	 * \brief B��dy zwi�zane z komunikacj�.
	 */
	COMMUNICATION = -300,

	/**
	 * \brief B��d zapisu.
	 */
	COMMUNICATION_WRITE = -301,

	/**
	 * \brief B��d odczytu.
	 */
	COMMUNICATION_READ = -302,

	/**
	 * \brief B��dy zwi�zane z przekroczeniem czasu.
	 */
	TIMEOUT = -400,
	TIMEOUT_FISCAL = -401,	

	/**
	 * @brief Przekroczony czas na odpowied� z ECR
	*/
	TIMEOUT_ANSWER = -402,	

	/**
	 * @brief B��dy zwi�zane z oprogramowaniem
	*/
	PROTOCOL = -1000,

	/**
	 * @brief Odpowiedzi u�ytkownika
	*/
	USER = -2000,
	/**
	 * @brief U�ytkownik odrzuci� ostatnie pytanie
	*/
	USER_REJECT = -2001

};

