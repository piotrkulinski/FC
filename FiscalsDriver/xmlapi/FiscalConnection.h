#pragma once
#include "ConnectionType.h"
#include <string>
#include "../pugixml.hpp"

#define GET_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); })
#define GET_VALUE_NODE(node,element) node.find_node([&](pugi::xml_node node) { return (_stricmp(node.name(), element) == 0); }).first_child().value()

using namespace std;
/**
 * @author Piotr Kuliñski
 * @brief Struktura po³¹czenia z portem komunikacyjnym
*/
struct FiscalConnection {
	ConnectionType typ = ConnectionType::Unknow;
	struct {
		string ip{ "" };
		string com{ "" };
		string port{ "" };
		unsigned long baudrate = 9600;
	} settings;

	FiscalConnection() {}
	FiscalConnection(const pugi::xml_node& conn) {
		typ = static_cast<ConnectionType>(atoi(GET_VALUE_NODE(conn, "Type")));
		pugi::xml_node cfgport = GET_NODE(conn, "Port");
		if (!cfgport.empty()) {
			if (typ == ConnectionType::RS232) {
				settings.com = GET_VALUE_NODE(cfgport, "Com");
				settings.baudrate = static_cast<long>(atol(GET_VALUE_NODE(cfgport, "BaudRate")));
			}
			else if (typ == ConnectionType::TCPIP) {
				settings.ip = GET_VALUE_NODE(cfgport, "IP");
				settings.port = static_cast<long>(atol(GET_VALUE_NODE(cfgport, "Port")));
			}
		}
	}
};