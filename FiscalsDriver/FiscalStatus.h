#pragma once
/*! \file */
/**
 \brief Typy status�w komunikacyjnych ECR<->FISCAL
 \ingroup Enumerations 
 \author pkulinski (&copy;) 2024
 */
enum class FiscalStatus : size_t {
	/** Status nieustalony*/
	UNKNOW = 0,

	/**
	 * @brief Status oczekiwania
	*/
	WAITING = 1,

	/** Zadanie wystartowa�o*/
	START = 2,

	/**Przygotowanie zadania do wykonania*/
	PREPARE = 3,

	/**Przetwarzanie zadania*/
	PROCESSING = 4,

	/**Zadanie pracuje*/
	WORKING = 5,

	/**Zadanie w trakcie drukowania	*/
	PRINTING = 6,

	/**Zadanie zako�czone*/
	FINISH = 7,

	/**
	 * @brief Warto�� bazowa ��dania odpowiedzi z ECR
	*/
	ANSWER = 100,
	/**
	 * ��danie odpowiedzi z ECR, z uwagi na brak papieru. \n
	 * Szczeg�y tre�ci mog� by� dost�pne poprzez metod� API \ref GetMessageState \n
	 * Odpowied�, mo�e mie� poni�esz warto�� typu \ref AnswerType \n
	 * \ref AnswerType::YES "YES" - papier wymieniony\n
	 * \ref AnswerType::NO "NO" - odrzucenie wydruku
	*/
	ANSWER_WAIT_PAPER = 101,

	/**
	 * ��danie odpowiedzi z ECR z uwagi na problem z urz�dzeniem (mechanizm, pokrywa). \n
	 * Szczeg�y tre�ci mog� by� dost�pne poprzez metod� API \ref GetMessageState \n
	 * Odpowied�, mo�e mie� poni�esz warto�� typu \ref AnswerType \n
	 * \ref AnswerType::YES "YES" - problem rozwi�zany\n
	 * \ref AnswerType::NO "NO" - odrzucenie wydruku
	*/
	ANSWER_WAIT_DEVICE = 102
};