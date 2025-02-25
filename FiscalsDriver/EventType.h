#pragma once
/*! \file */
/**
 \ingroup Enumerations
 \author pkulinski (&copy;)
 \brief Typy komunikatów zwracanych metodami obs³ugi zdarzeñ
 \n Definicja typów zdarzeñ
 \n komunikaty, wydruki
 \n \em \b evn_ < 1000 - typy komunikatów do odebrania metod¹ zdarzeniow¹ pod³¹czon¹ poprzez \ref eventAnswer
 \n \em \b evp_ >= 1000 - typy komunikatów do odebrania metod¹ zdarzeniow¹ pod³¹czon¹ poprzez \ref eventAnswerata
 */
enum class EventType : size_t
{
	/** Typ nieokreœlony dla notify */
	evn_unknow = 0,

	/** Typ informacyjny */
	evn_info = 1,

	/** Ostrze¿enie */
	evn_warrning = 2,

	/** B³¹d */
	evn_error = 3,

	/** Wyj¹tek */
	evn_exception = 4,

	/** Zapytanie wysy³ane do ECR */
	evn_request = 100,	
	
	/**
	 * \brief (106) \n 
	 * Zapytanie wysy³ane do ECR.\n
	 * w odpowiedzi oczekuje na \ref AnswerType::YES "YES"
	 */
	evn_request_yes = 101,	

	/**
	 * \brief (107) \n 
	 * Zapytanie wysy³ane do ECR.\n
	 * w odpowiedzi oczekuje na \ref AnswerType::YES "YES" lub \ref AnswerType::NO "NO"
	 */
	evn_request_yes_no = 102,

	/**
	 * \brief (107) \n 
	 * Zapytanie wysy³ane do ECR.\n
	 * w odpowiedzi oczekuje na \ref AnswerType::RETRY "RETRY" lub \ref AnswerType::NO "NO"
	 */
	evn_request_retry_cancel = 103,

	/** \b >=1000 \n Typ nieokreœlony dla wydruków */
	evp_unknow = 1000
};
