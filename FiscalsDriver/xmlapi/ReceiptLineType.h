#pragma once
/*! \file */

/**
 * \ingroup FiscalTypesDefinition
 * @author Piotr Kuli�ki
 * 
 * @date 2022-09-27
 * @brief Typ linii sprzeda�owej, atrybut "typ" \n
 * Je�li brak atrybutu program przymuje 1-Sale
*/
enum class ReceiptLineType : unsigned short {
	/**
	 * \brief (0)\n
	 * Typ nieokre�lony
	 */
	Unknow, 

	/**
	 * \brief (1)\n
	 * sprzeda�
	 */
	Sale,

	/**
	 * \brief (2)\n
	 * zwrot
	 */
	Refund, 

	/**
	 * \brief (3)\n
	 * kaucja pobrana, za sprzeda� opakowania
	 */
	KaucjaPobrana,

	/**
	 * \brief (4)\n
	 * kaucja zwr�cona
	 */
	KaucjaZwrocona,

	/**
	 * \brief (5)\n
	 * strono kaucji pobranej
	 */
	KaucjaPobranaStorno,

	/**
	 * \brief (6)\n
	 * storno kaucji zwr�conej
	 */
	KaucjaZwroconaStorno
};
