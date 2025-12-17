#include <QCoreApplication>
#include <QHttpServer>
#include <QTcpServer>
#include <QHostAddress>
#include <QDebug>
#include<QHttpServerResponse>
#include <functional>
#include"initDB.h"
#include"Login.h"
#include"currency.h"
#include"star.h"
#include"staticsource.cpp"
#include"ticket.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (!MysqlInitDB::init()) {
        return -1;
    }

    QTcpServer tcpServer;
    QHttpServer server;
    
    if (!tcpServer.listen(QHostAddress::Any, 8080)) {
        qWarning() << "yes" << tcpServer.errorString();
        return -1;
    }
    server.bind(&tcpServer);
    server.route("/", []() {
        QHttpServerResponse res("Hello, HttpServer 已运行");
        addCorsHeaders(res);
        return res;
        });

    //接口列表
    UserRegister::setupRoute(server);
    UserLogin::setupRoute(server);
    UserInfo::setupRoute(server);
    UpdateProfileColor::setupRoute(server);
    UpdateUsername::setupRoute(server);
    AdminPasswordVerify::setupRoute(server);
    GetCurrency::setupRoute(server);
    AddCurrency::setupRoute(server);
    SubtractCurrency::setupRoute(server);
    AddStar::setupRoute(server);
    RemoveStar::setupRoute(server);
    GetStarCount::setupRoute(server);
    GetStarTickets::setupRoute(server);
    UserGetTicketsNum::setupRoute(server);
    RootPushTickets::setupRoute(server);
    UserGetTickets::setupRoute(server);
    UserGetTicketDetails::setupRoute(server);
    UserGetRemainingTicketsNum::setupRoute(server);
    UserBuyTicket::setupRoute(server);
    UserGetOwnTickets::setupRoute(server);
    UserGetOwnTicketsNum::setupRoute(server);
    UserRefundTicket::setupRoute(server);
    UserGetOrderDetails::setupRoute(server);
    
    server.route(".*", QHttpServerRequest::Method::Options, [](const QHttpServerRequest& req) {
        auto headers = req.headers();
        QHttpServerResponse res(QHttpServerResponse::StatusCode::Ok);
        addCorsHeaders(res);
        return res;
        });

    qInfo() << "Server listening on:" << tcpServer.serverAddress().toString() << tcpServer.serverPort();

    qInfo() << "运行在端口：" << tcpServer.serverPort();

    return app.exec();
}