#pragma once
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include "ConverterType.h"

/**
 * @brief Podstawowe konwersje typów generycznych
*/
class ConverterBase {
public:
	virtual std::string convert(std::unordered_map<std::string, std::string>& mp) {return "";}
	virtual std::string convert(std::string& txt) { return txt; };
	virtual std::string convert(std::map<std::string, std::string>& mp) { return ""; };
	virtual std::string convert(std::map<std::string, double>& mp) { return ""; };
	virtual std::string convert(std::vector<std::pair<std::string, double>>& A) { return ""; };
	~ConverterBase() {
		std::cout << __FUNCTION__;
	}
};

/**
 * @brief Konwerter wyniku na tekst podzielony œrednikiem
*/
class ConverterCOMATEXT : public ConverterBase {
public:
	virtual std::string convert(std::unordered_map<std::string, std::string>& mp) {
		std::stringstream ss;
		for_each(mp.begin(), mp.end(), [&](const auto el) { ss << el.second << ";"; });
		ss.seekp(-1, ss.end) << '\0';
		return ss.str();
	}
	virtual std::string convert(std::map<std::string, std::string>& mp) {
		std::vector< std::pair<std::string, std::string> > A;
		for (auto& it : mp) { A.push_back(it); }
		std::sort(A.begin(), A.end(), [&](auto& a, auto& b) {return a.first < b.first;	});
		std::stringstream ss;
		std::for_each(A.begin(), A.end(), [&](const auto el) { ss << el.second << ";"; });
		ss.seekp(-1, ss.end) << '\0';
		return ss.str();
	}
	virtual std::string convert(std::map<std::string, double>& mp) {
		std::vector< std::pair<std::string, double> > A;
		for (auto& it : mp) { A.push_back(it); }
		std::sort(A.begin(), A.end(), [&](auto& a, auto& b) {return a.first < b.first;	});
		std::stringstream ss;
		for_each(A.begin(), A.end(), [&](const auto el) {	ss << el.second << ";"; });
		ss.seekp(-1, ss.end) << '\0';
		return ss.str();
	}	
	virtual std::string convert(std::vector<std::pair<std::string, double>>& A) {
		std::stringstream ss;
		std::for_each(A.begin(), A.end(), [&](const auto el) {	ss << el.first << ";"; });
		ss.seekp(-1, ss.end) << '\0';
		return ss.str();
	}
};

/**
 * @brief Konwerter wyniku na JSON
*/
class ConverterJSON : public ConverterBase {
public:
	virtual std::string convert(std::unordered_map<std::string, std::string>& mp) {
		std::stringstream ss; ss << "{";
		for_each(mp.begin(), mp.end(), [&](const auto el) {
			ss << "\"" << el.first << "\":\"" << el.second << "\"" << ",";
		});
		ss.seekp(-1, ss.end) << "}";
		return ss.str();
	}
	virtual std::string convert(std::map<std::string, std::string>& mp) {
		std::vector< std::pair<std::string, std::string> > A;
		for (auto& it : mp) { A.push_back(it); }
		std::sort(A.begin(), A.end(), [&](auto& a, auto& b) {return a.first < b.first;	});
		std::stringstream ss;
		ss << "{";
		for_each(A.begin(), A.end(), [&](auto el) {
			ss << "\"" << el.first << "\":\"" << el.second << "\"" << ",";
			});
		ss.seekp(-1, ss.end) << "}";
		return ss.str();
	}
	virtual std::string convert(std::map<std::string, double>& mp) {
		std::vector< std::pair<std::string, double> > A;
		for (auto& it : mp) { A.push_back(it); }
		std::sort(A.begin(), A.end(), [&](auto& a, auto& b) {return a.first < b.first;	});
		return convert(A);
	}
	virtual std::string convert(std::vector< std::pair<std::string, double> >& A) {
		std::stringstream ss;
		ss << "{";
		std::for_each(A.begin(), A.end(), [&](auto el) {
			ss << "\"" << el.first << "\":\"" << std::fixed << std::setprecision(2) << el.second << "\"" << ",";
			});
		ss.seekp(-1, ss.end) << "}";
		return ss.str();
	}
};

