#ifndef _posnetH_
#define _posnetH_

/*

	Nag³ówek w wersji: 1.0

*/


/*!
	\mainpage Nowy protokó³ komunikacyjny POSNET. Biblioteka interfejsu drukarek fiskalnych.
	

	Spis Treœci

	- \ref sec_intro 
	- \ref sec_flow 
	- \ref sec_async
	  - \ref sec_async_events
	  .
	- \ref sec_example
	- \ref sec_extern
	- \ref linux_port
	.

	<hr>

	\section sec_intro	Wprowadzenie

	Podstawowe za³o¿enia, które zosta³y przyjête przy projektowaniu biblioteki dla
	drukarek implementuj¹cych nowy protokó³ komunikacyjny POSNET s¹ nastêpuj¹ce:
	  - komunikacja za poœrednictwem RS-232, USB (VCP lub DXX), Ethernet
	  - wielow¹tkowoœæ i wielo-urz¹dzeniowoœæ biblioteki (brak zmiennych globalnych)
	  - obs³uga zdarzeñ asynchronicznych (zmiany statusu urz¹dzenia)
	  - kolejkowanie rozkazów w celu przyspieszenia wymiany danych
	  - wykorzystanie z ró¿nych jêzyków programowania (konwencja wywo³añ __stdcall zgodna 
		z WINAPI)
	  - minimalna zale¿noœæ od systemu operacyjnego w zakresie interfejsu biblioteki -
	    praktycznie tylko 2 funkcja s¹ typowo dedykowane dla Windows - \ref POS_GetEvent
		i \ref POS_SetMessageParams.
	  - interfejs œledzenia (\ref debugging)
	  - natywny port dla systemu Linux
	  - maksymalna zgodnoœæ API z bibliotek¹ dla kas COMBO/NEO
	  .
    
	Ze wzglêdu na to, ¿e iloœæ rozkazów wykorzystywanych w komunikacji z drukark¹ jest ogromna
	przy projektowaniu biblioteki zrezygnowano z literalnej realizacji ka¿dego rozkazu
	jako osobnej funkcji. Grupowanie rozkazów w/g rodzaju parametrów tak¿e nie jest dobrym
	rozwi¹zaniem. W tej sytuacji zosta³ zastosowany model "obiektu rozkazowego (Request)",
	wraz z zestawem funkcji umo¿liwiaj¹cych ustawianie parametrów (\ref req_param). 

	Zastosowanie takiego rozwi¹zania ma dodatkow¹ zaletê. Drobne korekty w rozkazach obs³ugi
	drukarki w tym dodanie nowych czy zmiana iloœci i rodzaju parametrów nie wymagaj¹ 
	zmian w bibliotece komunikacyjnej, a jedynie dostosowania aplikacji do tych zmian.

	Po wykonaniu takiego obiektu rozkazowego przez drukarkê (por. \ref sec_flow) wyniki 
	dostêpne s¹ w analogiczny sposób - przez zestaw funkcji pobieraj¹cych je kolejno
	z obiektu rozkazowego.

	Urz¹dzenia obs³uguj¹ce nowy protokó³ komunikacyjny:
	- 4610-1NR FV EJ 1.01
	- COMBO DF 1.01
	- Nixdorf TH 230+ FV EJ 1.01
	- Temo HS FV EJ 1.01
	- Thermal FV 3.02
	- Thermal FV 3.03
	- Thermal FV EJ 1.01
	- Thermal FV EJ 1.02
	- Thermal FV EJ 2.01
	- Thermal HD FV EJ 1.01
	- Thermal HS FV 1.01
	- Thermal HS FV 1.02
	- Thermal HS FV EJ 1.01
	-	Thermal HS FV EJ 1.02 
	.

	Lista urz¹dzeñ mo¿e ulec poszerzeniu, w celu zapoznania siê z aktualn¹ ofert¹ urz¹dzeñ zapraszamy do
	odwiedzenia firmowego serwisu WWW pod adresem http://www.posnet.com.pl.

	\section sec_flow	Przep³yw informacji

	W aplikacji istniej¹ 2 kolejki rozkazowe, pomiêdzy którymi nastêpuje przep³yw danych 
	jak w grafie poni¿ej.
	\dot
	digraph flow {
		node [shape=record, fontname=Helvetica, fontsize=8];
		K0 [label="Kolejka rozkazowa"  shape="ellipse" ]
		K1 [label="Kolejka wynikow"  shape="ellipse" ]	
		NOWY -> OCZEKUJACY [label="POS_PostRequest" fontname=Helvetica fontsize=8];
		OCZEKUJACY -> K0;
		K0 -> WYSLANY [label="transmisja do drukarki" fontname=Helvetica fontsize=8];
		WYSLANY -> ZAKONCZONY [label="sekwencja wykonana\n lub blad" fontname=Helvetica fontsize=8];
		ZAKONCZONY -> K1 [label="Rozkaz" fontname=Helvetica fontsize=8]
	}
	\enddot

	Ca³oœæ transmisji obs³ugiwana jest przez dwa niezale¿ne w¹tki, jeden transmisyjny,
	drugi odbiorczy operuj¹ce na odpowiednich kolejkach rozkazów. Dodatkowo dla ka¿dego
	urz¹dzenia istnieje osobny w¹tek wykonuj¹cy cykliczne zapytania o status urz¹dzenia.

	\section sec_async	Zdarzenia asynchroniczne

	\subsection sec_async_events Zdarzenia asynchroniczne

	Drukarki, które maj¹ zaimplementowany nowy protokó³ komunikacyjny POSNET posiadaj¹ mo¿liwoœæ generacji zdarzeñ
	zmiany statusu urz¹dzenia w sposób asynchroniczny w stosunku do wykonywanych poleceñ
	wydawanych przez nadzoruj¹c¹ j¹ aplikacjê steruj¹co-magazynow¹.
	
	Do obs³ugi zdarzeñ w tym trybie zosta³ przygotowany zestaw funkcji
	opisany w module \ref event_handling. Aplikacja g³ówna mo¿e zostaæ 
	powiadomiona o przyjœciu nowego zdarzenia
	za pomoc¹:
	  - zdarzenia IPC typu "Event" systemu Windows,
	  - komunikatu Windows wys³anego do wskazanego okna aplikacji
	
	Aplikacja mo¿e te¿ cyklicznie sprawdzaæ status urzadzenia (polling).

	UWAGA: Z natury dzia³ania drukarki sprawdzenie pe³nego statusu drukarki sk³ada siê
	z dwóch niezaleznych operacji, w zwi¹zku z tym w momencie przejscia drukarki ze stanu
	poprawngo do stanu awarii mechanizmu drukuj¹cego wyst¹pi¹ dwa kolejne zdarzenia 
	asynchroniczne (pierwsze wskazuj¹ce zmiane statusu drukarki, drugie statusu mechanizmu).

	UWAGA: Jeœli status drukarki nie wskazuje na b³¹d mechanizmu drukarki wartoœæ statusu
	mechanizmu jest nieistotna (niewa¿na).

	\section sec_example	Przyk³adowa sekwencja poleceñ

	Obs³uga drukarki wymaga wykonania pewnego zestawu standardowych
	operacji. Poni¿szy opis przedstawia jedn¹ z mo¿liwoœci na
	przyk³adzie prostego paragonu. Przyk³ad jest w jêzyku C++,
	dla czytelnoœci pominiêto obs³ugê sytuacji wyj¹tkowych.

	\code

	\\ Przygotowanie i otwarcie urzadzenia 
	void *hDevice=NULL;
	hDevice=POS_CreateDeviceHandle(type);
	POS_SetDebugLevel(hDevice,POSNET_DEBUG_ALL & 0xFFFFFFFE);
	POS_SetDeviceParam(hDevice,POSNET_DEV_PARAM_COMSETTINGS,(void*)"COM1,9600,8,N,1,H");
	void *hLocalDevice=POS_OpenDevice(hDevice);

	\\ Wys³anie kolejnych rozkazów paragonu
	\\ Wersja z uzupe³nianiem parametrów pojedynczo
	hRequest = POS_CreateRequest(hLocalDevice,"trinit");
	POS_PushRequestParam(hRequest,"bm","1");
	POS_PostRequest(hRequest,POSNET_REQMODE_SPOOL);

	\\ Oczekiwanie na wykonanie rozkazu
	POS_WaitForRequestCompleted(hRequest,5000);

	\\ Sprawdzenie statusu wykonania
	if (POS_GetRequestStatus(hRequest) != POSNET_STATUS_OK) { OBSLUGA_BLEDU }

	\\ Usuniecie rozkazu
	POS_DestroyRequest(hRequest);

	\\ Nastepne polecenia, wersja z tworzeniem rozkazu z parametrami	
	hRequest = POS_CreateRequestEx(hLocalDevice,"trline","na,Bu³ka Standardowa\nvt,0\npr,35");
	POS_PostRequest(hRequest,POSNET_REQMODE_SPOOL);
	POS_WaitForRequestCompleted(hRequest,5000);
	if (POS_GetRequestStatus(hRequest) != POSNET_STATUS_OK) { OBSLUGA_BLEDU }
	POS_DestroyRequest(hRequest);
	hRequest = POS_CreateRequestEx(hLocalDevice,"trpayment","ty,0\nre,0\nwa,35");
	POS_PostRequest(hRequest,POSNET_REQMODE_SPOOL);
	POS_WaitForRequestCompleted(hRequest,5000);
	if (POS_GetRequestStatus(hRequest) != POSNET_STATUS_OK) { OBSLUGA_BLEDU }
	POS_DestroyRequest(hRequest);
	hRequest = POS_CreateRequestEx(hLocalDevice,"trend","to,35\nfp,35");
	POS_PostRequest(hRequest,POSNET_REQMODE_SPOOL);
	POS_WaitForRequestCompleted(hRequest,5000);
	if (POS_GetRequestStatus(hRequest) != POSNET_STATUS_OK) { OBSLUGA_BLEDU }
	POS_DestroyRequest(hRequest);

	\\ zamkniecie urzadzenia
	POS_CloseDevice(hLocalDevice);
	POS_DestroyDeviceHandle(hDevice);

	\endcode

	Biblioteka posiada zestaw aplikacji demonstruj¹cych ró¿ne sekwencje
	poleceñ dla drukarki.

	  -  demo_lowlevel - demonstruje przyk³adow¹ realizacjê pojedynczej transakcji (paragonu)
	  -  demo_requests - prezentuje ró¿ne sposoby obs³ugi parametrów poleceñ oraz sposobu wysy³ania rozkazów i odbioru danych
	  -  demo_async2 - prezentuje realizacjê obs³ugi statusu drukarki za poœrednictwem zdarzeñ (Event) w systemie Windows lub semaforów dla systemu Linux
	  -  demo_async3 - prezentuje realizacjê obs³ugi statusu drukarki za poœrednictwem funkcji typu callback
	  -  demo_all - prezentuje oko³o 20 ró¿nych przydatnych sekwencji drukarki, w tym ró¿ne warianty sprzeda¿y, fakturê VAT, formatki, raporty i ustawienia,
	  demo to obs³uguje zdarzenia nieprzewidywane metod¹ pollingu za poœrednictwem funkcji POS_GetPrnDeviceStatus
	  .
    Kod poszczególnych programów demonstracyjnych zawiera w komenarzach dodatkowe informacje na temat ich dzia³ania.

	\section sec_extern	Wykorzystanie w jêzykach programowania

	W wersji prototypowej biblioteka zosta³a przetestowana w:
	  - VisualC++ .NET 2003
	  - Borland C++ Builder 4
	  - Delphi 7
	  - Visual Basic for Applications (MS Excel)
	  - Ch  (<a href='http://www.softintegration.com'>http://www.softintegration.com</a>)
	
	Poni¿ej przedstawiono przyk³adowy kod dla VBA.
	\verbatim
	Private Declare Function POS_WaitForRequestCompleted Lib "posnet.dll" (ByVal H As Long, ByVal P As Long) As Long
	\endverbatim

	Podobny kod dla Delphi 7

	\verbatim
	function POS_WaitForRequestCompleted (hDevice : THandle; k: longint) : THandle; stdcall; external 'posnet.dll'
	\endverbatim

	\section linux_port Wersja dla systemu Linux

	Natywny port dla systemu Linux sk³ada siê z bibliotek:

	libposcmbth.so.1.0

	oraz 

	libptypes.so.2.0.2 (http://www.melikyan.com/ptypes/)

	Biblioteki te s¹ zale¿ne tak¿e od dostêpnych na licencji LGPL bibliotek obs³uguj¹cych
	podsystem USB:

	libusb - w wersji 0.1.11+ dostêpnej standardowo w wiêkszoœci dystrybucji
	(http://libusb.sourceforge.net/)

	oraz 

	libftdi w wersji 0.7+ (http://www.intra2net.com/de/produkte/opensource/ftdi/index.php)
	do obs³ugi konwertera FTDI zastosowanego w urz¹dzeniu.

	UWAGA!!!  W przypadku korzystania z trybu natywnego USB (POSNET_INTERFACE_USB)
	do poprawnego dzia³ania drukarki niezbêdne jest zablokowanie automatycznego ³adowania
	i usuniêcie jesli jest za³adowany modu³u j¹dra ftdi_sio. W przeciwnym razie po³¹czenie z drukark¹ nie bêdzie
	mo¿liwe (w pliku logu zg³aszany bêdzie b³¹d o kodzie -5), gdy¿ modu³ ten przejmuje kontrolê nad konwerterem
	FTDI tworz¹c wirtualny port szeregowy (/dev/ttyUSBx).

	W przypadku korzystania w trybie wirtualnego portu szeregowego, sterownik ftdi_sio jest potrzebny.


	\section sec_license Informacje o licencjach komponentów

	<hr>

	Biblioteka wykorzystuje komponenty o nastêpuj¹cych licencjach:

	Copyright (c) 2005, The EasyBMP Project (http://easybmp.sourceforge.net)
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	<hr>

	
  C++ Portable Types Library (PTypes)
  
  Copyright (C) 2001-2007 Hovik Melikyan

  http://www.melikyan.com/ptypes/


  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software.  If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.


  Hovik Melikyan
  h@melikyan.com

  <hr>

//	Serial.h - Definition of the CSerial class
//
//	Copyright (C) 1999-2003 Ramon de Klein (Ramon.de.Klein@ict.nl)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


*/


