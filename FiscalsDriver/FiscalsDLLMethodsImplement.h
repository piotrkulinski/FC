#pragma once

#include <iostream>
#include <wtypes.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <WinBase.h>
#include "ErrorMode.h"

#include "ConverterType.h"
//#include <pugixml.cpp>

#include "FiscalStatus.h"
#include "FiscalTypes.h"
#include "CallbackType.h"


#define DECLARE_METHOD_0_PAR(c_method) typedef int(__stdcall* f_##c_method)(); f_##c_method CALL_##c_method { NULL };
#define DECLARE_METHOD_1_PAR(c_method,p1) typedef int(__stdcall* f_##c_method)(##p1); f_##c_method CALL_##c_method{ NULL };
#define DECLARE_METHOD_2_PAR(c_method,p1,p2) typedef int(__stdcall* f_##c_method)(##p1,##p2); f_##c_method CALL_##c_method{ NULL };
#define DECLARE_METHOD_3_PAR(c_method,p1,p2,p3) typedef int(__stdcall* f_##c_method)(##p1,##p2,##p3); f_##c_method CALL_##c_method{ NULL };
#define DECLARE_METHOD_4_PAR(c_method,p1,p2,p3,p4) typedef int(__stdcall* f_##c_method)(##p1,##p2,##p3,##p4); f_##c_method CALL_##c_method{ NULL };
#define DECLARE_METHOD_5_PAR(c_method,p1,p2,p3,p4,p5) typedef int(__stdcall* f_##c_method)(##p1,##p2,##p3,##p4,##p5); f_##c_method CALL_##c_method{ NULL };
#define DECLARE_METHOD_6_PAR(c_method,p1,p2,p3,p4,p5,p6) typedef int(__stdcall* f_##c_method)(##p1,##p2,##p3,##p4,##p5,##p6); f_##c_method CALL_##c_method{ NULL };
#define DECLARE_METHOD_7_PAR(c_method,p1,p2,p3,p4,p5,p6,p7) typedef int(__stdcall* f_##c_method)(##p1,##p2,##p3,##p4,##p5,##p6,##p7); f_##c_method CALL_##c_method{ NULL };
#define DECLARE_METHOD_8_PAR(c_method,p1,p2,p3,p4,p5,p6,p7,p8) typedef int(__stdcall* f_##c_method)(##p1,##p2,##p3,##p4,##p5,##p6,##p7,##p8); f_##c_method CALL_##c_method{ NULL };
#define DECLARE_METHOD_9_PAR(c_method,p1,p2,p3,p4,p5,p6,p7,p8,p9) typedef int(__stdcall* f_##c_method)(##p1,##p2,##p3,##p4,##p5,##p6,##p7,##p8,##p9); f_##c_method CALL_##c_method{ NULL };
#define DECLARE_METHOD_13_PAR(c_method,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13) typedef int(__stdcall* f_##c_method)(##p1,##p2,##p3,##p4,##p5,##p6,##p7,##p8,##p9,##p10,##p11,##p12,##p13); f_##c_method CALL_##c_method{ NULL };
#define DECLARE_METHOD_15_PAR(c_method,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15) typedef int(__stdcall* f_##c_method)(##p1,##p2,##p3,##p4,##p5,##p6,##p7,##p8,##p9,##p10,##p11,##p12,##p13,##p14,##p15); f_##c_method CALL_##c_method{ NULL };

