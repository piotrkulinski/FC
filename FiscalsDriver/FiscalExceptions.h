#pragma once
#include <string>
#include <sstream>
#include <exception>
#include "FiscalError.h"

class test_exception : std::exception {
private:
	const char* message;
public:
	test_exception(const char* msg) : std::exception(msg) {
		message = msg;
	}
	virtual const char* what() const {
		return message;
	}
};

template <typename T>
class fiscal_exception : std::exception {
private:
	std::string message{""};
	T last_error = (T)0;

public:
	fiscal_exception(const char* msg) : std::exception(msg) {
		Beep(750, 300);
		std::stringstream ss; ss << "Wyj¹tek podczas fiskalizacji\n" << msg;
		message = ss.str();
	}	

	fiscal_exception(T error_no) {
		Beep(750, 300);
		last_error = error_no;
	}
	fiscal_exception(T error_no, const char* msg) : fiscal_exception(msg) {
		last_error = error_no;
		std::stringstream ss; ss << "\nerror: " << static_cast<long>(error_no);
		message += ss.str();
	}
	virtual const char* what() const {
		return message.c_str();
	}

	T getErrorNo() {
		return (T)last_error;
	}
};