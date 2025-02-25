#pragma once
#include <string>
#include "../pugixml.hpp"
#include "XMLRequest.h"
#include <minwinbase.h>
#include <sysinfoapi.h>

//template <class T>
class XMLDailyReport : public XMLRequest {
public:
	size_t year = 2022;
	size_t month = 1;
	size_t day = 1;

	virtual XMLDailyReport& getType() override {
		return *this;
	}

	XMLDailyReport(pugi::xml_node request) {
		SYSTEMTIME st;
		GetLocalTime(&st);

		year = request.attribute("year").as_uint(st.wYear);
		month = request.attribute("month").as_uint(st.wMonth);
		day = request.attribute("day").as_uint(st.wDay);
		if (year <= 0) { year = st.wYear; }
		if (month <= 0) { month = st.wMonth; }
		if (day <= 0) { day = st.wDay; }
	}

};