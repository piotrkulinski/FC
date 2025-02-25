#pragma once
#include <string>
#include "../pugixml.hpp"
#include "XMLRequest.h"
#include <minwinbase.h>
#include <sysinfoapi.h>

typedef struct {
	size_t year = 2022;
	size_t month = 1;
	size_t day = 1;
} FiscalsData;

//template <class T>
class XMLPeriodicReport : public XMLRequest {
public:
	FiscalsData from;
	FiscalsData to;

	virtual XMLPeriodicReport& getType() override {
		return *this;
	}

	XMLPeriodicReport(pugi::xml_node request) {
		SYSTEMTIME st;
		GetLocalTime(&st);

		if (pugi::xml_node _from = GET_NODE(request, "From"); !_from.empty()) {
			from.year = _from.attribute("year").as_uint(st.wYear);
			from.month = _from.attribute("month").as_uint(st.wMonth);
			from.day = _from.attribute("day").as_uint(st.wDay);
			if (from.year <= 0) { from.year = st.wYear; }
			if (from.month <= 0) { from.month = st.wMonth; }
			if (from.day <= 0) { from.day = st.wDay; }
		}
		if (pugi::xml_node _to = GET_NODE(request, "To"); !_to.empty()) {
			to.year = _to.attribute("year").as_uint(st.wYear);
			to.month = _to.attribute("month").as_uint(st.wMonth);
			to.day = _to.attribute("day").as_uint(st.wDay);
			if (to.year <= 0) { to.year = st.wYear; }
			if (to.month <= 0) { to.month = st.wMonth; }
			if (to.day <= 0) { to.day = st.wDay; }
		}
	}

};
