#pragma once
enum VatFormatLine : int {

	/**
	Nie formatujemy linii
	*/
	frmNoFormat = 0,

	/**
	 * Sformatuj lini� dodaj�c stawk� VAT na pocz�tku.
	 */
	frmPrefix = 1,

	/**
	 * Sformatuj lini� dodaj�c stawk� VAT na ko�cu.
	 */
	frmSufix = 2
};


