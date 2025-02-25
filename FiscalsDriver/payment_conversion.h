#pragma once

/**
* @brief P�atno�ci z bazy hospitality \n
* 1 - Karta kredytowa, \n
* 2 - Got�wka,\n
* 4 - Przelew,\n
* 5 - U�ytek w�asny,\n
* 6 - Kredyt,\n
* 9 - Pobranie,\n
* 10 - Przedp�ata,\n
* 11 - Na pok�j,\n
* 13 - Dodatkowa 2,\n
* 15 - Pobranie k.,\n
* 17 - Karta podarunkowa,\n
* 21 - Bony,\n
* 24 - Grupa,\n
* 26 - PrePaid,\n
* 28 - GoldFinger,\n
* 30 - Kompensata,\n
* 36 - iKasa,\n
* 103 - Dodatkowa 100,\n
* 110 - PunktyTEST,\n
* 111 - Przelewy24,\n
* 112 - Bon,\n
* 113 - DOD,\n
* 116 - Na grup� zewn. fiskal.,\n
* 117 - Na pok�j zewn. fiskal.,\n
* 118 - Kredyt wew. fiskal.,\n
* 120 - Bilet Parkingowy,\n
*/
class payment_base {
public:
	static const enum HospIdx : int {
		Unknow = 0,
		KartaKredytowa = 1,
		Gotowka = 2,
		Przelew = 3,
		U�ytekW�asny = 4,
		Kredyt = 5,
		Pobranie = 6,
		Przedp�ata = 7,
		NaPokoj = 8,
		Dodatkowa2 = 9,
		PobranieK = 10,
		KartaPodarunkowa = 11,
		Bony = 12,
		Grupa = 13,
		PrePaid = 14,
		GoldFinger = 15,
		Kompensata = 16,
		iKasa = 17,
		Dodatkowa100 = 18,
		PunktyTEST = 19,
		Przelewy24 = 20,
		Bon = 21,
		DOD = 22,
		NaGrup�ZewnFiskal = 23,
		NaPok�jZewnFiskal = 24,
		KredytWewFiskal = 25,
		BiletParkingowy = 26,
		Opakowania = 27,
		Voucher = 28
	};

protected:
	unsigned int maxpay = 29;
	int ct[29];
	int base[29] ; //{ 0,1,2,4,5,6,9,10,11,13,15,17,21,24,26,28,30,36,103,110,111,112,113,116,117,118,120,48,104 };
public:
	payment_base() {
			base[HospIdx::Unknow] = 0;
			base[HospIdx::KartaKredytowa] = 1;
			base[HospIdx::Gotowka] = 2;
			base[HospIdx::Przelew] = 4;
			base[HospIdx::U�ytekW�asny] = 5;
			base[HospIdx::Kredyt] = 6;
			base[HospIdx::Pobranie] = 9;
			base[HospIdx::Przedp�ata] = 10;
			base[HospIdx::NaPokoj] = 11;
			base[HospIdx::Dodatkowa2] = 13;
			base[HospIdx::PobranieK] = 15;
			base[HospIdx::KartaPodarunkowa] = 17;
			base[HospIdx::Bony] = 21;
			base[HospIdx::Grupa] = 24;
			base[HospIdx::PrePaid] = 26;
			base[HospIdx::GoldFinger] = 28;
			base[HospIdx::Kompensata] = 30;
			base[HospIdx::iKasa] = 36;
			base[HospIdx::Dodatkowa100] = 103;
			base[HospIdx::PunktyTEST] = 101;
			base[HospIdx::Przelewy24] = 111;
			base[HospIdx::Bon] = 112;
			base[HospIdx::DOD] = 113;
			base[HospIdx::NaGrup�ZewnFiskal] = 116;
			base[HospIdx::NaPok�jZewnFiskal] = 117;
			base[HospIdx::KredytWewFiskal] = 118;
			base[HospIdx::BiletParkingowy] = 120;
			base[HospIdx::Opakowania] = 48;
			base[HospIdx::Voucher] = 104;
	}
	virtual int operator()(const int& to_convert) {
		return convert(to_convert);
	}
	virtual int convert(const int& to_convert) {
		for (unsigned int I = 0; I < maxpay; I++) {
			if (base[I] == to_convert) {
				return ct[I];
			}
		}
		return ct[HospIdx::Unknow];
	}
};

