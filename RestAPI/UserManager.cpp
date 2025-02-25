#pragma once
#include "UserManager.h"

std::mutex usersDBMutex;
// alias declaration of our In Memory database...
//using UserDatabase = std::map<std::string, UserInformation>;
//UserDatabase usersDB
//{
//	{"user@emaildomain.com",{"user@emaildomain.com","secretword\n","Piotr","Kuliñski",0}},
//	{"piotr.kulinski@gmail.com",{"piotr.kulinski@gmail.com","JHh67%j$#8#JHJHGHJ@","Piotr","Kuliñski",0}}
//};

bool UserManager::checkSession(int64_t nsession) {
	std::unique_lock<std::mutex> lock{ usersDBMutex };
	for (const auto& usr : usersDB) {
		if (usr.second.session == nsession) {
			return true;
		}
	}
	return false;
}

void UserManager::signUp(const UserInformation& userInfo) throw(UserManagerException) {
	std::unique_lock<std::mutex> lock{ usersDBMutex };
	if (usersDB.find(userInfo.email) != usersDB.end()) {
		//throw UserManagerException("user already exists!");
	} else {
	usersDB.insert(std::pair<std::string, UserInformation>(userInfo.email, userInfo));
	}
}

bool UserManager::logout(int session) throw(UserManagerException) {
	std::unique_lock<std::mutex> lock{ usersDBMutex };
	for (const auto& usr : usersDB) {
		if (usr.second.session == session) {
			usersDB[usr.first].session=0;
			return true;
		}
	}
	return false;
}

/**
 * @brief Logowanie u¿ytkiwnika
 * @param email email
 * @param password has³o dostêpowe
 * @param userInfo informacja o u¿ytkowniku, do bazy jest wrzucany numer nowej sesji
 * @return true - zalogowano poprawnie
*/
bool UserManager::signOn(const std::string email, const std::string password, UserInformation& userInfo) {
	if (usersDB.find(email) != usersDB.end()) {
		auto ui = usersDB[email];
		if (ui.password == password) {
			ui.session = GetTickCount64(); //create id session
			usersDB[email].session = ui.session;
			userInfo = ui;
			return true;
		}
	}
	return false;
}