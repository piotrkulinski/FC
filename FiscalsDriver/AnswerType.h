#pragma once
/*! \file */
/**
 \ingroup Enumerations
 \author pkulinski (&copy;) 2024
 \brief Typy odpowiedzi z ECR
 */
static enum AnswerType : size_t {
	/**
	 * @brief (0) OdpowiedŸ niezdefiniowana, nieustalona
	*/
	UNKNOW = 0,

	/**
	 * @brief (1) TAK (Yes) \n
	 * Potwierdzenie
	*/
	YES = 1, 

	/**
	 * @brief (2) NO (Cancel) \n
	 * Przerwij
	*/
	NO = 2,

	/**
	 * @brief (3) (Abort) \n
	 * Odrzuæ
	*/
	REJECT = 3, //anuluj

	/**
	 * @brief (4) (Retry) \n
	 * Ponów
	*/
	RETRY = 4
};

