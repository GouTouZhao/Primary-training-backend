#pragma once
#include<QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include "initDB.h"

class GetCurrency {
public:
	static void setupRoute(QHttpServer& server);
};

class AddCurrency {
public:
	static void setupRoute(QHttpServer& server);
};

class SubtractCurrency {
public:
	static void setupRoute(QHttpServer& server);
};