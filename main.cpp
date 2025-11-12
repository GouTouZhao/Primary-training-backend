#include <QCoreApplication>
#include <QHttpServer>
#include <QTcpServer>
#include <QHostAddress>
#include <QDebug>
#include<QHttpServerResponse>
#include <functional>
#include"initDB.h"
#include"Login.h"
#include"staticsource.cpp"

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

    UserRegister::setupRoute(server);
    UserLogin::setupRoute(server);

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