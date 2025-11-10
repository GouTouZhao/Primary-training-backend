<<<<<<< HEAD
﻿#include <QCoreApplication>
=======
#include <QCoreApplication>
>>>>>>> 090dd3a44fe9bcc3932be2a56257542184ac6e1d
#include <QHttpServer>
#include <QTcpServer>
#include <QHostAddress>
#include <QDebug>
#include"initDB.h"
#include"Login.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

<<<<<<< HEAD
    if (!MysqlInitDB::init()) {
        return -1;
    }

    QTcpServer tcpServer;
    if (!tcpServer.listen(QHostAddress::Any, 8080)) {
        qWarning() << "yes" << tcpServer.errorString();
        return -1;
    }

    QHttpServer server;
    //接口目录
    server.route("/", []() {
        return "Hello, HttpServer 已运行";
        });

    UserRegister::setupRoute(server);

    server.bind(&tcpServer);

    qInfo() << "123213" << tcpServer.serverPort();

=======
    QTcpServer tcpServer;
    if (!tcpServer.listen(QHostAddress::Any, 8080)) {
        qWarning() << "yes" << tcpServer.errorString();
        return -1;
    }

    QHttpServer server;
    server.route("/", []() {
        return "Hello, Qt HttpServer!";
        });

    server.bind(&tcpServer);

    qInfo() << "123213" << tcpServer.serverPort();

>>>>>>> 090dd3a44fe9bcc3932be2a56257542184ac6e1d
    return app.exec();
}
