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
