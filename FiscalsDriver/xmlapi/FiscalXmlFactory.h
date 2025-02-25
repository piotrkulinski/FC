#pragma once
#include <atlstr.h>
#include "FiscalXmlApi.h"
#include "../Paragon.h"

class FiscalXmlFactory {
private:
	FiscalXmlFactory() {}
public:
	static unique_ptr<FiscalXmlApi> Create(CString creceipt_xml) noexcept {

		auto api = std::make_unique<FiscalXmlApi>(creceipt_xml);
		if (api->CheckCode() != 0) {
			return std::move(api);
		}
		pugi::xml_document& xml = api->getXml();
		pugi::xml_node first = xml.first_child();
		if (strcmp(first.name(), "Paragon") == 0) {
			auto paragon = make_unique<Paragon>(xml);
			return std::move(paragon);
		}
	}
};