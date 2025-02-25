#pragma once
/*! \file */

/**
 \ingroup Enumerations
 \author Piotr Kuliñski (&copy;) 2024-06
 \brief Rodzaje akcji kontrolnych
*/
enum class FiscalControlType : size_t {
	/**
	 * \brief (0) \n
	 * Typ nieokreœlony.	 
	 */
	Unknow = 0,

	/**
	 * \brief (1) \n
	 * Otwarcie szuflady\n
	 * \code{xml}
	 *   <!-- 
	 *   	wartoœæ: 1, 
	 *   	akcja typu: Before, After, BeforeAndAfter 
	 *   -->
	 *   <OtworzSzuflade typ="...">1</OtworzSzuflade>
	 * \endcode
	 */
	 OpenDrawer = 1
};
