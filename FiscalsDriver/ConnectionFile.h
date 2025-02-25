#pragma once

#include <fstream>
#include <sstream>
#include "Connection.h"

class ConnectionFile : public Connection
{
private:
	std::ofstream fout;

public:
	virtual ConnectionState Open() override {
		fout.open("par.txt", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
		return (fout.is_open() ? ConnectionState::OPEN : ConnectionState::OPEN_FAILED);
	}
	virtual bool IsOpened() override {
		return fout.is_open();
	}
	virtual ConnectionState Close() override {
		if (fout.is_open()) {
			fout.close();
		}
		return ConnectionState::CLOSE;
	}
	virtual int WriteByte(unsigned char ucByte) override {
		fout << ucByte;
		return 1;
	}
	virtual int WriteBytes(const std::string& buffer) override {
		fout << buffer;
		return buffer.length();
	}
	virtual int SendData(unsigned char* buffer, int size) override {
		fout << buffer;
		return size;
	}
	virtual int ClearError() override {
		return 0;
	}
	virtual int ReadData(unsigned char* buffer, int limit) override {
		if (limit >= 2) {
			strcpy((char*)buffer, "ok");
			return 2;
		}
		return 0;
	}

	virtual unsigned long ReadByte(unsigned char* buffer) override {
		strcpy((char*)buffer, "ok");
		return 1;
	}

	virtual int XReadData(unsigned char* buffer, int limit) override {
		if (limit >= 2) {
			strcpy((char*)buffer, "ok");
			return 2;
		}
		return 0;
	}
	virtual int ReadData(unsigned char* buffer, int offset, int limit) override {
		if (limit >= 2) {
			strcpy((char*)buffer, "ok");
			return 2;
		}
		return 0;
	}
	virtual int WaitReadData(std::stringstream& ss, unsigned char stop='\0', unsigned long long readTimeout = 5000) override {
		ss << "OK";
		return 2;
	}
	virtual int WaitReadData(std::stringstream& ss, ControlBuffer fControlFrame=nullptr, unsigned long long readTimeout=5000) override {
		ss << "OK";
		return 2;
	}
	virtual int WriteSingleByte(unsigned char byte) override {
		fout << byte;
		return 1;
	}
	virtual void ClearPort() {

	}
};

