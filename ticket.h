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

class UserGetTicketDetails {
public:
	static void setupRoute(QHttpServer& server);
};

class UserGetRemainingTicketsNum {
public:
	static void setupRoute(QHttpServer& server);
};

class UserBuyTicket {
public:
	static void setupRoute(QHttpServer& server);
};

class UserGetOwnTicketsNum {
public:
	static void setupRoute(QHttpServer& server);
};

class UserGetOwnTickets {
public:
	static void setupRoute(QHttpServer& server);
};

class UserRefundTicket {
public:
	static void setupRoute(QHttpServer& server);
};

class UserGetOrderDetails{
	public:
		static void setupRoute(QHttpServer& server);
};

class AdminAddTicket {
public:
	static void setupRoute(QHttpServer& server);
};

class AdminDeleteFlight {
public:
	static void setupRoute(QHttpServer& server);
};

class AdminSearchTickets {
public:
	static void setupRoute(QHttpServer& server);
};

class AdminSearchTicketsCount {
public:
	static void setupRoute(QHttpServer& server);
};
