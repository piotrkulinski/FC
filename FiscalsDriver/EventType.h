#pragma once
/*! \file */
/**
 \ingroup Enumerations
 \author pkulinski (&copy;)
 \brief Typy komunikat�w zwracanych metodami obs�ugi zdarze�
 \n Definicja typ�w zdarze�
 \n komunikaty, wydruki
 \n \em \b evn_ < 1000 - typy komunikat�w do odebrania metod� zdarzeniow� pod��czon� poprzez \ref eventAnswer
 \n \em \b evp_ >= 1000 - typy komunikat�w do odebrania metod� zdarzeniow� pod��czon� poprzez \ref eventAnswerata
 */
enum class EventType : size_t
{
	/** Typ nieokre�lony dla notify */
	evn_unknow = 0,

	/** Typ informacyjny */
	evn_info = 1,

	/** Ostrze�enie */
	evn_warrning = 2,

	/** B��d */
	evn_error = 3,

	/** Wyj�tek */
	evn_exception = 4,

	/** Zapytanie wysy�ane do ECR */
	evn_request = 100,	
	
	/**
	 * \brief (106) \n 
	 * Zapytanie wysy�ane do ECR.\n
	 * w odpowiedzi oczekuje na \ref AnswerType::YES "YES"
	 */
	evn_request_yes = 101,	

	/**
	 * \brief (107) \n 
	 * Zapytanie wysy�ane do ECR.\n
	 * w odpowiedzi oczekuje na \ref AnswerType::YES "YES" lub \ref AnswerType::NO "NO"
	 */
	evn_request_yes_no = 102,

	/**
	 * \brief (107) \n 
	 * Zapytanie wysy�ane do ECR.\n
	 * w odpowiedzi oczekuje na \ref AnswerType::RETRY "RETRY" lub \ref AnswerType::NO "NO"
	 */
	evn_request_retry_cancel = 103,

	/** \b >=1000 \n Typ nieokre�lony dla wydruk�w */
	evp_unknow = 1000
};