/**
 * @brief Konwerter wyniku na XML
*/
class ConverterXML : public ConverterBase {
public:
	virtual std::string convert(std::unordered_map<std::string, std::string>& mp) {
		std::stringstream ss; ss << "<items>";
		std::for_each(mp.begin(), mp.end(), [&](const auto el) {
			ss << "<item name=\"" << el.first << "\" value=\"" << el.second << "\"" << "/>";
			});
		ss << "</items>";
		return ss.str();
	}
	virtual std::string convert(std::map<std::string, std::string>& mp) {
		std::vector< std::pair<std::string, std::string> > A;
		for (auto& it : mp) { A.push_back(it); }
		std::sort(A.begin(), A.end(), [&](auto& a, auto& b) {return a.first < b.first;	});
		std::stringstream ss;
		ss << "<items>";
		std::for_each(A.begin(), A.end(), [&](auto el) {
			ss << "<item name=\"" << el.first << "\" value=\"" << el.second << "\"" << "/>";
			});
		ss << "</items>";
		return ss.str();
	}
	virtual std::string convert(std::map<std::string, double>& mp) {
		std::vector< std::pair<std::string, double> > A;
		for (auto& it : mp) { A.push_back(it); }
		std::sort(A.begin(), A.end(), [&](auto& a, auto& b) {return a.first < b.first;	});
		return convert(A);
	}
	virtual std::string convert(std::vector<std::pair<std::string, double> >& A) {
		std::stringstream ss;
		ss << "<items>";
		std::for_each(A.begin(), A.end(), [&](auto el) {
			ss << "<item name=\"" << el.first << "\" value=\"" << std::fixed << std::setprecision(2) << el.second << "\"" << "/>";
			});
		ss << "</items>";
		return ss.str();
	}
};

/**
 * @brief Konwerter wyniku na RAW
*/
class ConverterRAW : public ConverterBase {
public:
	virtual std::string convert(std::unordered_map<std::string, std::string>& mp) {
		std::stringstream ss;
		std::for_each(mp.begin(), mp.end(), [&](const auto el) { ss << el.second; });
		return ss.str();
	}
	virtual std::string convert(std::map<std::string, std::string>& mp) {
		std::vector< std::pair<std::string, std::string> > A;
		for (auto& it : mp) { A.push_back(it); }
		std::sort(A.begin(), A.end(), [&](auto& a, auto& b) {return a.first < b.first;	});
		std::stringstream ss;
		std::for_each(A.begin(), A.end(), [&](auto el) {
			ss << el.second;
			});
		return ss.str();
	}
	virtual std::string convert(std::map<std::string, double>& mp) {
		std::vector< std::pair<std::string, double> > A;
		for (auto& it : mp) { A.push_back(it); }
		std::sort(A.begin(), A.end(), [&](auto& a, auto& b) {return a.first < b.first;	});
		return convert(A);
	}
	virtual std::string convert(std::vector<std::pair<std::string, double> >& A) {
		std::stringstream ss;
		std::for_each(A.begin(), A.end(), [&](auto el) {
			ss << std::fixed << std::setprecision(2) << el.second;
			});
		return ss.str();
	}
};

/**
 * @brief Pobranie konwertera wyników zwracanych przez DLL
*/
class GetConverter {
public:
	template <const ConverterType E>
	static std::unique_ptr<ConverterBase> get() {
		return get(E);
	}

	static std::unique_ptr<ConverterBase> get(ConverterType E) {
		std::unique_ptr<ConverterBase> ptr;
		switch (E) {
			case ConverterType::JSON: {
				ptr = std::make_unique<ConverterJSON>();
				break;
			}
			case ConverterType::COMATEXT: {
				ptr = std::make_unique<ConverterCOMATEXT>();
				break;
			}
			case ConverterType::RAW: {
				ptr = std::make_unique<ConverterRAW>();
				break;
			}
			default: {
				ptr = std::make_unique<ConverterXML>();
				break;
			}
		}
		return std::move(ptr);
	}
};



