#pragma once
/*! \file */

/**
 \ingroup Enumerations
 \brief Status po��czenia.
 \author Piotr Kuli�ski (&copy;) 2024-07
*/
static enum class ConnectionState : size_t {
	/**
	 * \brief Po��czenie nieustalone.
	 */
	UNKNOW = 0,

	/**
	 * \brief Po��czenie otwarte.
	 */
	OPEN = 1,

	/**
	 * \brief Po��czenie zamkni�te.
	 */
	CLOSE = 2,

	/**
	 * \brief B��dy po��czenia.
	 */
	FAILED = 100,

	/**
	 * \brief B��d podczas otwierania po��czenia.
	 */
	OPEN_FAILED = 101,

	/**
	 * \brief B��d podczas zamykania po��czenia.
	 */
	CLOSE_FAILED = 102
};