#define DDECLARE_METHOD_7_PAR(ret,c_method,p1,p2,p3,p4,p5,p6,p7) \
private: \
typedef int(__stdcall* f_##c_method)(##p1,##p2,##p3,##p4,##p5,##p6,##7); f_##c_method CALL_##c_method{ NULL }; \
public: \
##ret ##c_method(##p1,##p2,##p3,##p4,##p5,##p6,##p7);

#define DEF_STATIC_INT_METHOD_SIMPLE(c_method) static int CALL_##c_method = 0;\
if (CALL_##c_method==0) { \
	CALL_##c_method=(f_##c_method)GetProcAddress(hDLL, #c_method); \
};
#define DEF_METHOD_SIMPLE(c_method) if ( CALL_##c_method == NULL) { CALL_##c_method = (f_##c_method)GetProcAddress(hDLL, #c_method);}
#define DEF_METHOD(c_method,method) CALL_##c_method = (f_##c_method)GetProcAddress(hDLL, #method);

/**
 * \brief Typy komunikatów zwracanych metodami obs³ugi zdarzeñ
 * \author pkulinski (&copy;) 04-05-2021
 * \n Definicja typów zdarzeñ
 * \n komunikaty, wydruki
 * \n \em \b evn_ < 1000 - typy komunikatów do odebrania metod¹ zdarzeniow¹ pod³¹czon¹ poprzez \ref CTX_RegisterEventNotify lub [Java_pl_laser_positive_drivers_CardTerminalsDLL_CTXRegisterEventNotify] (\ref Java_pl_laser_positive_drivers_CardTerminalsDLL_CTXRegisterEventNotify)
 * \n \em \b evp_ >= 1000 - typy komunikatów do odebrania metod¹ zdarzeniow¹ pod³¹czon¹ poprzez \ref CTX_RegisterEventPrintData lub [Java_pl_laser_positive_drivers_CardTerminalsDLL_CTXRegisterEventPrintData] (\ref Java_pl_laser_positive_drivers_CardTerminalsDLL_CTXRegisterEventPrintData)
 */
//typedef int(CALLBACK* EventHandler)(char* context, EventType type);

const int FDRV_INITIALIZED = 1;
const int FDRV_ALREADY_INITIALIZED = 2;
const int FDRV_INIT_FAILURE = -1;
const int FDRV_WRONG_VER = -2;

class FiscalsDLLMethodsImplement
{
private:
	HINSTANCE hDLL;

	std::string dllFile{ "FiscalsDriver.dll" };

	DECLARE_METHOD_1_PAR(UstawObslugeBledow, char);
	DECLARE_METHOD_2_PAR(WydrukNiefiskalnyRozpoczecie, int, int);
	DECLARE_METHOD_7_PAR(WydrukNiefiskalnyLinia, int, int, char*, char*, char*, char*, char*);
	DECLARE_METHOD_5_PAR(WydrukNiefiskalnyZakonczenie, int, char*, char*, char*, char*);
	DECLARE_METHOD_1_PAR(WydrukXMLThermal, char*);
	DECLARE_METHOD_1_PAR(PrintNonFiscal, XmlApiNonFiscalRequest);
	DECLARE_METHOD_6_PAR(WydrukNiefiskalnyXML, char*, int, int, int, int, int);
	DECLARE_METHOD_1_PAR(ParagonXML, char*);
	DECLARE_METHOD_1_PAR(SendRequest, char*);
	DECLARE_METHOD_2_PAR(CheckFiscal, char*, size_t);
	DECLARE_METHOD_0_PAR(FiskalnyAnulowanieTransakcji);


public:
	FiscalsDLLMethodsImplement(std::string FiscalDriver) {

		//SetState(FiscalStatus::START);

		dllFile = FiscalDriver;
		hDLL = LoadLibraryA(dllFile.c_str());
		if (!hDLL) {
			std::cout << "Error load library: " << GetLastError() << std::endl;
			return;
		}

		DEF_METHOD_SIMPLE(UstawObslugeBledow);
		DEF_METHOD_SIMPLE(WydrukNiefiskalnyRozpoczecie);
		DEF_METHOD_SIMPLE(WydrukNiefiskalnyLinia);
		DEF_METHOD_SIMPLE(WydrukNiefiskalnyZakonczenie);
		DEF_METHOD_SIMPLE(WydrukNiefiskalnyXML);
		DEF_METHOD_SIMPLE(WydrukXMLThermal);
		DEF_METHOD_SIMPLE(PrintNonFiscal);
		DEF_METHOD_SIMPLE(FiskalnyAnulowanieTransakcji);
	}

	~FiscalsDLLMethodsImplement() {
		if (hDLL != NULL) {
			FreeLibrary(hDLL);
			hDLL = NULL;
		}
	}

	void Finalizacja() {
		if (hDLL) {
			FreeLibrary(hDLL);
			hDLL = NULL;
		}
	}

	int RaportDobowy() {
		DECLARE_METHOD_0_PAR(RaportDobowy);
		DEF_METHOD_SIMPLE(RaportDobowy);
		return CALL_RaportDobowy();
	}
	int OdeslijInformacjeKasowe() {
		DECLARE_METHOD_0_PAR(OdeslijInformacjeKasowe);
		DEF_METHOD_SIMPLE(OdeslijInformacjeKasowe);
		return CALL_OdeslijInformacjeKasowe();
	}

	void TestDrukarki() {
		DECLARE_METHOD_0_PAR(TestDrukarki);
		DEF_METHOD_SIMPLE(TestDrukarki);
		CALL_TestDrukarki();
	}

	void InitFiscal() {
		DECLARE_METHOD_0_PAR(InitFiscal);
		DEF_METHOD_SIMPLE(InitFiscal);
		CALL_InitFiscal();
	}

	void NipNabywcy(char* model) {
		DECLARE_METHOD_1_PAR(UstawNipNabywcy, char*);
		DEF_METHOD_SIMPLE(UstawNipNabywcy);
		CALL_UstawNipNabywcy(model);
	}


	int RaportDobowyNaDzien(int year, int month, int day) {
		DECLARE_METHOD_3_PAR(RaportDobowyNaDzien, int, int, int);
		DEF_METHOD_SIMPLE(RaportDobowyNaDzien);
		return CALL_RaportDobowyNaDzien(year, month, day);
	}

	int PoczatekTransakcji(int iloscLinii, char* numer_systemowy) {
		DECLARE_METHOD_2_PAR(FiskalnyPoczatekTransakcji2, int, char*);
		DEF_METHOD_SIMPLE(FiskalnyPoczatekTransakcji2);
		return CALL_FiskalnyPoczatekTransakcji2(iloscLinii, numer_systemowy);
	}

	int Open(char* portCom) {
		DECLARE_METHOD_1_PAR(Open, char*);
		DEF_METHOD_SIMPLE(Open);
		return CALL_Open(portCom);
	}

	int OpenDefault() {
		DECLARE_METHOD_0_PAR(OpenDefault);
		DEF_METHOD_SIMPLE(OpenDefault);
		return CALL_OpenDefault();
	}

	int OtwarcieSzuflady() {
		DECLARE_METHOD_0_PAR(OtwarcieSzuflady);
		DEF_METHOD_SIMPLE(OtwarcieSzuflady);
		return CALL_OtwarcieSzuflady();
	}

	int AnulowanieTransakcji() {
		return CALL_FiskalnyAnulowanieTransakcji();
	}

	int LiniaParagonu(int nr, char* nazwa, char* ilosc, double cena_bez_rabatu, double vat, BOOL vatZwolniony, double brutto_bez_rabatu, double rabat_od_brutto) {
		DECLARE_METHOD_8_PAR(FiskalnyLiniaParagonu, int, char*, char*, double, double, BOOL, double, double);
		DEF_METHOD_SIMPLE(FiskalnyLiniaParagonu);
		return CALL_FiskalnyLiniaParagonu(nr, nazwa, ilosc, cena_bez_rabatu, vat, vatZwolniony, brutto_bez_rabatu, rabat_od_brutto);
	}

	int ZatwierdzenieTransakcji_3(
		  double total, double gotowka, char* idFromPlatStr, char* wartoscFormPlatStr
		, char* nazwFormPlatStr, int formyPlaSize, int znakRozdzielajacy, char* nr_kasy
		, char* kasjer, char* nr_systemowy, double rabat, char* kod_kreskowy
		, char* liniaStopki1, char* liniaStopki2, char* liniaStopki3
	) {

		DECLARE_METHOD_15_PAR(ZatwierdzenieTransakcji_3_Str
		, double, double, char*, char*
		, char*, int, int, char*
		, char*, char*, double, char*
		, char*, char*, char*);
		DEF_METHOD_SIMPLE(ZatwierdzenieTransakcji_3_Str);
		return CALL_ZatwierdzenieTransakcji_3_Str(
		  total, gotowka, idFromPlatStr, wartoscFormPlatStr
		, nazwFormPlatStr, formyPlaSize, znakRozdzielajacy, nr_kasy
		, kasjer, nr_systemowy, rabat, kod_kreskowy
		, liniaStopki1, liniaStopki2, liniaStopki3);
	}

	int UstawObslugeBledow(ErrorMode error_mode) {
		return CALL_UstawObslugeBledow((char)(error_mode + 48));
	}

	int WydrukXMLThermal(XmlApiFiscalRequest XML) {
		DECLARE_METHOD_1_PAR(PrintNonFiscal, const char*);
		DEF_METHOD_SIMPLE(PrintNonFiscal);
		return CALL_PrintNonFiscal(XML);
	}

	int WydrukXMLPosnet(XmlApiFiscalRequest XML) {
		DECLARE_METHOD_1_PAR(PrintNonFiscal, const char*);
		DEF_METHOD_SIMPLE(PrintNonFiscal);
		return CALL_PrintNonFiscal(XML);
	}

	int PrintFiscal(XmlApiFiscalRequest xml) {
		DECLARE_METHOD_1_PAR(PrintFiscal, const char*);
		DEF_METHOD_SIMPLE(PrintFiscal);
		return CALL_PrintFiscal(xml);
	}

	int RGetConnection() {
		//typedef ConnectionDevice(std::shared_ptr<Connection>* f_GetConnection)();
		typedef int(__stdcall* f_GetConnection)();
		static f_GetConnection CALL_GetConnection{ NULL };
		if (CALL_GetConnection == NULL) {
			CALL_GetConnection = (f_GetConnection)GetProcAddress(hDLL, "GetConnection");
		}
		return CALL_GetConnection();
	}

	int setFormater(int formater) {
		DECLARE_METHOD_1_PAR(setFormater, int);
		DEF_METHOD_SIMPLE(setFormater);
		return CALL_setFormater(formater);
	}

	int getLastReceipt(char* buffer, size_t len) {
		DECLARE_METHOD_2_PAR(getLastReceipt, char*, size_t);
		DEF_METHOD_SIMPLE(getLastReceipt);
		return CALL_getLastReceipt(buffer, len);
	}

	int PobierzWartosciTotalizerow(char* buffer, size_t len) {
		DECLARE_METHOD_2_PAR(PobierzWartosciTotalizerow, char*, size_t);
		DEF_METHOD_SIMPLE(PobierzWartosciTotalizerow);
		return CALL_PobierzWartosciTotalizerow(buffer, len);
	}

	int UstawCzas(int year, int month, int date, int hour, int minute, int second) {
		DECLARE_METHOD_6_PAR(UstawCzas, int, int, int, int, int, int);
		DEF_METHOD_SIMPLE(UstawCzas);
		return CALL_UstawCzas(year, month, date, hour, minute, second);
	}
	int RaportOkresowyExt(int y1, int m1, int d1, int y2, int m2, int d2, int tryb, char* nrKasy, char* kasjer) {
		DECLARE_METHOD_9_PAR(RaportOkresowyExt, int, int, int, int, int, int, int, char*, char*);
		DEF_METHOD_SIMPLE(RaportOkresowyExt);
		return CALL_RaportOkresowyExt(y1, m1, d1, y2, m2, d2, tryb, nrKasy, kasjer);
	}

	int XMLRequest(const char* XML) {
		DEF_METHOD_SIMPLE(SendRequest);
		return CALL_SendRequest((char*)XML);
	}

	int CheckFiscal(char* outxml, size_t len) {
		DEF_METHOD_SIMPLE(CheckFiscal);
		return CALL_CheckFiscal(outxml, len);
	}

	void UstawKodKreskowy(char* kod, int typ) {
		DECLARE_METHOD_2_PAR(UstawKodKreskowy, char*, int);
		DEF_METHOD_SIMPLE(UstawKodKreskowy);
		CALL_UstawKodKreskowy(kod, typ);
	}

	int PobierzFunkcjeSterownikaDrukarki(char* buffer, size_t buffer_len) {
		DECLARE_METHOD_2_PAR(PobierzFunkcjeSterownikaDrukarki, char*, size_t);
		DEF_METHOD_SIMPLE(PobierzFunkcjeSterownikaDrukarki);
		return CALL_PobierzFunkcjeSterownikaDrukarki(buffer, buffer_len);
	}

	int PobierzInformacjeKasowe(char* outxml, size_t lendata) {
		DECLARE_METHOD_2_PAR(PobierzInformacjeKasowe, char*, size_t);
		DEF_METHOD_SIMPLE(PobierzInformacjeKasowe);
		return CALL_PobierzInformacjeKasowe(outxml, lendata);
	}

	int getProperties(char* outxml, size_t lendata) {
		DECLARE_METHOD_2_PAR(getProperties, char*, size_t);
		DEF_METHOD_SIMPLE(getProperties);
		return CALL_getProperties(outxml, lendata);
	}

	int getFiscalNamesFormat(char* outxml, size_t lendata, size_t convert) {
		DECLARE_METHOD_3_PAR(getFiscalNamesFormat, char*, size_t, size_t);
		DEF_METHOD_SIMPLE(getFiscalNamesFormat);
		return CALL_getFiscalNamesFormat(outxml, lendata, convert);
	}

	//int getFiscalNames(char* outxml, size_t lendata) {
	//	DECLARE_METHOD_2_PAR(getFiscalNames, char*, size_t, );
	//	DEF_METHOD_SIMPLE(getFiscalNames);
	//	return CALL_getFiscalNames(outxml, lendata);
	//}

	void RegisterEventNotify(EventHandler fn)
	{
		DECLARE_METHOD_1_PAR(RegisterEventNotify, EventHandler);
		DEF_METHOD_SIMPLE(RegisterEventNotify);
		CALL_RegisterEventNotify(fn);
	}

	void RegisterEventPrintData(EventHandler fn)
	{
		DECLARE_METHOD_1_PAR(RegisterEventPrintData, EventHandler);
		DEF_METHOD_SIMPLE(RegisterEventPrintData);
		CALL_RegisterEventPrintData(fn);
	}

	void UnregisterDispatcher()
	{
		DECLARE_METHOD_0_PAR(UnregisterDispatcher);
		DEF_METHOD_SIMPLE(UnregisterDispatcher);
		CALL_UnregisterDispatcher();
	}

	int SetConnection(char* connectionString)
	{
		DECLARE_METHOD_1_PAR(SetConnection, XmlApiConfigurationRequest);
		DEF_METHOD_SIMPLE(SetConnection);
		return CALL_SetConnection(connectionString);
	}

	int Close() {
		DECLARE_METHOD_0_PAR(Close);
		DEF_METHOD_SIMPLE(Close);
		return CALL_Close();
	}

	int fiscalDestroy() {
		DECLARE_METHOD_0_PAR(fiscalDestroy);
		DEF_METHOD_SIMPLE(fiscalDestroy);
		return CALL_fiscalDestroy();
	}

};
