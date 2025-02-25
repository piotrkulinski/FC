#pragma once
/*! \file */

/**
 \ingroup Enumerations
 \brief Status po³¹czenia.
 \author Piotr Kuliñski (&copy;) 2024-07
*/
static enum class ConnectionState : size_t {
	/**
	 * \brief Po³¹czenie nieustalone.
	 */
	UNKNOW = 0,

	/**
	 * \brief Po³¹czenie otwarte.
	 */
	OPEN = 1,

	/**
	 * \brief Po³¹czenie zamkniête.
	 */
	CLOSE = 2,

	/**
	 * \brief B³êdy po³¹czenia.
	 */
	FAILED = 100,

	/**
	 * \brief B³¹d podczas otwierania po³¹czenia.
	 */
	OPEN_FAILED = 101,

	/**
	 * \brief B³¹d podczas zamykania po³¹czenia.
	 */
	CLOSE_FAILED = 102
};
