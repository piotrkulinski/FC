#pragma once

	/**
	* \defgroup NONFISCAL_TYPES Typy linijek w XML definiuj¹cym wydruk niefiskalny
	* @{
	*/

enum class NonFiscalLineType : size_t {

	L_PID_RECNo = 1,

	/**
	 * Napis predefiniowany drukarki, numeru MID terminala.
	 */
	L_MID = 2,

	L_CardNoDate = 3,

	/**
	 * Napis predefiniowany drukarki, numer AID.
	 */
	L_AID = 4,

	/**
	 * Nazwa karty.
	 */
	L_CardName = 5,
	L_CardOperation = 6,
	L_TranAmount = 7,
	L_ExRate = 8,
	L_MarkUp = 9,
	L_MarkUpInfo = 10,
	L_TranAmountCurr = 11,
	L_Receipt = 12,
	L_CurrencyConversionInfo = 13,
	L_VerificationMethod = 14,
	L_AuthCode = 15,
	L_TC = 16,
	L_ExRateInfo = 17,
	L_DateTime = 18,
	L_Void = 19,
	L_ForReceipt = 20,
	L_TrVoided = 21,
	L_Declined = 22,
	L_AdditionalText = 23,
	L_Empty = 24,
	L_Sign = 25
};
/**
* @}
*/
