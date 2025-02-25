#pragma once
/*! \file */

/**
 \ingroup Enumerations
 \author Piotr Kuli�ski (&copy;) 2024-06
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
	 Przed rozpocz�ciem w�a�ciwego wydruku
	 */
	Before,

	/**
	 * \brief (2)\n
	 Po zako�czeniu w�a�ciwego wydruku
	 */
	After,

	/**
	 * \brief (0)\n
	 Przed i po zako�czeniu wydruku
	 */
	BeforeAndAfter
};
