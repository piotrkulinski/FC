#pragma once
#include <string>
#include "../pugixml.hpp"
#include "XMLRequest.h"

class XMLCashDocument : public XMLRequest {
public:
	size_t type;
	size_t paymentId;
	bool correct;
	double value;
	std::string description;

	virtual XMLCashDocument& getType() override {
		return *this;
	}

	XMLCashDocument(pugi::xml_node request) : type(0), paymentId(0), correct(false), value(0.00f), description("") {
		pugi::xml_node data = GET_NODE(request, "Data");
		type = data.attribute("type").as_uint(type);
		paymentId = data.attribute("paymentId").as_uint(paymentId);
		correct = data.attribute("correct").as_bool(correct);
		value = data.attribute("value").as_double(value);
		description = data.attribute("description").as_string("");
	}

};