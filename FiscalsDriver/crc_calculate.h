#pragma once

#include <string>
#include <sstream>
#include <iomanip>


namespace stx {
	uint16_t crc(std::string& buff);
}

namespace posnet {
	unsigned int crc(std::string& buff);
}

namespace thermal {
	std::string crc(const std::ostringstream& req);
}