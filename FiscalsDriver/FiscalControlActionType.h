#pragma once
/*! \file */

/**
 \ingroup Enumerations
 \author Piotr Kuliñski (&copy;) 2024-06
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
	 Przed rozpoczêciem w³aœciwego wydruku
	 */
	Before,

	/**
	 * \brief (2)\n
	 Po zakoñczeniu w³aœciwego wydruku
	 */
	After,

	/**
	 * \brief (0)\n
	 Przed i po zakoñczeniu wydruku
	 */
	BeforeAndAfter
};
