#pragma once
#include <Windows.h>
#include <string>
#include <mutex>
#include <map>
#include <sysinfoapi.h>

typedef struct {
	std::string email;
	std::string password;
	std::string name;
	std::string lastName;
	int64_t session;
} UserInformation;

using UserDatabase = std::map<std::string, UserInformation>;

class UserManagerException : public std::exception {
	std::string _message;
public:
	UserManagerException(const std::string& message) : _message(message) { }
	const char* what() const throw() {
		return _message.c_str();
	}
};

class UserManager {

private:
	UserDatabase usersDB;
public:
	bool logout(int session) throw(UserManagerException);
	void signUp(const UserInformation& userInfo) throw(UserManagerException);
	bool signOn(const std::string email, const std::string password, UserInformation& userInfo);
	bool checkSession(int64_t nsession);
};

