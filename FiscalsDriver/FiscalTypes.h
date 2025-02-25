#pragma once
/*! \file */

/**
\ingroup FiscalTypesDefinition
\author pkulinski (&copy;) 2024
\brief XML Definiuj¹cy wydruk fiskalny
\see ReceiptLineType, Prepayment, FiscalControl, XmlApiConfigurationRequest
\see FiscalControlType,FiscalControlActionType
\code{xml}
<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<Paragon>
  <!-- optional (XmlApiConfigurationRequest)
       <Connection type="0" name="COM23" baudrate="9600" databits="8" parity="N" flowcontrol="N"/>
  -->
  <!-- optional (FiscalControlType,FiscalControlActionType)
	   <Sterowanie>
			<OtworzSzuflade typ="After">1</OtworzSzuflade>
	   </Sterowanie>
  -->
  <Naglowek>
	<Opisy>
	  <Opis typ="1028" wartosc="HOSPITALITY POS"/>
	  <Opis typ="1027" wartosc="SALES_1"/>
	  <Opis typ="1005" wartosc="admin"/>
	  <Opis typ="30" wartosc="R:15 Z:Szyb/8874"/>
	  <Opis typ="1001" wartosc="15"/>
	</Opisy>
  </Naglowek>
  <Linijki>
	<!-- 
		typ: typ linii, szczegó³y w  ReceiptLineType
		ilosc: iloœæ jednostkowa
		jm: na razie nie obs³ugiwana
		cena: cena jednostkowa brutto
		opis: dodatkowa linia opisu, nie ka¿dy protokó³ mo¿e j¹ obs³u¿yæ, szczegó³y w manifeœcie prtoko³u
	-->
	<Linijka typ="1" nazwa="8% Karnet Strefa Wellness B/J 10 x 2h" jm="szt" ilosc="1.0" ptu="8.0" cena="360.0" opis="8%_Bilet Wellness 2h"/>
  </Linijki>
  <Platnosci>
	<Platnosc typ="2" kwota="360.0" nazwa=""/>
  </Platnosci>
  <Zaliczki>
  </Zaliczki>
</Paragon>
\endcode
*/
typedef const char* XmlApiFiscalRequest;

/**
\ingroup FiscalTypesDefinition
\author pkulinski (&copy;) 2024
\brief XML Definiuj¹cy wyduk niefiskalny
\code{xml}
<?xml version="1.0" encoding="windows-1250"?>
<Lines MaxWidth="34">
  <Line Param1="Left" Param2="Right" Param3="">
    <Typ>1</Typ>
    <Value1>POS ID: 34019165</Value1>
    <Value2>Nr paragonu: 3116</Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Left" Param2="" Param3="">
    <Typ>2</Typ>
    <Value1>MID: 000000000554729</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Left" Param2="Right" Param3="">
    <Typ>3</Typ>
    <Value1>************1863</Value1>
    <Value2>Wa¿na do: /</Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Left" Param2="" Param3="">
    <Typ>4</Typ>
    <Value1>AID: A0000000041010</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Left" Param2="" Param3="">
    <Typ>5</Typ>
    <Value1>DEBIT MASTERCARD (B)</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Left" Param2="" Param3="">
    <Typ>5</Typ>
    <Value1>Contactless</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Center Bold" Param2="" Param3="">
    <Typ>6</Typ>
    <Value1>Sprzeda¿</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Left" Param2="Right" Param3="">
    <Typ>7</Typ>
    <Value1>KWOTA:</Value1>
    <Value2>30,00 PLN</Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Bold" Param2="" Param3="">
    <Typ>12</Typ>
    <Value1>RACHUNEK DLA SPRZEDAWCY</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Center" Param2="" Param3="">
    <Typ>23</Typ>
    <Value1>Proszê obci¹¿yæ mój rachunek</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Center" Param2="" Param3="">
    <Typ>25</Typ>
    <Value1>Podpis niewymagany</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Left" Param2="" Param3="">
    <Typ>15</Typ>
    <Value1>Kod autoryzacji: (1) 069164</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Left" Param2="" Param3="">
    <Typ>16</Typ>
    <Value1>ARQC: 55B553049C4B4D2F</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Left" Param2="Right" Param3="">
    <Typ>17</Typ>
    <Value1>DATA: 2023.02.14</Value1>
    <Value2>GODZ: 16:26:36</Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="" Param2="" Param3="">
    <Typ>24</Typ>
    <Value1></Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
  <Line Param1="Center" Param2="" Param3="">
    <Typ>23</Typ>
    <Value1>Dziêkujê</Value1>
    <Value2></Value2>
    <Value3></Value3>
  </Line>
</Lines>
\endcode
*/
typedef const char* XmlApiNonFiscalRequest;


/**
\ingroup FiscalTypesDefinition
\author pkulinski (&copy;) 2024
\brief Response wykrywania urz¹dzeñ fiskalnych
\include Examples\DetectResult.xml
ew. nie wykrycie urz¹dzenia
\include Examples\DetectResultFail.xml
*/
typedef char* XmlApiDetectFiscalResponse;