/*
= 0 : GOT�WKA,
= 1 : KARTA,
= 2 : CZEK,
= 3 : BON,
= 4 : INNA,
= 5 : KREDYT,
= 6 : KONTO KLIENTA,
= 7 : VOUCHER
= 8 : WALUTA
*/
class payment_to_thermal : public payment_base {
public:
	payment_to_thermal() {
		for (auto& el : ct) { el = 4; }
		ct[HospIdx::Unknow] = 0;
		ct[HospIdx::KartaKredytowa] = 1;
		ct[HospIdx::Gotowka] = 0;
		ct[HospIdx::Przelew] = 5;
		ct[HospIdx::Kredyt] = 5;
		ct[HospIdx::Bon] = 3;
		ct[HospIdx::Bony] = 3;
		ct[HospIdx::NaPokoj] = 8;
	}
};

/**
 * @brief
0 � got�wka
2 � karta
3 � czek
4 � bon
5 � kredyt
6 � inna
7 � voucher
8 � przelew
*/
class payment_to_posnet : public payment_base {
public:
	payment_to_posnet() {
		for (auto& el : ct) { el = 6; }
		ct[HospIdx::Unknow] = 0;
		ct[HospIdx::KartaKredytowa] = 2;
		ct[HospIdx::Gotowka] = 0;
		ct[HospIdx::Przelew] = 8;
		ct[HospIdx::Kredyt] = 5;
		ct[HospIdx::Bon] = 4;
		ct[HospIdx::Bony] = 4;
		ct[HospIdx::iKasa] = 9;
		ct[HospIdx::Przelewy24] = 9;
		ct[HospIdx::Voucher] = 7;
	}
};

/**
* 0: Got�wka,
* 1: Karta,
* 2: Czek,
* 3: Bon,
* 4: Inna,
* 5: Kredyt,
* 6: Konto klienta,
* 7: Waluta (protok� NOVITUS), Voucher (protok� �Novitus zgodny�)
* 8: Przelew
* 9: Mobilna
* 10: Voucher
*/
class payment_to_novitus : public payment_base {
public:
	payment_to_novitus() {
		for (auto& el : ct) { el = 4; }
		ct[HospIdx::Unknow] = 4;
		ct[HospIdx::KartaKredytowa] = 1;
		ct[HospIdx::Gotowka] = 0;
		ct[HospIdx::Przelew] = 8;
		ct[HospIdx::Kredyt] = 5;
		ct[HospIdx::Bon] = 3;
		ct[HospIdx::Bony] = 3;
		ct[HospIdx::iKasa] = 9;
		ct[HospIdx::Przelewy24] = 9;
		ct[HospIdx::PrePaid] = 6;
		ct[HospIdx::NaGrup�ZewnFiskal] = 6;
		ct[HospIdx::NaPok�jZewnFiskal] = 6;
		ct[HospIdx::KredytWewFiskal] = 5;
		ct[HospIdx::Voucher] = 10;	
	}
};

/**
* 0: Got�wka,
* 1: Karta,
* 2: Czek,
* 3: Bon,
* 4: Inna,
* 5: Kredyt,
* 6: Konto klienta,
* 7: Waluta (protok� NOVITUS), Voucher (protok� �Novitus zgodny�)
* 8: Przelew
* 9: Mobilna
* 10: Voucher
*/
class payment_elzab : public payment_base {
public:
	payment_elzab() {
		for (auto& el : ct) { el = 0; }
		ct[HospIdx::Unknow] = 1;
		ct[HospIdx::Gotowka] = 1;
		ct[HospIdx::KartaKredytowa] = 2;
		ct[HospIdx::Przelew] = 7;
		ct[HospIdx::Kredyt] = 6;
	}
};
