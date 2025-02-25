#pragma once

#if defined(MOCK_PORT)

#include "stdafx.h"
#include <string>
#include <atlstr.h>
#include "MyUtils.h"
#include <iomanip>
#include <iostream>
#include "logger_definition.h"

class MOCK_SERIAL_PORT_THERMAL : public CSerial {
public:
	MOCK_SERIAL_PORT_THERMAL() {}

	string lastcommand;
	virtual const bool IsOpened() { return true; };
	virtual int Open(CString p, long baudrate) { return 1; };
	virtual int Open(CString p, long baudrate, cport::FlowControl flow) { return 1; };
	virtual int Open(CString sPort, int baudRate, int dataBits, cport::Parity parity, cport::StopBits stopBits, cport::FlowControl flowControl) { return 1; };
	virtual int WriteByte(unsigned char z) {
		lastcommand = z;
		LOGGER_RAW(0, lastcommand);
		return 1;
	};
	virtual int WriteSingleByte(unsigned char z) {
		lastcommand = z;
		LOGGER_RAW(0, lastcommand);
		return 1;
	};
	int WaitReadData(std::stringstream& ss, unsigned char z) {
		if (!lastcommand.empty() && lastcommand.find("P23#s") != std::string::npos) {
			ss << "P2#X0;0;0;0;1;0;0;1;1/23.00/08.00/05.00/00.00/100.00/101.00/101.00/283/13006.34/10077.70/22./44./122./0./0./121037209.13/BEV13511565D9\\";
			LOGGER_RAW(1, ss.str());
			return ss.str().length();
		}
		ss << lastcommand;
		LOGGER_RAW(0, ss.str());
		return ss.str().length();
	}
	virtual int WriteBytes(const std::string& request) {
		LOGGER_RAW(0, request);
		lastcommand = request;
		return request.length();
	};
	virtual void ClearPort() {

	}
	MOCK_SERIAL_PORT_THERMAL& operator<<(const std::ostringstream b) {
		WriteBytes(b.str());
		return (*this);
	}

	virtual MOCK_SERIAL_PORT_THERMAL& operator>>(DLE& b) {
		if (WriteByte((char)0x10) == 1) {
			Sleep(50);
			if (unsigned char byte_dle = 0; XReadData(&byte_dle, 1) == 1) {
				b = { byte_dle };
			}
			else {
				b = { 1 }; //err
			}
		}
		return (*this);
	}

	virtual int ReadData(unsigned char* b, int len) {
		LOGGER_RAW(1, lastcommand);
		return len;
	};	
	virtual int ReadData(unsigned char* b, int len, int xlen) {
		LOGGER_RAW(1, lastcommand);
		return len;
	};
	virtual int XReadData(unsigned char* b, int len) {
		LOGGER_RAW(1, lastcommand);
		if (lastcommand[0] == 0x05) {
			b[0] = (char)0x65;
			return 1; //01100101b;
		}
		if (lastcommand[0] == 0x10) {
			b[0] = (char)0x65;
			return 1; //01100101b;
		}
		return 0;
	}
	virtual void ClearError() {}
	virtual void Close() {}
};

class MOCK_SERIAL_PORT_POSNET : public MOCK_SERIAL_PORT_THERMAL {
public:
	MOCK_SERIAL_PORT_POSNET() {}

	int WaitReadData(std::stringstream& ss, unsigned char z) {
		if (!lastcommand.empty() && lastcommand.find("scid") != std::string::npos) {
			ss << "sid	nmPOSNET Thermal HS FV EJ	vr6.03	#E37D";
			LOGGER_RAW(1, ss.str());
			return ss.str().length();
		}
		if (!lastcommand.empty() && lastcommand.find("vatget") != std::string::npos) {
			ss << "vatget	va23,00	vb8,00	vc5,00	vd0,00	ve100,00	vf101,00	vg101,00	#635C";
			LOGGER_RAW(1, ss.str());
			return ss.str().length();
		}
		ss << lastcommand;
		LOGGER_RAW(0, ss.str());
		return ss.str().length();
	}

	MOCK_SERIAL_PORT_POSNET& operator<<(posnet::crc_stream& b) {
		WriteBytes(b.str());
		return (*this);
	}
	MOCK_SERIAL_PORT_POSNET& operator>>(stringstream& b) {
		WaitReadData(b, (char)0x03);
		return (*this);
	}
	//virtual MOCK_SERIAL_PORT_POSNET& operator>>(DLE& b) {
	//	if (WriteByte((char)0x10) == 1) {
	//		Sleep(50);
	//		if (unsigned char byte_dle = 0; XReadData(&byte_dle, 1) == 1) {
	//			b = { byte_dle };
	//		}
	//		else {
	//			b = { 0x01 }; //err
	//		}
	//	}
	//	return (*this);
	//}
};
#endif