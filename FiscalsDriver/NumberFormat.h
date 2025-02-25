#pragma once
#include <math.h>
#include <stdlib.h>
#include "MyUtils.h"

class CNumberFormat
{
	public:
		CNumberFormat(void);
		CNumberFormat(int, int);
		~CNumberFormat(void);

	public:
		void setMaximumFractionDigits(int);
		void setMinimumFractionDigits(int);
		CString format(double number);
		double parse(CString str);
		CString formatWithOutDot(double number);
		int parseCStringToInt(CString str);

	private:
		int pobierzLiczCyfrZnacz(double);

	private:
		int m_maxFractionDigits;	//max liczba cyfr po przecinku po formatowaniu
		int m_minFractionDigits;	//min liczba cyfr po przecinku po formatowaniu

};
