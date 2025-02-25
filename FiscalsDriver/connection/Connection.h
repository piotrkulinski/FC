#pragma once
#include <string>
#include <iostream>
#include "ConfigurationPort.h"
#include "ConnectionType.h"
#include "ConnectionState.h"

/**
Funkcja kontroluj¹ca odczytany bufor\n
Kontrola np. znaku koñcz¹cego odczytany pakiet dla specyficznego protoko³u
*/
typedef int(__stdcall* ControlBuffer)(const char* context, size_t len_context);

class Connection {

protected:
	bool open;
	ConnectionState state;
	ConfigurationPort settings;

public:
	ConnectionType connectionType;
	Connection():connectionType(ConnectionType::Unknow),open(false) {}
	virtual ~Connection() {
		std::cout << "Destructor " << __FUNCTION__ << std::endl;
	}

	virtual ConnectionState Open(){return ConnectionState::UNKNOW;};
	virtual ConnectionState Close(){return ConnectionState::UNKNOW;};
	virtual void ClearPort(){};
	virtual int ClearError() { return 0; };
	virtual int WriteByte(unsigned char ucByte) { return 0; };
	virtual int WriteBytes(const std::string& buffer) { return 0; };
	virtual int WriteSingleByte(unsigned char byte) { return 0; };
	virtual int XReadData(unsigned char* buffer, int limit) { return 0; };
	virtual unsigned long ReadByte(unsigned char* buffer) { return 0; };
	virtual int ReadData(unsigned char* buffer, int limit) { return 0; };
	virtual int ReadData(unsigned char* buffer, int offset, int limit) { return 0; };
	virtual int WaitReadData(std::stringstream& ss, unsigned char stop = (char)0x03, unsigned long long readTimeout = 5000) { return 0; };
	virtual int WaitReadData(std::stringstream& ss, ControlBuffer fControlFrame = nullptr, unsigned long long readTimeout = 5000) { return 0; };
	/**
	 * @brief Wys³anie ci¹gu bajtów
	 * @param buffer - bufor do wysy³ki
	 * @param size - rozmiar bufora
	 * @return 0 lub != size - b³¹d, nie wys³ano wszystkich danych 
	*/
	virtual int SendData(unsigned char* buffer, int size) { return 0; };
	virtual bool IsOpened() {
		return open;
	}

	virtual void setSettings(ConfigurationPort& new_setting) {
		settings = new_setting;
	}

	virtual void getSettings(ConfigurationPort& new_setting) {
		new_setting = settings;
	}
	virtual ConfigurationPort& getSettings() {
		return settings;
	}
	ConnectionType getConnectionType() {
		return connectionType;
	}

	bool isConnectionType(ConnectionType type) {
		return (connectionType==type);
	}
	virtual int WaitForData(std::stringstream& ss, ControlBuffer fControlFrame=nullptr, unsigned long long maxTime = 5000) {
		return 0;
	}
};