/*!
	\ingroup global_defs
	\brief	Standardowy sposób obs³ugi bibliotek DLL
*/
/* Patch dla Borland C++ Builder'a */
#ifdef __WIN32__
#define WIN32 
#endif

#ifdef WIN32
#ifdef POSNET_EXPORTS
#define POSNET_API  __declspec(dllexport)	
#else
#define POSNET_API  __declspec(dllimport)	
#endif
#else
#define POSNET_API
#define __stdcall
#define __cdecl
#endif

extern "C"
{

	/*!
		\defgroup	global_defs Definicje globalne
		@{
		
	*/
	/*!
	\brief Definicja typu uchwytu urz¹dzenia.
	*/
	typedef  void*					POSNET_HANDLE;
	/*!
	\brief Definicja typu wartoœci zwracanej jako status.
	*/
	typedef	 unsigned long			POSNET_STATUS;
	/*!
	\brief Definicja typu wartoœci zwracanej jako stan rozkazu.
	*/
	typedef	 unsigned long			POSNET_STATE;
	/*!
	\brief Definicja minimalnego rozmianu bufora znakowego nazwy polecenia, nazwy parametru
	*/
	#define	POSNET_NAMELENGTH_MAX	16

	/*!
	\brief	Pobierz wersjê biblioteki

	\return	Wersja jako liczba 32 bitowa. Liczba ta sk³ada siê z trzech czêœci. Najstarsze 8 bitów to g³ówny
	 numer wersji, nastêpne 8 to podrzêdny numer wersji, oststanie 16 bitów to numer kompilacji.
	*/

	POSNET_API unsigned long __stdcall POS_GetLibraryVersion();


	/*!
		@}
	*/


	/*! \defgroup debug_level	Poziomy informacyjnoœci
		\ingroup debugging
	@{
	*/
	#define	POSNET_DEBUG_NONE				0x00000000  //!< Brak rejestrowania informacji.
	#define POSNET_DEBUG_ALL				0xFFFFFFFF	//!< Rejestracja wszystkich informacji we wszystkich podsystemach
	#define POSNET_SUBSYSTEM_DEVICE			0x00001000	//!< Podsystem urz¹dzenia
	#define POSNET_SUBSYSTEM_DEVICERS232	0x00002000  //!< Podsystem RS232
	#define POSNET_SUBSYSTEM_DEVICEETHER	0x00004000  //!< Podstsystem Ethernet
	#define	POSNET_SUBSYSTEM_DEVICEUSB		0x00008000  //!< Podsystem USB (u¿ywa sterownika D2XX, w przypadku korzystania ze
														//!< sterownika VCP nale¿y korzystaæ z trybu RS232
	#define	POSNET_SUBSYSTEM_FRAME			0x00010000  //!< Podsystem ramki - wyœwietla zawartoœæ wysy³anych i odbieranych ramek
														//!< w formacie hexadecymalnym - mo¿liwoœæ konwersji na postaæ binarn¹ do³¹czonym konwerterem
	#define POSNET_DEBUG_EXTRA				0x00800000  //!< Rejestracja szczegó³owych informacji o stanach rozkazów
	/*!
	@}
	*/

	/*!
		\defgroup	debugging	Ustawienie poziomu œledzenia (informacyjnoœci) biblioteki
		@{
	*/

	/*!
		\brief Ustaw poziom informacyjnoœci biblioteki.

		System debuggingu opiera siê na tzw. "podsystemach".\n
		Definicje POSNET_DEBUG_SUBSYSTEM_* definiuj¹ te podsystemy.\n
		S¹ one maskami bitowymi, które zsumowane (OR) daja mo¿liwoœæ 
		jednoczesnego rejestrowania przep³ywu danych w róznych podsystemach.\n
		4 najm³odsze bity definiuj¹ poziom iloœci wysy³anych danych (sta³e
		POSNET_DEBUG_LEVEL_*).
		
		Ustawienie poziomu debuggingu na wartoœæ ró¿n¹ od 0 powoduje otwarcie
		aktualnie wybranego pliku (domyœlnie "POS_DBG.txt", mo¿na ustawiæ przy
		wykorzystaniu \ref POS_SetDebugFileName . W pliku tym zostan¹ zapisane
		odpowiednie informacje umo¿liwiaj¹ce œledzenie wykonania kodu biblioteki.
		Ponowne ustawienie poziomu debuggingu zamyka plik œledz¹cy.
		
		Maksymalna iloœæ podsystemów okreœlona zosta³a na 28.
		\param hGlobalDevice	Uchwyt stworzonego po³¹czenia z urz¹dzeniem
		\param debugLevel	Okreœlenie poziomu œledzenia
	*/
	POSNET_API	void	__stdcall POS_SetDebugLevel(POSNET_HANDLE hGlobalDevice, unsigned long debugLevel);
	/*!
		\brief	Ustawienie nazwy pliku œledz¹cego.

		Funkcja umo¿liwia zmianê domyœlnego pliku œledz¹cego.
		\param hGlobalDevice	Uchwyt stworzonego po³¹czenia z urz¹dzeniem
		\param fileName nowa nazwa pliku
	*/
	POSNET_API  void	__stdcall POS_SetDebugFileName(POSNET_HANDLE hGlobalDevice, const char *fileName);

	/*!
		@}
	*/



	/*! 
	*	\addtogroup dev_types Rodzaje interfejsu urz¹dzenia
		\ingroup basic_api

		@{
	*/

	/*!
		\brief	Pod³¹czenie przez RS232
	*/
	#define POSNET_INTERFACE_RS232	0x0001	
	/*!
		\brief	Pod³¹czenie przez USB

		Pod³¹czenie przez  USB mo¿e byæ wykonane na dwa sposoby:
		-  korzystaj¹c ze sterownika FTDI VCP (Virtual ComPort), gdzie tworzony jest dodatkowy, virtualny port szeregowy (odpowiednik RS232),
		w tym przypadku nale¿y korzystaæ z trybu RS232.
		-  przy wykorzystaniu sterownika D2XX, korzysta siê z trybu USB. Nale¿y w tym przypadku podaæ zaprogramowany w interfejsie numer seryjny
		drukarki.
		.

	*/
	#define POSNET_INTERFACE_USB	0x0002
	/*!
		\brief	Pod³¹czenie przez Ethernet

		Biblioteka, dla urz¹dzeñ sieciowych w systemie Windows nie wywo³uje WSA_Startup. Obowi¹zek inicjalizacji podsystemu
		Winsock spoczywa na aplikacji wykorzystuj¹cej bibliotekê. Brak inicjalizacji podsystemu Winsock objawia siê b³êdem w logach
		biblioteki: DEVICE_Ether: Socket Error (1)- 10093. 
	*/
	#define POSNET_INTERFACE_ETH	0x0003

	/*! @}

	*/


	/*!
		\defgroup dev_params	Identyfikatory parametrów urz¹dzenia
		\ingroup basic_api
		@{
	*/
	
	/*!
		\brief	Parametry portu szeregowego

		Zapis/Odczyt
		
		Parametry portu przekazuje siê jako ci¹g znaków w formacie
		port,baud rate,bits,parity,stopbits,flowcontrol np. "COM1,9600,8,N,1,H"
		Flowcontrol: (N)one, (S)oftware XON/XOFF, (H)ardware RTS/CTS+DTR/DSR, (R)Hardware RTS/CTS, (D)Hardware DTR/DSR
	*/
	#define	POSNET_DEV_PARAM_COMSETTINGS	0x00020001

	/*!
		\brief Czas w [s] po jakim ma byæ zaniechane wysy³anie ramki

		TYLKO ZAPIS

		Parametr - wskaŸnik na wartoœc long
	*/
	#define POSNET_DEV_PARAM_SENDTIMEOUT		0x00020004


	/*!
		\brief Numer seryjny drukarki do otwarcia przez typ urz¹dzenia \ref POSNET_INTERFACE_USB
		
		TYLKO ZAPIS

		Parametr - wskaŸnik na ci¹g znaków jêzyka C (zakoñczony 0) zawieraj¹cy numer seryjny.

	*/
	#define POSNET_DEV_PARAM_USBSERIAL			0x00020007

	/*!

		\brief Odczyt wszystkich numerów seryjnych drukarek pod³¹czonych do komputera poprzez interfejs USB i sterownik
				FTDI - D2XX

		TYLKO ODCZYT

		Parametr - bufor na numery seryjne rozdzielone znakiem koñca linii - 
					(ka¿dy numer ma max. 8znaków+2 bajty - koniec linii = 10 bajtów * max. 127 urz¹dzeñ na USB = 1270)
					w zwi¹zku z tym zalecany jest rozmiar bufora wiêkszy lub równy 1271 znaków.

	*/

	#define POSNET_DEV_PARAM_LISTUSBSERIALS		0x00020008

	/*!

	\brief D³ugoœæ kolejki wysy³kowej, po przekroczeniu, której rozkazy traktowane s¹ jak wysy³ane w trybie natychmiastowym.

	TYLKO ZAPIS

	Parametr - wskaŸnik na liczbê typu unsigned long (32 bit) zawieraj¹c¹ ¿¹dan¹ d³ugoœæ kolejki wysy³kowej.

	*/

	#define POSNET_DEV_PARAM_OUTQUEUELENGTH		0x00020009
	/*!
	\brief Interwa³ pomiêdzy automatycznymi odpytaniami o status drukarki

	TYLKO ZAPIS

	Parametr - wskaŸnik na liczbê typu unsigned long (32 bit) zawieraj¹c¹ ¿¹dany czas pomiêdzy zapytaniami w sekundach.
	*/
	#define POSNET_DEV_PARAM_STATUSPOLLINGINTERVAL		0x0002000A


	/*!
	\brief	Pobranie uchwytu portu szeregowego

	TYLKO ODCZYT

	Parametr - Windows - wskaŸnik na HANDLE, Linux - wskaŸnik na liczbê typu int
	*/

	#define POSNET_DEV_PARAM_FILEHANDLE		0x0002000E	

	/*!
		\brief Adres IP kasy dla urz¹dzenia typu \ref POSNET_INTERFACE_ETH

		TYLKO ZAPIS

		Parametr - wskaŸnik na ci¹g znaków jêzyka C (zakoñczony 0) zawieraj¹cy adres IP.
	*/
	#define POSNET_DEV_PARAM_IP					0x00020005
	/*!
		\brief Port dla protoko³u TCP/IP dla urz¹dzenia typu \ref POSNET_INTERFACE_ETH

		TYLKO ZAPIS

		Parametr - wskaŸnik na ci¹g znaków jêzyka C (zakoñczony 0) zawieraj¹cy numer portu.
	*/
	#define POSNET_DEV_PARAM_IPPORT				0x00020006


	/*!
		@}
	*/


	/*!
		\defgroup	basic_api	Obs³uga podstawowa urz¹dzenia
		@{
	*/

	/*!
		\brief	Utworzenie uchwytu do nowego urz¹dzenia. 
		
		Tworzy odpowiedni obiekt i ustawia
		niebêdne jego w³aœciwoœci, ale fizycznie go nie otwiera. Przed otwarciem urz¹dzenia 
		nale¿y ustawiæ niebêdne parametry, które s¹ rózne w zale¿noœci od typu urz¹dzenia.
		\param	deviceType	Typ pod³¹czonego urz¹dzenia \ref dev_types "Typy pod³¹czenia"
		\return Uchwyt globalny utworzonego urz¹dzenia. Wartoœc NULL zostanie zwrócona wy³¹cznie
				w przypadku braku pamiêci lub podania b³êdnego typu urz¹dzenia.
	*/
	POSNET_API POSNET_HANDLE	__stdcall POS_CreateDeviceHandle(unsigned long deviceType);

	/*!
		\brief	Otwórz urz¹dzenie.
		
		\param hGlobalDevice	Uchwyt globalny urzadzenia do otwarcia.
		\return Zwraca uchwyt lokalny do urz¹dzenia w kontekœcie bie¿¹cego w¹tku. NULL w przypadku b³êdu,
				kod b³êdu mo¿na odczytaæ funkcj¹ POS_GetError
	*/
	POSNET_API POSNET_HANDLE	__stdcall POS_OpenDevice(POSNET_HANDLE hGlobalDevice);		  
	/*!
		\brief	Zamknij urz¹dzenie. 
		
		Zamyka urzadzenie wskazywane przez uchwyt. Usuwa zawartoœci kolejek
		komunikacyjnych. Wszytskie dane nale¿y odczytaæ przed zamkniêciem urz¹dzenia.

		\param hLocalDevice	Uchwyt urzadzenia do zamkniêcia.
	*/
	POSNET_API POSNET_STATUS	__stdcall POS_CloseDevice(POSNET_HANDLE hLocalDevice);
	/*!
		\brief	Usuñ istniej¹cy uchwyt urz¹dzenia i zwolnij zajmowan¹ pamiêæ.

		Ostatecznie usuwa obiekt komunikacyjny i zwalnia pamiêæ.

		\param hGlobalDevice	Uchwyt globalny urz¹dzenia do usuniêcia. Wszystkie w¹tki musz¹ najpierw zamkn¹æ urzadzenie,
								w przeciwnym wypadku funkcja zwróci b³¹d POSNET_STATUS_BUSY.
	*/
	POSNET_API POSNET_STATUS	__stdcall POS_DestroyDeviceHandle(POSNET_HANDLE hGlobalDevice); 
	/*!
		\brief	Funkcja do ustawiania parametrów urz¹dzenia

		\param hDevice Uchwyt urz¹dzenia - zarówno lokalny jak i globalny
		\param paramCode Kod parametru por. \ref dev_params kody parametrów
		\param paramValue WskaŸnik lub wartoœæ parametru. Opis przy parametrach urz¹dzenia.

	*/
	POSNET_API POSNET_STATUS	__stdcall POS_SetDeviceParam(POSNET_HANDLE hDevice, unsigned long paramCode,void *paramValue);	
	/*!
		\brief	Funkcja do pobierania parametrów urz¹dzenia

		\param hDevice Uchwyt urz¹dzenia - zarówno lokalny jak i globalny
		\param paramCode Kod parametru por. \ref dev_params kody parametrów
		\param paramValue WskaŸnik lub wartoœæ parametru. Opis przy parametrach urz¹dzenia.

	*/
	POSNET_API POSNET_STATUS	__stdcall POS_GetDeviceParam(POSNET_HANDLE hDevice, unsigned long paramCode,void *paramValue);	



	/*!
		@}
	*/

	/*! \defgroup error_codes	Kody b³êdów biblioteki
		\ingroup	error_handling

		\brief	Kody b³êdów biblioteki. Biblioteka mo¿e zwracaæ kody
				b³êdów zarówno w³asne jak i kody bezpoœrednio uzyskane
				z drukarki.
		@{
	*/
	
	#define POSNET_STATUS_OK						0x00000000 //!< Brak b³êdu
	#define POSNET_STATUS_OUTOFMEMORY				0x00000001 //!< Brak pamiêci
	#define POSNET_STATUS_FRAMETOOSHORT				0x00000002 //!< Za krótka ramka
	#define POSNET_STATUS_FRAMINGERROR				0x00000003 //!< B³¹d ramki odebranej z drukarki
	#define POSNET_STATUS_COULDNOTOPEN				0x00000005 //!< Nie mo¿na otworzyc wskazanego urz¹dzenia
	#define POSNET_STATUS_CRCERROR					0x00000006 //!< B³¹d CRC w odebranej ramce
	#define POSNET_STATUS_IPCERROR					0x00000007 //!< B³¹d utworzenia obiektu IPC (Event)
	#define POSNET_STATUS_COMMERROR					0x00000008 //!< B³¹d komunikacji
	#define POSNET_STATUS_USBERROR					0x00000009 //!< B³¹d krytyczny USB - urz¹dzenie nie bêdzie funkcjonowaæ poprawnie
	#define POSNET_STATUS_FTLIBIMPORTFAIL			0x0000000A //!< Nieudany import sterownika FTDI
	#define POSNET_STATUS_COULDNOTSETUPPORT			0x0000000B //!< B³¹d ustawienia parametrów otwieranego portu
	#define POSNET_STATUS_COULDNOTOPEN_ACCESSDENIED 0x0000000C //!< B³¹d otwarcia urz¹dzenia - dostêp zabroniony
	#define POSNET_STATUS_COULDNOTOPEN_FILENOTFOUND 0x0000000D //!< B³¹d otwarcia urz¹dzenia - brak takiego pliku (urz¹dzenia)
	#define POSNET_STATUS_SETUP_INVALIDBAUD			0x0000000E //!< B³êdne parametry portu - baudrate
	#define POSNET_STATUS_SETUP_INVALIDDATA			0x0000000F //!< B³êdne parametry portu - databits
	#define POSNET_STATUS_SETUP_INVALIDPARITY		0x00000010 //!< B³êdne parametry portu - parity
	#define POSNET_STATUS_SETUP_INVALIDSTOP			0x00000011 //!< B³êdne parametry portu - stop bits
	#define POSNET_STATUS_SETUP_INVALIDHANDSHAKE	0x00000012 //!< B³êdne parametry portu - handshake
	#define POSNET_STATUS_INVALIDSTATE				0x00000013 //!< Wydano polecenie REPEAT dla ramki znajduj¹cej siê w niew³aœciwym stanie (innym ni¿ SENT lub ACK)
	#define POSNET_STATUS_DEVICE_BUSY				0x00000014 //!< Urz¹dzenie zajête

	#define	POSNET_STATUS_BUSY						0x00000020 //!< Urz¹dzenie zajête

	#define POSNET_STATUS_COULDNOTOPEN_WSAEINTR  		0x00000030 //!< B³¹d sieciowy - przerwane wywo³anie systemowe
	#define POSNET_STATUS_COULDNOTOPEN_WSAEACCES 		0x00000031 //!< B³¹d sieciowy - brak dostêpu
	#define POSNET_STATUS_COULDNOTOPEN_WSAEINPROGRESS 	0x00000032 //!< B³¹d sieciowy - operacja w toku
	#define POSNET_STATUS_COULDNOTOPEN_WSAEDESTADDRREQ  0x00000033 //!< B³¹d sieciowy - wymagany adres docelowy
	#define POSNET_STATUS_COULDNOTOPEN_WSAEADDRINUSE 	0x00000034 //!< B³¹d sieciowy - adres w u¿yciu
	#define POSNET_STATUS_COULDNOTOPEN_WSAEADDRNOTAVAIL	0x00000035 //!< B³¹d sieciowy - adres nieprawid³owy
	#define POSNET_STATUS_COULDNOTOPEN_WSAENETDOWN 		0x00000036 //!< B³¹d sieciowy - sieæ jest wy³¹czona
	#define POSNET_STATUS_COULDNOTOPEN_WSAENETUNREACH 	0x00000037 //!< B³¹d sieciowy - sieæ jest nieosi¹galna
	#define POSNET_STATUS_COULDNOTOPEN_WSAENETRESET 	0x00000038 //!< B³¹d sieciowy - sieæ roz³¹czy³a po³¹czenie
	#define POSNET_STATUS_COULDNOTOPEN_WSAECONNABORTED  0x00000039 //!< B³¹d sieciowy - po³¹czenie zerwane przez aplikacjê
	#define POSNET_STATUS_COULDNOTOPEN_WSAECONNRESET 	0x0000003A //!< B³¹d sieciowy - strona zdalna zerwa³a po³¹czenie
	#define POSNET_STATUS_COULDNOTOPEN_WSAETIMEDOUT 	0x0000003B //!< B³¹d sieciowy - up³yn¹³ czas oczekiwania na odpowiedŸ
	#define POSNET_STATUS_COULDNOTOPEN_WSAECONNREFUSED 	0x0000003c //!< B³¹d sieciowy - po³¹czenie odrzucone
	#define POSNET_STATUS_COULDNOTOPEN_WSAEHOSTDOWN  	0x0000003d //!< B³¹d sieciowy - serwer zdalny jest wy³¹czony
	#define POSNET_STATUS_COULDNOTOPEN_WSAEHOSTUNREACH 	0x0000003e //!< B³¹d sieciowy - serwer zdalny jest nieosi¹galny
	#define POSNET_STATUS_COULDNOTOPEN_WSAHOSTNOTFOUND 	0x0000003f //!< B³¹d sieciowy - serwer nieznaleziony
	#define POSNET_STATUS_COULDNOTOPEN_WSATRYAGAIN  	0x00000040 //!< B³¹d sieciowy - serwer nieznaleziony, spróbuj ponownie

	#define POSNET_STATUS_ADVBMP_READERROR				0x00000070 //!< B³¹d odczytu pliku bitmapy do zaprogramowania
	#define POSNET_STATUS_ADVBMP_BADSIZE				0x00000071 //!< B³¹d rozmiaru bitmapy


	#define POSNET_STATUS_ALREADY_COMPLETED		0x00010000 //!< Rozkaz ju¿ wykonany \sa POS_CancelRequest
	#define POSNET_STATUS_EMPTY					0x00010001 //!< Brak danych w kolejce
	#define	POSNET_STATUS_INVALIDVALUE			0x00010002 //!< B³êdna wartoœæ
	#define	POSNET_STATUS_TIMEOUT				0x00010003 //!< Oczekiwanie zakoñczone up³yniêciem czasu (timeout)
	#define	POSNET_STATUS_PENDING				0x00010004 //!< Polecenie w trakcie wykonywania
	#define POSNET_STATUS_INVALIDCOMMAND		0x00010005 //!< B³êdny numer polecenia
	#define POSNET_STATUS_INVALIDHANDLE			0x00010006 //!< B³êdny uchwyt
	#define POSNET_STATUS_BUFFERTOOSHORT		0x00010007 //!< Przekazany bufor znakowy jest za ma³y
	#define POSNET_STATUS_OUTOFRANGE			0x00010008 //!< Poza zakresem licznika
	#define POSNET_STATUS_INVALIDSPOOLMODE		0x00010009 //!< B³êdny tryb kolejkowania
	#define POSNET_STATUS_CANCELLED				0x0001000A //!< Rozkaz anulowany

	#define POSNET_STATUS_INVALID_PARAM1		0x00010101 //!< B³êdny 1 parametr polecenia
	#define POSNET_STATUS_INVALID_PARAM2		0x00010102 //!< B³êdny 2 parametr polecenia
	#define POSNET_STATUS_INVALID_PARAM3		0x00010103 //!< B³êdny 3 parametr polecenia
	#define POSNET_STATUS_INVALID_PARAM4		0x00010104 //!< B³êdny 4 parametr polecenia
	#define POSNET_STATUS_INVALID_PARAM5		0x00010105 //!< B³êdny 5 parametr polecenia
	#define POSNET_STATUS_INVALID_PARAM6		0x00010106 //!< B³êdny 6 parametr polecenia

	#define	POSNET_STATUS_CASHREGBASE			0x00430000 //!< Bazowy kod b³êdu wykonania rozkazu drukarki [b³êdy 0x00430000 - 0x0043FFFF]
	#define	POSNET_STATUS_CASHREGCOMMBASE		0x00440000 //!< Bazowy kod b³êdu transmisji rozkazu do drukarki [b³êdy 0x00440000 - 0x0044FFFF]
	/*!
		@}
	*/


	/*!
		\defgroup	error_handling	Obs³uga b³êdów
		@{
	*/

	/*!
		\brief	Podaj kod statusu zwi¹zany z uchwytem urz¹dzenia
		
		Zwraca ostatnio ustawiony kod statusu zwi¹zany z uchwytem urz¹dzenia. 
		Ka¿da wywo³ywana funkcja oprócz zwrócenia tej wartoœci jako wyniku dzia³ania
		(o ile nie zwraca innego rodzaju rezultatu)	ustawia wewnêtrzny znacznik b³êdu. 
		\param hLocalDevice Uchwyt lokalny urz¹dzenia lub uchwyt globalny urz¹dzenia.
		\return Kod statusu patrz \ref error_codes Kody b³êdów.
	*/
	POSNET_API POSNET_STATUS	__stdcall POS_GetError(POSNET_HANDLE hLocalDevice);	
	/*!
		\brief	Zwróæ tekstowy opis b³êdu

		Funkcja zwraca tekstowy opis b³êdu zrozumia³y dla u¿ytkownika / programisty.
		Komunikat mo¿e byæ zwrócony we wskazanym jêzyku (o ile przygotowano
		niezbêdny plik jêzykowy). W przeciwnym wypadku komunikat zwracany jest
		w domyœlnym jêzyku biblioteki. 

		<em>UWAGA:</em> W systemie Linux pliki z tekstowymi opisami kodów b³êdów (pliki jêzykowe
		o nazwach w postaci posnet_xx.lng mog¹ byæ umieszczone w katalogu, z którego
		uruchomiono program (nie zalecane) lub w miejscu wskazywanym przez zmienn¹
		œrodowiskow¹ POSNET_LIB_SHARE_DIR (zalecane).
		W systemie WINDOWS katalog wskazany w POSNET_LIB_SHARE_DIR musi byæ zakoñczony 
		znakiem „\” rozdzielaj¹cym katalogi np. POSNET_LIB_SHARE_DIR=c:\myapp\bin\

		\param code	Kod b³êdu do wyjaœnienia
		\param lang	2 znakowy kod jêzyka wg ISO-3166
		\return Ci¹g znaków opisuj¹cych zdarzenia (C-string) zakoczñony znakiem 0. Ci¹g ten zwracany
				jest z wewnêtrznego bufora. Nie wolno go zwalniaæ funkcjami allokacji pamiêci.
	*/
	POSNET_API const char *			__stdcall POS_GetErrorString(POSNET_STATUS code, char *lang);
	/*!
		\brief	Zwróæ status rozkazu

		UWAGA: W celu efektywnego zarz¹dzania b³êdami (w sytuacji, gdy drukarka zwraca b³êdy w róznych postaciach)
		zarz¹dzanie b³êdami zosta³o zunifikowane - nazwa rozkazu zostaje umieszczona w polu "cm" odpowiedzi, 
		kod b³êdu natomiast umieszczany jest tak¿e w polu o nazwie "!error" odpowiedzi. 
		Dziêki temu postaæ odpowiedzi jest identyczna w przypadku b³êdów ramki i b³êdów.

		\param hRequest Uchwyt rozkazu (¿¹dania wychodz¹cego lub odpowiedzi z drukarki)
		\return Funkcja zwraca kod b³êdu zwi¹zany z rozkazem. W przypadku odebrania odpowiedzi o b³êdzie 
				wykonania rozkazu zwracane s¹ kody odebrane od drukarki, s¹ one tak¿e dostêpne w polach rezultatu
				komendy w postaci zgodnej z Instrukcj¹ Programisty.
				Kody o wartoœciach 0x00000000-0x0042FFFF s³u¿¹ do przekazywania stanu rozkazu w ramach biblioteki.
				Kody o wartoœciach 0x00430000-0x0043FFFF wykorzystywane s¹ w przypadku negatywnej odpowiedzi drukarki na rozkaz, po jego zakoñczeniu w wyniku b³êdu drukarki – kod b³êdu drukarki zwiêkszony jest o wartoœæ 0x00430000.
				Kody o wartoœciach 0x00440000-0x0044FFFF wykorzystywane s¹ w przypadku negatywnej odpowiedzi drukarki na rozkaz, po jego zakoñczeniu w wyniku b³êdu transmisji – kod b³êdu transmisji zwiêkszony jest o wartoœæ 0x00440000.
		\sa sync_api "Obs³uga synchroniczna drukarki"
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_GetRequestStatus(POSNET_HANDLE hRequest);

	/*!
		\brief Pobranie statusu drukarki. Drukarka jest periodycznie odpytywana o status i rezultat tej operacji jest przechowywany.
		Funkcja jest blokuj¹ca jeœli wykonywane jest odpytywanie.

		\param hLocalDevice Lokalny uchwyt urz¹dzenia
		\param statusMode Sposób uzyskania statusu - 0 - zwróæ ostatni status automatyczny , 1 - odpytaj urz¹dzenie jeœli brak statusu - blokuj¹ce!
		\param globalStatus Status urz¹dzenia zwracany przez rozkaz sdev (-1 jeœli nie odpytano)
		\param printerStatus Status mechanizmu drukuj¹cego  (nieistotny jeœli sdev=-1)
		\return Funkcja zwraca status wykonania rozkazu, oraz wype³nia pola parametrów globalStatus i printerStatus
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_GetPrnDeviceStatus(POSNET_HANDLE hLocalDevice,const char statusMode,long *globalStatus,long *printerStatus);

	/*!
		@}
	*/



	// --------------------------------------------------------------------------------------------------------
	/*! \defgroup event_types Typy zdarzeñ
		\ingroup	event_handling
		@{
	*/
	/*!
		\brief	Wy³¹czenie obs³ugi zdarzeñ asynchronicznych - w tym trybie
		wszystkie zdarzenia s¹ automatycznie usuwane, a aplikacja nie jest
		o nich informowana.
	*/
	#define POSNET_EVENT_DISABLE	0x0000
#ifdef _WIN32
	/*!
		\brief	Sygnalizacja otrzymania zdarzenia za pomoc¹ komunikacji miêdzyprocesowej
		typu "Event" (zdarzenie) w systemie Windows.
		Po wybraniu tego typu obs³ugi za pomoc¹ funkcji \ref POS_GetEvent mo¿na
		uzyskaæ uchwyt zdarzenia zarezerwowanego przez urz¹dzenie. 

		Tylko dla WIN32
	*/
	#define POSNET_EVENT_EVENT		0x0001
	/*!
		\brief	Sygnalizacja otrzymania zdarzenia za pomoc¹ komunikatów systemu Windows.
		Po wybraniu tego trybu nale¿y za pomoc¹ funkcji \ref POS_SetMessageParams
		ustawiæ kod komunikatu i uchwyt okna do którego ma on byæ wys³any. Wysy³any
		komunikat ma jako lParam umieszczony uchwyt urz¹dzenia, które go wygenerowa³o.

		Tylko dla WIN32
	*/
	#define	POSNET_EVENT_MESSAGE	0x0002
#else
	/*!
		\brief Sygnalizacja otzrymania zdarzenia za pomoca semafora pthread

		Tylko dla Linux
	*/	
	#define POSNET_EVENT_SEMAPHORE	0x0005

#endif
	/*!
		\brief	Tryb obs³ugi poprzez wywo³anie funkcji w momencie otrzymania
		pakietu danych asynchronicznych.
	*/
	#define POSNET_EVENT_CALLBACK	0x0004
	/*! 
		@}
	*/

	/*!
		\defgroup	event_handling	Obs³uga zdarzeñ asynchronicznych statusu drukarek
		@{
	*/


	/*!
		\brief Ustaw metodê obs³ugi zdarzeñ asynchronicznych 

		Drukarka ma mo¿liwoœæ generacji zdarzeñ asynchronicznych w postaci zmian statusu drukarki, które musz¹ byæ przes³ane do aplikacji. 
		Tego typu zdarzenia mo¿na przes³aæ do aplikacji w nastêpuj¹cy sposób:
		- za pomoc¹ IPC typu "Event"
		- za pomoc¹ komunikatów systemu Windows (Windows Messages)
		- za pomoc¹ systemu pollingu, gdzie aplikacja odpytuje o status
		
		Funkcja s³u¿y to ustawienia trybu sygnalizacji zdarzeñ.
		\param hLocalDevice	Uchwyt lokalny urz¹dzenia. Funkcja pomimo pobierania uchwytu lokalnego ma zasiêg
				globalny dla urz¹dzenia.
		\param eventMode Typ sygnalizacji zdarzeñ definiowany przez \ref event_types "Typy zdarzeñ"
		\return Status wykonania funkcji (POSNET_STATUS_OK w przypadku sukcesu)
	*/
	POSNET_API POSNET_STATUS	__stdcall POS_SetEventHandlingMode(POSNET_HANDLE hLocalDevice, unsigned long eventMode);
#ifdef _WIN32
	/*!
		\brief	Funkcja ustawia kod komunikatu i uchwyt okna do którego ma byæ on wys³any
				przy obs³udze zdarzeñ asynchronicznych. Tylko dla Windows. Jako lParam takiego komunikatu wysy³any jest
				z³o¿ony status urz¹dzenia (w/g wzoru: (Status urz¹dzenia * 65536)+status mechanizmu )
		\param hLocalDevice	Uchwyt lokalny urz¹dzenia. Funkcja pomimo pobierania uchwytu lokalnego ma zasiêg
				globalny dla urz¹dzenia.
		\param	messageCode	Kod komunikatu (WM_xxxx)
		\param	hWnd	Uchwyt okna

		Tylko dla WIN32
	*/
	POSNET_API POSNET_STATUS	__stdcall POS_SetMessageParams(POSNET_HANDLE hLocalDevice, UINT messageCode, HANDLE hWnd);
	/*!
		\brief	Funkcja zwraca uchwyt do zdarzenia. Obiekt zdarzenia jest zarz¹dzany przez
				bibliotekê. Zdarzenie pracuje w trybie Manual Reset 
				patrz \sa http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/createevent.asp MSDN.
		\param hLocalDevice	Uchwyt lokalny urz¹dzenia. Funkcja pomimo pobierania uchwytu lokalnego ma zasiêg
				globalny dla urz¹dzenia.
		\return	NULL w przypadku b³êdu, w przeciwnym przypadku uchwyt zdarzenia (Event).

		Tylko dla WIN32
	*/
	POSNET_API HANDLE			__stdcall POS_GetEvent(POSNET_HANDLE hLocalDevice);
#else
	/*!
		\brief Pobranie semafora synchronizuj¹cego w¹tki pthread (typ sem_t)
		\param hLocalDevice Uchwyt lokalny urz¹dzenia. Funkcja pomimo pobierania uchwytu lokalnego ma zasiêg
				globalny dla urz¹dzenia.
		\return NULL w przypadku b³êdu, w przeciwnym wypadku wskaŸnik na semafor.

		Tylko dla Linux
	*/
	POSNET_API void * __stdcall POS_GetSemaphore(POSNET_HANDLE hLocalDevice);

#endif
	/*!
		\brief Typ definiuj¹cy funkcjê callback dla zdarzeñ asynchronicznych. Przyjmuje jeden parametr
		- z³o¿ony status urz¹dzenia (w/g wzoru: (Status urz¹dzenia * 65536)+status mechanizmu ) . 
	*/
	typedef void (POSNET_CALLBACK_T)(unsigned long status);

	/*!
		\brief Funkcja ustawia w bibliotece funkcjê callback, która bêdzie wywo³ywana po otrzymaniu zdarzenia asynchronicznego.
		\param hLocalDevice Uchwyt lokalny urz¹dzenia. Funkcja pomimo pobierania uchwytu lokalnego ma zasiêg
				globalny dla urz¹dzenia.
		\param callback wskaŸnik na funkcjê typu callback.
	*/
	POSNET_API POSNET_STATUS	__stdcall POS_SetCallback(POSNET_HANDLE hLocalDevice, POSNET_CALLBACK_T *callback);

	/*!
		@}
	*/
	// --------------------------------------------------------------------------------------------------------


	/* MAIN COMMAND API */

	/*!
		\defgroup	req_modes	Tryby wysy³ania rozkazów
		\ingroup sync_api
		@{
	*/

	#define	POSNET_REQMODE_SPOOL	0x00 //!< Tryb kolejkowania [domyœlny], w tym trybie rozkaz umieszczany jest na 
										 //!< koñcu kolejki rozkazów do wys³ania, za wyj¹tkiem nastêpuj¹cych komend,
										 //!< które ze swej natury umieszczane s¹ na samym pocz¹tku kolejki:
										 //!< !sdev, !sprn
	#define POSNET_REQMODE_IMMEDIATE 0x01 //!< Tryb bezpoœredni. Podobny do \ref POSNET_REQMODE_SPOOL kolejkowania, lecz
										  //!< w przypadku, gdy w kolejce s¹ ju¿ jakieœ rozkazy zwraca b³¹d - efektywnie
										  //!< umo¿liwia umieszczenie rozkazu w kolejce WY£¥CZNIE gdy jest ona pusta,
										  //!< za wyj¹tkiem rozkazów !sdev i !sprn
	#define	POSNET_REQMODE_SPOOLSPECIAL	0x02 //!< Tryb kolejkowania specjalnego, w którym
											 //!< w kolejce odbiorczej odpowiedŸ OK (brak b³êdu)
											 //!< na rozkaz nie zwracaj¹cy wyniku zostanie automatycznie
											 //!< usuniêta z kolejki odbiorczej. Ten tryb musi byæ u¿ywany 
											 //!< jedynie w trybie odbioru z kolejki odbiorczej (nie wolno
											 //!< stosowaæ \ref POS_WaitForRequestCompleted POS_WaitForRequestCompleted, gdy¿
											 //!< nie ma gwarancji poprawnoœci uchwytu urz¹dzenia - nie dotyczy to rozkazów
											 //!< wykonanych znajduj¹cych siê w kolejce odbiorczej).
	#define POSNET_REQMODE_AUTOCLEAR 0x03 //!< Ten tryb powoduje automatyczne czyszczenie odpowiedzi,
										  //!< bez wzglêdu na kod powrotu. W momencie wys³ania rozkazu w tym trybie nale¿y uznaæ,
										  //!< ¿e uchwyt rozkazu jest b³êdny.

	/*!
		@}
	*/
	
	/*!
		\defgroup	req_states	Stany rozkazu
		\ingroup sync_api
		@{
	*/

	#define POSNET_RSTATE_NEW			0x00019000	//!< Nowe polecenie, nie umieszczone w kolejce
	#define POSNET_RSTATE_PENDING		0x00019001	//!< Nowe polecenie, umieszczone w kolejce
	#define POSNET_RSTATE_SENT			0x00019002	//!< Polecenie wys³ane, nie potwierdzone
	#define	POSNET_RSTATE_COMPLETED		0x00019004	//!< Wykonanie rozkazu zakoñczone
	#define	POSNET_RSTATE_ERRCOMPLETED	0x00019005	//!< Wykonanie rozkazu zakoñczone b³êdem
	/*!
		@}
	*/


	// --------------------------------------------------------------------------------------------------------


	/*!
		\defgroup	request_base	Obs³uga obiektów rozkazowych
		\ingroup sync_api

		@{
	*/

	/*!
		\brief	Funkcja tworzy nowy obiekt rozkazowy dla drukarki.

		Funkcja tworzy obiekt rozkazowy dla drukarki. Obiekt tego typu jest podstawow¹
		komórk¹ transmisyjn¹ pomiêdzy aplikacj¹ a drukark¹. Jego zadaniem jest realizacja
		pojedynczej transakcji drukarka-aplikacja.

		Wiêcej informacji we wprowadzeniu - \ref sec_flow "Przep³yw informacji" 

		\param	hLocalDevice	Uchwyt lokalny urz¹dzenia
		\param	command	Identyfikator rozkazu dla drukarki
		\return	Zwracany jest uchwyt do nowego obiektu rozkazowego lub NULL w przypadku b³êdu
	*/
	POSNET_API	POSNET_HANDLE	 __stdcall POS_CreateRequest(POSNET_HANDLE hLocalDevice, const char *command);
	/*!
		\brief	Funkcja tworzy nowy obiekt rozkazowy dla drukarki oraz dodaje parametry rozkazu.

		Funkcja tworzy obiekt rozkazowy dla drukarki. Obiekt tego typu jest podstawow¹
		komórk¹ transmisyjn¹ pomiêdzy aplikacj¹ a drukark¹. Jego zadaniem jest realizacja
		pojedynczej transakcji drukarka-aplikacja.

		Wiêcej informacji we wprowadzeniu - \ref sec_flow "Przep³yw informacji" 

		\param	hLocalDevice	Uchwyt lokalny urz¹dzenia
		\param	command	Identyfikator rozkazu dla drukarki
		\param  parameters Lista parametrów do dodania (zastêpuje dodatkowe wywo³ania \ref POS_PushRequestParam) 
		format: [nazwa parametru],[wartosc]\n[nazwa parametru],[wartosc]\n....[nazwa parametru],[wartosc]
		UWAGA: Jeœli wartoœæ parametru zawiera w sobie znaki nowej linii lub przecinka nale¿y u¿yæ \ref POS_PushRequestParam aby
		dodaæ taki parametr do rozkazu.
		\return	Zwracany jest uchwyt do nowego obiektu rozkazowego lub NULL w przypadku b³êdu
	*/
	POSNET_API      POSNET_HANDLE  __stdcall POS_CreateRequestEx(POSNET_HANDLE hLocalDevice, const char* command,const char *parameters);
	/*!
		\brief Funkcja s³u¿y do pobrania uchwytu urz¹dzenia zwi¹zanego z rozkazem

		\param hRequest	Uchwyt rozkazu
		\return uchwyt lokalny urz¹dzenia lub NULL w przypadku b³êdu gdy¿ nie mo¿na ustaliæ adresata rozkazu.
	*/
	POSNET_API POSNET_HANDLE	__stdcall POS_RequestDevice(POSNET_HANDLE hRequest);

	/*!
		\brief	Fukcja umieszcza obiekt rozkazowy w kolejce rozkazów do wykonania.

		Istniej¹ dwa warianty wysy³ania rozkazów 
		  - tryb kolejkowania,
		  - tryb bezpoœredni.
		
		W trybie kolejkowania nowy rozkaz jest umieszczany w kolejce do wys³ania do drukarki i
		automatycznie wysy³any w miarê zwalniania siê bufora drukarki. 
		\warning D³ugoœc kolejki jest ograniczona. Po jej przekroczeniu rozkazy traktowane s¹ 
				 jakby by³y wysy³ane w trybie bezpoœrednim.
	   
	    Tryb bezpoœredni kontroluje, czy drukarka jest wolna i dopiero gdy jest wolna umo¿liwia
		umieszczenie rozkazu w buforze. W ten sposób w danej chwili do drukarki wys³any jest 
		co najwy¿ej 1 rozkaz.

		\param	hRequest	Uchwyt rozkazu
		\param	mode		Wybór trybu wysy³ania rozkazu \ref req_modes "Tryby wysy³ania rozkazów"
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_PostRequest(POSNET_HANDLE hRequest,unsigned char mode);
	/*!
		\brief	Anulowanie obiektu rozkazowego

		Funkcja anuluje wykonanie rozkazu. W zale¿noœci od stanu rozkazu mo¿e to oznaczaæ:
		  - jeœli rozkaz nie zosta³ jeszcze wys³any zostaje on usuniêty z kolejki rozkazów,
		  - jeœli rozkaz zosta³ wys³any do drukarki zostaje on usuniêty z kolejki rozkazów,
		  - jeœli rozkaz by³ ju¿ wykonany to nic nie jest wykonywane i zostaje zwrócony
			kod b³êdu \ref POSNET_STATUS_ALREADY_COMPLETED .

		W ka¿dym przypadku za zniszczenie obiektu odpowiada aplikacja -  dla anulowanego rozkazu nale¿y
		wywo³aæ \ref POS_DestroyRequest .

		\param hRequest	Uchwyt rozkazu
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_CancelRequest(POSNET_HANDLE hRequest);
	/*!
		\brief	Zniszczenie obiektu rozkazowego i zwolnienie zajmowanej przezeñ pamiêci.

		Funkcja kasuje obiekt rozkazowy oraz wszystkie powi¹zane z nim dane.
		Musi byæ u¿yta dla ka¿dego rozkazu allokowanego poprzez \ref POS_CreateRequest POS_CreateRequest 
		który nie jest automatycznie usuwany w ytybach SPOOLSPECIAL i AUTOCLEAR.

		\param hRequest Uchwyt rozkazu (¿¹dania wychodz¹cego lub odpowiedzi z drukarki)
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_DestroyRequest(POSNET_HANDLE hRequest);

	/*!
		\brief	Wiek rozkazu - czas jaki up³yn¹³ od wywo³ania \ref POS_PostRequest

		Funkcja ta ma na celu ustalenie wieku rozkazu i mo¿e byæ przydatna w obs³udze
		sytuacji nadzwyczajnych. Czas podawany jest w milisekundach.

		\param hRequest Uchwyt rozkazu (¿¹dania wychodz¹cego lub odpowiedzi z drukarki)
	*/
	POSNET_API	unsigned long	__stdcall POS_GetRequestAge(POSNET_HANDLE hRequest);
	/*!
		\brief	Pobierz numer polecenia, który zosta³ u¿yty przy tworzeniu obiektu

		\param hRequest Uchwyt rozkazu (¿¹dania wychodz¹cego lub odpowiedzi z drukarki)
		\param retCommand   Bufor znakowy, w którym zostanie umieszczona nazwa polecenia
		\return Status wykonania rozkazu
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_GetRequestCommandID(POSNET_HANDLE hRequest,char *retCommand);
	/*!
		\brief	Pobierz numer seryjny rozkazu

		\param hRequest Uchwyt rozkazu (¿¹dania wychodz¹cego lub odpowiedzi z drukarki)
		\return Token (numer seryjny) powi¹zany z rozkazem
	*/
	POSNET_API	unsigned long	__stdcall POS_GetRequestCommandSerial(POSNET_HANDLE hRequest);
	/*!
		\brief	Zwróæ stan rozkazu

		Funkcja zwraca stan w jakim aktualnie znajduje siê rozkaz.
		Wiêcej informacji we wprowadzeniu - \ref sec_flow "Przep³yw informacji" 

		\param hRequest Uchwyt rozkazu (¿¹dania wychodz¹cego lub odpowiedzi z drukarki)
	*/
	POSNET_API	POSNET_STATE	__stdcall POS_GetRequestState(POSNET_HANDLE hRequest);
	/*!
		\brief	Czekaj na zakoñczenie rozkazu

		Funkcja powoduje zatrzymanie (uœpienie) wo³aj¹cej j¹ aplikacji do momentu
		zmiany stanu rozkazu na \ref POSNET_RSTATE_COMPLETED lub POSNET_RSTATE_ERRCOMPLETED lub up³yniêcia czasu
		wskazanego przez parametr timeout.
 
		\param hRequest	Uchwyt rozkazu
		\param	timeout Czas w ms na jaki ma aplikacja byæ uœpiona. 0 oznacza natychmiastowy 
				powrót
		\return	zwraca POSNET_STATUS_OK jeœli rozkaz zosta³ juz ukoñczony lub POSNET_STATUS_TIMEOUT 
				jeœli rozkaz nie zd¹¿y³ siê zakoñczyæ we wskazanym czasie.
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_WaitForRequestCompleted(POSNET_HANDLE hRequest,unsigned long timeout);

	/*!
		@}
	*/

	// --------------------------------------------------------------------------------------------------------

	/*!
		\defgroup req_param	Obs³uga parametrów rozkazu
		\ingroup sync_api

		\brief  W zale¿noœci od typu rozkazu mo¿e on posiadaæ odpowiedni¹
				iloœæ parametrów. Parametry te  odk³adane s¹ na listê parametrów
				za pomoc¹ kolejnych wywo³añ funkcji POS_PushRequestParam. Drukarka 
				przyjmuje parametry bez wzglêdu na ich kolejnoœæ.
		@{
	*/

	/*!
		\brief	Wstaw parametr rozkazu
		\param	hRequest	Uchwyt rozkazu
		\param  param_name		Nazwa parametru reprezentowana jako ci¹g znaków w konwencji jêzyka C (zakoñczony znakiem 0)
		\param	param_value		Wartoœæ parametru reprezentowana jako ci¹g znaków w konwencji jêzyka C (zakoñczony znakiem 0)
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_PushRequestParam(POSNET_HANDLE hRequest,const char *param_name,const char *param_value);

	/*!
		@}
	*/
	// --------------------------------------------------------------------------------------------------------

	/*!
		\defgroup	response_handling	Analiza pól odpowiedzi
		\ingroup sync_api

		@{
	*/
	/*!
		\brief	Pobierz iloœæ otrzymanych pól wartoœci

		Funkcja zwraca iloœæ pól DATA otrzymanych w odpowiedzi na rozkaz. Poprawna tylko
		wtedy, gdy sam rozkaz jest w stanie \ref POSNET_RSTATE_COMPLETED

		\param hRequest	Uchwyt zapytania/odpowiedzi
		\return Iloœæ otrzymanych pól odpowiedzi lub -1 w przypadku b³êdu
	*/
	POSNET_API	long			__stdcall POS_GetResponseValueCount(POSNET_HANDLE hRequest);

	/*!
		\brief	Pobierz wartoœæ parametru i przesuñ na nastêpne pole

		\param	hRequest	Uchwyt zapytania/odpowiedzi
		\param retName WskaŸnik do bufora znakowego, w którym funkcja umieœci nazwê odebranego parametru. D³ugoœæ tego bufora MUSI byæ conajmniej
						POSNET_NAMELENGTH_MAX
		\param  retVal WskaŸnik do bufora znakowego, w którym funkcja umieœci odebrany ci¹g znaków. Aplikacja musi zapewniæ odpowiedni¹
						pojemnoœæ bufora, któr¹ przekazuje w parametrze retValLen. W przypadku zbyt krótkiego
						bufora zostanie zwrócony b³¹d \ref POSNET_STATUS_BUFFERTOOSHORT
		\param	retValLen	D³ugoœæ bufora
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_PopResponseValue(POSNET_HANDLE hRequest,char *retName, char * retVal, unsigned long retValLen);
	/*!
		\brief	Pobierz watoœæ parametru po nazwie

		\param	hRequest	Uchwyt zapytania/odpowiedzi
		\param paramName WskaŸnik do bufora znakowego, w którym znajduje siê nazwa poszukiwanego pola odpowiedzi
		\param  retVal WskaŸnik do bufora znakowego, w którym funkcja umieœci odebrany ci¹g znaków. Aplikacja musi zapewniæ odpowiedni¹
						pojemnoœæ bufora, któr¹ przekazuje w parametrze retValLen. W przypadku zbyt krótkiego
						bufora zostanie zwrócony b³¹d \ref POSNET_STATUS_BUFFERTOOSHORT
		\param	retValLen	D³ugoœæ bufora
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_GetResponseValue(POSNET_HANDLE hRequest,const char *paramName, char * retVal, unsigned long retValLen);
	/*!
		\brief	Przesuñ wskaŸnik pól na pocz¹tek listy

		Funkcja umo¿liwia rozpoczêcie analizy pól od pocz¹tku.

		\param	hRequest	Uchwyt zapytania/odpowiedzi
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_ResponseRewind(POSNET_HANDLE hRequest);
	
	/*!
		@}
	*/

	/*!
		\defgroup req_queue Obs³uga kolejek rozkazów i wyników
		\ingroup sync_api

		Aplikacja mo¿e obs³ugiwaæ wysy³ane rozkazy w dwóch trybach.
		  * W trybie "wyœlij-zapomnij" aplikacja tworzy rozkaz, wype³nia parametrami a nastêpnie
			umieszcza w kolejce rozkazów do wykonania. Jednoczeœnie nie przechowuje samodzielnie 
			uchwytów wys³anych rozkazów. Wyniki rozkazów pobiera z kolejki wyników za poœrednictwem
			\ref POS_GetNextResponse i obs³uguje odpowiednio.
		  * W trybie "interaktywnym" aplikacja tworzy rozkaz, wype³nia parametrami i wstawia
			do kolejki rozkazów. Jednak¿e zapamiêtuje uchwyt rozkazu i obserwuj¹c stan rozkazu
			za poœrednictwem \ref POS_GetRequestState obs³uguje go w momencie zakoñczenia.
		
		Tryb "wyœlij-zapomnij" jest bardziej odpowiedni dla operacji masowych (np. sprawdzanie bazy towarów)
		gdy¿ umo¿liwia efektywniejsze (szybsze) wykonywanie rozkazów, których wynik jest
		mniej istotny.

		Tryb "interaktywny" wskazany jest dla rozkazów, których wynik jest w danej chwili niezbêdny dla
		dalszej pracy aplikacji. W tym celu wprowadzony jest te¿ rozkaz \ref POS_WaitForRequestCompleted
		który umozliwia synchronizowane oczekiwanie na zakoñczenie wykonania rozkazu przez drukarkê.

		\warning	Nie ma mo¿liwoœci przegl¹dania kolejki rozkazów oczekuj¹cych/wys³anych. Jest to spowodowane 
		ich asynchronicznym w stosunku do apliacki wykonywaniem i zawartoœæ tej kolejki mo¿e siê
		zmieniaæ w trakcie wykonywania zapytañ przegl¹daj¹cych (rozkazy mog¹ znikaæ z kolejki mimo
		i¿ aplikacja pobra³a je jeszcze z kolejki oczekuj¹cych).

		@{
	*/

	/*!
		\brief	Pobierz z kolejki odpowiedzi (rozkazów zakoñczonych) nastêpny dostêpny wynik.

		Z kolejki odpowiedzi zostaje pobrany (i usuniêty) nastêpny wynik. Pobrany wynik nale¿y 
		zniszczyæ za pomoc¹ \ref POS_DestroyRequest. Pobierane s¹ wy³¹cznie wyniki otrzymane na
		rozkazy wys³ane za poœrednictwem bie¿¹cego kontekstu urz¹dzenia (hLocalDevice). Rozkazy 
		wys³ane za pomoc¹ innych kontekstów tego samego urz¹dzenia s¹ pomijane.
		UWAGA: Uchwyt wyniku jest to¿samy z uchwytem rozkazu, który go spowodowa³ - oznacza to,
		¿e niszczenie za pomoc¹ \ref POS_DestroyRequest rozkazu jest równoznaczne ze zniszczeniem
		odpowiedzi i odwrotnie.

		Funkcja jest nieblokuj¹ca.

		\param	hLocalDevice	Uchwyt lokalny urz¹dzenia
		\return	Uchwyt do odpowiedzi lub NULL w przypadku b³êdu lub braku nastêpnej odpowiedzi
	*/
	POSNET_API	POSNET_HANDLE	__stdcall POS_GetNextResponse(POSNET_HANDLE hLocalDevice);
	/*!
		\brief	Pobierz iloœæ wyników w kolejce wyników

		\param hLocalDevice	Uchwyt lokalny urz¹dzenia
		\return Iloœæ wyników dla bie¿¹cego kontekstu urz¹dzenia.
	*/
	POSNET_API	unsigned long	__stdcall POS_GetResponseCount(POSNET_HANDLE hLocalDevice);
	/*!
		\brief	Pobierz iloœæ rozkazów oczekuj¹cych w kolejce rozkazów wys³anych przez bie¿¹cy kontekst
				urz¹dzenia lub wszystkich rozkazów oczekuj¹cych w kolejce.

		\param hLocalDevice	Uchwyt lokalny urz¹dzenia
		\param globalnie Jeœli parametr ten jest ró¿ny od 0 zwracana wartoœæ podaje iloœæ wszystkich rozkazów 
				oczekuj¹cych w kolejce.
	*/
	POSNET_API	unsigned long	__stdcall POS_GetRequestCount(POSNET_HANDLE hLocalDevice,unsigned char globalnie=0);

	/*!
		\brief	Pobierz statystyki kolejek. Statystyki s¹ zwracane w kontekœcie ca³ego urz¹dzenia.

		\param hLocalDevice	Uchwyt lokalny urz¹dzenia
		\param sent		Do tej zmiennej wpisywana jest iloœæ wszystkich wys³anych do drukarki
						rozkazów
		\param completed	Do tej zmiennej wpisywana jest iloœæ wszystkich ukoñczonych rozkazów
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_RequestQueueStats(POSNET_HANDLE hLocalDevice, unsigned long *sent, unsigned long *completed);	


	/*!
		\brief	Wyczyœæ kolejkê rozkazów oczekuj¹cych na wys³anie (nie ignoruj odpowiedzi na rozkazy ju¿ wys³ane)

		Czyœci kolejkê rozkazów oczekuj¹cych na wys³anie. Funkcja powinna byæ wykorzystywana wy³¹cznie
		w sytuacji, gdy nast¹pi³ krytyczny b³¹d urz¹dzenia. Rozkazy ju¿ wys³ane zostan¹ obs³u¿one, jeœli
		w dalszej perspektywie pojawi siê na nie odpowiedŸ. Domyœlnie rozkazy czyszczone s¹ wy³¹cznie dla bie¿¹cego
		kontekstu urz¹dzenia.

		\param	hLocalDevice	Uchwyt lokalny urz¹dzenia
		\param globalnie Jeœli parametr ten jest ró¿ny od 0 ignorowany jest kontekst urz¹dzenia.
	*/
	POSNET_API	POSNET_STATUS	__stdcall POS_RequestQueueCancelPending(POSNET_HANDLE hLocalDevice,unsigned char globalnie=0);
	/*!
		\brief	Wyczyœæ kolejkê rozkazów oczekuj¹cych na wykonanie (zarówno wys³anych jak i oczekuj¹cych na wys³anie)

		Czyœci kolejkê rozkazów wychodz¹cych. Funkcja powinna byæ wykorzystywana wy³¹cznie
		w sytuacji, gdy nast¹pi³ krytyczny b³¹d urz¹dzenia. Wszystkie otrzymane póŸniej
		wyniki dla rozkazów, które by³y w trakcie wykonywania zostan¹ odrzucone. 
		Domyœlnie rozkazy czyszczone s¹ wy³¹cznie dla bie¿¹cego
		kontekstu urz¹dzenia.

		\param	hLocalDevice	Uchwyt lokalny urz¹dzenia
		\param globalnie Jeœli parametr ten jest ró¿ny od 0 ignorowany jest kontekst urz¹dzenia.
	*/
	POSNET_API  POSNET_STATUS	__stdcall POS_RequestQueueCancelAll(POSNET_HANDLE hLocalDevice,unsigned char globalnie=0);

	/*!
		\brief	Wysy³a do drukarki ¿¹danie powtórzenia transmisji odpowiedzi

		Funkcja jest przydatna w sytuacji, gdy chcemy powtórzyæ odbiór ramki jeœli nast¹pi³ timeout i istnieje
		podejrzenie, ¿e drukarka otrzyma³a rozkaz ale odpowiedŸ do nas nie powróci³a. Do powtórzenia u¿ywana
		jest de facto sekwencja 'rpt'. Uniemo¿liwia to przypadkowe podwójne wykonanie sekwencji jeœli wczeœniej nie
		dotar³a do drukarki (zostanie zwrócony b³¹d 13).
		U¿ycie mo¿liwe jest jeœli rozkaz znajduje siê w stanie POSNET_RSTATE_SENT. 
		(por. \ref POS_GetRequestState)

		\param hRequest	Uchwyt rozkazu, który ma byæ ponowiony
		\return Status wykonania
	*/
	POSNET_API      POSNET_STATUS  __stdcall POS_RepeatRequest(POSNET_HANDLE hRequest);

	/*!
		@}
	*/

	// --------------------------------------------------------------------------------------------------------
	/*!
	\defgroup	sync_api	Obs³uga synchroniczna drukarki
	@{
	*/
	
	// PLACEHOLDER

	/*!
		@}
	*/
	// --------------------------------------------------------------------------------------------------------


	// --------------------------------------------------------------------------------------------------------
	/*!
	\defgroup	adv_api	Obs³uga rozszerzonych mo¿liwoœci drukarek (funkcje pomocnicze)

	Funkcje z tej grupy s³u¿¹ jako u³atwienie realizacji niektórych funkcjonalnoœci urz¹dzenia.
	
	@{
	*/

	/*!
		\brief Funkcja umo¿liwia zaprogramowanie grafiki ze wskazanego pliku graficznego. Funkcja ³aduje wskazany plik bitmapy
		(format Windows BMP) dekoduje, proguje i ³aduje do urz¹dzenia fiskalnego w sposób blokuj¹cy poprzez wykonanie
		blokuj¹ce kolejno rozkazu imgslotset oraz odpowiedniej iloœci rozkazów imgslotappend.

		\param hLocalDevice Uchwyt lokalny urz¹dzenia
		\param filename Nazwa pliku z grafik¹ do zaprogramowania. Grafika musi byæ w formacie Windows BITMAP bez kompresji.
		\param number Numer grafiki do zaprogramowania
		\param threshold Poziom odciêcia dla wykonania operacji progowania bitmapy (0-255). Za³adowany plik bitmapy konwertowany
		jest na skalê szaroœci, która nastêpnie porównywana jest z progiem. Je¿eli kolor przekracza próg, wynikowy
		punkt obrazu bêdzie bia³y, je¿eli nie - czarny. Dla bitmap dwukolorowych nale¿y podaæ dowoln¹ wartoœæ powy¿ej zera.

		\return status wykonania operacji.
	*/
	POSNET_API POSNET_STATUS __stdcall POS_AdvProgramImage(POSNET_HANDLE hLocalDevice,const char *filename,const unsigned long number,const unsigned char threshold);

	/*!
		@}
	*/
	// --------------------------------------------------------------------------------------------------------

	

}


#endif

