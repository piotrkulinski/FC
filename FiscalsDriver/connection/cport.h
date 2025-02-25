#pragma once
#include <string>

namespace cport {
	//komu przysz�o do g�owy aby przedefiniowa� WinBase.h
	enum Parity			//kontrola parzystosci
	{
		EvenParity,		//parzysta
		MarkParity,		//bit kontroli parzystosci stale rowny 1
		NoParity,		//brak kontroli parzystosci
		OddParity,		//nieparzysta
		Space
	};

	enum StopBits		//bity stopu
	{
		OneStopBit,		//1 bit stopu
		OnePointFiveStopBits,	//w przypadku slowa 5 bitowego bit stopu wydluzony o 1/2
		TwoStopBits		//2 bity stopu
	};

	enum FlowControl  //kontrola przeplywu
	{
		NoFlowControl,
		CtsRtsFlowControl,
		CtsDtrFlowControl,
		DsrRtsFlowControl,
		DsrDtrFlowControl,
		XonXoffFlowControl
	};
}