/**
\ingroup FiscalTypesDefinition
\brief Request konfiguracyjny po³¹czenie \n \n
\ref ConnectionType::RS232 "Po³¹czenie RS232"\n
np.
\include Examples\ConnectionRS232.xml
\n
\ref ConnectionType::TCP "Po³¹czenie TCP/IP"\n
np.
\include Examples\ConnectionTCP.xml
\author pkulinski (&copy;) 2024
*/
typedef const char* XmlApiConfigurationRequest;

/**
\ingroup FiscalTypesDefinition
\author pkulinski (&copy;) 2024
\brief Request ogólny pozwalaj¹cy drukowaæ wszystkie mo¿liwe dane
\code{xml}
<Requests>
  <Request type="fiscal_ticket">
		<![CDATA[
		<Paragon>
		  <Naglowek>
			<Opisy>
			  <Opis typ="1028" wartosc="HOSPITALITY POS"/>
			  <Opis typ="1027" wartosc="SALES_1"/>
			  <Opis typ="1005" wartosc="admin"/>
			  <Opis typ="30" wartosc="R:15 Z:Szyb/8874"/>
			  <Opis typ="1001" wartosc="15"/>
			</Opisy>
		  </Naglowek>
		  <Linijki>
			<Linijka typ="1" nazwa="8% Karnet Strefa Wellness B/J 10 x 2h" ilosc="1.0" ptu="8.0" cena="360.0" opis="8%_Bilet Wellness 2h"/>
		  </Linijki>
		  <Platnosci>
			<Platnosc typ="2" kwota="360.0" nazwa=""/>
		  </Platnosci>
		  <Zaliczki>
		  </Zaliczki>
		</Paragon>
		]]>
  </Request>
  <Request type="non_fiscal_ticket">
		<![CDATA[
		<Lines MaxWidth="34">
			<Line Param1="Left" Param2="Right" Param3="">
				<Typ>1</Typ>
				<Value1>POS ID: 34019165</Value1>
				<Value2>Nr paragonu: 3116</Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Left" Param2="" Param3="">
				<Typ>2</Typ>
				<Value1>MID: 000000000554729</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Left" Param2="Right" Param3="">
				<Typ>3</Typ>
				<Value1>************1863</Value1>
				<Value2>Wa¿na do: /</Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Left" Param2="" Param3="">
				<Typ>4</Typ>
				<Value1>AID: A0000000041010</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Left" Param2="" Param3="">
				<Typ>5</Typ>
				<Value1>DEBIT MASTERCARD (B)</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Left" Param2="" Param3="">
				<Typ>5</Typ>
				<Value1>Contactless</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Center Bold" Param2="" Param3="">
				<Typ>6</Typ>
				<Value1>Sprzeda¿</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Left" Param2="Right" Param3="">
				<Typ>7</Typ>
				<Value1>KWOTA:</Value1>
				<Value2>30,00 PLN</Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Bold" Param2="" Param3="">
				<Typ>12</Typ>
				<Value1>RACHUNEK DLA SPRZEDAWCY</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Center" Param2="" Param3="">
				<Typ>23</Typ>
				<Value1>Proszê obci¹¿yæ mój rachunek</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Center" Param2="" Param3="">
				<Typ>25</Typ>
				<Value1>Podpis niewymagany</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Left" Param2="" Param3="">
				<Typ>15</Typ>
				<Value1>Kod autoryzacji: (1) 069164</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Left" Param2="" Param3="">
				<Typ>16</Typ>
				<Value1>ARQC: 55B553049C4B4D2F</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Left" Param2="Right" Param3="">
				<Typ>17</Typ>
				<Value1>DATA: 2023.02.14</Value1>
				<Value2>GODZ: 16:26:36</Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="" Param2="" Param3="">
				<Typ>24</Typ>
				<Value1></Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
			<Line Param1="Center" Param2="" Param3="">
				<Typ>23</Typ>
				<Value1>Dziêkujê</Value1>
				<Value2></Value2>
				<Value3></Value3>
			</Line>
		</Lines>
		]]>
  </Request>
</Requests>
\endcode
*/
typedef const char* XmlApiAllRequest;

/**
\ingroup FiscalTypesDefinition
\author pkulinski (&copy;) 2024
\brief String definiuj¹cy nazwê funkcji w VFP.\n
Nazwa funkcji musi mieæ dwa parametry np. 
\code{cpp}
local result as integer

declare integer RegisterEventAnswerFox in (FiscalDriver.dll) as fd_RegisterEventAnswerFox _proc_name_ @ string

result = fd_RegisterEventAnswerFox("GetAnswer")
? result
return

procedure GetAnswer(message as string, eventType as integer) as void
	MessageBox(message,16,"Komunikat z DLL")
endprroc
\endcode
*/
typedef const char* VFPFunctionSI;
