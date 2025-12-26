#pragma once
#include<QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include "initDB.h"

class UserRegister {
public:
	static void setupRoute(QHttpServer &server);
};

class UserLogin {
public:
	static void setupRoute(QHttpServer& server);
};

class UserInfo {
public:
	static void setupRoute(QHttpServer& server);
};

class UpdateProfileColor {
public:
	static void setupRoute(QHttpServer& server);
};

class UpdateUsername {
public:
	static void setupRoute(QHttpServer& server);
};

class AdminPasswordVerify {
public:
	static void setupRoute(QHttpServer& server);
};

class AdminGetAllUsers {
public:
	static void setupRoute(QHttpServer& server);
};

class AdminDeleteUser {
public:
	static void setupRoute(QHttpServer& server);
};
