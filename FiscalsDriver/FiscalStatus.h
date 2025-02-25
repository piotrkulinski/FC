#pragma once
/*! \file */
/**
 \brief Typy statusów komunikacyjnych ECR<->FISCAL
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

	/** Zadanie wystartowa³o*/
	START = 2,

	/**Przygotowanie zadania do wykonania*/
	PREPARE = 3,

	/**Przetwarzanie zadania*/
	PROCESSING = 4,

	/**Zadanie pracuje*/
	WORKING = 5,

	/**Zadanie w trakcie drukowania	*/
	PRINTING = 6,

	/**Zadanie zakoñczone*/
	FINISH = 7,

	/**
	 * @brief Wartoœæ bazowa ¿¹dania odpowiedzi z ECR
	*/
	ANSWER = 100,
	/**
	 * ¯¹danie odpowiedzi z ECR, z uwagi na brak papieru. \n
	 * Szczegó³y treœci mog¹ byæ dostêpne poprzez metodê API \ref GetMessageState \n
	 * OdpowiedŸ, mo¿e mieæ poni¿esz wartoœæ typu \ref AnswerType \n
	 * \ref AnswerType::YES "YES" - papier wymieniony\n
	 * \ref AnswerType::NO "NO" - odrzucenie wydruku
	*/
	ANSWER_WAIT_PAPER = 101,

	/**
	 * ¯¹danie odpowiedzi z ECR z uwagi na problem z urz¹dzeniem (mechanizm, pokrywa). \n
	 * Szczegó³y treœci mog¹ byæ dostêpne poprzez metodê API \ref GetMessageState \n
	 * OdpowiedŸ, mo¿e mieæ poni¿esz wartoœæ typu \ref AnswerType \n
	 * \ref AnswerType::YES "YES" - problem rozwi¹zany\n
	 * \ref AnswerType::NO "NO" - odrzucenie wydruku
	*/
	ANSWER_WAIT_DEVICE = 102
};