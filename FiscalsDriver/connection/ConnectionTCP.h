#pragma once

#include <winsock.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <ctype.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "iphlpapi.lib")
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#include "Connection.h"
#include "ConfigurationTCP.h"

struct TimeoutConnection {
	unsigned long open = 5000;
	unsigned long read = 5000;
	unsigned long write = 0;
	unsigned long discover = 10000;

};
class ConnectionTCP : public Connection {
protected:
	ConfigurationTCP settings;
	SOCKET fiscalDevice = INVALID_SOCKET;;
	struct sockaddr_in send_address;
	struct sockaddr_in read_address;
	int lasterror;
	TimeoutConnection timeout;

public:
	ConnectionTCP() {
		connectionType = ConnectionType::TCP;
	}	
	~ConnectionTCP() {
		std::cout << "Destructor " << __FUNCTION__ << std::endl;
	}

	virtual void setSettings(ConfigurationPort& cfg) {
		ConfigurationTCP rs = (ConfigurationTCP&)cfg;
		settings = rs;
	}
	virtual void getSettings(ConfigurationPort& new_setting) {
		new_setting = settings;
	}
	virtual ConfigurationPort& getSettings() {
		return settings;
	}

	virtual ConnectionState Open() {
		//constexpr short PACKAGE_ACCEPT{ 0x3A };

		auto checkError = [&]() {
			lasterror = WSAGetLastError();
			if (fiscalDevice != INVALID_SOCKET) {
				closesocket(fiscalDevice);
				fiscalDevice = INVALID_SOCKET;
			}
			WSACleanup();
			return -lasterror;
		};

		WSADATA data;
		memset((void*)&data, 0, sizeof(data));

		if (int r = WSAStartup(MAKEWORD(2, 2), &data); r == 0) {
			if ((fiscalDevice = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
				checkError();
				LOGGER_SS("connection error: " << lasterror);
				return ConnectionState::OPEN_FAILED;
			}
		}
		else {
			LOGGER_SS("WSA startup error: " << r);
			return ConnectionState::OPEN_FAILED;
		}

		memset(&send_address, 0, sizeof(send_address));
		send_address.sin_family = AF_INET;
		send_address.sin_addr.s_addr = inet_addr(settings.ip.c_str());
		send_address.sin_port = htons(settings.port);
		
		if (int r = connect(fiscalDevice, (sockaddr*)&send_address, sizeof(send_address)); r) {
			checkError();
			return ConnectionState::OPEN_FAILED;
		}

		if (timeout.read > 0) {
			setsockopt(fiscalDevice, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout.read, sizeof(timeout.read));
		}
		if (timeout.write > 0) {
			setsockopt(fiscalDevice, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout.write, sizeof(timeout.write));
		}

		return ConnectionState::OPEN;
	}

	virtual int SendData(unsigned char* buffer, int size) {
		int retval = 0;
		int baselen = size;
		bool attempt = false;
		while (fiscalDevice != INVALID_SOCKET && size > 0) {
			int r = send(fiscalDevice, (const char*)buffer, size, 0);
			if (r > 0) {
				buffer += r;
				size -= r;
				retval += r;
			}
			else if (r <= 0) {
				//if ((size == baselen) && !server && !attempt) {
				if ((size == baselen) && !attempt) {
					attempt = true;
					//if (try_reconnect()) {
					//	continue;
					//}
				}
				break;
			}
		}
		return retval;
	}

	virtual int ReadData(unsigned char* buffer, int limit) {
		int retval = 0;
		while (fiscalDevice != INVALID_SOCKET && limit > 0) {
			if (int r = recv(fiscalDevice, (char*)buffer, limit, 0); r > 0) {
				buffer += r;
				limit -= r;
				retval += r;
			}
			else if (r <= 0) {
				break;
			}
		}
		return retval;
	}

	virtual int ReadData(unsigned char* buffer, int offset, int limit) {
		return ReadData(&buffer[offset], limit);
	}

	virtual int XReadData(unsigned char* buffer, int limit) {
		return ReadData(buffer, limit);
	}

	virtual unsigned long ReadByte(unsigned char* buffer) {
		return ReadData(buffer, 1);
	}

	virtual int WriteByte(unsigned char ucByte) {
		return SendData(&ucByte, 1);
	}

	virtual int WriteBytes(const std::string& buffer) {
		return SendData((unsigned char*)buffer.c_str(), buffer.length());
	}

	virtual int WriteSingleByte(unsigned char byte)
	{
		LOGGER_RAW(0, (char)byte);
		return SendData(&byte,1);
	}

	virtual int WaitReadData(std::stringstream& ss, unsigned char stop) {
		char buffer[2]{'1','2'};
		int currentBytesRead = 1;
		if (currentBytesRead > 1 && (buffer[currentBytesRead - 1] == stop || stop == '\0')) {
			LOGGER_RAW(1, ss.str().c_str());
			return currentBytesRead;
		}
		return 0;
	}

	virtual int WaitReadData(std::stringstream& ss, ControlBuffer fControlFrame) {
		std::string buffer = "xx";
		int currentBytesRead = 1;
		if (currentBytesRead > 0 && fControlFrame((const char*)buffer.c_str(), currentBytesRead) == 0) {
			LOGGER_RAW(1, ss.str());
			return currentBytesRead;
		}
		return 0;
	}

	bool IsOpened() {

		if (fiscalDevice == INVALID_SOCKET) {
			LOGGER_SS("connecting unstable");
			return false;
		}

		int error = 0;
		socklen_t len = sizeof(error);
		if (getsockopt(fiscalDevice, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == SOCKET_ERROR) {
			lasterror = WSAGetLastError();
			LOGGER_SS("check connecting - is_connected error: " << lasterror);
			return false;
		}
		else if (error != 0) {
			LOGGER_SS("check connected error: " << error);
			return false;
		}

		return true;
	}

	ConnectionState Close()
	{
		auto dispossessed = [&]() {
			constexpr char PACKAGE_RELEASE = 0x2E;
			if (IsOpened() == 0) {
				//stringstream packageReleased; packageReleased
				//	<< PACKAGE_RELEASE
				//	<< (char)0b10000000 //flag 0b10000000 (0x80)- flaga LOCK - wyw³aszczenie
				//	<< setw(8) << setfill('0') << settings.tid;
				//if (write(packageReleased) > 0) { //zwalniamy i nie czytamy odpowiedzi, bo po³¹czenie zamkniete
				//	LogHelper::instance()->Stream(ostringstream() << __FUNCTION__" Terminal dispossessed");
				//}
			}
		};

		if (fiscalDevice != INVALID_SOCKET) {
			dispossessed();
			shutdown(fiscalDevice, SD_SEND);
			closesocket(fiscalDevice);
			fiscalDevice = INVALID_SOCKET;
			WSACleanup();
			//LogHelper::instance()->Stream(ostringstream() << __FUNCTION__": success");
			return ConnectionState::CLOSE_FAILED;
		}
		return ConnectionState::CLOSE;
	}

};