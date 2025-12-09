#pragma once
#include<QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include "initDB.h"

class AddStar {
public:
	static void setupRoute(QHttpServer& server);
};

class RemoveStar {
public:
	static void setupRoute(QHttpServer& server);
};

class GetStarCount {
public:
	static void setupRoute(QHttpServer& server);
};

class GetStarTickets {
public:
	static void setupRoute(QHttpServer& server);
};