#pragma once
/*! \file */

/**
 \ingroup Enumerations
 \author Piotr Kuliński (&copy;) 2024-06
 \brief Typy akcji kontrolnych
*/
enum class FiscalControlActionType : size_t {
	/**
	 * \brief (0)\n
	 Typ akcji nie jest znany.
	 */
	Unknow,

	/**
	 * \brief (1)\n
	 Przed rozpoczęciem właściwego wydruku
	 */
	Before,

	/**
	 * \brief (2)\n
	 Po zakończeniu właściwego wydruku
	 */
	After,

	/**
	 * \brief (0)\n
	 Przed i po zakończeniu wydruku
	 */
	BeforeAndAfter
};
