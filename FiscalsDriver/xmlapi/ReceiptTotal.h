#pragma once
#include "../MyUtils.h"
typedef struct {
	double ilosc{ 0.00 };
	long wartosc{ 0 };
} KAUCJA;

/**
 * @author Piotr Kuli�ski (c) 2022-09-16
 * @brief Podsumowanie paragonu
 * Rozlicznie wp�at, reszt, zaliczek i rabat�w \n
 * Wszystkie warto�ci s� przechowywane formacie bazowym, jakim s� grosze (long).
 * Powodem jest unikni�cie problem�w zaokr�gle� w podsumowaniach i obliczeniach.
 * Klasa udost�pnia metody template do pobierania i formatowania bazowego wyniku.
*/
struct ReceiptTotal
{
private:
	/**
	 * @brief Sprzeda� po rabacie/narzucie w rozbicu na totalizatory VAT\n
	 * Na potrzeby rozliczenia zaliczek w podsumowaniu (thermal)
	*/
	std::map<std::string, long> sum_przedaz{};

	struct {
		/** Podsumowanie zliczone z linijek */
		long przed_rabatem = 0;

		/** Podsumowanie zliczone z linijek z udzielonym rabatem */
		long po_rabacie = 0;
	} grosze;

	/**
	 * @brief Kaucje
	*/
	struct SKaucje {
		KAUCJA kpobrana{0,0};
		KAUCJA kzwrocona{0,0};

		/** Warto�� kaucji pobranych */
		long pobrane{ 0 };
		/** Warto�� kaucji zwr�conych */
		long zwrocone{ 0 };

		long wartosc() {
			return (kpobrana.wartosc - kzwrocona.wartosc);
			//return (pobrane - zwrocone);
		}		
		
		double ilosc_pobrana() {
			return (kpobrana.ilosc);
		}		
		double ilosc_zwrocona() {
			return (kpobrana.ilosc);
		}

		template <class ConversionPredicat>
		const auto wartosc() {
			ConversionPredicat convert;
			return convert(wartosc());
		}
	} kaucje;

	/**
	* @brief Podsumowanie zliczone z p�atno�ci
	*/
	long platnosci = 0;

	/**
	* @brief Podsumowanie zliczone z zaliczek
	*/
	long zaliczki = 0;

public:
	ReceiptTotal& operator+=(ReceiptLine& ln) {
		long przed_rabatem = ln.grosze.wartosc_przed_rabatem;
		long po_rabacie = (ln.grosze.wartosc_przed_rabatem + ln.grosze.rabat);
		if (ln.typ == ReceiptLineType::Sale) {
			grosze.przed_rabatem += przed_rabatem;
			grosze.po_rabacie += po_rabacie;
			#if defined(LOGGER)
			std::cout
				<< std::left << std::setw(40) << ln.name()
				<< std::right << std::setw(11) << grosze.przed_rabatem
				<< std::setw(17) << grosze.po_rabacie
				<< std::setw(10) << std::fixed << std::setprecision(3) << ln.ilosc
				<< std::setw(10) << ln.grosze.cena
				<< std::setw(14) << przed_rabatem
				<< std::setw(17) << po_rabacie
				<< std::setw(10) << ln.grosze.rabat
				<< std::endl;
			#endif
			auto pos = sum_przedaz.find(ln.vat);
			if (pos == sum_przedaz.end()) {
				sum_przedaz.insert(std::make_pair(ln.vat, abs(po_rabacie)));
			}
			else {
				pos->second += po_rabacie;
			}
		}
		else if (ln.typ == ReceiptLineType::KaucjaPobrana) {
			kaucje.kpobrana.ilosc += ln.ilosc;
			kaucje.kpobrana.wartosc += przed_rabatem;
			kaucje.pobrane += przed_rabatem;
		}
		else if (ln.typ == ReceiptLineType::KaucjaPobranaStorno) {
			kaucje.kpobrana.ilosc -= ln.ilosc;
			kaucje.kpobrana.wartosc -= przed_rabatem;
			kaucje.pobrane -= przed_rabatem;
		}
		else if (ln.typ == ReceiptLineType::KaucjaZwrocona) {
			kaucje.kzwrocona.ilosc += ln.ilosc;
			kaucje.kzwrocona.wartosc += przed_rabatem;
			kaucje.zwrocone += przed_rabatem;
		}
		else if (ln.typ == ReceiptLineType::KaucjaZwroconaStorno) {
			kaucje.kzwrocona.ilosc -= ln.ilosc;
			kaucje.kzwrocona.wartosc -= przed_rabatem;
			kaucje.zwrocone -= przed_rabatem;
		}
		return (*this);
	}

	ReceiptTotal& operator+=(Prepayment& pp) {
		zaliczki += pp.wartosc();
		return (*this);
	}

	ReceiptTotal& operator+=(Payment& pay) {
		platnosci += pay.wartosc();
		return (*this);
	}

	/**
	 * @brief Zwrto warto�ci sprzeda�y dla przekazanej stawki vat.
	 * Program rozbija pozycje na totalizery i mo�na wyci�gn�� poszczeg�lne warto�ci. \n
	 * Syma tych warto�ci powinna odpowiada� og�lnej warto�ci linijek po rabacie
	 * \ref wartosc.po_rabacie
	 * @param stawka - stawka taka jak przekazywana w linijkach sprzeda�owych
	 * @return wysumowana warto�� sprzeda�y we wskazanym totalizerze
	*/
	const long sprzedaz(const std::string& stawka) {
		auto pos = sum_przedaz.find(stawka);
		if (pos != sum_przedaz.end()) {
			return pos->second;
		}
		return 0;
	}

