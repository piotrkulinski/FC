#pragma once
#include "..\StdAfx.h"
#include <chrono>
#include "Connection.h"
#include "ConfigurationRS232.h"

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04
#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

#define READ_TIMEOUT	3000 //1000

using namespace std::chrono_literals;

class ConnectionRS232 : public Connection {

private:
	boolean Reconnect(u_int& counter) {
		while (++counter < 24) {
			Beep(950, 30); Beep(850, 40);
			Sleep(5000);
			if (Open() == ConnectionState::OPEN) {
				return true;
			}
		}
		return false;
	}

	bool checkConnection(DWORD& modemStatus) {
		if (GetCommModemStatus(handle, &modemStatus)) {
			// Sprawdzenie statusu sygna³u DCD (Data Carrier Detect)
			if (modemStatus & MS_RLSD_ON) {
				std::cout << "Po³¹czenie aktywne (DCD on)" << std::endl;
				return true; // Po³¹czenie jest aktywne
			}
			std::cout << "Po³¹czenie przerwane (DCD off)" << std::endl;
		}
		else {
			std::cerr << "B³¹d podczas sprawdzania statusu po³¹czenia." << std::endl;
		}
		Beep(950, 30); Beep(850, 40);
		return false;
	}

	boolean checkError(COMSTAT& status) {
		DWORD errors{ 0 };
		if (ClearCommError(handle, &errors, &status)) {
			if (errors != 0) {
				std::cout << "ClearCommError result: " << errors << std::endl;
				if (errors & CE_BREAK) {
					std::cout << "Wykryto BREAK w transmisji." << std::endl;
				}
				if (errors & CE_FRAME) {
					std::cout << "B³¹d ramki." << std::endl;
				}
				if (errors & CE_OVERRUN) {
					std::cout << "Przepe³nienie bufora." << std::endl;
				}
				if (errors & CE_RXOVER) {
					std::cout << "Przepe³nienie bufora odbioru." << std::endl;
				}
			}
			return (errors != 0);
		}
		std::cout << "B³¹d podczas odczytu danych." << std::endl;
		return (true);
	}

protected:
	HANDLE handle{ NULL };
	OVERLAPPED m_OverlappedRead;
	OVERLAPPED m_OverlappedWrite;
	ConfigurationRS232 settings;
	DCB dcb;

public:
	inline static unsigned char control_char='\0';

	ConnectionRS232() {
		connectionType = ConnectionType::RS232;
	}

	virtual ~ConnectionRS232() {
		std::cout << "Destructor " << __FUNCTION__ << std::endl;
		if (IsOpened()) {
			ClearPort();
			Close();
		}
	}

	virtual ConnectionState Open() override;
	virtual bool IsOpened() override;
	virtual ConnectionState Close() override;
	virtual int WriteByte(unsigned char ucByte) override;
	virtual int WriteBytes(const std::string& buffer) override;
	virtual int SendData(unsigned char* buffer, int size) override;
	virtual int ClearError() override;
	virtual int ReadData(unsigned char* buffer, int limit) override;
	virtual unsigned long ReadByte(unsigned char* buffer) override;
	virtual int XReadData(unsigned char* buffer, int limit) override;
	virtual int ReadData(unsigned char* buffer, int offset, int limit) override;
	virtual int WaitReadData(std::stringstream& ss, unsigned char stop, u_int64 readTimeout = READ_TIMEOUT) override;
	virtual int WaitReadData(std::stringstream& ss, ControlBuffer fControlFrame, u_int64 readTimeout = READ_TIMEOUT) override;
	virtual int WriteSingleByte(unsigned char byte) override;
	virtual void ClearPort();

	virtual void setSettings(ConfigurationPort& cfg) {
		ConfigurationRS232 rs = (ConfigurationRS232&)cfg;
		settings = rs;
	}
	virtual void getSettings(ConfigurationRS232& new_setting) {
		new_setting = settings;
	}
	virtual ConfigurationRS232& getSettings() override {
		return settings;
	}

	int WaitForData(std::stringstream& ss, ControlBuffer fControlFrame = nullptr, u_int64 maxTime = READ_TIMEOUT) override;
	
};