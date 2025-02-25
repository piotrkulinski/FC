#pragma once
#include <any>
#include <tuple>
#include "FiscalControlType.h"
#include "FiscalControlActionType.h"

#define GET_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); })
#define GET_VALUE_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); }).first_child().value()

/**
 * @author Piotr Kuliñski
 * @brief Dodatkowe parametry kontrolne przes³ane z paragonem \n
 * np. otwarcie szuflady, gong
 * \code{xml}
 * <Sterowanie>
 *		<OtworzSzuflade>1</OtworzSzuflade>
 * </Sterowanie>
 * \endcode
*/
struct FiscalControl {

	std::vector<std::tuple<FiscalControlType, FiscalControlActionType, std::any>> action;
	FiscalControl() {};
	FiscalControl(const pugi::xml_node& node)
	{
		auto GetActionType = [](pugi::xml_node& node) {
			FiscalControlActionType type{ FiscalControlActionType::Unknow };
			std::string typ = node.attribute("typ").as_string("After");
			if (typ.compare("After")) {
				type = FiscalControlActionType::After;
			}
			else if (typ.compare("Before")) {
				type = FiscalControlActionType::Before;
			}
			else if (typ.compare("BeforeAndAfter")) {
				type = FiscalControlActionType::BeforeAndAfter;
			}
			return type;
		};

		pugi::xml_node drawer = GET_NODE(node, "OtworzSzuflade");
		if (!drawer.empty()) {
			bool openDrawer = (strcmp(drawer.first_child().value(), "1") == 0);
			action.push_back({ FiscalControlType::OpenDrawer,GetActionType(drawer),openDrawer});
		}
	}
};
