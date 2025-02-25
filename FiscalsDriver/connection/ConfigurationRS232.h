#pragma once
#include "../pugixml.hpp"
#include "../MyUtils.h"
#include "cport.h"
#include "Connection.h"
#include "ConfigurationPort.h"

class ConfigurationRS232 : public ConfigurationPort {
public:
	std::string port{ "COM1" };
	unsigned long baudrate{ 9600 };
	unsigned char databits{ 8 };
	cport::FlowControl flowcontrol{ cport::FlowControl::NoFlowControl };
	cport::Parity parity{ cport::Parity::NoParity };
	cport::StopBits stopbits{ cport::StopBits::OneStopBit };

	ConfigurationRS232& operator=(ConfigurationRS232& cfg) {
		type = cfg.type;
		port = cfg.port;
		baudrate = cfg.baudrate;
		flowcontrol = cfg.flowcontrol;
		databits = cfg.databits;
		stopbits = cfg.stopbits;
		return *this;
	}
	ConfigurationRS232() {
		type = ConnectionType::RS232;
	}

	/**
	 * \brief XML po³¹czeniowy
	 * \headerfile connection\ConnectionType.h ""
	 * \param conn - deseriaizowany z XML-a \n
	 * \code{xml}
	 * <Connection type="[ConnectionType]" name="COM1" baudrate="9600" parity="E|M|N|O" flowcontrol="N|R|D|S|F" databits="1"/>
	 * \endcode
	 * \see [ConnectionType] (\ref ConnectionType)
	*/
	ConfigurationRS232(const pugi::xml_node& conn) {
		pugi::xml_attribute port_type = conn.attribute("type");
		type = static_cast<ConnectionType>(port_type.as_int(-1));
		pugi::xml_attribute name = conn.attribute("name");
		port = name.as_string("COM1");
		pugi::xml_attribute xbaudrate = conn.attribute("baudrate");
		baudrate = xbaudrate.as_llong(9600);
		pugi::xml_attribute xparity = conn.attribute("parity");
		parity = getParity(xparity.as_string("N"));
		pugi::xml_attribute flow = conn.attribute("flowcontrol");
		flowcontrol = getFlowControl(flow.as_string("N"));
		pugi::xml_attribute db = conn.attribute("databits");
		databits = db.as_uint(8);
	}
};