	/**
	 * @brief Warto�� wp�aty, kalkulowana z warto�ci po rabacie + zaliczki + kaucje
	 * Je�li jest mniejsza od kwoty p�atno�ci, to zwracana jest warto�� p�atno�ci.
	 * @return
	*/
	const long wplata() {
		long razem{ grosze.po_rabacie + zaliczki + (kaucje.pobrane - kaucje.zwrocone) };
		if (razem < platnosci) {
			razem = platnosci;
		}
		return razem;
	}

	/**
	* @brief Warto�� total kalkulowana z linijek i p�atno�ci \n
	* Je�li brakuje p�atno�ci, wi�c warto�� p�atno�ci b�dzie warto�ci� pozycji
	* Uwaga!:
	* Wbrew dokumentacji, total to niby kwota kt�r� ma zap�aci� klient, nie do ko�ca to prawda.
	* Total nie obejmuje pobranych kaucji, a klienta p�aci r�wnie� za nie. Wi�c np. na
	* paragonie mamy SUMA PLN: 100.00, kaucje pobrane 5.00 i klient p�aci 105.00 a nie total(100.00)
	*/
	const long total() {
		long razem{ grosze.po_rabacie + zaliczki };
		if (razem < platnosci) {
			razem = platnosci;
		}
		return (razem - reszta());
	}

	const long dsp() {
		return total();
	}

	const long platnosc() {
		return platnosci;
	}

	const long reszta() {
		long _reszta = 0;
		if (grosze.po_rabacie + zaliczki >= platnosci) {
			return _reszta / 100.0f;
		}
		_reszta = (platnosci - grosze.po_rabacie + zaliczki);
		return (long)(_reszta > 0 ? _reszta : 0);
	}

	/**
	 * @brief Metoda sprawdza poprawno�� wylicze� (w groszach) \n
	 * r�nica wynikaj�ca z: waro�ci pozyji, wys�anych zaliczek, kaucji z odj�ciem wys�anych p��tno�ci \n
	 * W przypadku wykrycia braku p�atno�ci zwraca ujemn� brakuj�c� kwot�.
	 * @return warto�� wyr�wnania \n
	 * \b mniejsze \b od \b zera - brak wystarczaj�cych p�atno�ci na pokrycie warto�ci paragonu \n
	 * \b wi�ksze \b od \b zera - nadmiar p�atno�ci, wys�ano wi�cej p�atno�ci ni� wynika z pozycji, wybrane drukarki potrafi� same wydrukowa� warto�� reszty.
	*/
	const long wyrownaj() {
	//by�o tak, do sprawdzenia na Thermal, problem by�z Novitusem
		//return (platnosci - (grosze.po_rabacie - zaliczki + (kaucje.pobrane - kaucje.zwrocone)));
		return (platnosci - (grosze.po_rabacie + zaliczki + (kaucje.pobrane - kaucje.zwrocone)));
	}

	const long KaucjePobrane() {
		return kaucje.kpobrana.wartosc;
	}

	const long KaucjeZwrocone() {
		return kaucje.kzwrocona.wartosc;
	}

	const auto KaucjePobraneIlosc() {
		return kaucje.ilosc_pobrana();
	}

	const auto KaucjeZwroconeIlosc() {
		return kaucje.ilosc_zwrocona();
	}

	template <class ConversionPredicat>
	const auto sprzedaz(const std::string& stawka) {
		ConversionPredicat convert;
		return convert(sprzedaz(stawka));
	}

	template <class ConversionPredicat>
	const auto wplata() {
		ConversionPredicat convert;
		return convert(wplata());
	}

	/**
	 * @brief Pobranie warto�ci total wg predykatu konwersji
	 * @tparam ConversionPredicat predykat konwersji, np. \ref Grosze, \ref DecimalStringTrim
	 * @return automatyczna warto�� skonwertowana
	*/
	template <class ConversionPredicat>
	const auto total() {
		ConversionPredicat convert;
		return convert(total());
	}

	template <class ConversionPredicat>
	const auto dsp() {
		ConversionPredicat convert;
		return convert(dsp());
	}

	template <class ConversionPredicat>
	const auto reszta() {
		ConversionPredicat convert;
		return convert(reszta());
	}

	/**
	 * @brief Pobranie kwoty wys�anych p�atno�ci
	 * @tparam ConversionPredicat konwerter warto�ci
	 * @return warto�� p�atno�ci po konwersji
	*/
	template <class ConversionPredicat>
	const auto platnosc() {
		ConversionPredicat convert;
		return convert(platnosci);
	}

	/**
	 * @brief Wyr�wnanie po formaowaniu
	 * @tparam ConversionPredicat konwerter wyniku
	 * @return Sformatowana warto�� wyr�wnania \n
	*/
	template <class ConversionPredicat>
	const auto wyrownaj() {
		ConversionPredicat convert;
		return convert(wyrownaj());
	}

	auto const KaucjeWartosc() {
		return kaucje.wartosc();
	}

	template <class ConversionPredicat>
	const auto KaucjePobrane() {
		ConversionPredicat convert;
		return convert(kaucje.pobrane);
	}

	template <class ConversionPredicat>
	const auto KaucjeZwrocone() {
		ConversionPredicat convert;
		return convert(kaucje.zwrocone);
	}
};