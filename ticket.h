#pragma once
#include<QHttpServer>
#include "initDB.h"
class UserGetTicketsNum {
public:
	static void setupRoute(QHttpServer &server);
};

class RootPushTickets {
public:
	static QString getFlightNumber();
	static int getPrice();
	static QString getAirport();
	static int getFlyTime();
	static void setupRoute(QHttpServer& server);
};

class UserGetTickets {
public:
	static void setupRoute(QHttpServer& server);
};
