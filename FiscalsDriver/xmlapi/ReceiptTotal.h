#pragma once
#include "../MyUtils.h"
typedef struct {
	double ilosc{ 0.00 };
	long wartosc{ 0 };
} KAUCJA;

/**
 * @author Piotr Kuliñski (c) 2022-09-16
 * @brief Podsumowanie paragonu
 * Rozlicznie wp³at, reszt, zaliczek i rabatów \n
 * Wszystkie wartoœci s¹ przechowywane formacie bazowym, jakim s¹ grosze (long).
 * Powodem jest unikniêcie problemów zaokr¹gleñ w podsumowaniach i obliczeniach.
 * Klasa udostêpnia metody template do pobierania i formatowania bazowego wyniku.
*/
struct ReceiptTotal
{
private:
	/**
	 * @brief Sprzeda¿ po rabacie/narzucie w rozbicu na totalizatory VAT\n
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

		/** Wartoœæ kaucji pobranych */
		long pobrane{ 0 };
		/** Wartoœæ kaucji zwróconych */
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
	* @brief Podsumowanie zliczone z p³atnoœci
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
	 * @brief Zwrto wartoœci sprzeda¿y dla przekazanej stawki vat.
	 * Program rozbija pozycje na totalizery i mo¿na wyci¹gn¹æ poszczególne wartoœci. \n
	 * Syma tych wartoœci powinna odpowiadaæ ogólnej wartoœci linijek po rabacie
	 * \ref wartosc.po_rabacie
	 * @param stawka - stawka taka jak przekazywana w linijkach sprzeda¿owych
	 * @return wysumowana wartoœæ sprzeda¿y we wskazanym totalizerze
	*/
	const long sprzedaz(const std::string& stawka) {
		auto pos = sum_przedaz.find(stawka);
		if (pos != sum_przedaz.end()) {
			return pos->second;
		}
		return 0;
	}

	/**
	 * @brief Wartoœæ wp³aty, kalkulowana z wartoœci po rabacie + zaliczki + kaucje
	 * Jeœli jest mniejsza od kwoty p³atnoœci, to zwracana jest wartoœæ p³atnoœci.
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
	* @brief Wartoœæ total kalkulowana z linijek i p³atnoœci \n
	* Jeœli brakuje p³atnoœci, wiêc wartoœæ p³atnoœci bêdzie wartoœci¹ pozycji
	* Uwaga!:
	* Wbrew dokumentacji, total to niby kwota któr¹ ma zap³aciæ klient, nie do koñca to prawda.
	* Total nie obejmuje pobranych kaucji, a klienta p³aci równie¿ za nie. Wiêc np. na
	* paragonie mamy SUMA PLN: 100.00, kaucje pobrane 5.00 i klient p³aci 105.00 a nie total(100.00)
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
	 * @brief Metoda sprawdza poprawnoœæ wyliczeñ (w groszach) \n
	 * ró¿nica wynikaj¹ca z: waroœci pozyji, wys³anych zaliczek, kaucji z odjêciem wys³anych p³¹tnoœci \n
	 * W przypadku wykrycia braku p³atnoœci zwraca ujemn¹ brakuj¹c¹ kwotê.
	 * @return wartoœæ wyrównania \n
	 * \b mniejsze \b od \b zera - brak wystarczaj¹cych p³atnoœci na pokrycie wartoœci paragonu \n
	 * \b wiêksze \b od \b zera - nadmiar p³atnoœci, wys³ano wiêcej p³atnoœci ni¿ wynika z pozycji, wybrane drukarki potrafi¹ same wydrukowaæ wartoœæ reszty.
	*/
	const long wyrownaj() {
	//by³o tak, do sprawdzenia na Thermal, problem by³z Novitusem
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
	 * @brief Pobranie wartoœci total wg predykatu konwersji
	 * @tparam ConversionPredicat predykat konwersji, np. \ref Grosze, \ref DecimalStringTrim
	 * @return automatyczna wartoœæ skonwertowana
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
	 * @brief Pobranie kwoty wys³anych p³atnoœci
	 * @tparam ConversionPredicat konwerter wartoœci
	 * @return wartoœæ p³atnoœci po konwersji
	*/
	template <class ConversionPredicat>
	const auto platnosc() {
		ConversionPredicat convert;
		return convert(platnosci);
	}

	/**
	 * @brief Wyrównanie po formaowaniu
	 * @tparam ConversionPredicat konwerter wyniku
	 * @return Sformatowana wartoœæ wyrównania \n
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