#pragma once
/*! \file */

/**
 \ingroup Enumerations
 \author pkulinski (&copy;) 2024
 \brief Rezultat wykonania operacji wydruku
 \see GetResult
 */
enum class FiscalResult : size_t {
	/**
	 * @brief (1)\n
	 * Status nieznany
	*/
	UNKNOW,

	/**
	 * @brief (1)\n
	 * Wykonano bezb��dnie
	*/
	OK,

	/**
	 * @brief (2)\n
	 * B��d wykonania
	*/
	FAILED
};

