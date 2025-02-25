#pragma once
#include "../pugixml.hpp"
#include "ConnectionType.h"

class ConfigurationPort {
public:
	ConnectionType type { ConnectionType::Unknow};
	ConfigurationPort(){}
	ConfigurationPort(const pugi::xml_node& conn){}
};