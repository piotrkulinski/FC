#pragma once
#include <string>
#include "ConfigurationPort.h"

class ConfigurationTCP : public ConfigurationPort {
public:
	std::string ip{ "127.0.0.1" };
	unsigned long port{ 8282 };

	ConfigurationTCP& operator=(ConfigurationTCP& cfg) {
		type = cfg.type;
		ip = cfg.ip;
		port = cfg.port;
		return *this;
	}

	ConfigurationTCP() {
		type = ConnectionType::TCP;
	}
	ConfigurationTCP(const pugi::xml_node& conn) {
		pugi::xml_attribute port_type = conn.attribute("type");
		type = static_cast<ConnectionType>(port_type.as_int(-1));
		pugi::xml_attribute ip = conn.attribute("ip");
		ip = ip.as_string("127.0.0.1");
		pugi::xml_attribute nport = conn.attribute("port");
		port = nport.as_llong(0);
	}
};
