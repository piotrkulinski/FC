#pragma once
#define GET_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); })
#define GET_VALUE_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); }).first_child().value()

/**
 * @author Piotr Kuliñski
 * @brief P³atnoœci (równie¿ w walutach)
*/
struct Payment
{
private:
	long value{ 0 };

public:
	std::string typ{ "" };
	std::string nazwa{ "" };
	/*
	DCC
	kurs - dok³adnoœæ tylko do 4 znaków po kropce dzisiêtnej
	*/
	struct DCC {
	private:
		std::string _symbol{ "" };
		long _kurs{ 1 };
		long value{ 0 };
	public:
		const auto wartosc() {
			return value;
		}
		const auto wartosc(long _wartosc) {
			value = _wartosc;
		}
		const auto kurs(long _wartosc) {
			_kurs = _wartosc;
		}
		const auto kurs() {
			return _kurs;
		}
		const auto symbol() {
			return _symbol;
		}
		const auto symbol(std::string _wartosc) {
			_symbol = _wartosc;
		}

		template <class ConversionPredicat>
		const auto wartosc() {
			ConversionPredicat conv;
			return conv(wartosc());
		}

	} dcc;

	Payment(std::string _typ, std::string _nazwa, long _wartosc) :typ(_typ), nazwa(_nazwa), value(_wartosc) {};

	Payment(const pugi::xml_node& payment) {
		Grosze g;
		typ = payment.attribute("typ").value();
		nazwa = payment.attribute("nazwa").value();
		double w = std::stof(payment.attribute("kwota").value());
		value = abs(g(w));

		if (pugi::xml_node waluta = GET_NODE(payment, "Waluta"); !waluta.empty()) {
			dcc.symbol(waluta.attribute("symbol").value());
			w = std::stof(waluta.attribute("kwota").value());
			dcc.wartosc(g(w));
			w = std::stof(waluta.attribute("kurs").value());
			dcc.kurs((long)floor(w * 10000.0f + (w >= 0 ? 0.5f : -0.5f)));
			long wartosc_wg_kursu = abs(g(dcc.wartosc() * dcc.kurs() / 1000000.0f));
			if (wartosc_wg_kursu != value) {
				dcc.wartosc(g(value/100.0f * (dcc.kurs() / 10000.0f)));
			}

		}
	}

	long getPeymentID() {
		return (u_int)atol(typ.c_str());
	}

	const auto wartosc() {
		return value;
	}

	std::string name() {
		return nazwa;
	}

	bool isDcc() {
		return (dcc.wartosc() != 0.00f);
	}

	template <class ConversionPredicat>
	std::string name(ConversionPredicat cnv) {
		return cnv(nazwa);
	}

	template <class ConversionPredicat>
	long PeymentID(ConversionPredicat fun) {
		return fun((u_int)atol(typ.c_str()));
	}

	template <class ConversionPredicat>
	const auto wartosc() {
		ConversionPredicat conv;
		return conv(wartosc());
	}

};