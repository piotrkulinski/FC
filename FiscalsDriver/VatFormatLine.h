#pragma once
enum VatFormatLine : int {

	/**
	Nie formatujemy linii
	*/
	frmNoFormat = 0,

	/**
	 * Sformatuj liniê dodaj¹c stawkê VAT na pocz¹tku.
	 */
	frmPrefix = 1,

	/**
	 * Sformatuj liniê dodaj¹c stawkê VAT na koñcu.
	 */
	frmSufix = 2
};


